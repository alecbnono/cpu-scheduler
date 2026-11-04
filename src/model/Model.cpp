#include "view/ProcessReportView.hpp"
#include "model/Model.hpp"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <random>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

SystemSnapshot Model::getSnapshot() const {
    std::lock_guard<std::mutex> lock(mutex_);

    SystemSnapshot snap;
    unsigned used = 0;
    for (const auto& core : cores_) if (core.entry) ++used;
    snap.coresUsed = used;
    snap.coresAvailable = static_cast<uint32_t>(cores_.size() - used);
    snap.cpuUtilization = cores_.empty() ? 0 : static_cast<uint32_t>((100.0 * used) / cores_.size());

    for (const auto& entry : processes_) {
        const auto& process = *entry->process;
        const auto& ts = entry->timestamp;
        if (process.state() == Process::State::Finished) {
            snap.finishedProcesses.emplace_back(
                process.name(),
                ts,
                -1,
                process.executedInstructions(),
                entry->total
            );
        } else {
            snap.runningProcesses.emplace_back(
                process.name(),
                ts,
                entry->core,
                process.executedInstructions(),
                entry->total
            );
        }
        if (process.state() != Process::State::Finished) {
            snap.runningProcesses.back().status = entry->core >= 0 ? "Running" :
                process.state() == Process::State::Sleeping && process.wakeTick() > tick_ ? "Sleeping" : "Ready";
        }
    }
    return snap;
}

namespace {
std::string currentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm bt{};
#if defined(_WIN32)
    localtime_s(&bt, &in_time_t);
#else
    localtime_r(&in_time_t, &bt);
#endif
    char buf[64];
    std::strftime(buf, sizeof(buf), "%m/%d/%Y %I:%M:%S%p", &bt);
    return std::string(buf);
}

Configuration readConfig(const std::string& path) {
    std::ifstream file(path);
    if (!file) throw std::runtime_error("Cannot open " + path);
    std::unordered_map<std::string, std::string> values;
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream row(line);
        std::string key, value, extra;
        if (!(row >> key)) continue;
        if (!(row >> value) || row >> extra || !values.emplace(key, value).second)
            throw std::runtime_error("Invalid or duplicate configuration: " + key);
    }
    const std::vector<std::string> keys = {"num-cpu", "scheduler", "quantum-cycles",
        "batch-process-freq", "min-ins", "max-ins", "delays-per-exec"};
    if (values.size() != keys.size()) throw std::runtime_error("Expected seven configuration settings");
    for (const auto& key : keys)
        if (!values.count(key)) throw std::runtime_error("Missing configuration: " + key);
    auto number = [&](const std::string& key, std::uint64_t low, std::uint64_t high) {
        const auto& value = values.at(key);
        if (value.empty() || value.find_first_not_of("0123456789") != std::string::npos)
            throw std::runtime_error("Invalid number for " + key);
        const auto result = std::stoull(value);
        if (result < low || result > high) throw std::runtime_error("Out of range: " + key);
        return static_cast<std::uint64_t>(result);
    };
    constexpr std::uint64_t maximum = std::uint64_t{1} << 32;
    Configuration result;
    result.cpus = static_cast<unsigned>(number("num-cpu", 1, 128));
    result.scheduler = values.at("scheduler");
    if (result.scheduler.size() >= 2 && result.scheduler.front() == '"' && result.scheduler.back() == '"')
        result.scheduler = result.scheduler.substr(1, result.scheduler.size() - 2);
    if (result.scheduler != "fcfs" && result.scheduler != "rr")
        throw std::runtime_error("scheduler must be fcfs or rr");
    result.quantum = number("quantum-cycles", 1, maximum);
    result.batchFrequency = number("batch-process-freq", 1, maximum);
    result.minInstructions = number("min-ins", 1, maximum);
    result.maxInstructions = number("max-ins", 1, maximum);
    result.delay = number("delays-per-exec", 0, maximum);
    if (result.minInstructions > result.maxInstructions)
        throw std::runtime_error("min-ins must not exceed max-ins");
    return result;
}

std::uint64_t instructionCount(const std::vector<Instruction>& program) {
    std::uint64_t count = 0;
    for (const auto& instruction : program)
        count += instruction.type == Instruction::Type::For
            ? instruction.repeats * instructionCount(instruction.body) : 1;
    return count;
}
}

Model::~Model() {
    { std::lock_guard<std::mutex> lock(mutex_); stopping_ = true; }
    changed_.notify_all();
    if (worker_.joinable()) worker_.join();
}

bool Model::initialize(const std::string& path, std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (initialized_) { message = "Already initialized."; return false; }
    try {
        config_ = readConfig(path);
        cores_.resize(config_.cpus);
        worker_ = std::thread(&Model::run, this);
        initialized_ = true;
        message = "Initialized: " + std::to_string(config_.cpus) + " CPUs, scheduler " + config_.scheduler + ".";
        return true;
    } catch (const std::exception& error) {
        message = std::string("Initialization failed: ") + error.what();
        return false;
    }
}

bool Model::initialized() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return initialized_;
}

void Model::enqueue(Entry* entry) {
    ready_.push_back(entry);
    entry->queued = true;
}

void Model::create(const std::string& name) {
    std::mt19937 random(static_cast<std::uint32_t>(nextId_));
    const auto count = std::uniform_int_distribution<std::uint64_t>(
        config_.minInstructions, config_.maxInstructions)(random);
    auto program = generateProgram(name, static_cast<std::size_t>(count), random());
    const auto total = instructionCount(program);
    auto entry = std::make_unique<Entry>();
    entry->process = std::make_unique<Process>(nextId_, name, std::move(program));
    entry->total = total;
    entry->delay = config_.delay;
    entry->timestamp = currentTimestamp();
    processes_.push_back(std::move(entry));
    try { enqueue(processes_.back().get()); }
    catch (...) { processes_.pop_back(); throw; }
    ++nextId_;
}

bool Model::createProcess(const std::string& name, std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) { message = "Run initialize first."; return false; }
    if (name.empty()) { message = "Process name cannot be empty."; return false; }
    for (const auto& entry : processes_)
        if (entry->process->name() == name) { message = "Process " + name + " already exists."; return false; }
    try { create(name); }
    catch (const std::exception& error) { message = std::string("Cannot create process: ") + error.what(); return false; }
    message = "Created process " + name + ".";
    return true;
}

bool Model::canAttach(const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& entry : processes_)
        if (entry->process->name() == name) return entry->process->state() != Process::State::Finished;
    return false;
}

std::string Model::processInfo(const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& entry : processes_) {
        const auto& process = *entry->process;
        if (process.name() != name) continue;
        std::ostringstream out;
        out << "Process name: " << name << "\nID: " << process.id() << "\nLogs:\n";
        for (const auto& log : entry->formattedLogs) out << log << '\n';
        out << '\n';
        if (process.state() == Process::State::Finished) out << "Finished!\n";
        else out << "Current instruction line: " << process.executedInstructions()
                 << "\nLines of code: " << entry->total << '\n';
        return out.str();
    }
    return "Process " + name + " not found.";
}

std::string Model::utilization() const {
    return ProcessReportView::renderToString(getSnapshot());
}

std::string Model::setGenerating(bool enabled) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) return "Run initialize first.";
    if (generating_ == enabled) return enabled ? "Process generation is already running." : "Process generation is already stopped.";
    generating_ = enabled;
    nextBatch_ = tick_ + config_.batchFrequency;
    return enabled ? "Process generation started." : "Process generation stopped; existing processes continue.";
}

void Model::run() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (!stopping_) {
        if (changed_.wait_for(lock, std::chrono::milliseconds(10), [this] { return stopping_; })) break;
        advance();
    }
}

void Model::advance() {
    ++tick_;
    if (generating_ && tick_ >= nextBatch_) {
        std::string name;
        bool exists;
        auto suffix = nextId_;
        do {
            name = "p" + std::to_string(suffix++);
            exists = false;
            for (const auto& entry : processes_) if (entry->process->name() == name) exists = true;
        } while (exists);
        try { create(name); } catch (const std::exception&) { generating_ = false; }
        nextBatch_ = tick_ + config_.batchFrequency;
    }
    for (const auto& entry : processes_) {
        if (entry->process->state() == Process::State::Sleeping && entry->process->wakeTick() <= tick_ &&
            !entry->queued && entry->core < 0) enqueue(entry.get());
    }
    for (std::size_t index = 0; index < cores_.size(); ++index) {
        auto& core = cores_[index];
        if (!core.entry && !ready_.empty()) {
            core.entry = ready_.front(); ready_.pop_front();
            core.entry->queued = false;
            core.entry->core = static_cast<int>(index);
            core.slice = 0;
        }
    }
    // Dispatch all cores before executing, preventing a requeued process from
    // running on a second core during the same tick.
    for (auto& core : cores_) {
        if (!core.entry) continue;
        ++core.slice;
        bool executed = false;
        if (core.entry->delay) --core.entry->delay;
        else {
            const auto oldLogCount = core.entry->process->logs().size();
            core.entry->process->step(tick_);
            const auto& logs = core.entry->process->logs();
            for (auto i = oldLogCount; i < logs.size(); ++i)
                core.entry->formattedLogs.push_back("(" + currentTimestamp() + ") Core:" +
                    std::to_string(core.entry->core) + " \"" + logs[i] + "\"");
            core.entry->delay = config_.delay;
            executed = true;
        }
        auto state = core.entry->process->state();
        if (executed && (state == Process::State::Finished || state == Process::State::Sleeping)) {
            core.entry->core = -1; core.entry = nullptr;
        } else if (config_.scheduler == "rr" && core.slice >= config_.quantum) {
            enqueue(core.entry);
            core.entry->core = -1; core.entry = nullptr;
        }
    }
}

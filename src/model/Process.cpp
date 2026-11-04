#include "model/Process.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>
#include <random>

namespace {
std::uint16_t clamp(std::int64_t value) {
    return static_cast<std::uint16_t>(std::clamp<std::int64_t>(value, 0, 65535));
}
}

std::vector<Instruction> generateProgram(const std::string& processName,
                                         std::size_t count, std::uint32_t seed) {
    std::mt19937 random(seed);
    std::vector<Instruction> program;
    program.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        const auto value = static_cast<std::int64_t>(random() % 65536);
        switch (random() % 6) {
        case 0: program.push_back(Instruction::print("Hello world from " + processName + "!")); break;
        case 1: program.push_back(Instruction::declare("x", value)); break;
        case 2: program.push_back(Instruction::add("x", std::string("x"), value)); break;
        case 3: program.push_back(Instruction::subtract("x", std::string("x"), value)); break;
        case 4: program.push_back(Instruction::sleep(random() % 256)); break;
        case 5: {
            const auto repeats = static_cast<std::uint32_t>(
                std::min<std::size_t>(count - i, 1 + random() % 3));
            program.push_back(Instruction::loop({Instruction::print(
                "Hello world from " + processName + "!")}, repeats));
            i += repeats - 1;
            break;
        }
        }
    }
    return program;
}

namespace {
void validate(const std::vector<Instruction>& program, unsigned depth = 0) {
    for (const auto& instruction : program) {
        using Type = Instruction::Type;
        if (instruction.type == Type::For) {
            if (depth == 3) throw std::invalid_argument("FOR nesting exceeds three levels");
            validate(instruction.body, depth + 1);
        } else if (instruction.type == Type::Sleep) {
            const auto* ticks = std::get_if<std::int64_t>(&instruction.left);
            if (!ticks || *ticks < 0 || *ticks > 255)
                throw std::invalid_argument("SLEEP requires uint8 ticks");
        } else if (instruction.type == Type::Declare || instruction.type == Type::Add ||
                   instruction.type == Type::Subtract) {
            if (instruction.target.empty()) throw std::invalid_argument("Variable name is empty");
        }
    }
}
}

Instruction Instruction::print(std::string message, std::optional<std::string> variable) {
    Instruction result{Type::Print, std::move(message)};
    result.printVariable = std::move(variable);
    return result;
}
Instruction Instruction::declare(std::string variable, std::int64_t value) {
    Instruction result{Type::Declare, std::move(variable)};
    result.left = value;
    return result;
}
Instruction Instruction::add(std::string target, Operand left, Operand right) {
    Instruction result{Type::Add, std::move(target)};
    result.left = std::move(left);
    result.right = std::move(right);
    return result;
}
Instruction Instruction::subtract(std::string target, Operand left, Operand right) {
    auto result = add(std::move(target), std::move(left), std::move(right));
    result.type = Type::Subtract;
    return result;
}
Instruction Instruction::sleep(unsigned ticks) {
    if (ticks > 255) throw std::invalid_argument("SLEEP requires uint8 ticks");
    Instruction result{Type::Sleep, {}};
    result.left = static_cast<std::int64_t>(ticks);
    return result;
}
Instruction Instruction::loop(std::vector<Instruction> body, std::uint32_t repeats) {
    Instruction result{Type::For, {}};
    result.body = std::move(body);
    result.repeats = repeats;
    return result;
}

Process::Process(std::uint64_t id, std::string name, std::vector<Instruction> program)
    : id_(id), name_(std::move(name)), program_(std::move(program)) {
    validate(program_);
    stack_.push_back({&program_, 0, 1});
    normalize();
}

void Process::normalize() {
    while (!stack_.empty()) {
        auto& frame = stack_.back();
        if (frame.index == frame.instructions->size()) {
            if (--frame.remaining != 0) frame.index = 0;
            else stack_.pop_back();
            continue;
        }
        const auto& instruction = (*frame.instructions)[frame.index];
        if (instruction.type != Instruction::Type::For) return;
        ++frame.index;
        if (instruction.repeats && !instruction.body.empty())
            stack_.push_back({&instruction.body, 0, instruction.repeats});
    }
    state_ = State::Finished;
    variables_.clear();
}

std::int64_t Process::resolve(const Operand& operand) {
    if (const auto* literal = std::get_if<std::int64_t>(&operand)) return clamp(*literal);
    return variables_[std::get<std::string>(operand)];
}

Process::StepResult Process::step(std::uint64_t tick) {
    if (state_ == State::Finished) return StepResult::Finished;
    if (state_ == State::Sleeping && tick < wakeTick_) return StepResult::Sleeping;
    state_ = State::Ready;
    // A final SLEEP retains variables until its wake tick.
    normalize();
    if (state_ == State::Finished) return StepResult::Finished;
    auto& frame = stack_.back();
    const auto& instruction = (*frame.instructions)[frame.index++];
    using Type = Instruction::Type;
    switch (instruction.type) {
    case Type::Print:
        logs_.push_back(instruction.target + (instruction.printVariable
            ? std::to_string(variables_[*instruction.printVariable]) : ""));
        break;
    case Type::Declare:
        variables_[instruction.target] = clamp(resolve(instruction.left));
        break;
    case Type::Add:
        variables_[instruction.target] = clamp(resolve(instruction.left) + resolve(instruction.right));
        break;
    case Type::Subtract:
        variables_[instruction.target] = clamp(resolve(instruction.left) - resolve(instruction.right));
        break;
    case Type::Sleep: {
        const auto ticks = static_cast<std::uint64_t>(std::get<std::int64_t>(instruction.left));
        if (tick > std::numeric_limits<std::uint64_t>::max() - ticks)
            wakeTick_ = std::numeric_limits<std::uint64_t>::max();
        else wakeTick_ = tick + ticks;
        state_ = State::Sleeping;
        break;
    }
    case Type::For: break; // normalize handles loop control without expanding the program.
    }
    ++executed_;
    if (state_ != State::Sleeping) normalize();
    return StepResult::Executed;
}

#include "controller/Controller.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <thread>

namespace fs = std::filesystem;
void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}
void config(const std::string& scheduler, unsigned delay = 0) {
    std::ofstream out("config.txt");
    out << "num-cpu 2\nscheduler \"" << scheduler << "\"\nquantum-cycles 2\n"
        << "batch-process-freq 2\nmin-ins 1\nmax-ins 1\ndelays-per-exec " << delay << '\n';
}
bool waitFinished(Model& model, const std::string& name) {
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (std::chrono::steady_clock::now() < deadline) {
        if (model.processInfo(name).find("Finished!") != std::string::npos) return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return false;
}
int main() {
    const auto original = fs::current_path();
    const auto fixture = original / "build" / ("menu-test-" + std::to_string(
        std::chrono::steady_clock::now().time_since_epoch().count()));
    fs::create_directories(fixture);
    fs::current_path(fixture);
    std::ostringstream output;
    auto* previous = std::cout.rdbuf(output.rdbuf());
    try {
        {
            Model model;
            View view;
            Controller controller(model, view);
            auto command = [&](const std::string& text) {
                output.str(""); output.clear();
                const bool running = controller.handleCommand(text);
                return std::make_pair(running, output.str());
            };
            require(command("help").second.find("Run initialize first") != std::string::npos, "Initialization gate");
            require(command("screen -s demo").second.find("Run initialize first") != std::string::npos, "Screen gate");
            require(command("initialize").second.find("failed") != std::string::npos, "Missing config");
            config("invalid");
            require(command("initialize").second.find("failed") != std::string::npos, "Invalid scheduler");
            config("rr", 8); // Delay exceeds quantum: preemption must not starve execution.
            require(command("initialize").second.find("Initialized:") != std::string::npos, "Initialize");
            require(command("initialize").second.find("Already initialized") != std::string::npos, "Repeated initialization");
            require(command("screen").second.find("Usage:") != std::string::npos, "Malformed screen");
            require(command("screen -r absent").second.find("not found") != std::string::npos, "Missing process");
            command("screen -s demo");
            require(controller.prompt() == "demo> ", "Attach on creation");
            const auto details = command("process-smi").second;
            require(details.find("Process name: demo\nID: 1\nLogs:\n") == 0 &&
                    details.find("Current instruction line: ") != std::string::npos &&
                    details.find("Lines of code: ") != std::string::npos, "PDF process details");
            require(command("scheduler-start").second.find("Process screen commands") != std::string::npos, "Screen command isolation");
            require(command("exit").first && controller.prompt() == "main> ", "Detach does not terminate");
            require(command("screen -s demo").second.find("already exists") != std::string::npos, "Duplicate names");
            command("screen -r demo");
            require(controller.prompt() == "demo> ", "Reattach unfinished process");
            require(waitFinished(model, "demo"), "Background completion while attached");
            require(command("process-smi").second.find("Finished!") != std::string::npos, "Attached finished details");
            command("exit");
            require(command("screen -r demo").second.find("not found") != std::string::npos, "Finished screen unavailable");
            require(command("screen -ls").second.find("demo") != std::string::npos, "Finished process retained");
            require(command("report-util").second.find("csopesy-log.txt") != std::string::npos, "Report export");
            std::ifstream report("csopesy-log.txt");
            std::stringstream contents; contents << report.rdbuf();
            require(contents.str() == model.utilization(), "Report matches listing");
            command("scheduler-start");
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            command("scheduler-stop");
            require(model.utilization().find("p2") != std::string::npos, "Generated processes");
            require(!command("exit").first, "Main exit");
        }
        {
            config("fcfs");
            Model model;
            std::string message;
            require(model.initialize("config.txt", message), "FCFS initialization");
            require(model.createProcess("fcfs-demo", message), "FCFS creation");
            require(waitFinished(model, "fcfs-demo"), "FCFS background execution");
            require(!model.canAttach("fcfs-demo"), "Finished process attachment guard");
        }
        std::cout.rdbuf(previous);
        fs::current_path(original);
        std::cout << "All menu and screen tests passed.\n";
    } catch (const std::exception& error) {
        std::cout.rdbuf(previous);
        fs::current_path(original);
        std::cerr << "Menu test failed: " << error.what() << '\n';
        return 1;
    }
}

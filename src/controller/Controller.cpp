#include "controller/Controller.hpp"
#include "model/Process.hpp"

#include <sstream>

Controller::Controller(Model& model, View& view) : model_(model), view_(view) {}

std::vector<std::string> Controller::tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::istringstream stream(line);
    std::string token;
    while (stream >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

bool Controller::handleCommand(const std::string& line) {
    std::vector<std::string> tokens = tokenize(line);

    if (tokens.empty()) {
        return true;
    }

    const std::string& command = tokens[0];

    if (command == "exit") {
        return false;
    }

    if (command == "initialize") {
        view_.showMessage("initialize: not implemented yet.");
        return true;
    }

    if (command == "screen") {
        view_.showMessage("screen: not implemented yet.");
        return true;
    }

    if (command == "scheduler-start") {
        view_.showMessage("scheduler-start: not implemented yet.");
        return true;
    }

    if (command == "scheduler-stop") {
        view_.showMessage("scheduler-stop: not implemented yet.");
        return true;
    }

    if (command == "report-util") {
        view_.showMessage("report-util: not implemented yet.");
        return true;
    }

    if (command == "help") {
        view_.showHelp();
        return true;
    }

    if (command == "process-demo") {
        if (tokens.size() != 1) {
            view_.showMessage("Usage: process-demo");
            return true;
        }
        using I = Instruction;
        view_.showMessage("Process demo: predetermined instructions, one step per CPU tick.");
        view_.showMessage("Program: DECLARE(x,10), ADD(x,x,5), PRINT(x), SUBTRACT(x,x,20), PRINT(x),");
        view_.showMessage("         FOR([FOR([ADD(x,x,1), PRINT(x)],2)],2), SLEEP(3), PRINT(done)");
        Process process(1, "demo", {
            I::declare("x", 10), I::add("x", std::string("x"), std::int64_t{5}),
            I::print("Value after ADD: ", "x"),
            I::subtract("x", std::string("x"), std::int64_t{20}),
            I::print("Value after SUBTRACT (clamped): ", "x"),
            I::loop({I::loop({I::add("x", std::string("x"), std::int64_t{1}),
                             I::print("Nested loop x: ", "x")}, 2)}, 2),
            I::sleep(3), I::print("Hello world from demo!")});
        std::size_t shown = 0;
        for (std::uint64_t tick = 0; process.state() != Process::State::Finished; ++tick) {
            const auto result = process.step(tick);
            const std::string prefix = "[tick " + std::to_string(tick) + "] ";
            if (result == Process::StepResult::Sleeping) {
                view_.showMessage(prefix + "Sleeping; CPU available for another process.");
            } else if (process.state() == Process::State::Sleeping) {
                view_.showMessage(prefix + "SLEEP: yielded CPU until tick " + std::to_string(process.wakeTick()));
            } else {
                view_.showMessage(prefix + "Executed instruction " + std::to_string(process.executedInstructions()));
            }
            while (shown < process.logs().size()) view_.showMessage("  " + process.logs()[shown++]);
        }
        view_.showMessage("Finished! Process variables released; logs retained.");
        return true;
    }

    view_.showMessage("Unknown command: " + command + " (type 'help')");
    return true;
}

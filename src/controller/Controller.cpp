#include "controller/Controller.hpp"
#include "model/Process.hpp"

#include <sstream>
#include "controller/commands/ScreenLsCommand.hpp"
#include "controller/commands/ReportUtilCommand.hpp"

Controller::Controller(Model& model, View& view)
    : model_(model),
      view_(view),
      screenLsCommand_(model_),
      reportUtilCommand_(model_) {}

std::string Controller::prompt() const {
    return attached_.empty() ? "main> " : attached_ + "> ";
}

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

    if (!attached_.empty()) {
        if (command == "exit" && tokens.size() == 1) {
            attached_.clear();
            view_.clearScreen();
            view_.showMessage("Main menu. Type help for commands.");
        } else if (command == "process-smi" && tokens.size() == 1) {
            view_.showMessage(model_.processInfo(attached_));
        } else {
            view_.showMessage("Process screen commands: process-smi, exit (return to main menu).");
        }
        return true;
    }

    if (command == "exit") {
        if (tokens.size() != 1) { view_.showMessage("Usage: exit"); return true; }
        return false;
    }

    if (command == "initialize") {
        if (tokens.size() != 1) { view_.showMessage("Usage: initialize"); return true; }
        std::string message;
        model_.initialize("config.txt", message);
        view_.showMessage(message);
        return true;
    }

    if (!model_.initialized()) {
        view_.showMessage("Run initialize first. Only initialize and exit are available.");
        return true;
    }

    if (command == "screen-ls") {
        return screenLsCommand_.execute(tokens);
    }

    if (command == "screen") {
        if (tokens.size() == 2 && tokens[1] == "-ls") {
            return screenLsCommand_.execute(tokens);
        } else if (tokens.size() == 3 && (tokens[1] == "-s" || tokens[1] == "-r")) {
            const auto& name = tokens[2];
            if (tokens[1] == "-s") {
                std::string message;
                if (!model_.createProcess(name, message)) { view_.showMessage(message); return true; }
            } else if (!model_.canAttach(name)) {
                view_.showMessage("Process " + name + " not found.");
                return true;
            }
            attached_ = name;
            view_.clearScreen();
            view_.showMessage(model_.processInfo(name));
            view_.showMessage("Type process-smi for updated details and logs; exit to return to the main menu.");
        } else {
            view_.showMessage("Usage: screen -s <process name> | screen -r <process name> | screen -ls");
        }
        return true;
    }

    if (command == "scheduler-start") {
        view_.showMessage(tokens.size() == 1 ? model_.setGenerating(true) : "Usage: scheduler-start");
        return true;
    }

    if (command == "scheduler-stop") {
        view_.showMessage(tokens.size() == 1 ? model_.setGenerating(false) : "Usage: scheduler-stop");
        return true;
    }

    if (command == "report-util") {
        if (tokens.size() != 1) { view_.showMessage("Usage: report-util"); return true; }
        return reportUtilCommand_.execute(tokens);
    }

    if (command == "help") {
        if (tokens.size() != 1) { view_.showMessage("Usage: help"); return true; }
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

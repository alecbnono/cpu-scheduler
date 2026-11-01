#include "Controller.hpp"
#include "AsciiArt.hpp"
#include "Process.hpp"

#include <sstream>
#include <charconv>

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

    if (command == "help") {
        view_.showHelp();
        return true;
    }

    if (command == "process-demo") {
        if (tokens.size() != 1) {
            view_.showMessage("Usage: process-demo");
            return true;
        }
        view_.stopMarquee();
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

    if (command == "set_text") {
        if (tokens.size() < 2) {
            view_.showMessage("Usage: set_text <text>");
            return true;
        }
        std::string text = tokens[1];
        for (std::size_t i = 2; i < tokens.size(); ++i) {
            text += " " + tokens[i];
        }
        model_.setText(text);
        view_.showMessage("Text saved for marquee: " + text);
        return true;
    }

    if (command == "set_speed") {
        if (tokens.size() != 2) {
            view_.showMessage("Usage: set_speed <positive_int>");
            return true;
        }

        try {
            std::size_t pos = 0;
            int speed = std::stoi(tokens[1], &pos);

            // Rejects trailing characters such as "12abc"
            if (pos != tokens[1].size()) {
                view_.showMessage("Error: Speed contains non-numeric characters.");
                return true;
            }

            // Must be positive
            if (speed <= 0) {
                view_.showMessage("Error: Speed must be greater than 0.");
                return true;
            }

            model_.setSpeed(speed);
            view_.showMessage("Speed set to " + std::to_string(speed));
        } 
        catch (const std::invalid_argument&) {
            view_.showMessage("Error: Invalid number format for speed.");
        } 
        catch (const std::out_of_range&) {
            view_.showMessage("Error: Speed value is too large.");
        }

        return true;
    }

    if (command == "start_marquee") {
        view_.showMarquee(renderAsciiArt(model_.getText()), model_.getSpeed());
        return true;
    }

    if (command == "stop_marquee") {
        if (!view_.isMarqueeRunning()) {
            view_.showMessage("No marquee is currently running.");
        } else {
            view_.stopMarquee();
        }
        return true;
    }

    view_.showMessage("Unknown command: " + command + " (type 'help')");
    return true;
}

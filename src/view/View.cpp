#include "view/View.hpp"

#include <iostream>

#include "view/AsciiArt.hpp"

void View::showMessage(const std::string& message) const {
    std::cout << message << "\n";
}

void View::showAsciiArt(const std::string& text) const {
    for (const std::string& row : renderAsciiArt(text)) {
        std::cout << row << "\n";
    }
}

void View::showHelp() const {
    std::cout <<
        "Commands:\n"
        "  initialize       initialize the processor configuration\n"
        "  screen           create, reattach to, or list processes\n"
        "  scheduler-start  start generating dummy processes\n"
        "  scheduler-stop   stop generating dummy processes\n"
        "  report-util      generate the CPU utilization report\n"
        "  process-demo     run a visible process instruction demonstration\n"
        "  help             show this message\n"
        "  exit             quit the program\n";
}

void View::showPrompt() const {
    std::cout << "> ";
}

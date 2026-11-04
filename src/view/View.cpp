#include "view/View.hpp"

#include <iostream>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <unistd.h>
#endif

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
        "  screen -s <name> create and attach to a process\n"
        "  screen -r <name> reattach to an unfinished process\n"
        "  screen -ls       list processes and CPU utilization\n"
        "  scheduler-start  start generating dummy processes\n"
        "  scheduler-stop   stop generating dummy processes\n"
        "  report-util      generate the CPU utilization report\n"
        "  process-demo     run a visible process instruction demonstration\n"
        "  help             show this message\n"
        "  exit             quit the program\n";
}

void View::showPrompt(const std::string& prompt) const {
    std::cout << prompt << std::flush;
}

void View::clearScreen() const {
#ifdef _WIN32
    const auto console = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;
    if (!GetConsoleMode(console, &mode)) return; // Preserve redirected output.
    if (!SetConsoleMode(console, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) return;
#else
    if (!isatty(STDOUT_FILENO)) return;
#endif
    std::cout << "\x1b[2J\x1b[H" << std::flush;
}

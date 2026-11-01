#include "View.hpp"

#include <iostream>

#include "AsciiArt.hpp"

namespace {
// Region boundary constants for split-screen console display.
// Rows 1 to 6:   Marquee Banner Region
// Row 7:         Blank space separator
// Rows 8 to 14:  Author & Info Header Block
// Rows 15 to 16: Blank spaces below author names
// Row 17+:       Console Region & Command Prompt
constexpr int kMarqueeStartRow = 1;
constexpr int kMarqueeMaxRows = 6;
constexpr int kHeaderStartRow = 8;
constexpr int kConsolePromptRow = 17;
constexpr int kWindowWidth = 100;

// Header text containing author names maintained below the marquee animation
const std::vector<std::string> kAuthorHeader = {
    "Welcome to CSOPESY!.",
    "Group Developers:",
    "Nono, Alec Marx",
    "Obregon, Sian Ysabelle",
    "Ponce, Jean Rondel",
    "Sy, Prince Matthew",
    "Type 'help' for a list of commands."
};
}  // namespace

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
        "  process-demo             run a visible process instruction demonstration\n"
        "  set_text <text>          save the text to use for the marquee\n"
        "  set_speed <positive_int> save the speed of marquee animation (in ms)\n"
        "  start_marquee            starts the marquee \"animation\"\n"
        "  stop_marquee             stops the marquee \"animation\"\n"
        "  help                     show this message\n"
        "  exit                     quit the program\n";
}

void View::showPrompt() const {
    std::cout << "> ";
}

View::~View() {
    stopMarquee();
}

bool View::isMarqueeRunning() const {
    return isRunning_.load();
}

void View::stopMarquee() {
    if (marqueeThread_.joinable()) {
        if (isRunning_.load()) {
            stopRequested_ = true;
            marqueeThread_.join();
            showMessage("\nMarquee stopped.");
        } else {
            marqueeThread_.join();
        }
    }
}

void View::showMarquee(const std::vector<std::string>& text, int speed) {
    stopMarquee();

    stopRequested_ = false;

    // Pads all rows to the same width
    int rowWidth = 0;
    for (const auto& r : text) {
        if ((int)r.size() > rowWidth) rowWidth = (int)r.size();
    }

    // Pads each row to the same width so scrolling window is stable
    std::vector<std::string> paddedText = text;
    for (auto& r : paddedText) {
        while ((int)r.size() < rowWidth) r += ' ';
    }

    // 1. Clear the entire screen when the marquee animation starts
    std::cout << "\x1b[2J\x1b[1;1H" << std::flush;

    // 2. Clear upper region (rows 1 to 16) and position prompt line at row 17
    for (int r = 1; r < kConsolePromptRow; ++r) {
        std::cout << "\x1b[" << r << ";1H" << std::string(kWindowWidth, ' ');
    }
    std::cout << "\x1b[" << kConsolePromptRow << ";1H> " << std::flush;

    isRunning_ = true;
    marqueeThread_ = std::thread([this, paddedText, text, rowWidth, speed]() {
        // Offset scrolls from off-screen right to off-screen left
        int offset = -kWindowWidth;

        while (!stopRequested_) {

            // Save current user cursor position in the console region
            std::cout << "\x1b[s";

            // 1. Draw marquee animation frame on rows 1 to 6
            for (int r = 0; r < kMarqueeMaxRows; ++r) {
                std::cout << "\x1b[" << (kMarqueeStartRow + r) << ";1H";
                std::string line;
                if (r < (int)text.size()) {
                    for (int c = 0; c < kWindowWidth; ++c) {
                        int src = offset + c;
                        if (src >= 0 && src < rowWidth) {
                            line += paddedText[r][src];
                        } else {
                            line += ' ';
                        }
                    }
                } else {
                    line = std::string(kWindowWidth, ' ');
                }
                std::cout << line;
            }

            // 2. Row 7 blank space separator
            std::cout << "\x1b[7;1H" << std::string(kWindowWidth, ' ');

            // 3. Draw / maintain author header block on rows 8 to 14
            for (std::size_t i = 0; i < kAuthorHeader.size(); ++i) {
                std::cout << "\x1b[" << (kHeaderStartRow + static_cast<int>(i)) << ";1H";
                std::string line = kAuthorHeader[i];
                if ((int)line.size() < kWindowWidth) {
                    line.append(kWindowWidth - line.size(), ' ');
                }
                std::cout << line;
            }

            // 4. Rows 15 & 16 explicit blank space rows below author names
            std::cout << "\x1b[15;1H" << std::string(kWindowWidth, ' ');
            std::cout << "\x1b[16;1H" << std::string(kWindowWidth, ' ');

            // 5. Restore cursor position back to active prompt line in Console Region
            std::cout << "\x1b[u" << std::flush;

            std::this_thread::sleep_for(std::chrono::milliseconds(speed));

            // 6. Loop continuously until stop_marquee is called
            ++offset;
            if (offset > rowWidth) {
                offset = -kWindowWidth;
            }
        }

        // Restore cursor position on animation completion
        std::cout << "\x1b[u" << std::flush;
        isRunning_ = false;
    });
}

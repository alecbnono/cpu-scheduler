#pragma once

#include <string>
#include <vector>

// Placeholder command handler for 'screen -ls'.
// Full implementation will be developed in a feature branch.
class ScreenLsCommand {
public:
    ScreenLsCommand() = default;

    void execute(const std::vector<std::string>& args);
};

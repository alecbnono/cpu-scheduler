#include "controller/commands/ScreenLsCommand.h"

#include <iostream>

void ScreenLsCommand::execute(const std::vector<std::string>& /*args*/) {
    std::cout << "screen -ls: not implemented yet.\n";
}

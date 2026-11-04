#include "controller/commands/ScreenLsCommand.h"

#include <iostream>

void ScreenLsCommand::execute(const std::vector<std::string>& /*args*/) {
    view_.showMessage(model_.utilization());
}

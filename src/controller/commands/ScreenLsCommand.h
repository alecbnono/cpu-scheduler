#pragma once

#include <string>
#include <vector>
#include "model/Model.hpp"
#include "view/View.hpp"

// Displays a synchronized snapshot of all processes and core utilization.
class ScreenLsCommand {
public:
    ScreenLsCommand(Model& model, View& view) : model_(model), view_(view) {}

    void execute(const std::vector<std::string>& args);
private:
    Model& model_;
    View& view_;
};

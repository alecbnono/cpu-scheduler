#pragma once

#include <string>
#include <vector>
#include "model/Model.hpp"
#include "view/View.hpp"

// Writes the same utilization snapshot displayed by screen -ls.
class ReportUtilCommand {
public:
    ReportUtilCommand(Model& model, View& view) : model_(model), view_(view) {}

    void execute(const std::vector<std::string>& args);
private:
    Model& model_;
    View& view_;
};

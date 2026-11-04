#pragma once

#include <string>
#include <vector>

#include "model/Model.hpp"
#include "view/View.hpp"

// Parses a raw command line, updates the Model, and tells the View what to show.
class Controller {
public:
    Controller(Model& model, View& view);

    // Returns false when the app should exit.
    bool handleCommand(const std::string& line);
    std::string prompt() const;

private:
    Model& model_;
    View& view_;
    std::string attached_;

    // Splits a line into whitespace-separated tokens.
    static std::vector<std::string> tokenize(const std::string& line);
};

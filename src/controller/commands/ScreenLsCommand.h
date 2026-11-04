#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "model/Model.hpp"
#include "view/View.hpp"

#include "model/SystemSnapshot.hpp"

// Command handler for 'screen -ls' (and 'screen-ls').
// Fetches a snapshot and writes the formatted report to output stream.
class ScreenLsCommand {
public:
    // Accepts snapshot provider and optionally an output stream (defaults to std::cout)
    explicit ScreenLsCommand(const ISnapshotProvider& provider, std::ostream& out = std::cout);
    ScreenLsCommand(const ISnapshotProvider& provider, View& view);

    // Executes the command: fetches snapshot and renders the report
    bool execute(const std::vector<std::string>& args);

private:
    const ISnapshotProvider& provider_;
    std::ostream& out_;
};

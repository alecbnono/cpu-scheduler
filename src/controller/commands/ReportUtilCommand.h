#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "model/Model.hpp"
#include "view/View.hpp"

#include "model/SystemSnapshot.hpp"

// Command handler for 'report-util'.
// Fetches a snapshot, saves the formatted report to csopesy-log.txt,
// and outputs confirmation feedback.
class ReportUtilCommand {
public:
    explicit ReportUtilCommand(const ISnapshotProvider& provider,
                               std::string targetFilePath = "csopesy-log.txt",
                               std::string displayPath = "C:/csopesy-log.txt",
                               std::ostream& feedbackOut = std::cout);
    ReportUtilCommand(const ISnapshotProvider& provider, View& view);

    bool execute(const std::vector<std::string>& args);

    const std::string& getTargetFilePath() const { return targetFilePath_; }
    void setTargetFilePath(std::string path) { targetFilePath_ = std::move(path); }

    const std::string& getDisplayPath() const { return displayPath_; }
    void setDisplayPath(std::string path) { displayPath_ = std::move(path); }

private:
    const ISnapshotProvider& provider_;
    std::string targetFilePath_;
    std::string displayPath_;
    std::ostream& feedbackOut_;
};

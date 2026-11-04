#include "controller/commands/ReportUtilCommand.h"
#include "view/ProcessReportView.hpp"

#include <fstream>

ReportUtilCommand::ReportUtilCommand(const ISnapshotProvider& provider,
                                     std::string targetFilePath,
                                     std::string displayPath,
                                     std::ostream& feedbackOut)
    : provider_(provider),
      targetFilePath_(std::move(targetFilePath)),
      displayPath_(std::move(displayPath)),
      feedbackOut_(feedbackOut) {}

bool ReportUtilCommand::execute(const std::vector<std::string>& /*args*/) {
    SystemSnapshot snapshot = provider_.getSnapshot();

    // Opens csopesy-log.txt in the project directory
    std::ofstream outFile(targetFilePath_);
    if (!outFile.is_open()) {
        feedbackOut_ << "Error: Unable to open file for writing at " << targetFilePath_ << "\n";
        return true;
    }

    ProcessReportView::renderReport(outFile, snapshot);
    outFile.close();

    feedbackOut_ << "Report generated at " << displayPath_ << "!\n";
    return true;
}

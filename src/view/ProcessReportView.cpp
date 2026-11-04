#include "view/ProcessReportView.hpp"

#include <sstream>

// Through polymorphism, utilizes std::ostream as the abstract class for output streams
void ProcessReportView::renderReport(std::ostream& os, const SystemSnapshot& snapshot) {
    // System Metrics
    os << "CPU utilization: " << snapshot.cpuUtilization << "%\n";
    os << "Cores used: " << snapshot.coresUsed << "\n";
    os << "Cores available: " << snapshot.coresAvailable << "\n\n";

    // Running Processes Section
    os << "Running processes:\n";
    for (const auto& proc : snapshot.runningProcesses) {
        os << proc.name << " (" << proc.timestamp << ") Core: " << proc.coreId
           << " " << proc.currentInstruction << " / " << proc.totalInstructions << "\n";
    }
    os << "\n";

    // inished Processes Section
    os << "Finished processes:\n";
    for (const auto& proc : snapshot.finishedProcesses) {
        os << proc.name << " (" << proc.timestamp << ") Finished "
           << proc.currentInstruction << " / " << proc.totalInstructions << "\n";
    }
}

std::string ProcessReportView::renderToString(const SystemSnapshot& snapshot) {
    std::ostringstream oss;
    renderReport(oss, snapshot);
    return oss.str();
}
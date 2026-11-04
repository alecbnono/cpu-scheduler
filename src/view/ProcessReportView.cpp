#include "view/ProcessReportView.hpp"

#include <sstream>
#include <iomanip>

// Through polymorphism, utilizes std::ostream as the abstract class for output streams
void ProcessReportView::renderReport(std::ostream& os, const SystemSnapshot& snapshot) {
    // System Metrics
    os << "CPU utilization: " << snapshot.cpuUtilization << "%\n";
    os << "Cores used: " << snapshot.coresUsed << "\n";
    os << "Cores available: " << snapshot.coresAvailable << "\n\n";

    // Running Processes Section
    os << "Running processes:\n";
    for (const auto& proc : snapshot.runningProcesses) {
        os << std::left << std::setw(16) << proc.name << " (" << proc.timestamp << ") "
           << std::setw(12) << (proc.coreId >= 0 ? "Core: " + std::to_string(proc.coreId) : proc.status)
           << std::right << std::setw(8) << proc.currentInstruction << " / " << proc.totalInstructions << "\n";
    }
    os << "\n";

    // inished Processes Section
    os << "Finished processes:\n";
    for (const auto& proc : snapshot.finishedProcesses) {
        os << std::left << std::setw(16) << proc.name << " (" << proc.timestamp << ") "
           << std::setw(12) << "Finished" << std::right << std::setw(8)
           << proc.currentInstruction << " / " << proc.totalInstructions << "\n";
    }
}

std::string ProcessReportView::renderToString(const SystemSnapshot& snapshot) {
    std::ostringstream oss;
    renderReport(oss, snapshot);
    return oss.str();
}

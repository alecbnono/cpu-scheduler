#include "model/Model.hpp"
#include "model/SystemSnapshot.hpp"
#include "view/ProcessReportView.hpp"
#include "controller/commands/ScreenLsCommand.h"
#include "controller/commands/ReportUtilCommand.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>

// Mock provider for testing edge cases
class MockCustomProvider : public ISnapshotProvider {
public:
    explicit MockCustomProvider(SystemSnapshot snap) : snap_(std::move(snap)) {}
    SystemSnapshot getSnapshot() const override { return snap_; }

private:
    SystemSnapshot snap_;
};

int main() {
    Model model;

    // 1. Test Polymorphism: verify Model works through the ISnapshotProvider interface
    const ISnapshotProvider& provider = model;

    // 2. Fetch the snapshot from Model
    SystemSnapshot snapshot = provider.getSnapshot();

    // 3. Test System Metrics
    assert(snapshot.cpuUtilization == 100);
    assert(snapshot.coresUsed == 4);
    assert(snapshot.coresAvailable == 0);

    // 4. Test Running Processes
    assert(snapshot.runningProcesses.size() == 2);
    assert(snapshot.runningProcesses[0].name == "process05");
    assert(snapshot.runningProcesses[0].coreId == 0);
    assert(snapshot.runningProcesses[0].currentInstruction == 1235);
    assert(snapshot.runningProcesses[0].totalInstructions == 5876);

    // 5. Test Finished Processes
    assert(snapshot.finishedProcesses.size() == 2);
    assert(snapshot.finishedProcesses[0].name == "process01");
    assert(snapshot.finishedProcesses[0].currentInstruction == 5876);
    assert(snapshot.finishedProcesses[0].totalInstructions == 5876);

    // 6. Test Report Output formatting
    std::string report = ProcessReportView::renderToString(snapshot);
    assert(report.find("CPU utilization: 100%") != std::string::npos);
    assert(report.find("process05 (01/18/2024 05:15:22AM) Core: 0 1235 / 5876") != std::string::npos);
    assert(report.find("process01 (01/18/2024 05:00:21AM) Finished 5876 / 5876") != std::string::npos);

    // 7. Test ScreenLsCommand
    std::ostringstream screenOutput;
    ScreenLsCommand screenCmd(model, screenOutput);
    bool screenResult = screenCmd.execute({});
    assert(screenResult == true);
    assert(screenOutput.str() == report);

    // 8. Test ReportUtilCommand file creation & content verification
    std::string testLogPath = "build/test-csopesy-log.txt";
    std::ostringstream feedbackOutput;
    ReportUtilCommand reportCmd(model, testLogPath, "C:/csopesy-log.txt", feedbackOutput);
    bool reportResult = reportCmd.execute({});
    assert(reportResult == true);
    assert(feedbackOutput.str().find("Report generated at C:/csopesy-log.txt!") != std::string::npos);

    // Verify file on disk
    std::ifstream logFile(testLogPath);
    assert(logFile.is_open());
    std::stringstream fileBuffer;
    fileBuffer << logFile.rdbuf();
    logFile.close();
    assert(fileBuffer.str() == report);

    // 9. Edge Case: Empty System (0% CPU, 0 running, 0 finished)
    SystemSnapshot emptySnap;
    emptySnap.cpuUtilization = 0;
    emptySnap.coresUsed = 0;
    emptySnap.coresAvailable = 4;
    MockCustomProvider emptyProvider(emptySnap);
    std::string emptyReport = ProcessReportView::renderToString(emptyProvider.getSnapshot());
    assert(emptyReport.find("CPU utilization: 0%") != std::string::npos);
    assert(emptyReport.find("Cores used: 0") != std::string::npos);
    assert(emptyReport.find("Cores available: 4") != std::string::npos);
    assert(emptyReport.find("Running processes:\n\n") != std::string::npos);
    assert(emptyReport.find("Finished processes:\n") != std::string::npos);

    // 10. Edge Case: Invalid file path error handling
    std::ostringstream errorFeedback;
    ReportUtilCommand invalidPathCmd(model, "Z:/non_existent_folder/log.txt", "Z:/non_existent_folder/log.txt", errorFeedback);
    bool invalidResult = invalidPathCmd.execute({});
    assert(invalidResult == true); // Should handle gracefully without crashing
    assert(errorFeedback.str().find("Error: Unable to open file for writing") != std::string::npos);

    std::cout << "\n--- Generated Report Preview ---\n";
    std::cout << report;
    std::cout << "--------------------------------\n";
    std::cout << "All 10 unit test suites and edge cases passed successfully!\n";
    return 0;
}

#include "model/Model.hpp"
#include "model/SystemSnapshot.hpp"
#include "view/ProcessReportView.hpp"
#include "controller/commands/ScreenLsCommand.h"
#include "controller/commands/ReportUtilCommand.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>

int main() {
    Model model;

    // 1. Test Polymorphism: verify Model works through the ISnapshotProvider interface
    const ISnapshotProvider& provider = model;

    // 2. Fetch the snapshot
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

    // 8. Test ReportUtilCommand
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

    std::cout << "\n--- Generated Report Preview ---\n";
    std::cout << report;
    std::cout << "--------------------------------\n";
    std::cout << "All snapshot, view, and command unit tests passed successfully!\n";
    return 0;
}

#include "model/Model.hpp"
#include "view/ProcessReportView.hpp"
#include "controller/commands/ScreenLsCommand.h"
#include "controller/commands/ReportUtilCommand.h"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>

struct FixtureProvider : ISnapshotProvider {
    SystemSnapshot snapshot;
    SystemSnapshot getSnapshot() const override { return snapshot; }
};
int main() {
    Model model;
    const auto empty = model.getSnapshot();
    assert(empty.cpuUtilization == 0 && empty.coresUsed == 0);
    assert(empty.runningProcesses.empty() && empty.finishedProcesses.empty());
    FixtureProvider fixture;
    fixture.snapshot.coresUsed = 1;
    fixture.snapshot.coresAvailable = 3;
    fixture.snapshot.cpuUtilization = 25;
    fixture.snapshot.runningProcesses = {
        {"active", "11/04/2026 01:02:03PM", 0, 4, 10},
        {"waiting", "11/04/2026 01:02:03PM", -1, 2, 10}};
    fixture.snapshot.runningProcesses.back().status = "Sleeping";
    fixture.snapshot.finishedProcesses = {{"done", "11/04/2026 01:02:03PM", -1, 10, 10}};
    const auto report = ProcessReportView::renderToString(fixture.snapshot);
    assert(report.find("CPU utilization: 25%") != std::string::npos);
    assert(report.find("Core: 0") != std::string::npos);
    assert(report.find("Core: -1") == std::string::npos);
    assert(report.find("Sleeping") != std::string::npos);
    assert(report.find("Finished") != std::string::npos);
    std::ostringstream output;
    ScreenLsCommand screen(fixture, output);
    screen.execute({});
    assert(output.str() == report);
    const std::string path = "build/test-csopesy-log.txt";
    std::ostringstream feedback;
    ReportUtilCommand file(fixture, path, "wrong-path", feedback);
    file.execute({});
    assert(feedback.str().find(std::filesystem::absolute(path).string()) != std::string::npos);
    assert(feedback.str().find("wrong-path") == std::string::npos);
    std::ifstream input(path);
    std::stringstream content; content << input.rdbuf();
    assert(content.str() == report);
    std::ostringstream error;
    ReportUtilCommand invalid(fixture, "build/absent-directory/report.txt", "", error);
    invalid.execute({});
    assert(error.str().find("Error:") != std::string::npos);
#ifndef _WIN32
    if (std::filesystem::exists("/dev/full")) {
        std::ostringstream failure;
        ReportUtilCommand full(fixture, "/dev/full", "", failure);
        full.execute({});
        assert(failure.str().find("Error:") != std::string::npos);
        assert(failure.str().find("Report generated") == std::string::npos);
    }
#endif
    std::cout << "All snapshot and report tests passed.\n";
}

#include "controller/commands/ReportUtilCommand.h"

#include <iostream>
#include <fstream>
#include <filesystem>

void ReportUtilCommand::execute(const std::vector<std::string>& /*args*/) {
    std::ofstream file("csopesy-log.txt");
    file << model_.utilization();
    file.close();
    view_.showMessage(file ? "Report generated at " + std::filesystem::absolute("csopesy-log.txt").string()
                          : "Failed to write csopesy-log.txt.");
}

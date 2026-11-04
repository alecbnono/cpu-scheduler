#include "controller/commands/ReportUtilCommand.h"

#include <iostream>
#include <fstream>

void ReportUtilCommand::execute(const std::vector<std::string>& /*args*/) {
    std::ofstream file("csopesy-log.txt");
    file << model_.utilization();
    file.close();
    view_.showMessage(file ? "Report saved to csopesy-log.txt." : "Failed to write csopesy-log.txt.");
}

#pragma once

#include <ostream>
#include <string>
#include "model/SystemSnapshot.hpp"

class ProcessReportView {
public:
    // Writes the report directly to ANY output stream (std::cout, std::ofstream, etc.)
    static void renderReport(std::ostream& os, const SystemSnapshot& snapshot);

    // Helper: returns the report as an std::string
    static std::string renderToString(const SystemSnapshot& snapshot);
};
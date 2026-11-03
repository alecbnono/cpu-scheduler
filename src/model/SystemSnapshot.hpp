#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

// Process Snapshot
// Represents a single process at a moment in time.
struct ProcessSnapshot {
    std::string name;
    std::string timestamp;         // Formatted string, e.g., "01/18/2024 05:15:22AM"
    int coreId = -1;               // Assigned core (e.g., 0, 1), or -1 if not on a core
    std::uint64_t currentInstruction = 0;
    std::uint64_t totalInstructions = 0;

    ProcessSnapshot() = default;
    ProcessSnapshot(std::string processName,
                    std::string timeStr,
                    int core,
                    std::uint64_t currentIns,
                    std::uint64_t totalIns)
        : name(std::move(processName)),
          timestamp(std::move(timeStr)),
          coreId(core),
          currentInstruction(currentIns),
          totalInstructions(totalIns) {}
};

// System Snapshot
// Represents the entire system's state at a captured moment in time.
struct SystemSnapshot {
    std::uint32_t cpuUtilization = 0; // e.g., 100 for 100%
    std::uint32_t coresUsed = 0;
    std::uint32_t coresAvailable = 0;
    std::vector<ProcessSnapshot> runningProcesses;
    std::vector<ProcessSnapshot> finishedProcesses;
};

// Provider Interface
// Decouples reporting from scheduler background threads.
class ISnapshotProvider {
public:
    virtual ~ISnapshotProvider() = default;
    virtual SystemSnapshot getSnapshot() const = 0;
};
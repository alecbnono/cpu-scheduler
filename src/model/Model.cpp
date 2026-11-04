#include "model/Model.hpp"

SystemSnapshot Model::getSnapshot() const {
    SystemSnapshot snap;
    snap.cpuUtilization = 100;
    snap.coresUsed = 4;
    snap.coresAvailable = 0;
    snap.runningProcesses = { // Sample snapshots, matches placeholder in PDF specs
        ProcessSnapshot("process05", "01/18/2024 05:15:22AM", 0, 1235, 5876),
        ProcessSnapshot("process06", "01/18/2024 05:17:22AM", 1, 3, 5876)
    };
    snap.finishedProcesses = {
        ProcessSnapshot("process01", "01/18/2024 05:00:21AM", -1, 5876, 5876),
        ProcessSnapshot("process02", "01/18/2024 05:00:22AM", -1, 5876, 5876)
    };
    return snap;
}
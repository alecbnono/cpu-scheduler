#include "model/Model.hpp"
#include "model/SystemSnapshot.hpp"

#include <cassert>
#include <iostream>

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

    std::cout << "All snapshot tests passed successfully!\n";
    return 0;
}
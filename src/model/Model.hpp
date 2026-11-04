#pragma once

#include "model/SystemSnapshot.hpp"
#include "model/Process.hpp"
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>

struct Configuration {
    unsigned cpus = 4;
    std::string scheduler = "rr";
    std::uint64_t quantum = 5, batchFrequency = 1, minInstructions = 1000,
                  maxInstructions = 2000, delay = 0;
};

// Owns processes and the background clock. All access is synchronized.
class Model : public ISnapshotProvider {
public:
    ~Model();
    bool initialize(const std::string& path, std::string& message);
    bool initialized() const;
    bool createProcess(const std::string& name, std::string& message);
    bool canAttach(const std::string& name) const;
    std::string processInfo(const std::string& name) const;
    std::string utilization() const;
    std::string setGenerating(bool enabled);
  
    SystemSnapshot getSnapshot() const override;



  
private:
    friend struct ModelTestAccess;
    struct Entry {
        std::unique_ptr<Process> process;
        std::uint64_t total;
        int core = -1;
        bool queued = false;
        std::uint64_t delay = 0;
        std::string timestamp;
        std::vector<std::string> formattedLogs;
    };
    struct Core { Entry* entry = nullptr; std::uint64_t slice = 0; };
    void run();
    void advance();
    void create(const std::string& name);
    void enqueue(Entry* entry);
    mutable std::mutex mutex_;
    std::condition_variable changed_;
    std::thread worker_;
    Configuration config_;
    bool initialized_ = false, stopping_ = false, generating_ = false;
    std::uint64_t tick_ = 0, nextId_ = 1, nextBatch_ = 0;
    std::vector<std::unique_ptr<Entry>> processes_;
    std::deque<Entry*> ready_;
    std::vector<Core> cores_;
};

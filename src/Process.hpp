#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <utility>

using Operand = std::variant<std::int64_t, std::string>;

// Programs are constructed by the application, not parsed from console input.
struct Instruction {
    enum class Type { Print, Declare, Add, Subtract, Sleep, For };
    Type type;
    std::string target;
    Operand left = std::int64_t{0};
    Operand right = std::int64_t{0};
    std::optional<std::string> printVariable;
    std::vector<Instruction> body;
    std::uint32_t repeats = 0;

    explicit Instruction(Type kind, std::string name = {}) : type(kind), target(std::move(name)) {}

    static Instruction print(std::string message, std::optional<std::string> variable = std::nullopt);
    static Instruction declare(std::string variable, std::int64_t value);
    static Instruction add(std::string target, Operand left, Operand right);
    static Instruction subtract(std::string target, Operand left, Operand right);
    static Instruction sleep(unsigned ticks);
    static Instruction loop(std::vector<Instruction> body, std::uint32_t repeats);
};

// Seeded generation makes workloads reproducible. Count is top-level instructions.
std::vector<Instruction> generateProgram(const std::string& processName,
                                         std::size_t count, std::uint32_t seed);

class Process {
public:
    enum class State { Ready, Sleeping, Finished };
    enum class StepResult { Executed, Sleeping, Finished };

    Process(std::uint64_t id, std::string name, std::vector<Instruction> program);
    // Execute at most one leaf instruction. Calls while sleeping do no work.
    // After SLEEP(X) at tick T, execution is eligible at T + X (next call for X=0).
    StepResult step(std::uint64_t tick);
    State state() const { return state_; }
    std::uint64_t id() const { return id_; }
    const std::string& name() const { return name_; }
    const std::vector<std::string>& logs() const { return logs_; }
    const std::unordered_map<std::string, std::uint16_t>& variables() const { return variables_; }
    std::uint64_t executedInstructions() const { return executed_; }
    std::uint64_t wakeTick() const { return wakeTick_; }

private:
    struct Frame {
        const std::vector<Instruction>* instructions;
        std::size_t index;
        std::uint32_t remaining;
    };
    // Stack frames point into this process's immutable program.
    Process(const Process&) = delete;
    Process& operator=(const Process&) = delete;
    Process(Process&&) = delete;
    Process& operator=(Process&&) = delete;

    void normalize();
    std::int64_t resolve(const Operand& operand);
    std::uint64_t id_;
    std::string name_;
    const std::vector<Instruction> program_;
    std::vector<Frame> stack_;
    std::unordered_map<std::string, std::uint16_t> variables_;
    std::vector<std::string> logs_;
    State state_ = State::Ready;
    std::uint64_t wakeTick_ = 0;
    std::uint64_t executed_ = 0;
};

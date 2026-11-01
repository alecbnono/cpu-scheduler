#include "Process.hpp"

#include <cassert>
#include <iostream>
#include <stdexcept>

using I = Instruction;

int main() {
    Process arithmetic(1, "math", {
        I::declare("x", 65535), I::add("x", std::string("x"), std::int64_t{1}),
        I::print("upper=", "x"), I::subtract("x", std::int64_t{0}, std::int64_t{1}),
        I::print("lower=", "x"), I::add("y", std::string("missing"), std::int64_t{12}),
        I::print("auto=", "y"), I::declare("negative", -100), I::print("clamp=", "negative")});
    for (unsigned tick = 0; tick < 9; ++tick) arithmetic.step(tick);
    assert((arithmetic.logs() == std::vector<std::string>{"upper=65535", "lower=0", "auto=12", "clamp=0"}));
    assert(arithmetic.state() == Process::State::Finished);
    assert(arithmetic.variables().empty());
    assert(arithmetic.step(100) == Process::StepResult::Finished);

    Process sleeper(2, "sleep", {I::declare("x", 9), I::sleep(3), I::print("x=", "x")});
    sleeper.step(0);
    sleeper.step(1);
    assert(sleeper.state() == Process::State::Sleeping);
    assert(sleeper.step(3) == Process::StepResult::Sleeping);
    assert(sleeper.executedInstructions() == 2);
    assert(sleeper.variables().at("x") == 9);
    sleeper.step(4);
    assert(sleeper.logs().at(0) == "x=9");
    assert(sleeper.state() == Process::State::Finished);

    Process loops(3, "loops", {I::loop({I::loop({I::loop({I::print("hello")}, 2)}, 3)}, 4),
                               I::loop({I::print("never")}, 0)});
    for (unsigned tick = 0; tick < 24; ++tick) loops.step(tick);
    assert(loops.logs().size() == 24);
    assert(loops.state() == Process::State::Finished);
    bool rejected = false;
    try { Process invalid(4, "invalid", {I::loop({I::loop({I::loop({I::loop({}, 1)}, 1)}, 1)}, 1)}); }
    catch (const std::invalid_argument&) { rejected = true; }
    assert(rejected);
    rejected = false;
    try { I::sleep(256); } catch (const std::invalid_argument&) { rejected = true; }
    assert(rejected);
    Process empty(5, "empty", {});
    assert(empty.state() == Process::State::Finished);
    Process zeroSleep(6, "zero", {I::sleep(0), I::print("awake")});
    zeroSleep.step(0);
    zeroSleep.step(1);
    assert(zeroSleep.logs().size() == 1);
    Process finalSleep(7, "final", {I::declare("x", 1), I::sleep(255)});
    finalSleep.step(0);
    finalSleep.step(1);
    assert(finalSleep.step(255) == Process::StepResult::Sleeping);
    finalSleep.step(256);
    assert(finalSleep.state() == Process::State::Finished);
    assert(finalSleep.variables().empty());
    Process generated(8, "generated", generateProgram("generated", 100, 42));
    for (unsigned tick = 0; tick < 100000 && generated.state() != Process::State::Finished; ++tick)
        generated.step(tick);
    assert(generated.state() == Process::State::Finished);
    for (const auto& log : generated.logs()) assert(log == "Hello world from generated!");
    std::cout << "All process instruction tests passed.\n";
}

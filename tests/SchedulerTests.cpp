#include "model/Model.hpp"
#include <cassert>
#include <iostream>
#include <set>
#include <regex>

// Exercise the actual scheduler without a wall clock or background thread.
struct ModelTestAccess {
    static void configure(Model& m, std::string algorithm, unsigned cpus = 1,
                          unsigned quantum = 2, unsigned delay = 0) {
        m.initialized_ = true;
        m.config_.scheduler = algorithm; m.config_.cpus = cpus;
        m.config_.quantum = quantum; m.config_.delay = delay;
        m.config_.minInstructions = m.config_.maxInstructions = 7;
        m.config_.batchFrequency = 3;
        m.cores_.resize(cpus);
    }
    static void add(Model& m, const std::string& name, std::vector<Instruction> program) {
        auto e = std::make_unique<Model::Entry>();
        e->total = program.size();
        e->process = std::make_unique<Process>(m.nextId_++, name, std::move(program));
        e->timestamp = "11/04/2026 01:02:03PM";
        e->delay = m.config_.delay;
        m.processes_.push_back(std::move(e));
        m.enqueue(m.processes_.back().get());
    }
    static void tick(Model& m) { m.advance(); }
    static std::uint64_t count(Model& m, unsigned i) { return m.processes_.at(i)->process->executedInstructions(); }
    static bool finished(Model& m, unsigned i) { return m.processes_.at(i)->process->state() == Process::State::Finished; }
    static std::size_t size(Model& m) { return m.processes_.size(); }
    static int core(Model& m, unsigned i) { return m.processes_.at(i)->core; }
};
using A = ModelTestAccess;
using I = Instruction;
std::vector<Instruction> prints(unsigned n) { return std::vector<Instruction>(n, I::print("hello")); }
int main() {
    {
        Model m; A::configure(m, "fcfs"); A::add(m, "first", prints(3)); A::add(m, "second", prints(3));
        for (unsigned i = 1; i <= 3; ++i) { A::tick(m); assert(A::count(m,0) == i && A::count(m,1) == 0); }
        A::tick(m); assert(A::count(m,1) == 1);
        const auto text = m.processInfo("first");
        assert(text.find("Process name: first\nID: 1\nLogs:\n") == 0);
        assert(text.find("Finished!") != std::string::npos);
        assert(text.find("Current instruction line:") == std::string::npos);
        assert(std::regex_search(text, std::regex(R"(\(\d{2}/\d{2}/\d{4} \d{2}:\d{2}:\d{2}[AP]M\) Core:0 "hello")")));
    }
    {
        Model m; A::configure(m, "rr"); A::add(m,"a",prints(6)); A::add(m,"b",prints(6));
        for (unsigned cycle = 0; cycle < 3; ++cycle) {
            A::tick(m); A::tick(m);
            assert(A::count(m,0) == 2*(cycle+1) && A::count(m,1) == 2*cycle);
            A::tick(m); A::tick(m);
            assert(A::count(m,0) == A::count(m,1));
        }
        assert(A::finished(m,0) && A::finished(m,1));
    }
    {
        Model m; A::configure(m,"fcfs",2);
        for (unsigned i=0;i<4;++i) A::add(m,std::to_string(i),prints(4));
        A::tick(m);
        assert(A::core(m,0) == 0 && A::core(m,1) == 1);
        assert(A::count(m,0)==1 && A::count(m,1)==1 && A::count(m,2)==0);
        const auto snap = m.getSnapshot(); assert(snap.coresUsed==2 && snap.cpuUtilization==100);
    }
    {
        Model m; A::configure(m,"rr",4,1); A::add(m,"single",prints(8));
        for(unsigned i=1;i<=8;++i) { A::tick(m); assert(A::count(m,0)==i); }
    }
    {
        Model m; A::configure(m,"fcfs");
        A::add(m,"sleeper",{I::sleep(3),I::print("awake")}); A::add(m,"other",prints(2));
        A::tick(m); assert(A::core(m,0)==-1);
        assert(m.utilization().find("Sleeping")!=std::string::npos);
        A::tick(m); A::tick(m); assert(A::count(m,0)==1 && A::count(m,1)==2);
        A::tick(m); assert(A::finished(m,0));
    }
    {
        Model m; A::configure(m,"fcfs",1,2,2); A::add(m,"delayed",prints(2)); A::add(m,"waiting",prints(1));
        A::tick(m); A::tick(m); assert(A::count(m,0)==0 && A::core(m,0)==0);
        A::tick(m); assert(A::count(m,0)==1);
        A::tick(m); A::tick(m); assert(A::count(m,0)==1 && A::count(m,1)==0);
        A::tick(m); assert(A::finished(m,0));
    }
    {
        Model m; A::configure(m,"rr",1,1,3); A::add(m,"a",prints(1)); A::add(m,"b",prints(1));
        for(unsigned i=0;i<8;++i) A::tick(m);
        assert(A::finished(m,0) && A::finished(m,1));
    }
    {
        Model m; A::configure(m,"rr"); m.setGenerating(true);
        for(unsigned tick=1;tick<=12;++tick) { A::tick(m); assert(A::size(m)==tick/3); }
        m.setGenerating(false);
        for(unsigned tick=0;tick<12;++tick) A::tick(m);
        assert(A::size(m)==4);
        auto snap=m.getSnapshot();
        for(const auto& p:snap.runningProcesses) assert(p.totalInstructions==7);
        for(const auto& p:snap.finishedProcesses) assert(p.totalInstructions==7);
    }
    for(unsigned n=0;n<=30;++n) for(unsigned seed=0;seed<20;++seed) {
        Process p(1,"bounds",generateProgram("bounds",n,seed));
        for(unsigned tick=0;tick<20000 && p.state()!=Process::State::Finished;++tick) p.step(tick);
        assert(p.state()==Process::State::Finished && p.executedInstructions()==n);
    }
    std::cout << "All scheduler behavior tests passed.\n";
}

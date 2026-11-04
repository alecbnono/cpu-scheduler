# CPU Scheduler Emulator

A CPU scheduler emulator with a command-line interface, written in C++17 to practice operating systems concepts (CSOPESY). It reads commands from a prompt, parses them, and responds. The code is split into Model / View / Controller.

## Group members
- Nono, Alec Marx  
- Ponce, Jean Rondel  
- Obregon, Sian Ysabelle  
- Sy, Prince Matthew  

## Commands

Start with `initialize` to load `config.txt`. Before initialization, only
`initialize` and `exit` are accepted. Configuration errors leave the application
uninitialized so you can fix the file and retry. Repeated initialization does not
reset existing processes.

| Command           | Description                                       |
| ----------------- | ------------------------------------------------- |
| `initialize`      | Initializes the processor configuration           |
| `screen -s <name>` | Creates a process and opens its screen            |
| `screen -r <name>` | Reattaches to an unfinished process               |
| `screen -ls`      | Lists CPU usage and running/finished processes     |
| `scheduler-start` | Starts generating dummy processes                 |
| `scheduler-stop`  | Stops generating dummy processes                  |
| `report-util`     | Generates the CPU utilization report              |
| `process-demo`    | Shows process instructions executing tick by tick |
| `help`            | Lists the available commands                      |
| `exit`            | Quits the program                                 |

Inside a process screen, `process-smi` shows its ID, instruction progress, status,
and PRINT logs. `exit` returns to the main menu while execution continues.
Finished processes remain listed, but cannot be reattached after leaving their
screen. Process names must be unique, including names of finished processes.

Process output follows the PDF labels: `Process name:`, `ID:`, `Logs:`,
`Current instruction line:`, and `Lines of code:`. Each PRINT log records its
actual execution timestamp and CPU core, for example:

```text
Process name: screen_01
ID: 1
Logs:
(11/04/2026 09:15:22AM) Core:0 "Hello world from screen_01!"

Current instruction line: 153
Lines of code: 1651
```

On completion, `Finished!` replaces the progress lines. `screen -ls` and the
exported report use aligned name, creation timestamp, core/status, and progress
columns under `Running processes:` and `Finished processes:`. Ready/sleeping
processes show their status instead of an assigned core. Times use the machine's
local timezone. Actual timestamps, progress, and utilization vary during execution.

### Try the main menu and process screens

Build and run from the repository root using `make run` (or the PowerShell build
command below), then enter these commands one at a time:

```text
initialize
screen -s demo
process-smi
exit
screen -ls
screen -r demo
process-smi
exit
report-util
exit
```

The prompt changes from `main>` to `demo>` when attached. Use `process-smi`
again after a few seconds to see updated progress. `screen -r demo` reports
not found if the process has already finished. `report-util` writes the same
summary as `screen -ls` to `csopesy-log.txt` in the working directory.

For generated workloads, enter `scheduler-start` in the main menu, wait a few
seconds, then enter `scheduler-stop` and `screen -ls`. Existing processes keep
executing after generation stops. PRINT logs appear only inside their process
screens; they do not interrupt the main menu.

### Configuration

The included `config.txt` uses the PDF sample values. Edit it before launching
and initializing. Supported schedulers are `"fcfs"` and `"rr"`; CPU count is
1..128. Quantum, generation frequency, and instruction bounds accept 1..2^32;
execution delay accepts 0..2^32. The minimum instruction bound must not exceed
the maximum. Large instruction bounds allocate correspondingly large programs.

The background runtime advances one simulated CPU tick every 10 ms. Each core
executes at most one leaf instruction per tick, with configured delays retaining
the core. Sleeping processes release their core. Round robin preempts after the
configured quantum; FCFS retains a core until the process sleeps or finishes.
Instruction bounds count generated top-level instructions; loop bodies add
executed instructions. Workloads are seeded by process ID for reproducibility.

## Requirements

- A C++17 compiler (`g++` / GCC 7 or newer)
- `make` (optional, you can also compile with `g++` directly)

## Linux

Install the tools if needed:

```bash
# Debian/Ubuntu
sudo apt install build-essential
# Arch
sudo pacman -S base-devel
# Fedora
sudo dnf install gcc-c++ make
```

Build and run with make:

```bash
make          # builds build/mo3
make run      # builds and runs
make clean    # removes build/
```

Or with g++ only:

```bash
mkdir -p build
g++ -std=c++17 -Wall -Wextra -pthread -Isrc src/main.cpp src/model/*.cpp src/view/*.cpp src/controller/*.cpp src/controller/commands/*.cpp -o build/mo3
./build/mo3
```

## Windows

Install GCC through [MSYS2](https://www.msys2.org/) (or any MinGW-w64 distribution), then in the **MSYS2 UCRT64** shell:

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc make
```

Build and run with make from the project root (in the MSYS2 shell or with `mingw32-make`):

```bash
make
./build/mo3.exe
```

Or with g++ only, which also works from PowerShell or Command Prompt once GCC is on your `PATH`:

```bat
mkdir build
g++ -std=c++17 -Wall -Wextra -Isrc src/main.cpp src/model/*.cpp src/view/*.cpp src/controller/*.cpp src/controller/commands/*.cpp -o build/mo3.exe
build\mo3.exe
```

If wildcards are not expanded by your shell, list the files explicitly:

```bat
g++ -std=c++17 -Isrc src/main.cpp src/controller/Controller.cpp src/controller/commands/ScreenLsCommand.cpp src/controller/commands/ReportUtilCommand.cpp src/model/Model.cpp src/view/View.cpp src/view/AsciiArt.cpp src/model/Process.cpp -o build/mo3.exe
```

## Process instruction engine

### See it running

From PowerShell in the repository root, build and launch the application:

```powershell
New-Item -ItemType Directory -Force build | Out-Null
g++ -std=c++17 -Wall -Wextra -pthread -Isrc src/main.cpp src/controller/Controller.cpp src/controller/commands/ScreenLsCommand.cpp src/controller/commands/ReportUtilCommand.cpp src/model/Model.cpp src/view/View.cpp src/view/AsciiArt.cpp src/model/Process.cpp -o build/mo3.exe
if ($LASTEXITCODE -eq 0) { ./build/mo3.exe }
```

At the application's prompt, enter:

```text
initialize
process-demo
```

You will see DECLARE/ADD produce 15, SUBTRACT clamp to 0, nested loops print
1 through 4, and SLEEP yield at tick 13 until tick 16. The demo then prints
`Hello world from demo!` and `Finished!`. It uses simulated ticks without real-time
pauses. This runs the actual instruction engine with a fixed program. Use the
screen commands above for background execution. Enter `exit` to quit.

`src/model/Process.hpp` provides predetermined programs with `Instruction::print`,
`declare`, `add`, `subtract`, `sleep`, and `loop`. Arithmetic operands are either
`std::int64_t` literals or `std::string` variable names. Values are clamped to
0..65535; missing variables start at zero. PRINT accepts a message and optionally
one variable name to append. Output is retained in `Process::logs()` and never
written directly to the console. The future attached-screen UI should display it.

Call `Process::step(cpuTick)` once per assigned execution opportunity using
monotonically increasing CPU ticks. It executes at most one leaf instruction.
FOR control does not consume a tick and supports three nesting levels; loops are
executed using stack frames, without expanding repeated instructions in memory.
SLEEP accepts 0..255 ticks: at tick T, SLEEP(X) yields immediately and becomes
eligible at T+X. Even SLEEP(0) ends the current step. A scheduler must release the
core when the state is Sleeping and check wake eligibility using `wakeTick()`.
Variables remain private to the process until completion, then are cleared;
logs remain available for finished-process reports.

`generateProgram(name, count, seed)` creates reproducible randomized workloads
covering all six instruction types, with the default PRINT message
`Hello world from <process_name>!`. Count measures top-level instructions; loop
bodies add executed instructions. Programs can also be constructed explicitly
for quiz scenarios. The background runtime uses this engine for screen-created
and scheduler-generated processes.

### Testing the process instructions

Run these commands in a terminal from the repository root. These are shell
commands, not commands entered at the application's `>` prompt. The instruction
engine is also tested through a separate executable.

**Windows PowerShell** (requires `g++` on your PATH):

```powershell
New-Item -ItemType Directory -Force build | Out-Null
g++ -std=c++17 -Wall -Wextra -Isrc tests/ProcessTests.cpp src/model/Process.cpp -o build/process-tests.exe
if ($LASTEXITCODE -eq 0) { ./build/process-tests.exe }
```

**Linux or MSYS2 UCRT64 shell:**

```bash
make test
```

Expected output:

```text
All process instruction tests passed.
```

`make test` additionally runs menu/screen integration tests and prints
`All menu and screen tests passed.` These check initialization errors and gating,
screen navigation, duplicate and missing names, background execution under FCFS
and round robin, finished-process access, workload generation, and report export.
Test fixtures are stored under the ignored `build/` directory.

The tests check PRINT output and variable interpolation, DECLARE, ADD/SUBTRACT
clamping and implicit variables, sleep/wake timing (including 0 and 255 ticks),
three nested FOR levels, zero-repeat loops, invalid sleep/nesting rejection,
variable cleanup on completion, and generated workloads. A failed check reports
an assertion failure and exits with a nonzero status. Do not compile tests with
`-DNDEBUG`, because that disables assertions.

## Project layout

```
Makefile
src/
  main.cpp                read-eval loop
  controller/
    Controller.*          tokenizes and dispatches commands
    commands/
      ScreenLsCommand.*   lists processes and CPU utilization
      ReportUtilCommand.* generates utilization report
  model/
    Model.*               configuration, process registry, background runtime
    Process.*             process state, instruction execution, randomized programs
  view/
    View.*                all console output
    AsciiArt.*            renders text as figlet-style ASCII art
```

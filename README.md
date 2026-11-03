# CPU Scheduler Emulator

A CPU scheduler emulator with a command-line interface, written in C++17 to practice operating systems concepts (CSOPESY). It reads commands from a prompt, parses them, and responds. The code is split into Model / View / Controller.

## Group members
- Nono, Alec Marx  
- Ponce, Jean Rondel  
- Obregon, Sian Ysabelle  
- Sy, Prince Matthew  

## Commands

`initialize`, `screen`, `scheduler-start`, `scheduler-stop`, and `report-util` are placeholders for now.

| Command           | Description                                       |
| ----------------- | ------------------------------------------------- |
| `initialize`      | Initializes the processor configuration           |
| `screen`          | Creates, reattaches to, or lists processes        |
| `scheduler-start` | Starts generating dummy processes                 |
| `scheduler-stop`  | Stops generating dummy processes                  |
| `report-util`     | Generates the CPU utilization report              |
| `process-demo`    | Shows process instructions executing tick by tick |
| `help`            | Lists the available commands                      |
| `exit`            | Quits the program                                 |

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
g++ -std=c++17 -Wall -Wextra src/*.cpp -o build/mo3
./build/mo3
```

## Windows

Install GCC through [MSYS2](https://www.msys2.org/) (or any MinGW-w64 distribution), then in the **MSYS2 UCRT64** shell:

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc make
```

Build and run with make from the project root (in the MSYS2 shell, so `mkdir -p` and `rm -rf` in the Makefile work):

```bash
make
./build/mo3.exe
```

Or with g++ only, which also works from PowerShell or Command Prompt once GCC is on your `PATH`:

```bat
mkdir build
g++ -std=c++17 -Wall -Wextra src/*.cpp -o build/mo3.exe
build\mo3.exe
```

If `src/*.cpp` is not expanded by your shell, list the files explicitly:

```bat
g++ -std=c++17 src/main.cpp src/Controller.cpp src/Model.cpp src/View.cpp src/AsciiArt.cpp src/Process.cpp -o build/mo3.exe
```

## Process instruction engine

### See it running

From PowerShell in the repository root, build and launch the application:

```powershell
New-Item -ItemType Directory -Force build | Out-Null
g++ -std=c++17 -Wall -Wextra -pthread src/main.cpp src/Controller.cpp src/Model.cpp src/View.cpp src/AsciiArt.cpp src/Process.cpp -o build/mo3.exe
if ($LASTEXITCODE -eq 0) { ./build/mo3.exe }
```

At the application's `>` prompt, enter:

```text
process-demo
```

You will see DECLARE/ADD produce 15, SUBTRACT clamp to 0, nested loops print
1 through 4, and SLEEP yield at tick 13 until tick 16. The demo then prints
`Hello world from demo!` and `Finished!`. It uses simulated ticks without real-time
pauses. This runs the actual instruction engine with a fixed program; the full
background scheduler and process screens are still pending. Enter `exit` to quit.

`src/Process.hpp` provides predetermined programs with `Instruction::print`,
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
for quiz scenarios. Configuration loading, scheduling and screen commands are
separate work and are not connected to this engine yet.

### Testing the process instructions

Run these commands in a terminal from the repository root. These are shell
commands, not commands entered at the application's `>` prompt. The instruction
engine is tested through a separate executable until scheduler/screens are wired up.

**Windows PowerShell** (requires `g++` on your PATH):

```powershell
New-Item -ItemType Directory -Force build | Out-Null
g++ -std=c++17 -Wall -Wextra -Isrc tests/ProcessTests.cpp src/Process.cpp -o build/process-tests.exe
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
  main.cpp        read-eval loop
  Controller.*    tokenizes and dispatches commands
  Model.*         holds application state
  View.*          all console output
  AsciiArt.*      renders text as figlet-style ASCII art
  Process.*       process state, instruction execution, randomized programs
```

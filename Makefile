CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -g -pthread -Isrc -MMD -MP
SRC_DIR := src
BUILD_DIR := build
TARGET := $(BUILD_DIR)/mo3

# Support Windows (cmd.exe) and Unix/MSYS2 shells
ifeq ($(OS),Windows_NT)
  ifeq ($(shell where sh 2>nul),)
    SHELL := cmd.exe
    MKDIR_BUILD := if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
    RM_BUILD := if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)
  else
    MKDIR_BUILD := mkdir -p $(BUILD_DIR)
    RM_BUILD := rm -rf $(BUILD_DIR)
  endif
  RUN_TARGET := $(TARGET)
  RUN_TEST := $(BUILD_DIR)/process-tests
else
  MKDIR_BUILD := mkdir -p $(BUILD_DIR)
  RM_BUILD := rm -rf $(BUILD_DIR)
  RUN_TARGET := ./$(TARGET)
  RUN_TEST := ./$(BUILD_DIR)/process-tests
endif

SOURCES := $(wildcard $(SRC_DIR)/*.cpp) \
           $(wildcard $(SRC_DIR)/model/*.cpp) \
           $(wildcard $(SRC_DIR)/view/*.cpp) \
           $(wildcard $(SRC_DIR)/controller/*.cpp) \
           $(wildcard $(SRC_DIR)/controller/commands/*.cpp)

OBJECTS := $(addprefix $(BUILD_DIR)/, $(notdir $(SOURCES:.cpp=.o)))

-include $(OBJECTS:.o=.d)
.DEFAULT_GOAL := all

vpath %.cpp $(SRC_DIR) $(SRC_DIR)/model $(SRC_DIR)/view $(SRC_DIR)/controller $(SRC_DIR)/controller/commands

.PHONY: all clean run test test-report

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) -pthread $(OBJECTS) -o $(TARGET)

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	$(MKDIR_BUILD)

run: all
	$(RUN_TARGET)

test: | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) tests/ProcessTests.cpp src/model/Process.cpp -o $(BUILD_DIR)/process-tests
	$(RUN_TEST)
	$(CXX) $(CXXFLAGS) tests/MenuTests.cpp src/model/Model.cpp src/model/Process.cpp src/view/View.cpp src/view/AsciiArt.cpp src/view/ProcessReportView.cpp src/controller/Controller.cpp src/controller/commands/ScreenLsCommand.cpp src/controller/commands/ReportUtilCommand.cpp -o $(BUILD_DIR)/menu-tests
	$(BUILD_DIR)/menu-tests

test-report: | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) tests/SnapshotTests.cpp src/model/Model.cpp src/model/Process.cpp src/view/ProcessReportView.cpp src/controller/commands/ScreenLsCommand.cpp src/controller/commands/ReportUtilCommand.cpp -o $(BUILD_DIR)/snapshot-tests
	$(BUILD_DIR)/snapshot-tests

clean:
	$(RM_BUILD)

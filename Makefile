CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -g
SRC_DIR := src
BUILD_DIR := build
TARGET := $(BUILD_DIR)/mo3

SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS := $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

.PHONY: all clean run test

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

run: all
	./$(TARGET)

test: | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -Isrc tests/ProcessTests.cpp src/Process.cpp -o $(BUILD_DIR)/process-tests
	./$(BUILD_DIR)/process-tests

clean:
	rm -rf $(BUILD_DIR)

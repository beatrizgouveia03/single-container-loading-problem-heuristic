CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -g -Isrc

SRC_DIR   := src
TEST_DIR  := test
BUILD_DIR := build

TARGET := $(BUILD_DIR)/test_corners2d

SRCS := $(SRC_DIR)/Coordinates2d.cpp \
        $(SRC_DIR)/Corners2d.cpp \
        $(SRC_DIR)/Package.cpp \
        $(TEST_DIR)/test_corners2d.cpp

OBJS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRCS))

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)
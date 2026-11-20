CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -g -Isrc

SRC_DIR   := src
TEST_DIR  := test
BUILD_DIR := build

LIB_SRCS := $(SRC_DIR)/Corners2d.cpp \
            $(SRC_DIR)/Corners3d.cpp \
            $(SRC_DIR)/Package.cpp \
            $(SRC_DIR)/BranchAndBound.cpp \
            $(SRC_DIR)/InstanceManager.cpp
LIB_OBJS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(LIB_SRCS))

TESTBAB := $(BUILD_DIR)/test_branch_and_bound

.PHONY: all run clean

all: $(TESTBAB)

$(TESTBAB): $(LIB_OBJS) $(BUILD_DIR)/$(TEST_DIR)/test_branch_and_bound.o
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# uso: make run ARGS="f36_32_6 32 1 3 6 10 8 8 14"
run: $(TESTBAB)
	./$(TESTBAB) $(ARGS)

clean:
	rm -rf $(BUILD_DIR)
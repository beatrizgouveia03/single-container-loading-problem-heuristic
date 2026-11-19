CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -g -Isrc

SRC_DIR   := src
TEST_DIR  := test
BUILD_DIR := build

LIB_SRCS := $(SRC_DIR)/Corners2d.cpp \
            $(SRC_DIR)/Corners3d.cpp \
            $(SRC_DIR)/Package.cpp \
            $(SRC_DIR)/BranchAndBound.cpp \
            $(SRC_DIR)/InstanceManager.cpp
			
LIB_OBJS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(LIB_SRCS))

TEST2D := $(BUILD_DIR)/test_corners2d
TEST3D := $(BUILD_DIR)/test_corners3d
TESTVOLUME := $(BUILD_DIR)/test_volume
TESTBAB := $(BUILD_DIR)/test_branch_and_bound
TESTBABSOLVE := $(BUILD_DIR)/test_bab_solve

.PHONY: all run run2d run3d runVolume runBab clean

all: $(TEST2D) $(TEST3D) $(TESTVOLUME) $(TESTBABSOLVE)

$(TESTBABSOLVE): $(LIB_OBJS) $(BUILD_DIR)/$(TEST_DIR)/test_bab_solve.o
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TEST2D): $(LIB_OBJS) $(BUILD_DIR)/$(TEST_DIR)/test_corners2d.o
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TEST3D): $(LIB_OBJS) $(BUILD_DIR)/$(TEST_DIR)/test_corners3d.o
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TESTVOLUME): $(LIB_OBJS) $(BUILD_DIR)/$(TEST_DIR)/test_volume.o
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TESTBAB): $(LIB_OBJS) $(BUILD_DIR)/$(TEST_DIR)/test_branch_and_bound.o
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: run2d run3d runVolume runBabSolve

runBabSolve: $(TESTBABSOLVE)
	./$(TESTBABSOLVE)

run2d: $(TEST2D)
	./$(TEST2D)

run3d: $(TEST3D)
	./$(TEST3D)

runVolume: $(TESTVOLUME)
	./$(TESTVOLUME)

runBab: $(TESTBAB)
	./$(TESTBAB)

clean:
	rm -rf $(BUILD_DIR)

TESTBABSORTED := $(BUILD_DIR)/test_bab_sorted


$(TESTBABSORTED): $(LIB_OBJS) $(BUILD_DIR)/$(TEST_DIR)/test_bab_sorted.o
	$(CXX) $(CXXFLAGS) -o $@ $^

runBabSorted: $(TESTBABSORTED)
	./$(TESTBABSORTED)
CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -g -Isrc

SRC_DIR   := src
TEST_DIR  := test
BUILD_DIR := build

LIB_SRCS := $(SRC_DIR)/Corners2d.cpp \
            $(SRC_DIR)/Corners3d.cpp \
            $(SRC_DIR)/Package.cpp
			
LIB_OBJS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(LIB_SRCS))

TEST2D := $(BUILD_DIR)/test_corners2d
TEST3D := $(BUILD_DIR)/test_corners3d
TESTVOLUME := $(BUILD_DIR)/test_volume

.PHONY: all run run2d run3d runVolume clean

all: $(TEST2D) $(TEST3D)

$(TEST2D): $(LIB_OBJS) $(BUILD_DIR)/$(TEST_DIR)/test_corners2d.o
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TEST3D): $(LIB_OBJS) $(BUILD_DIR)/$(TEST_DIR)/test_corners3d.o
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TESTVOLUME): $(LIB_OBJS) $(BUILD_DIR)/$(TEST_DIR)/test_volume.o
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: run2d run3d runVolume

run2d: $(TEST2D)
	./$(TEST2D)

run3d: $(TEST3D)
	./$(TEST3D)

runVolume: $(TESTVOLUME)
	./$(TESTVOLUME)

clean:
	rm -rf $(BUILD_DIR)
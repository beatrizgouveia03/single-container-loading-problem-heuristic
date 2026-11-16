CXX      := g++
BUILD    := build
CXXFLAGS := -O2 -Iheaders -std=c++17 -Wall -Wextra -MMD -MP

EXACT_SRCS := $(wildcard src/*.cpp)
HEUR_SRCS  := $(wildcard ../heuristic/cpps/*.cpp)
TEST_SRCS  := $(wildcard test/*.cpp)

EXACT_OBJS := $(patsubst src/%.cpp,$(BUILD)/%.o,$(EXACT_SRCS))
HEUR_OBJS  := $(patsubst ../heuristic/cpps/%.cpp,$(BUILD)/%.o,$(HEUR_SRCS))
TEST_OBJS  := $(patsubst test/%.cpp,$(BUILD)/test/%.o,$(TEST_SRCS))
TEST_BINS  := $(patsubst test/%.cpp,$(BUILD)/test/%,$(TEST_SRCS))

LIB_OBJS := $(EXACT_OBJS) $(HEUR_OBJS)
MAIN_OBJ := $(BUILD)/main.o

.PHONY: all test clean
.SECONDARY:   # keep the .o files that sit between a .cpp and a test binary

all: $(LIB_OBJS) $(TEST_OBJS)

# builds and runs every test executable, stops at the first failure
test: $(TEST_BINS)
	@for t in $(TEST_BINS); do echo "== $$t"; ./$$t || exit 1; done

# one executable per test file: its own .o + all library objects
$(BUILD)/test/%: $(BUILD)/test/%.o $(LIB_OBJS)
	$(CXX) $^ -o $@

$(BUILD)/%.o: src/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/%.o: ../heuristic/cpps/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/main.o: main.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/test/%.o: test/%.cpp | $(BUILD)/test
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/test:
	mkdir -p $(BUILD)/test

clean:
	rm -rf $(BUILD)

-include $(wildcard $(BUILD)/*.d $(BUILD)/test/*.d)
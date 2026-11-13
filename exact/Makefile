CXX      := g++
BUILD    := build
CXXFLAGS := -Iheaders -std=c++17 -Wall -Wextra

EXACT_SRCS := $(wildcard src/*.cpp)
HEUR_SRCS  := $(wildcard ../heuristic/cpps/*.cpp)
TEST_SRCS := $(wildcard test/*.cpp)

EXACT_OBJS := $(patsubst src/%.cpp,$(BUILD)/%.o,$(EXACT_SRCS))
HEUR_OBJS  := $(patsubst ../heuristic/cpps/%.cpp,$(BUILD)/%.o,$(HEUR_SRCS))
TEST_OBJS := $(patsubst test/%.cpp,$(BUILD)/test/%.o,$(TEST_SRCS))

LIB_OBJS := $(EXACT_OBJS) $(HEUR_OBJS)
MAIN_OBJ := $(BUILD)/main.o
TEST     := $(BUILD)/test/run_tests

.PHONY: all c test clean

all: $(LIB_OBJS) | $(TEST_OBJS)

test: $(TEST)
	./$(TEST)

$(TEST): $(TEST_OBJS) $(LIB_OBJS)
	$(CXX) $^ -o $@

$(BUILD)/%.o: src/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/%.o: ../heuristic/cpps/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/main.o: main.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/test/%.o: test/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD):
	mkdir -p $(BUILD)
	mkdir -p $(BUILD)/test

clean:
	rm -rf $(BUILD)
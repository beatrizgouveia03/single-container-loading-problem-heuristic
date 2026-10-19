CXX      := g++
CXXFLAGS := -Iheaders -std=c++17 -Wall -Wextra
BUILD    := build

# Código compartilhado (sem main)
LIB_SRCS := $(wildcard cpps/*.cpp)
LIB_OBJS := $(LIB_SRCS:cpps/%.cpp=$(BUILD)/%.o)

# Programa principal
MAIN_OBJ := $(BUILD)/main.o
APP      := exec_trabalho

# Arquivos de teste
TEST_SRCS := $(wildcard test/*.cpp)
TEST_OBJS := $(TEST_SRCS:test/%.cpp=$(BUILD)/test_%.o)
TEST      := $(BUILD)/run_tests

.PHONY: all run test clean

all: $(APP)

run: $(APP)
	./$(APP)

test: $(TEST)
	./$(TEST)

# Executável principal
$(APP): $(MAIN_OBJ) $(LIB_OBJS)
	$(CXX) $^ -o $@

# Executável de testes
$(TEST): $(TEST_OBJS) $(LIB_OBJS)
	$(CXX) $^ -o $@

# main.cpp -> build/main.o
$(BUILD)/main.o: main.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# cpps/*.cpp -> build/*.o
$(BUILD)/%.o: cpps/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# test/*.cpp -> build/test_*.o
$(BUILD)/test_%.o: test/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD) $(APP) relatorio_saida.txt

-include $(wildcard $(BUILD)/*.d)
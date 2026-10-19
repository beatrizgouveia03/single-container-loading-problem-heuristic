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

.PHONY: all run test visual clean

all: $(APP)

run: $(APP)
	./$(APP)

test: $(TEST)
	./$(TEST)

# Gera a visualização 3D da solução em visual/index.html
#   make visual        -> usa o exemplo embutido (demo)
#   make visual ID=6   -> roda a heurística na instância 6 e usa o relatório dela
visual: $(APP)
	./$(APP) $(if $(ID),rodar $(ID),demo) > /dev/null
	python3 visual/gerar_visualizacao.py saidas/saida_$(if $(ID),$(ID),demo).txt

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
	rm -rf $(BUILD) $(APP) saidas relatorio_saida.txt

-include $(wildcard $(BUILD)/*.d)
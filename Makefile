CXX      := g++
CXXFLAGS := -Iheaders -std=c++17 -Wall -Wextra
BUILD    := build

# Grupo usado pelos alvos grupo/grafico (make grupo GRUPO=escala)
GRUPO ?= escala

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

.PHONY: all run test visual grupo grafico clean

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

# Cria e roda um grupo de 50 instâncias com dimensões crescentes
#   make grupo GRUPO=escala
# Cada instância i usa pacotes entre 1*i e 5*i e contêiner 15*i x 10*i x 10*i.
grupo: $(APP)
	./$(APP) criar-grupo $(GRUPO) 50 20 1 5 15 10 10
	./$(APP) rodar-grupo $(GRUPO)

# Gera a imagem do desempenho do grupo (a partir de desempenho.csv)
#   make grafico GRUPO=escala  ->  visual/desempenho_escala.svg
grafico:
	python3 visual/gerar_grafico_desempenho.py instances/$(GRUPO)

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
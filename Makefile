CXX = g++
CXXFLAGS = -Iheaders -std=c++17

target:
	$(CXX) $(CXXFLAGS) main.cpp cpps/Package.cpp cpps/Container.cpp cpps/Heuristic.cpp -o exec_trabalho

clean:
	rm -f exec_trabalho relatorio_saida.txtCXX = g++

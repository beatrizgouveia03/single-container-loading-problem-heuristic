#ifndef HEURISTIC_H
#define HEURISTIC_H

#include "Container.h"
#include "Package.h"
#include <chrono>
#include <string>
#include <vector>

class GreedyHeuristic {
private:
  Container container;
  std::vector<Package> packages;
  bool verbose = true;

  double cavity_degree(int x, int y, int z, int dx, int dy, int dz);

public:
  GreedyHeuristic(Container cont, std::vector<Package> pkgs);
  // Liga/desliga o log detalhado (útil ao rodar um grupo inteiro).
  void set_verbose(bool value);
  void execute();
  // Contêiner preenchido pela última execução (caixas alocadas).
  const Container &get_container() const;
  void generate_resume(std::string archive = "relatorio_saida.txt");
};

#endif

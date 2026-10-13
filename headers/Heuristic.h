#ifndef HEURISTIC_H
#define HEURISTIC_H

#include "Container.h"
#include "Package.h"
#include <string>
#include <vector>

class GreedyHeuristic {
private:
  Container container;
  std::vector<Package> packages;

public:
  GreedyHeuristic(Container cont, std::vector<Package> pkgs);
  void execute();
  void generate_resume(std::string archive = "relatorio_saida.txt");
};

#endif

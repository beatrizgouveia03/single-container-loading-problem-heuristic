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

  double cavity_degree(const Package &pkg, int x, int y, int z, int dx, int dy,
                       int dz);

public:
  GreedyHeuristic(Container cont, std::vector<Package> pkgs);
  void execute();
  void generate_resume(std::string archive = "relatorio_saida.txt");
};

#endif

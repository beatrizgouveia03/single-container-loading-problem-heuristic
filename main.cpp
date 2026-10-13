#include "Container.h"
#include "Heuristic.h"
#include "Package.h"
#include <vector>

int main() {
  Container container(10, 10, 10);

  std::vector<Package> packages_list = {
      Package(1, 4, 4, 4), Package(2, 5, 5, 5), Package(3, 2, 2, 2)};

  GreedyHeuristic solver(container, packages_list);
  solver.execute();
  solver.generate_resume();

  return 0;
}

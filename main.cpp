#include "Container.h"
#include "Heuristic.h"
#include "Package.h"

int main() {
  Container container(15, 10, 10);

  std::vector<Package> packages_list = {
      Package(1, 4, 4, 4), Package(2, 5, 5, 5), Package(3, 2, 2, 2)};

  std::vector<Package> packages_2 = {
      Package(101, 6, 4, 3), Package(102, 3, 3, 3), Package(103, 5, 2, 4),
      Package(104, 4, 4, 4), Package(105, 2, 2, 2), Package(106, 5, 5, 5)};

  GreedyHeuristic solver(container, packages_2);
  solver.execute();
  solver.generate_resume();

  return 0;
}

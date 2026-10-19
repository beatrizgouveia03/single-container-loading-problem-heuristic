#ifndef CONTAINER_H
#define CONTAINER_H

#include "Package.h"
#include <tuple>
#include <vector>

class Container {
public:
  int c, l, a;
  std::vector<Package> allocated_packages;

  Container(int _c, int _l, int _a);
  bool space_available(Package &package, int x, int y, int z,
                       std::tuple<int, int, int> orientation);
};

#endif

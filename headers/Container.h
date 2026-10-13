#ifndef CONTAINER_H
#define CONTAINER_H

#include "Package.h"
#include <tuple>
#include <vector>

class Container {
public:
  double c, l, a;
  std::vector<Package> alocated_packages;

  Container(double _c, double _l, double _a);
  bool space_available(Package &package, double x, double y, double z,
                       std::tuple<double, double, double> orientation);
};

#endif

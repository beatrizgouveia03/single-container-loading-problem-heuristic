#include "Package.h"

Package::Package(int _id, double _c, double _l, double _a) {
  id = _id;
  c = _c;
  l = _l;
  a = _a;
  volume = c * l * a;
}

std::vector<std::tuple<double, double, double>>
Package::get_valid_orientations() {
  std::set<std::tuple<double, double, double>> orientations = {
      {c, l, a}, {c, a, l}, {l, c, a}, {l, a, c}, {a, c, l}, {a, l, c}};
  return std::vector<std::tuple<double, double, double>>(orientations.begin(),
                                                         orientations.end());
}

#include "Package.h"

Package::Package(int _id, int _c, int _l, int _a) {
  id = _id;
  c = _c;
  l = _l;
  a = _a;
  volume = c * l * a;
}

std::vector<std::tuple<int, int, int>> Package::get_valid_orientations() {
  std::set<std::tuple<int, int, int>> orientations = {
      {c, l, a}, {c, a, l}, {l, c, a}, {l, a, c}, {a, c, l}, {a, l, c}};
  return std::vector<std::tuple<int, int, int>>(orientations.begin(),
                                                orientations.end());
}

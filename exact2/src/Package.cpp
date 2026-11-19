#include "Package.h"
#include <set>

Package::Package(int _c, int _l, int _a) 
{
  c = _c;
  l = _l;
  a = _a;
  volume = c * l * a;
}

Package::Package() : c(0), l(0), a(0), position(0, 0, 0) {}

std::vector<std::tuple<int, int, int>> Package::get_valid_orientations() {
  std::set<std::tuple<int, int, int>> orientations = {
      {c, l, a}, {c, a, l}, {l, c, a}, {l, a, c}, {a, c, l}, {a, l, c}};
  return std::vector<std::tuple<int, int, int>>(orientations.begin(),
                                                orientations.end());
}


void Package::set_quantity(int _q)
{
  quantity = _q;
}

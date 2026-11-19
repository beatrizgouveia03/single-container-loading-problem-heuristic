#include "Package.h"
#include <set>
#include "Types.h"

Package::Package(ll _c, ll _l, ll _a) 
{
  c = _c;
  l = _l;
  a = _a;
  volume = c * l * a;
}

Package::Package() : c(0), l(0), a(0), position(0, 0, 0) {}

std::vector<std::tuple<ll, ll, ll>> Package::get_valid_orientations() {
  std::set<std::tuple<ll, ll, ll>> orientations = {
      {c, l, a}, {c, a, l}, {l, c, a}, {l, a, c}, {a, c, l}, {a, l, c}};
  return std::vector<std::tuple<ll, ll, ll>>(orientations.begin(),
                                                orientations.end());
}


void Package::set_quantity(ll _q)
{
  quantity = _q;
}

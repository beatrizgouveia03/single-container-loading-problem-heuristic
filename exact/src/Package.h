#ifndef PACKAGE_H
#define PACKAGE_H

#include <vector>
#include <tuple>
#include "Coordinates3d.h"
#include "Types.h"

using namespace std;

struct Package {
  ll id;
  ll c, l, a;
  ll volume;
  Coordinates3d position;
  ll ori_c = 0, ori_l = 0, ori_a = 0;
  bool allocated = false;
  ll quantity;

  Package(ll _c, ll _l, ll _a);
  Package();
  std::vector<std::tuple<ll, ll, ll>> get_valid_orientations();
  void set_quantity(ll _q);
};


#endif

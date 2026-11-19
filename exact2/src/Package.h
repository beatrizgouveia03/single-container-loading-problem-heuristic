#ifndef PACKAGE_H
#define PACKAGE_H

#include <vector>
#include <tuple>
#include "Coordinates3d.h"

using namespace std;

struct Package {
  int id;
  int c, l, a;
  int volume;
  Coordinates3d position;
  int ori_c = 0, ori_l = 0, ori_a = 0;
  bool allocated = false;
  int quantity;

  Package(int _c, int _l, int _a);
  Package();
  std::vector<std::tuple<int, int, int>> get_valid_orientations();
  void set_quantity(int _q);
};


#endif

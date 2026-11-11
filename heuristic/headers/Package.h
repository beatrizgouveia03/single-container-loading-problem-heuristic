#ifndef PACKAGE_H
#define PACKAGE_H

#include <set>
#include <tuple>
#include <vector>

struct Package {
  int id;
  int c, l, a;
  int volume;
  int pos_x = 0, pos_y = 0, pos_z = 0;
  int ori_c = 0, ori_l = 0, ori_a = 0;
  bool allocated = false;

  Package(int _id, int _c, int _l, int _a);
  std::vector<std::tuple<int, int, int>> get_valid_orientations();
};

#endif

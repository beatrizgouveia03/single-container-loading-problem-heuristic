#include "Container.h"

Container::Container(int _c, int _l, int _a) {
  c = _c;
  l = _l;
  a = _a;
}

bool Container::space_available(Package &package, int x, int y, int z,
                                std::tuple<int, int, int> orientation) {
  int oc = std::get<0>(orientation);
  int ol = std::get<1>(orientation);
  int oa = std::get<2>(orientation);

  if (x + oc > c || y + ol > l || z + oa > a) {
    return false;
  }

  for (const auto &pkg : allocated_packages) {
    int ix = pkg.pos_x, iy = pkg.pos_y, iz = pkg.pos_z;
    int ic = pkg.ori_c, il = pkg.ori_l, ia = pkg.ori_a;

    bool separated = (x + oc <= ix || x >= ix + ic || y + ol <= iy ||
                      y >= iy + il || z + oa <= iz || z >= iz + ia);
    if (!separated) {
      return false;
    }
  }
  return true;
}

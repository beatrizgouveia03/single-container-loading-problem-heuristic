#include "Container.h"

Container::Container(double _c, double _l, double _a) {
  c = _c;
  l = _l;
  a = _a;
}

bool Container::space_available(
    Package &package, double x, double y, double z,
    std::tuple<double, double, double> orientation) {
  double oc = std::get<0>(orientation);
  double ol = std::get<1>(orientation);
  double oa = std::get<2>(orientation);

  if (x + oc > c || y + ol > l || z + oa > a) {
    return false;
  }

  for (const auto &item : alocated_packages) {
    double ix = item.pos_x, iy = item.pos_y, iz = item.pos_z;
    double ic = item.ori_c, il = item.ori_l, ia = item.ori_a;

    bool separated = (x + oc <= ix || x >= ix + ic || y + ol <= iy ||
                      y >= iy + il || z + oa <= iz || z >= iz + ia);
    if (!separated) {
      return false;
    }
  }
  return true;
}

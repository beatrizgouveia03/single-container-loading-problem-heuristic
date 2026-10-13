#ifndef PACKAGE_H
#define PACKAGE_H

#include <set>
#include <tuple>
#include <vector>

struct Package {
    int id;
    double c, l, a;
    double volume;
    double pos_x = 0, pos_y = 0, pos_z = 0;
    double ori_c = 0, ori_l = 0, ori_a = 0;
    bool alocated = false;

    Package(int _id, double _c, double _l, double _a);
    std::vector<std::tuple<double, double, double>> get_valid_orientations();
};

#endif

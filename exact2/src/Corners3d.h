#pragma once

#include <vector>
#include "Package.h"
#include "Coordinates3d.h"
#include "Types.h"

using namespace std;

class Corners3d
{
public:
    public:
    static vector<Coordinates3d> getValidCorners(vector<Package> &insertedPackages,
                                                 vector<Package> &Packages,
                                                 ll W, ll H, ll D);

    static ll getEnvelopeVolume(const vector<Package>& insertedPackages, const vector<Package>& Packages, ll W, ll H, ll D);
};
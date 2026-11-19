#ifndef BRANCH_AND_BOUND
#define BRANCH_AND_BOUND

#include <vector>
#include "Corners3d.h"
#include "Package.h"
#include "Types.h"

class BranchAndBound
{
    vector<Package> best;
    void solve(vector<Package> &remainingPackages, ll W, ll H, ll D, ll &F, vector<Package> &insertedPackages);
};

#endif
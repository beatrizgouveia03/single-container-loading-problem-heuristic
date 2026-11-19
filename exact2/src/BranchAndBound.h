#ifndef BRANCH_AND_BOUND
#define BRANCH_AND_BOUND

#include <vector>
#include "Corners3d.h"
#include "Package.h"
#include "Types.h"

class BranchAndBound
{
    public:
    vector<Package> best;
    void solve(vector<Package> &remainingPackages, ll W, ll H, ll D, ll &F, vector<Package> &insertedPackages);
    void solveSorted(std::vector<Package> pkgs, ll W, ll H, ll D, ll &F,
                 std::vector<Package> &inserted);
};

#endif
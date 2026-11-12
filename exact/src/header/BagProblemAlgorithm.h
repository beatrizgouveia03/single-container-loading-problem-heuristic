#ifndef __BAGPROBLEMALGORITHM_H__
#define __BAGPROBLEMALGORITHM_H__

#include <vector>
#include <tuple>
#include "../../../heuristic/headers/Package.h"
#include "Utils.cpp"

class BagProblemAlgorithm
{
    public:
    /**
     * returns a subset that has the highest possible volume occupation without considering
     * spatial constraints
     */
    std::vector<int> get_maximum_subset(std::vector<Package> &package_list, int capacity);
    
    private:

    Path bag_problem_recur(
        std::vector<Package> &package_list, 
        std::vector<std::vector<Path>> &memo,
        Path &path,
        int capacity,
        int index
    );
};

#endif // __BAGPROBLEMALGORITHM_H__
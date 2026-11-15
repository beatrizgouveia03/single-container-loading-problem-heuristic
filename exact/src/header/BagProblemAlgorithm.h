#ifndef __BAGPROBLEMALGORITHM_H__
#define __BAGPROBLEMALGORITHM_H__

#include <vector>
#include "../../../heuristic/headers/Package.h"
#include "../Utils2.cpp"

struct Node {
    int volume;                        // best volume the knapsack found in this subproblem
    std::vector<int> limits;           // max quantity allowed per package type
    std::vector<int> candidate;        // knapsack answer: quantity chosen per type

    bool operator<(const Node& other) const {
        return volume < other.volume;  // makes priority_queue a max-heap by volume
    }
};

class BagProblemAlgorithm
{
    public:
    /**
     * returns a subset that has the highest possible volume occupation without considering
     * spatial constraints
     */
    static std::vector<int> get_maximum_subset(std::vector<Package> &package_list, int capacity);
    
    static Node solve_knapsack(std::vector<Package> packages, const std::vector<int>& limits, int capacity);
    private:

    static Path bag_problem_recur(
        std::vector<Package> &package_list, 
        std::vector<std::vector<Path>> &memo,
        Path &path,
        int capacity,
        int index
    );
};

#endif // __BAGPROBLEMALGORITHM_H__
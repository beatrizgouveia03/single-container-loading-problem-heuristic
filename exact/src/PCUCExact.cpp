#include "header/PCUCExact.h" 
#include "header/BagProblemAlgorithm.h"
#include "header/PackingChecker.h"
#include <queue>

std::vector<int> PCUCExact::solve_pcuc(std::vector<Package>& packages, int capacity, PackingChecker packingChecker)
{
    std::priority_queue<Node> queue;
    std::set<std::vector<int>> visited;   // avoids solving the same subproblem twice

    std::vector<int> initial_limits;
    for (auto& p : packages)
        initial_limits.push_back(p.quantity);

    queue.push(BagProblemAlgorithm::solve_knapsack(packages, initial_limits, capacity));
    visited.insert(initial_limits);

    while (!queue.empty()) {
        Node node = queue.top();
        queue.pop();

        if (packingChecker.fits_without_overlap(node.candidate, packages))   // part 2 (your packing check)
            return node.candidate;                  // first feasible = optimal

        // cut: one child for each type present in the candidate
        for (size_t k = 0; k < node.candidate.size(); k++) {
            if (node.candidate[k] == 0) continue;

            std::vector<int> new_limits = node.limits;
            new_limits[k] = node.candidate[k] - 1;

            if (visited.insert(new_limits).second)  // true only if it was not seen before
                queue.push(BagProblemAlgorithm::solve_knapsack(packages, new_limits, capacity));
        }
    }
    return {};  // should never happen (the empty selection always fits)
}

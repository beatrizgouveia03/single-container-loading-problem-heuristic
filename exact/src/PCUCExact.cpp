#include "header/PCUCExact.h"
#include "header/BagProblemAlgorithm.h"
#include "header/PackingChecker.h"
#include <queue>
#include <set>

std::vector<int> PCUCExact::solve_pcuc(std::vector<Package>& packages, int capacity, PackingChecker packingChecker)
{
    SolveReport report = solve(packages, capacity, packingChecker,
                               std::chrono::steady_clock::time_point::max());
    return report.selection;
}

SolveReport PCUCExact::solve(std::vector<Package>& packages, int capacity, PackingChecker& packingChecker,
                             std::chrono::steady_clock::time_point deadline)
{
    SolveReport report;
    const size_t types = packages.size();

    // ---------------------------------------------------------- incumbent
    // Constructive greedy (biggest box first, first free position). It costs a
    // few microseconds and it gives the branch-and-bound a lower bound to prune
    // with from the start -- and something to report if the budget runs out.
    std::vector<int> available(types, 0);
    for (size_t k = 0; k < types; k++) available[k] = packages[k].quantity;

    std::vector<int> best = packingChecker.greedy_packing(available, packages);
    long best_volume = 0;
    for (size_t k = 0; k < types; k++) best_volume += (long)best[k] * packages[k].volume;

    packingChecker.set_deadline(deadline);

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
        ++report.pops;

        // The queue is a max-heap by volume: nothing left can beat the
        // incumbent, so the search is over and the incumbent is optimal.
        if (node.volume <= best_volume) {
            ++report.pruned;
            continue;
        }

        ++report.checks;
        bool fits = packingChecker.fits_without_overlap(node.candidate, packages);

        if (packingChecker.aborted()) {
            // "gave up" is NOT "does not fit": stop instead of walking the
            // whole neighbourhood with no budget.
            ++report.aborted_checks;
            report.status = SolveStatus::OutOfTime;
            break;
        }

        if (fits) {                              // best-first: first feasible = optimal
            best = node.candidate;
            best_volume = node.volume;
            report.status = SolveStatus::Optimal;
            break;
        }

        // cut: one child for each type present in the candidate
        for (size_t k = 0; k < types; k++) {
            if (node.candidate[k] == 0) continue;

            std::vector<int> new_limits = node.limits;
            new_limits[k] = node.candidate[k] - 1;

            if (!visited.insert(new_limits).second)   // true only if it was not seen before
                continue;

            Node child = BagProblemAlgorithm::solve_knapsack(packages, new_limits, capacity);

            // Same candidate with tighter limits: that packing was just proven
            // infeasible, so the check would repeat and fail again.
            if (child.candidate == node.candidate || child.volume <= best_volume) {
                ++report.pruned;
                continue;
            }
            queue.push(child);
        }
    }

    report.selection = best;
    report.volume = best_volume;
    return report;
}

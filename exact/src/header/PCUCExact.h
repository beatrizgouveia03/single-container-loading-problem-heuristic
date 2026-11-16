#ifndef __PCUCEXACT_H__
#define __PCUCEXACT_H__
#include <chrono>
#include <vector>
#include "../../../heuristic/headers/Package.h"
#include "../header/PackingChecker.h"

// "Optimal" means the search was exhausted down to the answer, so the volume
// returned is the true optimum. "OutOfTime" means the budget ran out: the
// selection is the best one proven feasible so far, but a better one may exist.
enum class SolveStatus { Optimal, OutOfTime };

struct SolveReport {
    SolveStatus status = SolveStatus::Optimal;
    std::vector<int> selection;
    long volume = 0;          // volume of `selection`
    long checks = 0;          // packing checks performed
    long pops = 0;            // nodes taken from the queue
    long pruned = 0;          // nodes discarded without a check
    long aborted_checks = 0;  // checks that gave up

    bool proven() const { return status == SolveStatus::Optimal; }
};

class PCUCExact
{
    public:
    // Original entry point. Returns the optimal selection, or the best feasible
    // one found before the budget ran out; use solve() to tell them apart.
    std::vector<int> solve_pcuc(std::vector<Package>& packages, int capacity, PackingChecker packingChecker);

    // Same search, but the caller keeps the checker (deadline, counters) and
    // gets the status and the timing counters back.
    SolveReport solve(std::vector<Package>& packages, int capacity, PackingChecker& packingChecker,
                      std::chrono::steady_clock::time_point deadline =
                          std::chrono::steady_clock::time_point::max());
};

#endif // __PCUCEXACT_H__

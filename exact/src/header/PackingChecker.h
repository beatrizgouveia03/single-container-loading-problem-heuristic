#ifndef PACKING_CHECKER_H
#define PACKING_CHECKER_H

#include <chrono>
#include <cstddef>
#include <vector>
#include "../../../heuristic/headers/Package.h"

struct Box {
    int w, d, h;     // width (x), depth/length (y), height (z)
    int type;        // package type index
};

struct Placement {
    int x, y, z;
};

class PackingChecker {
public:
    PackingChecker(int L, int D, int H);

    // candidate[k] = how many boxes of type k (dimensions come from packages[k]).
    // Returns false both for "does not fit" and for "gave up": call aborted() to
    // tell them apart. An aborted check says nothing about feasibility.
    bool fits_without_overlap(const std::vector<int>& candidate,
                              const std::vector<Package>& packages);

    // Constructive greedy: biggest boxes first, each one at the first position
    // where it does not collide. No backtracking, so it is cheap and the result
    // is a packing by construction. Returns how many boxes of each type it
    // placed; boxes()/placements_of_boxes() then describe that packing.
    std::vector<int> greedy_packing(const std::vector<int>& available,
                                    const std::vector<Package>& packages);

    // Maximum number of backtracking calls per check (0 = unlimited, the default).
    void set_node_limit(long limit) { node_limit_ = limit; }

    // Give up when this instant is reached (checked every 64 nodes).
    void set_deadline(std::chrono::steady_clock::time_point deadline);
    void clear_deadline() { has_deadline_ = false; }

    // true if the LAST check stopped because it hit the node limit or the deadline
    bool aborted() const { return aborted_; }

    // Counters for the last check.
    long nodes() const { return nodes_; }
    long placements() const { return placements_; }

    // Last check: the boxes in search order and where each one went.
    const std::vector<Box>& boxes() const { return boxes_; }
    const std::vector<Placement>& placements_of_boxes() const { return placed_; }

private:
    int L_, D_, H_;
    std::vector<Box> boxes_;
    std::vector<Placement> placed_;
    std::vector<int> xs_, ys_, zs_;   // candidate coordinates per axis
    long node_limit_ = 0;
    long nodes_ = 0;
    long placements_ = 0;
    bool aborted_ = false;
    bool has_deadline_ = false;
    std::chrono::steady_clock::time_point deadline_;

    static std::vector<int> subset_sums(const std::vector<Box>& boxes, int axis, int limit);
    static bool overlaps(const Box& a, const Placement& pa, const Box& b, const Placement& pb);
    bool budget_exhausted();
    bool backtrack(size_t index);
};

#endif

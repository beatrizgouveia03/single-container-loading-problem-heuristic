
#ifndef PACKING_CHECKER_H
#define PACKING_CHECKER_H
 
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
    // Returns false both for "does not fit" and for "gave up"; call aborted() to tell them apart.
    bool fits_without_overlap(const std::vector<int>& candidate,
                              const std::vector<Package>& packages);
 
    // Maximum number of backtracking calls per check (0 = unlimited, the default).
    void set_node_limit(long limit) { node_limit_ = limit; }
 
    // true if the LAST check stopped because it hit the node limit (answer unknown)
    bool aborted() const { return aborted_; }
 
private:
    int L_, D_, H_;
    std::vector<Box> boxes_;
    std::vector<Placement> placed_;
    std::vector<int> xs_, ys_, zs_;   // candidate coordinates per axis
    long node_limit_ = 0;
    long nodes_ = 0;
    bool aborted_ = false;
 
    static std::vector<int> subset_sums(const std::vector<Box>& boxes, int axis, int limit);
    static bool overlaps(const Box& a, const Placement& pa, const Box& b, const Placement& pb);
    bool backtrack(size_t index);
};
 
#endif
 
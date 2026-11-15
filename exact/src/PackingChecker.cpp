#include "header/PackingChecker.h"
#include <algorithm>
#include <tuple>

PackingChecker::PackingChecker(int L, int D, int H) : L_(L), D_(D), H_(H) {}

bool PackingChecker::fits_without_overlap(const std::vector<int>& candidate,
                                          const std::vector<Package>& packages)
{
    boxes_.clear();
    for (size_t k = 0; k < candidate.size(); k++) {
        const Package& package = packages[k];
        for (int i = 0; i < candidate[k]; i++) {
            boxes_.push_back({
                package.c,     // x axis
                package.l,     // y axis
                package.a,     // z axis
                (int)k
            });
        }
    }

    // biggest first: fails earlier. Same type stays adjacent (needed for symmetry rule)
    std::sort(boxes_.begin(), boxes_.end(), [](const Box& a, const Box& b) {
        long va = (long)a.w * a.d * a.h, vb = (long)b.w * b.d * b.h;
        if (va != vb) return va > vb;
        return a.type < b.type;
    });

    // quick rejection: any box bigger than the container
    for (const Box& b : boxes_)
        if (b.w > L_ || b.d > D_ || b.h > H_) return false;

    xs_ = subset_sums(boxes_, 0, L_);
    ys_ = subset_sums(boxes_, 1, D_);
    zs_ = subset_sums(boxes_, 2, H_);

    placed_.assign(boxes_.size(), {0, 0, 0});
    return backtrack(0);
}

std::vector<int> PackingChecker::subset_sums(const std::vector<Box>& boxes, int axis, int limit)
{
    std::vector<bool> reachable(limit + 1, false);
    reachable[0] = true;
    for (const Box& b : boxes) {
        int size = (axis == 0) ? b.w : (axis == 1) ? b.d : b.h;
        for (int s = limit - size; s >= 0; s--)       // backwards: each box used once
            if (reachable[s] && s + size <= limit) reachable[s + size] = true;
    }
    std::vector<int> result;
    for (int s = 0; s <= limit; s++)
        if (reachable[s]) result.push_back(s);
    return result;
}

bool PackingChecker::overlaps(const Box& a, const Placement& pa, const Box& b, const Placement& pb)
{
    return pa.x < pb.x + b.w && pb.x < pa.x + a.w &&
           pa.y < pb.y + b.d && pb.y < pa.y + a.d &&
           pa.z < pb.z + b.h && pb.z < pa.z + a.h;
}

bool PackingChecker::backtrack(size_t index)
{
    if (index == boxes_.size()) return true;           // everything placed

    const Box& box = boxes_[index];

    // symmetry: same type as previous box -> position must be >= previous one
    bool same_as_previous = index > 0 && boxes_[index - 1].type == box.type;

    for (int z : zs_) {
        if (z + box.h > H_) break;                     // sorted ascending, can stop
        for (int y : ys_) {
            if (y + box.d > D_) break;
            for (int x : xs_) {
                if (x + box.w > L_) break;

                if (same_as_previous) {
                    const Placement& prev = placed_[index - 1];
                    if (std::make_tuple(z, y, x) < std::make_tuple(prev.z, prev.y, prev.x))
                        continue;
                }

                Placement p{x, y, z};
                bool collision = false;
                for (size_t j = 0; j < index; j++) {
                    if (overlaps(box, p, boxes_[j], placed_[j])) { collision = true; break; }
                }
                if (collision) continue;

                placed_[index] = p;
                if (backtrack(index + 1)) return true;
            }
        }
    }
    return false;                                      // no position works: go back
}
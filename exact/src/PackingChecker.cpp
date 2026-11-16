#include "header/PackingChecker.h"

#include <algorithm>
#include <tuple>

PackingChecker::PackingChecker(int L, int D, int H) : L_(L), D_(D), H_(H) {}

void PackingChecker::set_deadline(std::chrono::steady_clock::time_point deadline)
{
    if (deadline == std::chrono::steady_clock::time_point::max()) {
        has_deadline_ = false;
        return;
    }
    deadline_ = deadline;
    has_deadline_ = true;
}

bool PackingChecker::fits_without_overlap(const std::vector<int>& candidate,
                                          const std::vector<Package>& packages)
{
    aborted_ = false;
    nodes_ = 0;
    placements_ = 0;
    boxes_.clear();

    long total_volume = 0;
    for (size_t k = 0; k < candidate.size(); k++) {
        const Package& package = packages[k];
        for (int i = 0; i < candidate[k]; i++) {
            boxes_.push_back({package.c, package.l, package.a, (int)k});
            total_volume += (long)package.c * package.l * package.a;
        }
    }

    // cheap rejections, no search needed
    if (total_volume > (long)L_ * D_ * H_) return false;
    for (const Box& b : boxes_)
        if (b.w > L_ || b.d > D_ || b.h > H_) return false;

    // biggest first: fails earlier. Same type stays adjacent (needed for symmetry rule)
    std::sort(boxes_.begin(), boxes_.end(), [](const Box& a, const Box& b) {
        long va = (long)a.w * a.d * a.h, vb = (long)b.w * b.d * b.h;
        if (va != vb) return va > vb;
        return a.type < b.type;
    });

    xs_ = subset_sums(boxes_, 0, L_);
    ys_ = subset_sums(boxes_, 1, D_);
    zs_ = subset_sums(boxes_, 2, H_);

    placed_.assign(boxes_.size(), {0, 0, 0});
    return backtrack(0);
}

std::vector<int> PackingChecker::greedy_packing(const std::vector<int>& available,
                                               const std::vector<Package>& packages)
{
    std::vector<int> chosen(available.size(), 0);

    std::vector<Box> items;
    for (size_t k = 0; k < available.size() && k < packages.size(); k++) {
        const Package& p = packages[k];
        if (p.c > L_ || p.l > D_ || p.a > H_) continue;   // can never fit
        for (int i = 0; i < available[k]; i++)
            items.push_back({p.c, p.l, p.a, (int)k});
    }
    std::sort(items.begin(), items.end(), [](const Box& a, const Box& b) {
        return (long)a.w * a.d * a.h > (long)b.w * b.d * b.h;
    });

    boxes_.clear();
    placed_.clear();

    for (const Box& box : items) {
        bool done = false;
        for (int z = 0; z + box.h <= H_ && !done; ++z) {
            for (int y = 0; y + box.d <= D_ && !done; ++y) {
                for (int x = 0; x + box.w <= L_; ++x) {
                    Placement p{x, y, z};
                    bool collision = false;
                    for (size_t j = 0; j < boxes_.size(); j++) {
                        if (overlaps(box, p, boxes_[j], placed_[j])) { collision = true; break; }
                    }
                    if (collision) continue;
                    boxes_.push_back(box);
                    placed_.push_back(p);
                    ++chosen[box.type];
                    done = true;
                    break;
                }
            }
        }
    }
    return chosen;
}

std::vector<int> PackingChecker::subset_sums(const std::vector<Box>& boxes, int axis, int limit)
{
    std::vector<bool> reachable(limit + 1, false);
    reachable[0] = true;
    for (const Box& b : boxes) {
        int size = (axis == 0) ? b.w : (axis == 1) ? b.d : b.h;
        for (int s = limit - size; s >= 0; s--)       // backwards: each box used once
            if (reachable[s]) reachable[s + size] = true;
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

bool PackingChecker::budget_exhausted()
{
    if (node_limit_ > 0 && nodes_ > node_limit_) {
        aborted_ = true;
        return true;
    }
    if (has_deadline_ && (nodes_ & 63) == 0 &&
        std::chrono::steady_clock::now() >= deadline_) {
        aborted_ = true;
        return true;
    }
    return false;
}

bool PackingChecker::backtrack(size_t index)
{
    if (index == boxes_.size()) return true;               // everything placed

    // search budget: give up instead of running forever (result is then inconclusive)
    ++nodes_;
    if (budget_exhausted()) return false;

    const Box& box = boxes_[index];
    bool same_as_previous = index > 0 && boxes_[index - 1].type == box.type;

    for (int z : zs_) {
        if (z + box.h > H_) break;
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
                ++placements_;
                if (backtrack(index + 1)) return true;
                if (aborted_) return false;                // unwind immediately
            }
        }
    }
    return false;
}

// Tests for the exact PCUC solver (knapsack + cuts + packing check).
//
// Build example (adjust paths/sources to your project):
//   g++ -std=c++17 -O2 -I. test_pcuc_exact.cpp \
//       exact/src/PCUCExact.cpp exact/src/BagProblemAlgorithm.cpp \
//       heuristic/src/Package.cpp -o test_pcuc_exact
//   ./test_pcuc_exact
//
// Strategy:
//   1. PackingChecker is tested alone with hand-made cases (known answers).
//   2. PCUCExact is tested with hand-made cases (known optimum).
//   3. PCUCExact is compared against a brute-force oracle on many small
//      random instances: try EVERY quantity vector, keep the best volume that
//      the PackingChecker accepts. The exact solver must match that volume.

#include <array>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "../src/header/PCUCExact.h"
#include "../src/header/PackingChecker.h"   // same include style used in PCUCExact.cpp

// ---------------------------------------------------------------- mini framework

static int g_checks = 0;
static int g_failures = 0;

#define CHECK(cond, msg)                                                     \
    do {                                                                     \
        g_checks++;                                                          \
        if (!(cond)) {                                                       \
            g_failures++;                                                    \
            std::cerr << "  [FAIL] " << (msg) << "  (line " << __LINE__      \
                      << ")\n";                                              \
        }                                                                    \
    } while (0)

// ---------------------------------------------------------------- helpers

// each entry: {c, l, a, quantity}
static std::vector<Package> make_packages(const std::vector<std::array<int, 4>>& specs) {
    std::vector<Package> packages;
    for (size_t k = 0; k < specs.size(); k++)
        packages.push_back(Package((int)k, specs[k][0], specs[k][1], specs[k][2], specs[k][3]));
    return packages;
}

static long volume_of(const std::vector<Package>& packages, const std::vector<int>& selection) {
    long total = 0;
    for (size_t k = 0; k < selection.size(); k++)
        total += (long)selection[k] * packages[k].volume;
    return total;
}

static bool fits(int L, int D, int H, const std::vector<Package>& packages,
                 const std::vector<int>& selection) {
    PackingChecker checker(L, D, H);
    return checker.fits_without_overlap(selection, packages);
}

// Oracle: best volume among ALL quantity vectors that the checker accepts.
static void enumerate(int L, int D, int H, const std::vector<Package>& packages, size_t k,
                      std::vector<int>& current, long volume, long capacity, long& best) {
    if (volume > capacity) return;
    if (k == packages.size()) {
        if (volume > best && fits(L, D, H, packages, current)) best = volume;
        return;
    }
    for (int q = 0; q <= packages[k].quantity; q++) {
        current[k] = q;
        enumerate(L, D, H, packages, k + 1, current,
                  volume + (long)q * packages[k].volume, capacity, best);
    }
    current[k] = 0;
}

static long brute_force_best(int L, int D, int H, const std::vector<Package>& packages) {
    std::vector<int> current(packages.size(), 0);
    long best = 0;   // empty selection always fits
    enumerate(L, D, H, packages, 0, current, 0, (long)L * D * H, best);
    return best;
}

// Runs the exact solver and validates everything about its answer.
// Returns the volume of the returned selection (or -1 if the answer is malformed).
static long run_solver(int L, int D, int H, std::vector<Package>& packages,
                       const std::string& label, std::vector<int>* out = nullptr) {
    PCUCExact solver;
    PackingChecker checker(L, D, H);
    std::vector<int> result = solver.solve_pcuc(packages, L * D * H, checker);

    if (result.size() != packages.size()) {
        CHECK(false, label + ": result has wrong size");
        return -1;
    }
    for (size_t k = 0; k < result.size(); k++)
        CHECK(result[k] >= 0 && result[k] <= packages[k].quantity,
              label + ": quantity out of bounds for type " + std::to_string(k));

    CHECK(volume_of(packages, result) <= (long)L * D * H, label + ": volume exceeds container");
    CHECK(fits(L, D, H, packages, result), label + ": returned selection does not actually fit");

    if (out) *out = result;
    return volume_of(packages, result);
}

// ---------------------------------------------------------------- tests: PackingChecker

static void test_packing_checker() {
    std::cout << "PackingChecker (known answers)\n";

    // empty selection always fits
    {
        auto p = make_packages({{3, 3, 3, 2}});
        CHECK(fits(10, 10, 10, p, {0}), "empty selection fits");
    }
    // one box exactly the container
    {
        auto p = make_packages({{10, 10, 10, 1}});
        CHECK(fits(10, 10, 10, p, {1}), "box == container fits");
    }
    // box bigger than container in one dimension
    {
        auto p = make_packages({{11, 1, 1, 1}});
        CHECK(!fits(10, 10, 10, p, {1}), "box longer than container does not fit");
    }
    // two half-containers fit, three do not
    {
        auto p = make_packages({{5, 10, 10, 3}});
        CHECK(fits(10, 10, 10, p, {2}), "two halves fit");
        CHECK(!fits(10, 10, 10, p, {3}), "three halves do not fit (volume)");
    }
    // volume is fine (432 < 1000) but geometry says no
    {
        auto p = make_packages({{6, 6, 6, 2}});
        CHECK(fits(10, 10, 10, p, {1}), "one 6x6x6 fits");
        CHECK(!fits(10, 10, 10, p, {2}), "two 6x6x6 do not fit despite volume");
    }
    // perfect fill with identical cubes
    {
        auto p = make_packages({{5, 5, 5, 8}});
        CHECK(fits(10, 10, 10, p, {8}), "eight 5-cubes fill 10-cube exactly");
    }
    // perfect fill with mixed boxes
    {
        auto p = make_packages({{10, 10, 5, 1}, {5, 10, 5, 2}});
        CHECK(fits(10, 10, 10, p, {1, 2}), "slab + two half-slabs fill the container");
    }
    // tall boxes cannot sit on a shorter slab
    {
        auto p = make_packages({{10, 10, 5, 1}, {5, 5, 10, 2}});
        CHECK(!fits(10, 10, 10, p, {1, 2}), "slab + two tall posts do not fit");
        CHECK(fits(10, 10, 10, p, {0, 2}), "two tall posts alone fit");
    }
    // checker can be reused for different selections
    {
        PackingChecker checker(10, 10, 10);
        auto p = make_packages({{6, 6, 6, 2}});
        CHECK(!checker.fits_without_overlap({2}, p), "reuse: first call (does not fit)");
        CHECK(checker.fits_without_overlap({1}, p), "reuse: second call (fits)");
    }
}

// ---------------------------------------------------------------- tests: PCUCExact (hand-made)

static void test_pcuc_known() {
    std::cout << "PCUCExact (known optimum)\n";

    // knapsack would pick 2 (432 <= 1000) but only 1 fits
    {
        auto p = make_packages({{6, 6, 6, 2}});
        std::vector<int> r;
        run_solver(10, 10, 10, p, "two 6-cubes", &r);
        CHECK(r == std::vector<int>({1}), "two 6-cubes -> answer is exactly 1 box");
    }
    // 9 available, only 8 fit
    {
        auto p = make_packages({{5, 5, 5, 9}});
        std::vector<int> r;
        run_solver(10, 10, 10, p, "nine 5-cubes", &r);
        CHECK(r == std::vector<int>({8}), "nine 5-cubes -> answer is 8");
    }
    // everything fits: answer is the full quantity
    {
        auto p = make_packages({{2, 2, 2, 3}, {1, 1, 1, 4}});
        std::vector<int> r;
        run_solver(10, 10, 10, p, "everything fits", &r);
        CHECK(r == std::vector<int>({3, 4}), "everything fits -> take everything");
    }
    // slab + posts: best is 4 posts (volume 1000); slab + 2 posts is infeasible
    {
        auto p = make_packages({{10, 10, 5, 1}, {5, 5, 10, 4}});
        long volume = run_solver(10, 10, 10, p, "slab and posts");
        CHECK(volume == 1000, "slab and posts -> optimum volume is 1000");
    }
    // nothing fits at all (every box bigger than the container)
    {
        auto p = make_packages({{11, 11, 11, 3}});
        std::vector<int> r;
        run_solver(10, 10, 10, p, "nothing fits", &r);
        CHECK(r == std::vector<int>({0}), "nothing fits -> empty selection");
    }
}

// ---------------------------------------------------------------- tests: PCUCExact vs oracle

static void test_pcuc_random(int iterations, unsigned seed) {
    std::cout << "PCUCExact vs brute force (" << iterations << " random instances, seed "
              << seed << ")\n";

    std::mt19937 rng(seed);
    auto rnd = [&](int lo, int hi) { return std::uniform_int_distribution<int>(lo, hi)(rng); };

    for (int it = 0; it < iterations; it++) {
        int L = rnd(3, 6), D = rnd(3, 6), H = rnd(3, 6);
        int types = rnd(2, 4);

        std::vector<std::array<int, 4>> specs;
        for (int t = 0; t < types; t++)
            specs.push_back({rnd(1, 5), rnd(1, 5), rnd(1, 5), rnd(1, 3)});
        auto packages = make_packages(specs);

        std::string label = "random #" + std::to_string(it) + " (container " +
                            std::to_string(L) + "x" + std::to_string(D) + "x" +
                            std::to_string(H) + ")";

        long expected = brute_force_best(L, D, H, packages);
        long got = run_solver(L, D, H, packages, label);

        CHECK(got == expected,
              label + ": solver volume " + std::to_string(got) +
                  " != brute force " + std::to_string(expected));
    }
}

// ---------------------------------------------------------------- main

int main(int argc, char** argv) {
    int iterations = 200;
    if (argc > 1) iterations = std::atoi(argv[1]);

    test_packing_checker();
    test_pcuc_known();
    test_pcuc_random(iterations, 12345);

    std::cout << "\n" << (g_checks - g_failures) << "/" << g_checks << " checks passed\n";
    return g_failures == 0 ? 0 : 1;
}

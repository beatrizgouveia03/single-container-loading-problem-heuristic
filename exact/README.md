# Single Container Loading Problem — Exact Algorithm

An **exact** solver for the Single Container Loading Problem (PCumC): choose how
many boxes of each type to load into one container so that the loaded volume is
maximal and the boxes can actually be placed without overlapping.

The method is a **best-first branch-and-bound over knapsack subproblems**: every
search node is a vector of per-type limits, its bound is the *unconstrained*
knapsack optimum for those limits, and a node is expanded by lowering one limit
when its knapsack answer does not fit geometrically. Because nodes are processed
from the largest bound downwards, the first candidate that passes the packing
check is a global optimum.

The search accepts a **wall-clock budget**. Without one it always proves the
optimum; with one it returns `OutOfTime` and the best packing it found so far
(started from a greedy construction), so an instance that is too hard still
produces a usable answer and an optimality gap.

The folder also ships a **stress test** that generates random instances in three
difficulty scenarios, solves each one in an isolated child process (wall-clock
and memory limits), and writes a CSV plus Typst tables ready to paste into the
report.

---

## Table of Contents

- [Requirements](#requirements)
- [Building](#building)
- [Tests](#tests)
- [How the algorithm works](#how-the-algorithm-works)
  - [1. Upper bound: bounded knapsack](#1-upper-bound-bounded-knapsack)
  - [2. Feasibility: packing check](#2-feasibility-packing-check)
  - [3. Branch-and-bound](#3-branch-and-bound)
  - [4. Termination and correctness](#4-termination-and-correctness)
- [Instances and groups](#instances-and-groups)
- [Stress test](#stress-test)
  - [Scenarios](#scenarios)
  - [Statuses](#statuses)
  - [CSV output](#csv-output)
  - [Typst output](#typst-output)
  - [Reproducing a run](#reproducing-a-run)
- [Cost and measured behaviour](#cost-and-measured-behaviour)
- [Known limitations](#known-limitations)
- [Directory map](#directory-map)

---

## Requirements

| Tool | Tested version | Needed for |
| --- | --- | --- |
| `g++` (any C++17 compiler) | 11.4.0 | building the project |
| `make` | 4.3 | build and helper targets |

There are no external dependencies: the JSON parser
([nlohmann/json](https://github.com/nlohmann/json)) is vendored in `lib/json.hpp`.
The container, package and instance-generation code is shared with the
heuristic (`../heuristic/cpps`, `../heuristic/headers`), which the Makefile
compiles together with `src/`.

## Building

```bash
make          # builds every object file into build/
make test     # builds and runs every test executable (see the warning below)
```

The build uses `-O2 -std=c++17 -Wall -Wextra`; the scenarios in `StressTest`
were calibrated with `-O2`, and dropping the flag makes everything several times
slower. To reproduce the unoptimized numbers, pass `CXXFLAGS` explicitly:

```bash
make clean && make CXXFLAGS="-Iheaders -std=c++17 -Wall -Wextra -MMD -MP"
```

`make` alone does not produce a command-line program: `main.cpp` is an empty
placeholder and each *test file* becomes its own executable under `build/test/`.
To build a single one:

```bash
make build/test/PCUCExactTest
./build/test/PCUCExactTest
```

`make clean` removes `build/` (it does not remove `stress_instances/`,
`stress_results.csv` or `stress_summary.typ`).

## Tests

| Executable | What it checks |
| --- | --- |
| `build/test/PCUCExactTest [iterations]` | four blocks: hand-made `PackingChecker` cases (a box equal to the container, a box longer than the container, two half-containers that fit and three that do not, two 6×6×6 cubes in a 10-cube, eight 5-cubes filling a 10-cube, slab + posts, reuse of a checker instance), the **differential check** described below (200 random cases, `seed 999`), hand-made `PCUCExact` optima, and a **brute-force oracle**: it enumerates *every* quantity vector whose volume fits the container, keeps the best one the packing checker accepts, and requires the solver to match it. Default: 200 random instances (`seed 12345`), containers 3..6 per side, 2..4 types, quantities 1..3. |
| `build/test/BagProblemAlgorithmTest` | generates a 5-instance group (10 types, dimensions 1..6, 4..10 units per type, 15×10×10 container) and prints the knapsack selection and occupied volume of each one. It is a smoke test, not an assertion suite. |
| `build/test/StressTest [args]` | the stress test described below. |

The differential check (`test_checker_vs_reference`) keeps an independent,
dumb reference checker in the test file — every box at every position, comparing
against every placed box — and requires the fast checker to return the same
verdict on random small cases. Every accepted check is also validated: the
placements it reports must be inside the container and pairwise disjoint.

Example:

```console
$ ./build/test/PCUCExactTest
PackingChecker (known answers)
PackingChecker vs exhaustive reference (200 random cases, seed 999)
PCUCExact (known optimum)
PCUCExact vs brute force (200 random instances, seed 12345)

1792/1792 checks passed
```

> **Warning:** `make test` runs *every* test binary, including `StressTest`
> with its defaults (1 instance per scenario and the per-instance timeout set in
> `main()`), so a hard instance makes the suite sit there for minutes. Prefer
> running the unit test binaries directly, or call `StressTest` with an explicit
> small timeout.

## How the algorithm works

Let `q_k` be the number of available boxes of type `k` and `C` the container
volume. A solution is a vector `x` with `0 ≤ x_k ≤ q_k`.

### 1. Upper bound: bounded knapsack

`BagProblemAlgorithm::solve_knapsack(packages, limits, capacity)` ignores the
geometry and returns the volume-maximal vector `x` with `x_k ≤ limits[k]` and
`Σ x_k·vol_k ≤ C`, together with that volume. It is a classic dynamic program
over `(type, remaining capacity)` memoized in a table of `Path`
(`src/Utils2.cpp`), and it is the source of the **upper bound** of a node: no
packing inside those limits can beat the knapsack volume.

### 2. Feasibility: packing check

`PackingChecker::fits_without_overlap(candidate, packages)` decides whether the
boxes of `candidate` can be placed in the container (boxes keep their `c × l × a`
orientation; the heuristic is the part of the project that rotates boxes).

1. Cheap rejections: total volume above the container, or any box bigger than
   the container along an axis.
2. Boxes are sorted by volume, biggest first, so failures happen early and
   identical boxes stay adjacent.
3. Candidate coordinates per axis are the **subset sums** of the box sizes along
   that axis (`subset_sums`), not every integer coordinate: any packing can be
   pushed against a wall or another box, so a coordinate is always a sum of box
   sizes.
4. `backtrack` places the boxes in order, trying the three nested loops
   `z ∈ zs`, `y ∈ ys`, `x ∈ xs` (skipping the ones that overflow the container),
   rejecting positions that overlap an already placed box. Boxes of the same
   type are only tried in **non-decreasing position** relative to the previous
   one, which removes permutations of identical boxes.
5. `set_node_limit(n)` and `set_deadline(t)` cap the search: `n` backtracking
   calls, and the wall-clock instant `t` (checked every 64 nodes). When either is
   reached the search unwinds immediately and `aborted()` becomes true, which is
   **not** the same as "does not fit" — the answer is unknown.
6. `greedy_packing(available, packages)` uses the same collision test without
   backtracking to build a feasible packing in microseconds; the solver uses it
   for its first incumbent.

A `true` means a full packing was found; a `false` after the space is exhausted
is a proof that the candidate is infeasible, and that proof is what makes the
branch-and-bound exact.

### 3. Branch-and-bound

`PCUCExact::solve(packages, capacity, checker, deadline)`:

```
best ← greedy_packing()                # incumbent, feasible by construction
limits ← (q_1, …, q_n)
queue  ← { knapsack(limits) }          # max-heap by volume
visited ← { limits }

while queue not empty:
    node ← queue.pop()                 # largest bound first
    if node.volume ≤ volume(best): break # nothing left can beat the incumbent
    if fits(node.candidate): best ← node.candidate; break
    if aborted(): return OutOfTime with `best`   # gave up, not "does not fit"
    for each type k with node.candidate[k] > 0:
        child.limits[k] ← node.candidate[k] − 1
        if child.limits ∉ visited:
            visited.insert(child.limits)
            child ← knapsack(child.limits)
            if child.candidate ≠ node.candidate and child.volume > volume(best):
                queue.push(child)
```

`solve` returns a `SolveReport` with the status (`Optimal` or `OutOfTime`), the
selection, its volume and the counters. `solve_pcuc` is the old entry point and
just returns `report.selection`, so an out-of-time run hands back the best
packing found instead of nothing.

The branching rule comes from the monotonicity of the packing: if a candidate
`x` does not fit, no vector `y ≥ x` fits either (it contains the same boxes plus
more), so every feasible solution has at least one type `k` with
`y_k ≤ x_k − 1`. The children cover that whole space, and the `visited` set
avoids solving the same limit vector twice.

### 4. Termination and correctness

The queue is ordered by the knapsack bound, and a child's bound never exceeds
its parent's (it is the same knapsack with tighter limits). Therefore the first
feasible candidate popped is optimal, and so is the incumbent at the moment the
queue is exhausted with every remaining bound at or below it. Without a budget
the search always ends: the empty selection always fits, limits decrease at every
branch, and `visited` is finite.

With a budget the search can stop early. Then the answer is still a real packing
(the greedy incumbent or a candidate already proven feasible), but nothing says
it is the best one: `SolveStatus::OutOfTime` and the gap between the incumbent
and the bound of the node that was being examined are what the stress test
reports for those instances.

## Instances and groups

`InstanceManagerExact` extends the heuristic's `InstanceManager`. Instances are
JSON files with the container and the box dimensions:

```json
{
    "container": [20, 20, 20],
    "packages": [[4, 4, 6], [10, 7, 5]],
    "tipos": [
        { "dimensoes": [4, 4, 6], "quantidade": 1 },
        { "dimensoes": [10, 7, 5], "quantidade": 3 }
    ]
}
```

`generateGroup(name, count, types, minDim, maxDim, minQty, maxQty, C, L, A)`
writes `<folder>/<name>/instance_<i>.json` plus a `grupo.json` with the
generation parameters, and returns the instances in memory (`StressTest` passes
`stress_instances` as the folder). Each type gets
random dimensions in `[minDim, maxDim]` and a random quantity in
`[minQty, maxQty]`; the container is always `C × L × A` (unlike the heuristic's
manager, the exact variant does **not** scale the container per iteration). An
existing folder is never overwritten.

Two things worth knowing when reading instances back:

- `readInstance` fills `container` and the flat `packages` list but **not**
  `packages_exact`, which is where the per-type quantities live. The solver and
  the stress test work on the in-memory instances returned by `generateGroup`.
- `tipos` and the generator parameters are metadata; the reader ignores them.

## Stress test

```bash
./build/test/StressTest [timeout_s] [instances_per_scenario] [mem_limit_mb] [node_limit] [scenario]
```

| Argument | Default | Meaning |
| --- | --- | --- |
| `timeout_s` | `3600` | wall-clock limit **per instance** (set in `main()`; the usage comment at the top of the file still says 30, so pass it explicitly when the number matters) |
| `instances_per_scenario` | `1` | instances generated for each of the three scenarios |
| `mem_limit_mb` | `4096` | `RLIMIT_AS` of the child process (`0` disables it) |
| `node_limit` | `50000000` | backtracking budget of *each* packing check (`0` disables it) |
| `scenario` | all | run a single scenario: `fraco`, `medio` or `forte` |

Each instance runs in a `fork`ed child so the parent can enforce the limits
without instrumenting the solver. The child sets `RLIMIT_AS`, sends the first
knapsack bound through a pipe, calls `PCUCExact::solve` with a deadline 0.1 s
before `timeout_s` (so it stops itself and the answer still comes out), verifies
that the returned selection is really a packing, and sends the report. The
parent still kills the child with `SIGKILL` when the timeout expires, which is
now only a backstop.

### Scenarios

| Scenario | Container | Types | Box dimensions | Units per type |
| --- | --- | --- | --- | --- |
| `fraco` | 10 × 10 × 10 | 5 | 1..10 | 1..6 |
| `medio` | 15 × 10 × 10 | 10 | 2..6 | 1..2 |
| `forte` | 20 × 20 × 20 | 12 | 3..10 | 1..3 |

The table above is the current content of `default_scenarios()`. The three
levels are calibrated by measurement (so that each one stresses a different
part of the search), and they do change as the scenarios are retuned — read the
function when you need the exact parameters of a run.

Dimensions never exceed the smallest container side, so no box is individually
larger than the container.

### Statuses

| Status | Meaning |
| --- | --- |
| `resolvida` | the search proved the volume optimal (`SolveStatus::Optimal`) |
| `limite` | the budget ran out (`SolveStatus::OutOfTime`): the volume shown is the best packing found, not proven optimal |
| `timeout` | the child was killed by the parent before it could report — should not happen, since the solver stops 0.1 s earlier |
| `memoria` | the child hit `RLIMIT_AS` (`std::bad_alloc`, exit code 2) |
| `falha` | any other exit (segfault, OOM killer, unexpected exception) |

### CSV output

`stress_results.csv` is written in the current directory, one row per instance,
flushed after each row so an interrupted run keeps its data:

```csv
scenario,instance,container,types,boxes_available,offered_volume_pct,status,time_s,bound_volume,optimal_volume,fill_pct,gap_pct,boxes_packed,instance_file
medio,1,15x10x10,10,13,52.1,resolvida,0.0042,782,782,52.13,0.000,13,stress_instances/stress_medio_<timestamp>/instance_1.json
```

- `offered_volume_pct`: volume of every available box divided by the container.
- `bound_volume`: volume of the **root** knapsack (the unconstrained bound).
- `optimal_volume`, `fill_pct`, `gap_pct`, `boxes_packed`: for `resolvida`
  and `limite` rows (for `limite` the volume is the incumbent, so the gap is a
  real optimality gap, not a mistake); the gap is `(bound − optimal) / bound`.
- `time_s`: time inside `solve` for `resolvida`/`limite` rows, wall time for the
  others.

### Typst output

`stress_summary.typ` contains two figures with tables and labels ready to
reference in the report:

- `<tab-estresse-tempo>`: solved instances, search limits, timeouts, failures,
  mean/median/max time per scenario.
- `<tab-estresse-qualidade>`: offered volume, mean occupancy of the optimum and
  mean gap to the knapsack bound per scenario.

Both captions mention the timeout that was passed on the command line.

### Reproducing a run

```bash
cd exact
./build/test/StressTest 30 10 4096 50000000          # 3 scenarios × 10 instances, 30 s each
./build/test/StressTest 60 20 4096 50000000 forte    # only the hard scenario
```

Artifacts are written next to the binary's working directory:

```
exact/
├── stress_instances/stress_<scenario>_<timestamp>/   # generated instances + grupo.json
├── stress_results.csv
└── stress_summary.typ
```

The worst-case duration of a run is `scenarios × instances × timeout`, because
the timeout applies to each instance separately: `StressTest 600 10` can run for
up to `3 × 10 × 600 s` (five hours) if every instance hits the limit.

## Cost and measured behaviour

- **Knapsack:** `O(types × capacity × max_quantity)` states, but the current
  implementation stores a whole `Path` (a `std::vector<int>` with one entry per
  type) in every memo cell and rebuilds the table from scratch for each node.
  On a 12-type, capacity-8000 subproblem this costs ~20–35 ms per node in the
  unoptimized build.
- **Packing check:** exponential, and proving that a candidate does *not* fit is
  the expensive half. With `-O2` the search runs at ~0.25 µs per backtracking
  call (4 million calls/s) on a 10³ container: one 6-box candidate of the
  `fraco` scenario takes 624 456 calls and 160 ms. A 20-box candidate at 81 % of
  the container needs ~500 000–1 000 000 calls and about 0.35–0.5 s *for a single
  check*, and the search visits dozens of such candidates.
- **Search tree:** at most `∏(q_k + 1)` limit vectors (16.7 million for twelve
  types with three units each), visited in decreasing bound order.
- **Distribution of runtimes:** bimodal. An instance either admits a packing of
  the root candidate and finishes in thousands of milliseconds at most, or
  spends the whole budget inside the first few feasibility checks. It depends on
  how close the knapsack bound is to a perfect fill: 81 % or 100 % of the
  container with many boxes is hard, 100 % with one box is trivial.
- **Deadline vs. work done:** the budget bounds the wall time, not the number of
  instances. An instance that cannot be decided consumes the whole budget and
  still reports an incumbent and a bound.

## Known limitations

- **No global budget.** `StressTest` applies `timeout_s` to each instance and
  then multiplies it by every scenario and instance (see above). The solver stops
  itself 0.1 s before the harness would kill it, so the answer always comes out,
  but a run can still last `scenarios × instances × timeout`.
- **The incumbent is greedy only.** `greedy_packing` fills the container from the
  biggest box down and is never improved while the search runs, so a “não
  provada” instance reports a usable but mediocre volume (the measured gap to the
  bound is 9–30 %).
- **`node_limit` is still the wrong unit.** The default 50 million is ~50× more
  than any check needs and it is unpredictable in seconds; it is kept only as a
  backstop for callers that do not pass a deadline.
- **`visited` is unbounded**: up to `∏(q_k + 1)` vectors of `types` integers
  (~1.7 GB in the worst case of the `forte` scenario), which is why the stress
  test also enforces `RLIMIT_AS`.
- **`waitpid` handling**: if `waitpid` returns `-1` (for instance `EINTR`), the
  polling loop breaks without killing the child and the following blocking
  `read()` waits for it to finish, which effectively disables the timeout for
  that instance.
- **Only the direct child is killed**: interrupting `StressTest` from outside
  can leave the solving children running.

## Directory map

```
exact/
├── Makefile                    # build, test and clean
├── main.cpp                    # empty placeholder (entry points are the test binaries)
├── lib/json.hpp                # vendored nlohmann/json
├── src/
│   ├── PCUCExact.cpp           # best-first branch-and-bound over limit vectors
│   ├── BagProblemAlgorithm.cpp # bounded knapsack (upper bound of a node)
│   ├── PackingChecker.cpp      # 3D feasibility check (subset sums + backtracking)
│   ├── InstanceManagerExact.cpp # instance/group generation for the exact solver
│   ├── Utils2.cpp              # Path (knapsack memo entry)
│   ├── header/                 # public headers of the classes above
│   └── ...
├── test/
│   ├── PCUCExactTest.cpp       # known answers + brute-force oracle
│   ├── BagProblemAlgorithmTest.cpp
│   └── StressTest.cpp          # isolated, timed stress runs + CSV/Typst output
├── stress_instances/           # instances generated by StressTest runs
├── stress_results.csv          # last stress run, one row per instance
└── stress_summary.typ          # last stress run, Typst tables
```

The shared building blocks — `Package`, `Container`, `InstanceManager` and the
JSON helpers — live in `../heuristic/` and are compiled into the exact test
binaries by the Makefile.

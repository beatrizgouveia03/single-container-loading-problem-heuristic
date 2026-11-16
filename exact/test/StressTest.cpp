
// Stress test for the exact PCUC algorithm.
//
// For each scenario it generates a group of random instances with the project's
// InstanceManagerExact, solves every instance in a separate child process
// (so a time limit and a memory limit can be enforced without touching the
// solver), and writes:
//   - stress_results.csv   one row per instance
//   - stress_summary.typ   two Typst tables, ready to paste into the report
// and prints a readable summary on the console.
//
// Usage:  StressTest [timeout_s=30] [instances_per_scenario=1] [mem_limit_mb=4096] [node_limit=50000000] [scenario_name]
//   e.g.  ./build/stress/StressTest 60 20 4096
//         ./build/stress/StressTest 30 10 4096 S3
 
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <new>
#include <string>
#include <vector>
 
#include <signal.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
 
#include "../src/header/BagProblemAlgorithm.h"
#include "../src/header/InstanceManagerExact.h"
#include "../src/header/PCUCExact.h"
#include "../src/header/PackingChecker.h"
 
using Clock = std::chrono::steady_clock;
 
// ------------------------------------------------------------------ scenarios
 
struct Scenario {
    std::string name;
    std::string description;
    int item_types;
    int min_dim, max_dim;      // dimension range of each package type
    int min_qty, max_qty;      // quantity range of each package type
    int C, L, A;               // container
};
 
// Dimensions never exceed the smallest container side, so no box is
// individually bigger than the container.
static std::vector<Scenario> default_scenarios() {
    // Three levels, calibrated by measurement (-O2). The exact method is exponential in
    // the number of boxes of a candidate, so difficulty grows with types and copies.
    return {
        {"fraco", "cenário fraco",   5,  1,  10, 1, 4, 10, 10, 10},
        {"medio", "cenário médio",  10,  2,  6, 1, 2, 15, 10, 10},
        {"forte", "cenário forte",  12,  3, 10, 1, 3, 20, 20, 20},
    };
}
 
// ------------------------------------------------------------------ isolated run
 
enum class Status { Solved, Timeout, Limit, Memory, Crash };
 
static const char* status_name(Status s) {
    switch (s) {
        case Status::Solved:  return "resolvida";
        case Status::Timeout: return "timeout";
        case Status::Limit:   return "limite";
        case Status::Memory:  return "memoria";
        default:              return "falha";
    }
}
 
// message sent from the child to the parent through a pipe
struct Report {
    int  stage;     // 1 = knapsack bound computed, 2 = solver finished
    long bound;     // volume of the first (unconstrained) knapsack solution
    double ms;      // time spent inside solve
    long volume;    // volume of the returned selection (optimal or incumbent)
    int  boxes;     // number of boxes in that selection
    int  valid;     // basic sanity checks on the answer
    int  proven;    // 1 = the search proved the volume optimal, 0 = out of time
    long checks;    // packing checks performed
};
 
struct RunResult {
    Status status = Status::Crash;
    double wall_s = 0;       // wall time seen by the parent
    double solve_s = 0;      // time inside the solver (only if it reported)
    bool have_bound = false;
    long bound = 0;
    long volume = 0;
    int boxes = 0;
    bool valid = false;
    bool proven = false;     // optimality proven, not just "a feasible selection"
    long checks = 0;
};
 
static void send_report(int fd, const Report& r) {
    ssize_t n = write(fd, &r, sizeof r);
    (void)n;
}
 
static RunResult run_isolated(const Instance& instance, int timeout_s, long mem_mb, long node_limit) {
    int fds[2];
    if (pipe(fds) != 0) { std::perror("pipe"); return {}; }
 
    pid_t pid = fork();
    if (pid < 0) { std::perror("fork"); return {}; }
 
    if (pid == 0) {                                   // ---------- child
        close(fds[0]);
 
        if (mem_mb > 0) {
            rlimit rl;
            rl.rlim_cur = rl.rlim_max = (rlim_t)mem_mb * 1024 * 1024;
            setrlimit(RLIMIT_AS, &rl);
        }
 
        try {
            std::vector<Package> packages = instance.packages_exact;
            int capacity = instance.c * instance.l * instance.a;
 
            std::vector<int> limits;
            for (const Package& p : packages) limits.push_back(p.quantity);
 
            Report r{};
            r.stage = 1;
            r.bound = BagProblemAlgorithm::solve_knapsack(packages, limits, capacity).volume;
            send_report(fds[1], r);
 
            PCUCExact solver;
            PackingChecker checker(instance.c, instance.l, instance.a);
            checker.set_node_limit(node_limit);
 
            // Stop a little before the parent's SIGKILL, so "out of time" comes
            // out as an answer (incumbent + bound) instead of a lost pipe.
            double budget = timeout_s > 1 ? timeout_s - 0.1 : timeout_s * 0.9;
 
            auto t0 = Clock::now();
            SolveReport rep = solver.solve(packages, capacity, checker,
                                           t0 + std::chrono::milliseconds((long)(budget * 1000)));
            const std::vector<int>& result = rep.selection;
            auto t1 = Clock::now();
 
            long volume = 0;
            int boxes = 0;
            bool valid = result.size() == packages.size();
            for (size_t k = 0; valid && k < result.size(); k++) {
                if (result[k] < 0 || result[k] > packages[k].quantity) valid = false;
                volume += (long)result[k] * packages[k].volume;
                boxes += result[k];
            }
            if (volume > (long)capacity) valid = false;
 
            r.ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

            // Independent check of the claim: the reported selection must really
            // be a packing. Only done when there is time left in the budget.
            if (valid && std::chrono::duration<double>(t1 - t0).count() < 0.8 * budget) {
                checker.clear_deadline();
                checker.set_node_limit(2000000);
                bool feasible = checker.fits_without_overlap(result, packages);
                if (!feasible && !checker.aborted()) valid = false;
            }

            r.stage = 2;
            r.volume = volume;
            r.boxes = boxes;
            r.valid = valid ? 1 : 0;
            r.proven = rep.proven() ? 1 : 0;
            r.checks = rep.checks;
            send_report(fds[1], r);
        } catch (const std::bad_alloc&) {
            _exit(2);
        } catch (...) {
            _exit(3);
        }
        _exit(0);
    }
 
    // ---------------------------------------------- parent
    close(fds[1]);
 
    auto start = Clock::now();
    int status = 0;
    bool timed_out = false;
    while (true) {
        pid_t w = waitpid(pid, &status, WNOHANG);
        if (w == pid || w < 0) break;
        double elapsed = std::chrono::duration<double>(Clock::now() - start).count();
        if (elapsed > timeout_s) {
            kill(pid, SIGKILL);
            waitpid(pid, &status, 0);
            timed_out = true;
            break;
        }
        usleep(5000);
    }
 
    RunResult out;
    out.wall_s = std::chrono::duration<double>(Clock::now() - start).count();
 
    Report r{}, first{}, last{};
    bool have1 = false, have2 = false;
    while (read(fds[0], &r, sizeof r) == (ssize_t)sizeof r) {
        if (r.stage == 1) { first = r; have1 = true; }
        else              { last = r;  have2 = true; }
    }
    close(fds[0]);
 
    if (have1) { out.have_bound = true; out.bound = first.bound; }
 
    if (timed_out) {
        out.status = Status::Timeout;     // should not happen: the solver stops itself
    } else if (WIFEXITED(status) && WEXITSTATUS(status) == 0 && have2) {
        out.status = last.proven ? Status::Solved : Status::Limit;
        out.solve_s = last.ms / 1000.0;
        out.volume = last.volume;
        out.boxes = last.boxes;
        out.valid = last.valid != 0;
        out.proven = last.proven != 0;
        out.checks = last.checks;
    } else if (WIFEXITED(status) && WEXITSTATUS(status) == 4) {
        out.status = Status::Limit;
    } else if (WIFEXITED(status) && WEXITSTATUS(status) == 2) {
        out.status = Status::Memory;
    } else {
        out.status = Status::Crash;     // segfault, OOM-killer, unexpected exit
    }
    return out;
}
 
// ------------------------------------------------------------------ statistics
 
struct Row {
    int instance;
    long container_volume;
    long offered_volume;     // sum of all boxes available, volume
    int boxes_available;
    RunResult r;
};
 
struct Summary {
    int n = 0, solved = 0, timeouts = 0, limits = 0, failures = 0, invalid = 0;
    double time_mean = 0, time_median = 0, time_max = 0;
    double offered_pct = 0, fill_pct = 0, gap_pct = 0;
};
 
static Summary summarize(const std::vector<Row>& rows) {
    Summary s;
    std::vector<double> times;
    double offered_sum = 0, fill_sum = 0, gap_sum = 0;
    int gap_count = 0;
 
    for (const Row& row : rows) {
        s.n++;
        offered_sum += 100.0 * row.offered_volume / row.container_volume;
        switch (row.r.status) {
            case Status::Solved:
                s.solved++;
                if (!row.r.valid) s.invalid++;
                times.push_back(row.r.solve_s);
                fill_sum += 100.0 * row.r.volume / row.container_volume;
                if (row.r.have_bound && row.r.bound > 0) {
                    gap_sum += 100.0 * (row.r.bound - row.r.volume) / row.r.bound;
                    gap_count++;
                }
                break;
            case Status::Timeout: s.timeouts++; break;
            case Status::Limit:   s.limits++; break;
            default:              s.failures++; break;
        }
    }
 
    if (s.n > 0) s.offered_pct = offered_sum / s.n;
    if (s.solved > 0) {
        std::sort(times.begin(), times.end());
        double total = 0;
        for (double t : times) total += t;
        s.time_mean = total / times.size();
        s.time_median = (times.size() % 2) ? times[times.size() / 2]
                                           : (times[times.size() / 2 - 1] + times[times.size() / 2]) / 2.0;
        s.time_max = times.back();
        s.fill_pct = fill_sum / s.solved;
    }
    if (gap_count > 0) s.gap_pct = gap_sum / gap_count;
    return s;
}
 
static std::string fmt(double v, int decimals = 2) {
    char buf[64];
    std::snprintf(buf, sizeof buf, "%.*f", decimals, v);
    return buf;
}
 
// ------------------------------------------------------------------ output
 
static void write_typst(const std::string& path, const std::vector<Scenario>& scenarios,
                        const std::vector<Summary>& sums, int timeout_s) {
    std::ofstream out(path);
 
    out << "#figure(\n  table(\n    columns: (auto, auto, auto, auto, auto, auto, auto, auto, auto, auto),\n"
        << "    align: center,\n"
        << "    [*Cenário*], [*Contêiner*], [*Tipos*], [*Resolvidas*], [*Limite de busca*], [*Timeout*], [*Falhas*],"
        << " [*Tempo médio (s)*], [*Mediana (s)*], [*Máximo (s)*],\n";
    for (size_t i = 0; i < scenarios.size(); i++) {
        const Scenario& sc = scenarios[i];
        const Summary& s = sums[i];
        bool any = s.solved > 0;
        out << "    [" << sc.name << "], [" << sc.C << "×" << sc.L << "×" << sc.A << "], ["
            << sc.item_types << "], [" << s.solved << "/" << s.n << "], [" << s.limits << "], [" << s.timeouts << "], ["
            << s.failures << "], [" << (any ? fmt(s.time_mean, 3) : "-") << "], ["
            << (any ? fmt(s.time_median, 3) : "-") << "], [" << (any ? fmt(s.time_max, 3) : "-") << "],\n";
    }
    out << "  ),\n  caption: [Desempenho do algoritmo exato nos cenários de estresse (limite de "
        << timeout_s << " s por instância). Os tempos consideram apenas instâncias resolvidas.],\n"
        << ") <tab-estresse-tempo>\n\n";
 
    out << "#figure(\n  table(\n    columns: (auto, auto, auto, auto),\n    align: center,\n"
        << "    [*Cenário*], [*Volume oferecido / contêiner (%)*], [*Ocupação média do ótimo (%)*],"
        << " [*Lacuna média para o limite da mochila (%)*],\n";
    for (size_t i = 0; i < scenarios.size(); i++) {
        const Summary& s = sums[i];
        bool any = s.solved > 0;
        out << "    [" << scenarios[i].name << "], [" << fmt(s.offered_pct, 1) << "], ["
            << (any ? fmt(s.fill_pct, 1) : "-") << "], [" << (any ? fmt(s.gap_pct, 2) : "-") << "],\n";
    }
    out << "  ),\n  caption: [Qualidade das soluções nos cenários de estresse. A lacuna é a diferença entre o"
        << " volume do primeiro candidato da mochila (limite superior) e o volume da solução ótima.],\n"
        << ") <tab-estresse-qualidade>\n";
}
 
// ------------------------------------------------------------------ main
 
int main(int argc, char** argv) {
    int timeout_s = argc > 1 ? std::atoi(argv[1]) : 3600;
    int count     = argc > 2 ? std::atoi(argv[2]) : 1;
    long mem_mb   = argc > 3 ? std::atol(argv[3]) : 4096;
    long node_limit = argc > 4 ? std::atol(argv[4]) : 50000000;
    std::string only = argc > 5 ? argv[5] : "";
 
    if (timeout_s < 1 || count < 1) {
        std::cerr << "usage: StressTest [timeout_s] [instances_per_scenario] [mem_limit_mb] [node_limit] [scenario]\n";
        return 2;
    }
 
    // If your InstanceManagerExact constructor takes the folder differently, adjust this line.
    InstanceManagerExact manager("stress_instances");
 
    std::vector<Scenario> all = default_scenarios();
    std::vector<Scenario> scenarios;
    for (const Scenario& sc : all)
        if (only.empty() || sc.name == only) scenarios.push_back(sc);
    if (scenarios.empty()) {
        std::cerr << "unknown scenario '" << only << "'\n";
        return 2;
    }
 
    std::cout << "Stress test: " << count << " instances/scenario, timeout " << timeout_s
              << " s, memory limit " << mem_mb << " MB\n\n";
 
    std::ofstream csv("stress_results.csv");
    csv << "scenario,instance,container,types,boxes_available,offered_volume_pct,status,time_s,"
           "bound_volume,optimal_volume,fill_pct,gap_pct,boxes_packed,instance_file\n";
 
    std::vector<Summary> summaries;
    std::vector<Scenario> done;
    std::string stamp = std::to_string((long)std::time(nullptr));
 
    for (const Scenario& sc : scenarios) {
        std::string group = "stress_" + sc.name + "_" + stamp;
        std::vector<Instance> instances = manager.generateGroup(
            group, count, sc.item_types, sc.min_dim, sc.max_dim, sc.min_qty, sc.max_qty,
            sc.C, sc.L, sc.A);
        if (instances.empty()) {
            std::cerr << "could not generate scenario " << sc.name << ", skipping\n";
            continue;
        }
 
        std::cout << "[" << sc.name << "] " << sc.description << " - container " << sc.C << "x"
                  << sc.L << "x" << sc.A << ", " << sc.item_types << " types\n";
 
        std::vector<Row> rows;
        for (const Instance& instance : instances) {
            Row row;
            row.instance = instance.id;
            row.container_volume = (long)instance.c * instance.l * instance.a;
            row.offered_volume = 0;
            row.boxes_available = 0;
            for (const Package& p : instance.packages_exact) {
                row.offered_volume += (long)p.quantity * p.volume;
                row.boxes_available += p.quantity;
            }
 
            row.r = run_isolated(instance, timeout_s, mem_mb, node_limit);
            rows.push_back(row);
 
            const RunResult& r = row.r;
            bool solved = r.status == Status::Solved;
            // A "limite" run still has a feasible selection (the incumbent) and the
            // bound it was trying to close, which is worth reporting.
            bool answered = solved || r.status == Status::Limit;
            double time = answered ? r.solve_s : r.wall_s;
            double fill = answered ? 100.0 * r.volume / row.container_volume : 0;
            double gap = (answered && r.have_bound && r.bound > 0)
                             ? 100.0 * (r.bound - r.volume) / r.bound : 0;
 
            csv << sc.name << "," << instance.id << "," << sc.C << "x" << sc.L << "x" << sc.A << ","
                << sc.item_types << "," << row.boxes_available << ","
                << fmt(100.0 * row.offered_volume / row.container_volume, 1) << ","
                << status_name(r.status) << "," << fmt(time, 4) << ","                << (r.have_bound ? std::to_string(r.bound) : "") << ","
                << (answered ? std::to_string(r.volume) : "") << ","
                << (answered ? fmt(fill, 2) : "") << "," << (answered ? fmt(gap, 3) : "") << ","
                << (answered ? std::to_string(r.boxes) : "") << "," << instance.file << "\n";
            csv.flush();

            std::cout << "  instance " << instance.id << " (" << instance.file << "): " << status_name(r.status) << "  "
                      << fmt(time, 3) << " s";
            if (answered) std::cout << "  volume " << r.volume << "/" << row.container_volume;
            if (r.status == Status::Limit) std::cout << " (incumbent, not proven)";
            if (solved && !r.valid) std::cout << "  [INVALID ANSWER]";
            std::cout << "\n";
        }
 
        Summary s = summarize(rows);
        summaries.push_back(s);
        done.push_back(sc);
 
        std::cout << "  -> solved " << s.solved << "/" << s.n << ", limit " << s.limits << ", timeouts " << s.timeouts
                  << ", failures " << s.failures;
        if (s.solved > 0)
            std::cout << ", mean " << fmt(s.time_mean, 3) << " s, median " << fmt(s.time_median, 3)
                      << " s, max " << fmt(s.time_max, 3) << " s";
        std::cout << "\n\n";
    }
 
    write_typst("stress_summary.typ", done, summaries, timeout_s);
 
    int invalid = 0;
    for (const Summary& s : summaries) invalid += s.invalid;
    std::cout << "Wrote stress_results.csv and stress_summary.typ\n";
    if (invalid > 0) {
        std::cout << "WARNING: " << invalid << " solved instance(s) returned an invalid answer\n";
        return 1;
    }
    return 0;
}
 
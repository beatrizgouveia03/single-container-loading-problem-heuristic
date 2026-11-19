#include "BranchAndBound.h"
#include "Types.h"
#include <iostream>
#include <random>
#include <string>
#include <vector>

static int passed = 0, failed = 0;

static void check(bool cond, const std::string &name, const std::string &detail = "")
{
    if (cond) { std::cout << "[ OK ] " << name << "\n"; passed++; }
    else      { std::cout << "[FAIL] " << name << (detail.empty() ? "" : "  -> " + detail) << "\n"; failed++; }
}

static bool overlap(const Package &p, const Package &q)
{
    return p.position.x < q.position.x + q.l && q.position.x < p.position.x + p.l &&
           p.position.y < q.position.y + q.a && q.position.y < p.position.y + p.a &&
           p.position.z < q.position.z + q.c && q.position.z < p.position.z + p.c;
}

static bool validPacking(const std::vector<Package> &v, ll W, ll H, ll D)
{
    for (size_t i = 0; i < v.size(); i++)
    {
        const Package &p = v[i];
        if (p.position.x < 0 || p.position.y < 0 || p.position.z < 0) return false;
        if (p.position.x + p.l > W || p.position.y + p.a > H || p.position.z + p.c > D) return false;
        for (size_t j = i + 1; j < v.size(); j++)
            if (overlap(p, v[j])) return false;
    }
    return true;
}

static ll sumVolume(const std::vector<Package> &v)
{
    ll s = 0;
    for (const auto &p : v) s += p.volume;
    return s;
}

int main()
{
    // Container: c = D (profundidade), l = W (largura), a = H (altura)
    std::mt19937 rng(12345);
    const int TRIALS = 30;

    int mismatches = 0, invalid = 0, inconsistent = 0, mutated = 0;
    std::string firstMismatch;

    for (int t = 0; t < TRIALS; t++)
    {
        ll C = 4 + rng() % 4;  // 4..7
        ll L = 4 + rng() % 4;
        ll A = 4 + rng() % 4;
        int n = 4 + rng() % 3; // 4..6 pacotes

        std::vector<Package> pkgs;
        for (int i = 0; i < n; i++)
            pkgs.push_back(Package(1 + rng() % 4, 1 + rng() % 4, 1 + rng() % 4));

        // sem ordenação
        BranchAndBound plain;
        std::vector<Package> p1 = pkgs, ins1;
        ll F1 = 0;
        plain.solve(p1, L, A, C, F1, ins1);

        // com ordenação
        BranchAndBound sorted;
        std::vector<Package> p2 = pkgs, ins2;
        ll F2 = 0;
        sorted.solveSorted(p2, L, A, C, F2, ins2);

        if (F1 != F2)
        {
            mismatches++;
            if (firstMismatch.empty())
                firstMismatch = "trial " + std::to_string(t) + ": sem ordenar = " +
                                std::to_string(F1) + ", ordenado = " + std::to_string(F2);
        }
        if (!validPacking(plain.best, L, A, C) || !validPacking(sorted.best, L, A, C)) invalid++;
        if (sumVolume(plain.best) != F1 || sumVolume(sorted.best) != F2) inconsistent++;

        // solveSorted recebe por valor: o vetor do chamador não pode mudar de ordem
        bool same = p2.size() == pkgs.size();
        for (size_t i = 0; same && i < pkgs.size(); i++)
            if (p2[i].c != pkgs[i].c || p2[i].l != pkgs[i].l || p2[i].a != pkgs[i].a) same = false;
        if (!same) mutated++;
    }

    check(mismatches == 0, "ordenar nao altera o otimo (" + std::to_string(TRIALS) + " instancias)", firstMismatch);
    check(invalid == 0, "melhor solucao valida (sem sobreposicao, dentro do conteiner) nos dois modos");
    check(inconsistent == 0, "volume da melhor solucao == F nos dois modos");
    check(mutated == 0, "solveSorted nao altera o vetor do chamador");

    {
        // caso determinístico: ordem ruim de entrada (pequenos primeiro)
        std::vector<Package> pk = {Package(1, 1, 1), Package(1, 2, 1), Package(1, 6, 1), Package(1, 5, 1), Package(1, 5, 1)};
        BranchAndBound a, b;
        std::vector<Package> r1 = pk, i1, i2;
        ll F1 = 0, F2 = 0;
        a.solve(r1, 10, 1, 1, F1, i1);
        b.solveSorted(pk, 10, 1, 1, F2, i2);
        check(F1 == 10 && F2 == 10, "entrada em ordem crescente: ambos F = 10",
              "F1 = " + std::to_string(F1) + ", F2 = " + std::to_string(F2));
    }

    std::cout << "\n" << passed << "/" << (passed + failed) << " testes passaram\n";
    return failed == 0 ? 0 : 1;
}
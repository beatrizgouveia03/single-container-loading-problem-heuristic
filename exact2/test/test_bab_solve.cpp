#include "BranchAndBound.h"
#include "Types.h"
#include <algorithm>
#include <iostream>
#include <random>
#include <vector>

static int passed = 0, failed = 0;

static void check(bool cond, const std::string &name, const std::string &detail = "")
{
    if (cond) { std::cout << "[ OK ] " << name << "\n"; passed++; }
    else      { std::cout << "[FAIL] " << name << (detail.empty() ? "" : "  -> " + detail) << "\n"; failed++; }
}

// Roda o solver numa instância nova. Convenção do projeto: solve(p, W=l, H=a, D=c, ...)
static ll run(BranchAndBound &bab, std::vector<Package> pkgs, ll c, ll l, ll a)
{
    std::vector<Package> inserted;
    ll F = 0;
    bab.solve(pkgs, l, a, c, F, inserted);
    return F;
}

static ll sumVolume(const std::vector<Package> &v)
{
    ll s = 0;
    for (const auto &p : v) s += p.volume;
    return s;
}

// Caixa ocupa x:[x, x+l), y:[y, y+a), z:[z, z+c)
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

int main()
{
    // Package(c, l, a)  |  container: c = profundidade (D), l = largura (W), a = altura (H)

    {
        BranchAndBound bab;
        check(run(bab, {}, 10, 10, 10) == 0, "sem pacotes: F = 0");
    }
    {
        BranchAndBound bab;
        ll F = run(bab, {Package(2, 3, 4)}, 10, 10, 10);
        check(F == 24, "um pacote que cabe: F = 24", "F = " + std::to_string(F));
    }
    {
        BranchAndBound bab;
        ll F = run(bab, {Package(6, 1, 1)}, 5, 5, 5);
        check(F == 0, "pacote maior que o contêiner: F = 0", "F = " + std::to_string(F));
    }
    {
        // dois 1x2x2 empilhados em z preenchem 2x2x2
        BranchAndBound bab;
        ll F = run(bab, {Package(1, 2, 2), Package(1, 2, 2)}, 2, 2, 2);
        check(F == 8, "preenchimento perfeito (2 caixas empilhadas): F = 8", "F = " + std::to_string(F));
    }
    {
        // 1D (c=1, a=1, l=10): guloso pegaria o 6 e ficaria com 6; o ótimo é 5+5
        BranchAndBound bab;
        ll F = run(bab, {Package(1, 6, 1), Package(1, 5, 1), Package(1, 5, 1)}, 1, 10, 1);
        check(F == 10, "otimalidade: 5+5 vence o 6 sozinho: F = 10", "F = " + std::to_string(F));
    }
    {
        // subset-sum: 4+3+3 = 10 em 4,3,3,2
        BranchAndBound bab;
        ll F = run(bab, {Package(1, 4, 1), Package(1, 3, 1), Package(1, 3, 1), Package(1, 2, 1)}, 1, 10, 1);
        check(F == 10, "subset-sum 4,3,3,2 em 10: F = 10", "F = " + std::to_string(F));
    }
    {
        // nada cabe junto: só o maior cabe
        BranchAndBound bab;
        ll F = run(bab, {Package(1, 7, 1), Package(1, 6, 1), Package(1, 5, 1)}, 1, 10, 1);
        check(F == 7, "só um cabe por vez: F = 7", "F = " + std::to_string(F));
    }

    // Instância 3D genérica: valida invariantes
    std::vector<Package> mix = {Package(2, 3, 2), Package(1, 2, 4), Package(3, 3, 1), Package(2, 2, 2),
                                Package(1, 1, 3), Package(2, 4, 1)};
    const ll C = 4, L = 4, A = 4;
    {
        BranchAndBound bab;
        std::vector<Package> rem = mix, inserted;
        ll F = 0;
        bab.solve(rem, L, A, C, F, inserted);

        check(F > 0 && F <= C * L * A, "3D: 0 < F <= volume do contêiner", "F = " + std::to_string(F));
        check(F <= sumVolume(mix), "3D: F <= volume total dos pacotes");
        check(validPacking(bab.best, L, A, C), "3D: melhor solução sem sobreposição e dentro do contêiner");
        check(sumVolume(bab.best) == F, "3D: volume da melhor solução == F",
              "soma = " + std::to_string(sumVolume(bab.best)) + ", F = " + std::to_string(F));
        check(inserted.empty() && rem.size() == mix.size(), "3D: solve desfaz as inserções (backtracking)");
    }
    {
        // ordem de entrada não pode mudar o ótimo
        BranchAndBound ref;
        ll F0 = run(ref, mix, C, L, A);
        std::mt19937 rng(42);
        bool same = true;
        for (int t = 0; t < 5; t++)
        {
            std::shuffle(mix.begin(), mix.end(), rng);
            BranchAndBound b;
            if (run(b, mix, C, L, A) != F0) same = false;
        }
        check(same, "ordem de entrada não altera F (5 permutações)");
    }
    {
        // F inicial maior que o ótimo: nada deve ser melhorado
        BranchAndBound bab;
        std::vector<Package> rem = {Package(2, 3, 4)}, inserted;
        ll F = 1000;
        bab.solve(rem, 10, 10, 10, F, inserted);
        check(F == 1000, "F inicial acima do ótimo é preservado");
    }

    std::cout << "\n" << passed << "/" << (passed + failed) << " testes passaram\n";
    return failed == 0 ? 0 : 1;
}
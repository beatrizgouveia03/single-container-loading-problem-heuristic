#include "BranchAndBound.h"

using namespace std;
#include "Types.h"
#include <algorithm>

ll getVolume(const vector<Package> &packages)
{
    ll v = 0;
    for (const auto &p : packages)
        v += p.volume;
    return v;
}

void BranchAndBound::solve(vector<Package> &remainingPackages, ll W, ll H, ll D,
                           ll &F, vector<Package> &insertedPackages)
{
    vector<Coordinates3d> C = Corners3d::getValidCorners(insertedPackages, remainingPackages, W, H, D);
    ll B = W * H * D;
    ll current = getVolume(insertedPackages); 

    if (remainingPackages.empty() || C.empty())
    {
        if (current > F) { F = current; best = insertedPackages; }
        return;
    }

    ll V = Corners3d::getEnvelopeVolume(insertedPackages, remainingPackages, W, H, D);
    if (current + (B - V) <= F) // bound
        return;

    for (const Coordinates3d &c : C)
    {
        for (size_t i = 0; i < remainingPackages.size(); i++)
        {
            const Package &q = remainingPackages[i];

            // pula pacote com dimensões iguais a um já tentado neste nível
            bool dup = false;
            for (size_t k = 0; k < i; k++)
            {
                const Package &o = remainingPackages[k];
                if (o.c == q.c && o.l == q.l && o.a == q.a) { dup = true; break; }
            }
            if (dup) continue;

            if (c.x + q.l > W || c.y + q.a > H || c.z + q.c > D)
                continue;

            Package original = q;       
            Package p = original;
            p.position = c;
            insertedPackages.push_back(p);
            remainingPackages.erase(remainingPackages.begin() + i);

            solve(remainingPackages, W, H, D, F, insertedPackages);

            // desfaz inserção
            remainingPackages.insert(remainingPackages.begin() + i, original);
            insertedPackages.pop_back();

            // contêiner cheio
            if (F >= B) return;
        }
    }
}

void BranchAndBound::solveSorted(vector<Package> pkgs, ll W, ll H, ll D, ll &F,
                                 vector<Package> &inserted)
{
    stable_sort(pkgs.begin(), pkgs.end(), [](const Package &x, const Package &y) {
        return x.volume > y.volume;
    });
    solve(pkgs, W, H, D, F, inserted);
}
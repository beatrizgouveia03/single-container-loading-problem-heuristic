#include "BranchAndBound.h"

using namespace std;
#include "Types.h"

ll getVolume(vector<Package> package)
{
    ll v = 0;
    for(auto p:package)
    {
        v += p.volume;
    }
    return v;
}
void BranchAndBound::solve(vector<Package> &remainingPackages, ll W, ll H, ll D,ll &F, vector<Package> &insertedPackages)
{
    vector<Coordinates3d> C = Corners3d::getValidCorners(insertedPackages, remainingPackages, W, H, D);
    ll B = W * H * D;   

    if(remainingPackages.empty() || C.empty())
    {
        if (getVolume(insertedPackages) > F) { F = getVolume(insertedPackages); best = insertedPackages; }
        return;                           
    }

    ll V = Corners3d::getEnvelopeVolume(insertedPackages, remainingPackages, W, H, D);  
    if(getVolume(insertedPackages) + (B - V) <= F) // bound   
        return;

    for(Coordinates3d c: C)
    {
        for(int i=0; i<remainingPackages.size(); i++)
        {
            Package p = remainingPackages[i];
            if (c.x + p.l > W || c.y + p.a > H || c.z + p.c > D)
                continue; 

            p.position = c;
            insertedPackages.push_back(p);
            remainingPackages.erase(remainingPackages.begin() + i);

            solve(remainingPackages, W, H, D, F, insertedPackages);

            // desfaz inserção
            remainingPackages.insert(remainingPackages.begin() + i, p);
            insertedPackages.pop_back();
        }
    }
}
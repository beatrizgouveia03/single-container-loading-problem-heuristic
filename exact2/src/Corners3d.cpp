#include "Corners3d.h"
#include "Corners2d.h"
#include "Coordinates2d.h"
#include <set>
#include <algorithm>

vector<Coordinates3d> Corners3d::getValidCorners(vector<Package> insertedPackages, vector<Package> Packages, int W, int H, int D)
{
    if(insertedPackages.empty())
    {
        return vector<Coordinates3d>(1,Coordinates3d(0,0,0));
    }

    if(Packages.empty())
    {
        return vector<Coordinates3d>(); // nada mais a colocar
    }

    int minD = Packages[0].c;
    for(const Package& p : Packages)
    {
        minD = min(minD, p.c);
    }

    set<int> T_aux;
    vector<int> T;
    T.push_back(0);
    for(auto p:insertedPackages)
    {
        if(T_aux.find(p.position.z + p.c) == T_aux.end())
        {
            T_aux.insert(p.position.z + p.c);
            T.push_back(p.position.z + p.c);
        }
    }
    sort(T.begin(), T.end());

    vector<Coordinates3d> validPoints;
    set<Coordinates2d> previous; // Ĉ(Î_{k-1}), começa vazio

    for(int z:T)
    {
        if(z + minD > D) break; // nenhuma caixa restante cabe daqui pra frente

        vector<Package> subInsertedPackages; 
        for(Package p:insertedPackages)
        {
            if(p.position.z + p.c > z)
            {
                subInsertedPackages.push_back(p);
            }
        }

        sort(subInsertedPackages.begin(), subInsertedPackages.end(),
             [](const Package& a, const Package& b) {
                 int ta = a.position.y + a.a, tb = b.position.y + b.a;
                 if(ta != tb) return ta > tb;
                 return a.position.x + a.l > b.position.x + b.l;
             });

        vector<Coordinates2d> c2d = Corners2d::getValidCorners(subInsertedPackages, Packages, W, H);

        for(auto c:c2d)
        {
            if(previous.find(c) == previous.end())
            {
                validPoints.push_back(Coordinates3d(c.x, c.y, z));
            }
        }

        previous = set<Coordinates2d>(c2d.begin(), c2d.end());
    }
    return validPoints;
}

long long Corners3d::getEnvelopeVolume(const vector<Package>& insertedPackages,const vector<Package>& Packages, int W, int H, int D)
{
    if(Packages.empty())
        return (long long)W * H * D;

    if(insertedPackages.empty())
        return 0;

    int minD = Packages[0].c;
    for(const Package& p : Packages)
        minD = min(minD, p.c);

    set<int> Tset;
    Tset.insert(0);
    for(const Package& p : insertedPackages)
        Tset.insert(p.position.z + p.c);

    long long volume = 0;
    long long prevArea = 0;
    int prevZ = 0;
    bool first = true;

    for(int z : Tset)
    {
        if(z + minD > D) break; 

        vector<Package> sub;
        for(const Package& p : insertedPackages)
            if(p.position.z + p.c > z)
                sub.push_back(p);

        sort(sub.begin(), sub.end(),
             [](const Package& a, const Package& b) {
                 int ta = a.position.y + a.a, tb = b.position.y + b.a;
                 if(ta != tb) return ta > tb;
                 return a.position.x + a.l > b.position.x + b.l;
             });

        vector<Coordinates2d> c2d = Corners2d::getValidCorners(sub, Packages, W, H);
        long long area = Corners2d::getEnvelopeArea(c2d, W, H);

        if(!first)
            volume += (long long)(z - prevZ) * prevArea;

        prevZ = z;
        prevArea = area;
        first = false;
    }

    volume += (long long)(D - prevZ) * prevArea;

    return volume;
}
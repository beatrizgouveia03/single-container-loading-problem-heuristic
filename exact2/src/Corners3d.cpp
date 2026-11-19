#include "Corners3d.h"
#include "Corners2d.h"
#include "Coordinates2d.h"
#include <set>
#include <algorithm>
#include "Types.h"

vector<Coordinates3d> Corners3d::getValidCorners(vector<Package> &insertedPackages, vector<Package> &Packages, ll W, ll H, ll D)
{
    if(insertedPackages.empty())
    {
        return vector<Coordinates3d>(1,Coordinates3d(0,0,0));
    }

    if(Packages.empty())
    {
        return vector<Coordinates3d>(); // nada mais a colocar
    }

    ll minD = Packages[0].c;
    for(const Package& p : Packages)
    {
        minD = min(minD, p.c);
    }

    set<ll> T_aux;
    vector<ll> T;
    T.push_back(0);
    for(const auto &p:insertedPackages)
    {
        if(T_aux.find(p.position.z + p.c) == T_aux.end())
        {
            T_aux.insert(p.position.z + p.c);
            T.push_back(p.position.z + p.c);
        }
    }
    sort(T.begin(), T.end());

    vector<Coordinates3d> validPolls;
    set<Coordinates2d> previous; // Ĉ(Î_{k-1}), começa vazio

    for(ll z:T)
    {
        if(z + minD > D) break; // nenhuma caixa restante cabe daqui pra frente

        vector<Package> subInsertedPackages; 
        for(const Package &p:insertedPackages)
        {
            if(p.position.z + p.c > z)
            {
                subInsertedPackages.push_back(p);
            }
        }

        sort(subInsertedPackages.begin(), subInsertedPackages.end(),
             [](const Package& a, const Package& b) {
                 ll ta = a.position.y + a.a, tb = b.position.y + b.a;
                 if(ta != tb) return ta > tb;
                 return a.position.x + a.l > b.position.x + b.l;
             });

        vector<Coordinates2d> c2d = Corners2d::getValidCorners(subInsertedPackages, Packages, W, H);

        for(auto c:c2d)
        {
            if(previous.find(c) == previous.end())
            {
                validPolls.push_back(Coordinates3d(c.x, c.y, z));
            }
        }

        previous = set<Coordinates2d>(c2d.begin(), c2d.end());
    }
    return validPolls;
}

ll Corners3d::getEnvelopeVolume(const vector<Package>& insertedPackages,const vector<Package>& Packages, ll W, ll H, ll D)
{
    if(Packages.empty())
        return (ll)W * H * D;

    if(insertedPackages.empty())
        return 0;

    ll minD = Packages[0].c;
    for(const Package& p : Packages)
        minD = min(minD, p.c);

    set<ll> Tset;
    Tset.insert(0);
    for(const Package& p : insertedPackages)
        Tset.insert(p.position.z + p.c);

    ll volume = 0;
    ll prevArea = 0;
    ll prevZ = 0;
    bool first = true;

    for(ll z : Tset)
    {
        if(z + minD > D) break; 

        vector<Package> sub;
        for(const Package& p : insertedPackages)
            if(p.position.z + p.c > z)
                sub.push_back(p);

        sort(sub.begin(), sub.end(),
             [](const Package& a, const Package& b) {
                 ll ta = a.position.y + a.a, tb = b.position.y + b.a;
                 if(ta != tb) return ta > tb;
                 return a.position.x + a.l > b.position.x + b.l;
             });

        vector<Coordinates2d> c2d = Corners2d::getValidCorners(sub, Packages, W, H);
        ll area = Corners2d::getEnvelopeArea(c2d, W, H);

        if(!first)
            volume += (ll)(z - prevZ) * prevArea;

        prevZ = z;
        prevArea = area;
        first = false;
    }

    volume += (ll)(D - prevZ) * prevArea;

    return volume;
}
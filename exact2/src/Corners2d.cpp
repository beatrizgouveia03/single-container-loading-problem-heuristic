#include "Corners2d.h"
#include "Coordinates2d.h"
#include <set>
#include <algorithm>
#include "Types.h"

using namespace std;

// não esquecer de ordenar os pacotes certo
vector<Coordinates2d> Corners2d::getValidCorners(const vector<Package> &insertedPackages, const vector<Package> &packages, ll W, ll H)
{
    if(insertedPackages.empty())
    {
        return vector<Coordinates2d>(1, Coordinates2d(0,0)); // ponto inicial
    }

    // caixas extremas
    ll x = 0;
    ll m = 0;
    vector<Package> extremeBoxes;

    for(ll i=0; i<insertedPackages.size(); i++)
    {
        const Package &package = insertedPackages[i];
        Coordinates2d position = Coordinates2d(package.position.x, package.position.y);

        if(position.x + package.l > x)
        {
            extremeBoxes.push_back(package);
            x = position.x + package.l;
        }
    }

    // corner polls
    set<Coordinates2d> cornerPolls;
    vector<Coordinates2d> cp;

    cornerPolls.insert(Coordinates2d(0, extremeBoxes[0].position.y + extremeBoxes[0].a)); // ponto colado na parede y
    cp.push_back(Coordinates2d(0, extremeBoxes[0].position.y + extremeBoxes[0].a));
    for(ll i=1; i<extremeBoxes.size(); i++)
    {
        auto behindX = extremeBoxes[i-1].position.x + extremeBoxes[i-1].l; // x do ponto estremo anterior 
        auto currentY = extremeBoxes[i].position.y + extremeBoxes[i].a; // y do ponto estremo atual
        if(cornerPolls.find(Coordinates2d(behindX, currentY)) == cornerPolls.end())
        {
            cornerPolls.insert(Coordinates2d(behindX, currentY));
            cp.push_back(Coordinates2d(behindX, currentY));
        }
    }
    cp.push_back(Coordinates2d(Coordinates2d(extremeBoxes.back().position.x + extremeBoxes.back().l, 0)));

    // remover pontos onde nenhuma caixa restante cabe
    vector<Coordinates2d> validCorners;

    if(packages.empty())
        return validCorners;

    ll minL = packages[0].l;
    ll minA = packages[0].a;
    for(const Package& p : packages)
    {
        minL = min(minL, p.l);
        minA = min(minA, p.a);
    }

    for(const Coordinates2d& c : cp)
    {
        if(c.x + minL > W || c.y + minA > H)
            continue; // nem a menor caixa cabe aqui

        validCorners.push_back(c);
    }

    return validCorners;
};

ll Corners2d::getEnvelopeArea(vector<Coordinates2d> &polls, ll W, ll H)
{

    if(polls.empty())
        return W * H;
        
    ll area = 0;
    area += (polls[0].x * H) + (W - polls.back().x) * polls.back().y;
    
    for(ll i=1; i<polls.size(); i++)
    {
        area += (polls[i].x - polls[i-1].x) * polls[i-1].y;
    }
    return area;
}
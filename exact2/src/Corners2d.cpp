#include "Corners2d.h"
#include <set>
#include <algorithm>

using namespace std;

// não esquecer de ordenar os pacotes certo
vector<Coordinates2d> Corners2d::getValidCorners(vector<Package> insertedPackages, vector<Package> packages, int W, int H)
{
    if(insertedPackages.empty())
    {
        return vector<Coordinates2d>(1, Coordinates2d(0,0)); // ponto inicial
    }

    // caixas extremas
    int x = 0;
    int m = 0;
    vector<Package> extremeBoxes;

    for(int i=0; i<insertedPackages.size(); i++)
    {
        Package package = insertedPackages[i];
        Coordinates2d position = package.position;

        if(position.x + package.l > x)
        {
            extremeBoxes.push_back(package);
            x = position.x + package.l;
        }
    }

    // corner points
    set<Coordinates2d> cornerPoints;

    cornerPoints.insert(Coordinates2d(0, extremeBoxes[0].position.y + extremeBoxes[0].a)); // ponto colado na parede y
    for(int i=1; i<extremeBoxes.size(); i++)
    {
        auto behindX = extremeBoxes[i-1].position.x + extremeBoxes[i-1].l; // x do ponto estremo anterior 
        auto currentY = extremeBoxes[i].position.y + extremeBoxes[i].a; // y do ponto estremo atual
        cornerPoints.insert(Coordinates2d(behindX, currentY));
    }
    cornerPoints.insert(Coordinates2d(extremeBoxes.back().position.x + extremeBoxes.back().l, 0)); // ponto colado na parede x

    // remover pontos onde nenhuma caixa restante cabe
    vector<Coordinates2d> validCorners;

    if(packages.empty())
        return validCorners;

    int minL = packages[0].l;
    int minA = packages[0].a;
    for(const Package& p : packages)
    {
        minL = min(minL, p.l);
        minA = min(minA, p.a);
    }

    for(const Coordinates2d& c : cornerPoints)
    {
        if(c.x + minL > W || c.y + minA > H)
            continue; // nem a menor caixa cabe aqui

        validCorners.push_back(c);
    }

    return validCorners;
};
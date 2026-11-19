#include "Corners2d.h"
#include "Coordinates2d.h"
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
        Coordinates2d position = Coordinates2d(package.position.x, package.position.y);

        if(position.x + package.l > x)
        {
            extremeBoxes.push_back(package);
            x = position.x + package.l;
        }
    }

    // corner points
    set<Coordinates2d> cornerPoints;
    vector<Coordinates2d> cp;

    cornerPoints.insert(Coordinates2d(0, extremeBoxes[0].position.y + extremeBoxes[0].a)); // ponto colado na parede y
    cp.push_back(Coordinates2d(0, extremeBoxes[0].position.y + extremeBoxes[0].a));
    for(int i=1; i<extremeBoxes.size(); i++)
    {
        auto behindX = extremeBoxes[i-1].position.x + extremeBoxes[i-1].l; // x do ponto estremo anterior 
        auto currentY = extremeBoxes[i].position.y + extremeBoxes[i].a; // y do ponto estremo atual
        if(cornerPoints.find(Coordinates2d(behindX, currentY)) == cornerPoints.end())
        {
            cornerPoints.insert(Coordinates2d(behindX, currentY));
            cp.push_back(Coordinates2d(behindX, currentY));
        }
    }
    cp.push_back(Coordinates2d(Coordinates2d(extremeBoxes.back().position.x + extremeBoxes.back().l, 0)));

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

    for(const Coordinates2d& c : cp)
    {
        if(c.x + minL > W || c.y + minA > H)
            continue; // nem a menor caixa cabe aqui

        validCorners.push_back(c);
    }

    return validCorners;
};

int Corners2d::getEnvelopeArea(vector<Coordinates2d> points, int W, int H)
{

    if(points.empty())
        return W * H;

    int area = 0;
    area += (points[0].x * H) + (W - points.back().x) * points.back().y;
    
    for(int i=1; i<points.size(); i++)
    {
        area += (points[i].x - points[i-1].x) * points[i-1].y;
    }
    return area;
}
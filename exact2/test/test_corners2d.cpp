// arquivo de teste feito por IA para teste do algoritmo Corners2d

#include "Corners2d.h"
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

static int failures = 0;

// Assume que Package tem os campos: position (Coordinates2d), l (largura), a (altura)
// e um construtor padrao. Ajuste aqui se o seu Package for diferente.
static Package makePackage(int x, int y, int l, int a)
{
    Package p;
    p.position = Coordinates2d(x, y);
    p.l = l;
    p.a = a;
    return p;
}

static Package makeRemaining(int l, int a)
{
    return makePackage(0, 0, l, a);
}

static void check(const string& name, vector<Coordinates2d> got, vector<Coordinates2d> expected)
{
    auto cmp = [](const Coordinates2d& a, const Coordinates2d& b) { return a < b; };
    sort(got.begin(), got.end(), cmp);
    sort(expected.begin(), expected.end(), cmp);

    bool ok = got.size() == expected.size();
    for (size_t i = 0; ok && i < got.size(); i++)
        ok = got[i].x == expected[i].x && got[i].y == expected[i].y;

    cout << (ok ? "[OK]   " : "[FAIL] ") << name << endl;
    if (!ok)
    {
        failures++;
        cout << "   esperado:";
        for (auto& c : expected) cout << " (" << c.x << "," << c.y << ")";
        cout << "\n   obtido:  ";
        for (auto& c : got) cout << " (" << c.x << "," << c.y << ")";
        cout << endl;
    }
}

int main()
{
    const int W = 10, H = 10;
    Corners2d corners;

    // 1) Bin vazio: apenas a origem
    {
        vector<Package> inserted, remaining = { makeRemaining(1, 1) };
        check("bin vazio retorna (0,0)",
              corners.getValidCorners(inserted, remaining, W, H),
              { Coordinates2d(0, 0) });
    }

    // Escada com 3 caixas, ja ordenadas por (y+a) decrescente:
    //   A: x=0 y=0 l=3 a=8   topo 8, direita 3
    //   B: x=3 y=0 l=3 a=5   topo 5, direita 6
    //   C: x=6 y=0 l=2 a=2   topo 2, direita 8
    // Corners esperados: (0,8) (3,5) (6,2) (8,0)
    vector<Package> staircase = {
        makePackage(0, 0, 3, 8),
        makePackage(3, 0, 3, 5),
        makePackage(6, 0, 2, 2)
    };

    // 2) Caixa restante 1x1: todos os cantos sao validos
    {
        vector<Package> remaining = { makeRemaining(1, 1) };
        check("escada, restante 1x1: todos validos",
              corners.getValidCorners(staircase, remaining, W, H),
              { Coordinates2d(0, 8), Coordinates2d(3, 5), Coordinates2d(6, 2), Coordinates2d(8, 0) });
    }

    // 3) Menor restante 3x3: (0,8) e (8,0) estouram o bin e sao removidos
    {
        vector<Package> remaining = { makeRemaining(3, 3), makeRemaining(5, 4) };
        check("escada, menor restante 3x3: remove (0,8) e (8,0)",
              corners.getValidCorners(staircase, remaining, W, H),
              { Coordinates2d(3, 5), Coordinates2d(6, 2) });
    }

    // 4) Menores dimensoes vem de caixas diferentes (min l = 1, min a = 3)
    //    (8,0): 8+1<=10 e 0+3<=10 -> valido
    //    (0,8): 8+3=11>10 -> removido
    {
        vector<Package> remaining = { makeRemaining(1, 5), makeRemaining(4, 3) };
        check("min l e min a de caixas diferentes",
              corners.getValidCorners(staircase, remaining, W, H),
              { Coordinates2d(3, 5), Coordinates2d(6, 2), Coordinates2d(8, 0) });
    }

    // 5) Sem caixas restantes: nenhum ponto util
    {
        vector<Package> remaining;
        check("sem caixas restantes: vazio",
              corners.getValidCorners(staircase, remaining, W, H),
              {});
    }

    // 6) Caixa nao extrema (P2 fica escondida atras de P1)
    //   P1: x=0 y=5 l=4 a=3   topo 8, direita 4
    //   P2: x=0 y=0 l=2 a=5   topo 5, direita 2  (nao extrema)
    //   P3: x=4 y=0 l=3 a=4   topo 4, direita 7
    // Corners esperados: (0,8) (4,4) (7,0)
    {
        vector<Package> inserted = {
            makePackage(0, 5, 4, 3),
            makePackage(0, 0, 2, 5),
            makePackage(4, 0, 3, 4)
        };
        vector<Package> remaining = { makeRemaining(1, 1) };
        check("caixa nao extrema e ignorada",
              corners.getValidCorners(inserted, remaining, W, H),
              { Coordinates2d(0, 8), Coordinates2d(4, 4), Coordinates2d(7, 0) });
    }

    // 7) Uma unica caixa: corners (0,a) e (l,0)
    {
        vector<Package> inserted = { makePackage(0, 0, 4, 6) };
        vector<Package> remaining = { makeRemaining(2, 2) };
        check("uma unica caixa",
              corners.getValidCorners(inserted, remaining, W, H),
              { Coordinates2d(0, 6), Coordinates2d(4, 0) });
    }

    cout << "\n" << (failures == 0 ? "Todos os testes passaram." : "Houve falhas.") << endl;
    return failures == 0 ? 0 : 1;
}

#include "Corners3d.h"
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

static int failures = 0;

// Assume que Package tem: position (Coordinates3d), l (largura), a (altura), c (profundidade)
// e um construtor padrao. Ajuste aqui se o seu Package for diferente.
static Package makePackage(int x, int y, int z, int l, int a, int c)
{
    Package p;
    p.position = Coordinates3d(x, y, z);
    p.l = l;
    p.a = a;
    p.c = c;
    return p;
}

// Pacote ainda nao colocado (so as dimensoes importam)
static Package makeRemaining(int l, int a, int c)
{
    return makePackage(0, 0, 0, l, a, c);
}

static void check(const string& name, vector<Coordinates3d> got, vector<Coordinates3d> expected)
{
    auto cmp = [](const Coordinates3d& a, const Coordinates3d& b) { return a < b; };
    sort(got.begin(), got.end(), cmp);
    sort(expected.begin(), expected.end(), cmp);

    bool ok = got.size() == expected.size();
    for (size_t i = 0; ok && i < got.size(); i++)
        ok = got[i].x == expected[i].x && got[i].y == expected[i].y && got[i].z == expected[i].z;

    cout << (ok ? "[OK]   " : "[FAIL] ") << name << endl;
    if (!ok)
    {
        failures++;
        cout << "   esperado:";
        for (auto& c : expected) cout << " (" << c.x << "," << c.y << "," << c.z << ")";
        cout << "\n   obtido:  ";
        for (auto& c : got) cout << " (" << c.x << "," << c.y << "," << c.z << ")";
        cout << endl;
    }
}

int main()
{
    const int W = 10, H = 10, D = 10;
    Corners3d corners;

    // 1) Bin vazio: apenas a origem
    {
        vector<Package> inserted, remaining = { makeRemaining(1, 1, 1) };
        check("bin vazio retorna (0,0,0)",
              corners.getValidCorners(inserted, remaining, W, H, D),
              { Coordinates3d(0, 0, 0) });
    }

    // 2) Uma caixa em (0,0,0) com 4x6x5
    //    z=0: cantos 2D (0,6) e (4,0)
    //    z=5: nenhuma caixa termina depois de 5 -> (0,0), que e novo em relacao ao nivel anterior
    {
        vector<Package> inserted = { makePackage(0, 0, 0, 4, 6, 5) };
        vector<Package> remaining = { makeRemaining(1, 1, 1) };
        check("uma caixa: cantos na frente e ao lado",
              corners.getValidCorners(inserted, remaining, W, H, D),
              { Coordinates3d(0, 6, 0), Coordinates3d(4, 0, 0), Coordinates3d(0, 0, 5) });
    }

    // 3) Duas caixas empilhadas em z (A: z=0..4, B: z=4..8), ambas 4x4
    //    z=0: cantos (0,4) e (4,0)
    //    z=4: mesmos cantos 2D -> falsos cantos (escondidos atras), nao devem ser repetidos
    //    z=8: nenhuma caixa -> (0,0)
    {
        vector<Package> inserted = {
            makePackage(0, 0, 0, 4, 4, 4),
            makePackage(0, 0, 4, 4, 4, 4)
        };
        vector<Package> remaining = { makeRemaining(1, 1, 1) };
        check("falsos cantos sao removidos por dominancia",
              corners.getValidCorners(inserted, remaining, W, H, D),
              { Coordinates3d(0, 4, 0), Coordinates3d(4, 0, 0), Coordinates3d(0, 0, 8) });
    }

    // 4) Caixa que comeca depois de z=0 (z=3..5) ainda entra no nivel z=0,
    //    pois o filtro e "termina depois de z" (z_i + d_i > z)
    //    z=0: cantos (0,4) e (4,0)
    //    z=5: nenhuma caixa -> (0,0)
    {
        vector<Package> inserted = { makePackage(0, 0, 3, 4, 4, 2) };
        vector<Package> remaining = { makeRemaining(1, 1, 1) };
        check("caixa que comeca depois de z=0 conta no nivel z=0",
              corners.getValidCorners(inserted, remaining, W, H, D),
              { Coordinates3d(0, 4, 0), Coordinates3d(4, 0, 0), Coordinates3d(0, 0, 5) });
    }

    // 5) Condicao de parada: menor profundidade restante 6.
    //    z=0: 0+6 <= 10, gera cantos. z=5: 5+6 > 10, para.
    {
        vector<Package> inserted = { makePackage(0, 0, 0, 4, 6, 5) };
        vector<Package> remaining = { makeRemaining(1, 1, 6) };
        check("para quando z + menor profundidade > D",
              corners.getValidCorners(inserted, remaining, W, H, D),
              { Coordinates3d(0, 6, 0), Coordinates3d(4, 0, 0) });
    }

    // 6) Sem caixas restantes: nenhum ponto util
    {
        vector<Package> inserted = { makePackage(0, 0, 0, 4, 6, 5) };
        vector<Package> remaining;
        check("sem caixas restantes: vazio",
              corners.getValidCorners(inserted, remaining, W, H, D),
              {});
    }

    cout << "\n" << (failures == 0 ? "Todos os testes passaram." : "Houve falhas.") << endl;
    return failures == 0 ? 0 : 1;
}

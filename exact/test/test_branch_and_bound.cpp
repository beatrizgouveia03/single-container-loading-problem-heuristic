#include "../src/InstanceManager.h"
#include "../src/BranchAndBound.h"
#include "../src/Types.h"
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

// Uso:
//   ./build/test_branch_and_bound [grupo tipos qtd minDim maxDim C L A limiteMin [unsorted]]
// Exemplo:
//   ./build/test_branch_and_bound t12 12 1 3 6 10 8 8 15
int main(int argc, char **argv)
{
    std::string group = "group_1";
    int itemTypes = 4, quantity = 2, minDim = 1, maxDim = 6;
    int C = 10, L = 8, A = 8;
    int limitMin = 15;
    bool useSort = true;

    if (argc >= 10)
    {
        group     = argv[1];
        itemTypes = atoi(argv[2]);
        quantity  = atoi(argv[3]);
        minDim    = atoi(argv[4]);
        maxDim    = atoi(argv[5]);
        C         = atoi(argv[6]);
        L         = atoi(argv[7]);
        A         = atoi(argv[8]);
        limitMin  = atoi(argv[9]);
    }
    if (argc >= 11 && std::string(argv[10]) == "unsorted") useSort = false;

    InstanceManager im("test/instances");
    fs::path dir = fs::path("test/instances") / group;

    // só gera se a pasta ainda não existe (assim a instância calibrada é reutilizada)
    if (!fs::is_directory(dir))
    {
        cout << "Gerando grupo " << group << "...\n";
        im.generateGroup(group, 1, itemTypes, minDim, maxDim, quantity, quantity, C, L, A);
    }

    Instance ins = im.readInstance(1, dir);
    if (!ins.has_container() || ins.packages_exact.empty())
    {
        cerr << "Instancia invalida ou vazia em " << dir << "\n";
        return 1;
    }

    ll B = ins.c * ins.l * ins.a;
    ll totalVol = 0;
    for (const Package &pk : ins.packages_exact) totalVol += pk.volume;

    cout << "Grupo: " << group << "\n";
    cout << "Container: " << ins.c << "x" << ins.l << "x" << ins.a
         << "  (volume " << B << ")\n";
    cout << "Pacotes: " << ins.packages_exact.size()
         << "  volume total = " << totalVol
         << "  (" << (double)totalVol / B << "x o container)\n";
    cout << "Modo: " << (useSort ? "solveSorted" : "solve (sem ordenar)")
         << "  |  limite: " << limitMin << " min\n" << flush;

    BranchAndBound bab;
    bab.deadline = chrono::steady_clock::now() + chrono::minutes(limitMin);

    vector<Package> p = ins.packages_exact;
    vector<Package> inserted;
    ll F = 0;

    auto t0 = chrono::steady_clock::now();
    if (useSort) bab.solveSorted(p, ins.l, ins.a, ins.c, F, inserted);
    else         bab.solve(p, ins.l, ins.a, ins.c, F, inserted);
    double secs = chrono::duration<double>(chrono::steady_clock::now() - t0).count();

    cout << "\nResultado\n";
    cout << "  F = " << F << "\n";
    cout << "  Ocupacao: " << (100.0 * F / B) << "%\n";
    cout << "  Packages inserted: " << bab.best.size() << " of " << p.size() << "\n";
    for (const Package &pk : bab.best)
    {
        cout << "    (" << pk.c << ", " << pk.l << ", " << pk.a << ")"
             << "  volume = " << pk.volume << "\n";
    }
    cout << "  Tempo: " << secs << " s (" << secs / 60.0 << " min)\n";
    cout << (bab.timedOut ? "  TEMPO ESGOTADO: F pode nao ser o otimo\n"
                          : "  Otimo provado\n");
    return 0;
}
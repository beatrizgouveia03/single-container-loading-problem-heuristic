#include "../src/InstanceManager.h"
#include "../src/BranchAndBound.h"
#include "../src/Types.h"
#include <filesystem>
#include <iostream>

int main()
{
    InstanceManager im("test/instances");
    BranchAndBound bab;


    fs::path dir = "test/instances/group_1";
    vector<Instance> instances;

    if (!fs::is_directory(dir)) 
    {
        instances = im.generateGroup("group_1", 5, 5, 1, 6, 3, 3, 15, 10, 10);
    }
    else
    {
        for(int i=1; i<=5; i++) instances.push_back(im.readInstance(i, "test/instances/group_1"));
        cout << "oi" << instances[0].packages_exact[0].volume << '\n';
    }

    int idx = 1;
    for (Instance ins : instances)
    {

        vector<Package> p = ins.packages_exact;
        vector<Package> inserted;
        ll F = 0;
        bab.solve(p, ins.l, ins.a, ins.c, F, inserted);

        auto best = bab.best;
        
        cout << "Instance " << idx++ << "\n";
        cout << "  F = " << F << "\n";
        cout << "  Packages inserted: " << best.size() << " of " << p.size() << "\n";

        for (const Package& pk : bab.best)
        {
            cout << "    (" << pk.c << ", " << pk.l << ", " << pk.a << ")"
                 << "  volume = " << pk.volume << "\n";
        }
        cout << "\n";
    }

    return 0;
}
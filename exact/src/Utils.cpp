#pragma once
#include <vector>
#include <tuple>
#include "../../heuristic/headers/Package.h"

struct Path
{
    Path(int size, int num = 0)
    {
        inserted_packages = std::vector<int>(size, 0);
        occupied_volume = num;
    }

    std::vector<int> inserted_packages;
    int occupied_volume;
};
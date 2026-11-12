#include <vector>
#include <tuple>
#include "../../heuristic/headers/Package.h"

struct Path
{
    Path(std::vector<int> _inserted_packages, int _occupied_volume)
    {
        inserted_packages = _inserted_packages;
        occupied_volume = _occupied_volume;
    }

    std::vector<int> inserted_packages;
    int occupied_volume;
};
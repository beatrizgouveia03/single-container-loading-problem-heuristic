#ifndef __PCUCEXACT_H__
#define __PCUCEXACT_H__
#include <vector>
#include "../../../heuristic/headers/Package.h"
#include "../header/PackingChecker.h"

class PCUCExact
{
    public:
    std::vector<int> solve_pcuc(std::vector<Package>& packages, int capacity, PackingChecker packingChecker);
};

#endif // __PCUCEXACT_H__
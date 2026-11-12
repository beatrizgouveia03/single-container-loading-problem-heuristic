#include "BagProblemAlgorithm.h"
#include <algorithm>

std::vector<int> BagProblemAlgorithm::get_maximum_subset
(
    std::vector<Package> &package_list, 
    int capacity
)
{   
    std::vector<std::vector<Path>> memo(
    package_list.size(),
    std::vector<Path>(capacity + 1, Path(package_list.size(), -1)));
    auto p = Path(package_list.size());
    return bag_problem_recur(package_list, memo, p, capacity, 0).inserted_packages;
}


Path BagProblemAlgorithm::bag_problem_recur
(
    std::vector<Package> &package_list, 
    std::vector<std::vector<Path>> &memo,
    Path &path,
    int capacity,
    int index
)
{
    if(index == package_list.size())
    {
        return path;
    }

    if(memo[index][capacity].occupied_volume != -1)
    {
        return memo[index][capacity];
    }

    
    Path best_solution = Path(package_list.size(), -1); 
    Package package = package_list[index];
    for(int i=0; i<=std::min(package.quantity, capacity/package.volume); i++) // only add while there is enough capacity and enought quantity
    {
        Path new_path = path;
        new_path.inserted_packages[index] = i;
        new_path.occupied_volume += i*package.volume;

        Path new_solution = bag_problem_recur(package_list, memo, new_path, capacity - (i*package.volume), index + 1);
        if(new_solution.occupied_volume > best_solution.occupied_volume)
        {
            best_solution = new_solution;
        }
    }

    return memo[index][capacity] = best_solution;
}

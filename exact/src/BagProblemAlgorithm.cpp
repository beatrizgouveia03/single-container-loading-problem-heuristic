#include "BagProblemAlgorithm.h"
#include <math.h>

std::vector<int> BagProblemAlgorithm::get_maximum_subset
(
    std::vector<Package> package_list, 
    int capacity
)
{
    std::vector<std::vector<Path>> memo(
    package_list.size(),
    std::vector<Path>(capacity + 1, Path(std::vector<int>(), 0)));
    
    return bag_problem_recur(package_list, 0, capacity, memo, Path(std::vector<int>(package_list.size(), 0), 0)).inserted_packages;
}


Path bag_problem_recur
(
    std::vector<Package> package_list, 
    int index, 
    int capacity,
    std::vector<std::vector<Path>> &memo,
    Path path
)
{
    if(index >= package_list.size())
    {
        return path;
    }

    if(memo[index][capacity].occupied_volume != 0)
    {
        return memo[index][capacity];
    }

    Path no_take = bag_problem_recur(package_list, index + 1, capacity, memo, path);

    Path take = Path(std::vector<int>(), 0);
    auto package = package_list[index];

    if(package.volume <= capacity // checks if the volume cap is reached
    && package.quantity > path.inserted_packages[index]) // checks if there are still packages available for allocation
    {
        path.inserted_packages[index]++;
        path.occupied_volume += package.volume;
        take = bag_problem_recur(package_list, index, capacity - package.volume, memo, path);
    }

    if(take.occupied_volume > no_take.occupied_volume)
    {
        memo[index][capacity] = take;
    }
    else
    {
        memo[index][capacity] = no_take;
    }
    return memo[index][capacity];
}

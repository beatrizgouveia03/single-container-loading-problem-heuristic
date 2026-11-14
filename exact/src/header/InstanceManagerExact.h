#ifndef __INSTANCEMANAGEREXACT_H__
#define __INSTANCEMANAGEREXACT_H__

#include "../../../heuristic/headers/InstanceManager.h"
#include <vector>
#include <string>
#include "../../../heuristic/headers/Package.h"
#include "../../../heuristic/headers/Container.h"

class InstanceManagerExact : public InstanceManager
{
    public:
    
    using InstanceManager::InstanceManager;
    
    vector<Instance> generateGroup(
        const string &group, int count, int itemTypes, int minDimension,
        int maxDimension, int minQuantity, int maxQuantity, int containerC,
        int containerL, int containerA) override;

    Instance generateAt(const fs::path &dir, int id,
                                     int itemTypes, int minDimension,
                                     int maxDimension, int minQuantity,
                                     int maxQuantity, int containerC,
                                     int containerL, int containerA,
                                     const string &group, int iteration) override;
};

#endif // __INSTANCEMANAGEREXACT_H__
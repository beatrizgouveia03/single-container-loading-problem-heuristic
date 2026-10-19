#ifndef INSTANCE_GENERATOR_H
#define INSTANCE_GENERATOR_H

#include "../lib/json.hpp"
#include <string>
#include <vector>

using json = nlohmann::json;
using namespace std;
namespace fs = std::filesystem;

class InstanceManager
{
    public:
    fs::path folder = "instances";

    InstanceManager(string _folder);

    void generate(int packageNumber, int maxDimension, int minDimension);

    void save(json j, string name);

    int randomInt(int min, int max);

    int countFiles(const fs::path& folder);

    vector<vector<int>> readInstance(int instanceIndex);
};

#endif
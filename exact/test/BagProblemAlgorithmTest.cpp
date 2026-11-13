#include "../src/header/BagProblemAlgorithm.h"
#include "../../heuristic/headers/InstanceManager.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;
using namespace std;

static int failures = 0;

static void check(bool condition, const string &message) {
  if (!condition) {
    cerr << "ERRO: " << message << "\n";
    ++failures;
  }
}

void print_subset(vector<int> _subset){
    for(auto item:_subset)
    {
        std::cout << item << " "; 
    }
    std:: cout << "\n";
}

int main()
{

    fs::path root = fs::path("build") / "test_instances";
    const string group_name = "_teste_grupo";
    fs::remove_all(root);
    fs::remove_all(fs::path("saidas") / group_name);
    fs::create_directories(root);

    InstanceManager manager(root.string());

    const int 
    instance_quantity = 5,
    total_types = 5,
    min_quantity = 5,
    max_quantity = 10,
    min_dimension = 1,
    max_dimension = 5,
    container_c = 15,
    container_l = 10,
    container_a = 10;

    vector<Instance> generated = manager.generateGroup
    (
        group_name,
        instance_quantity,
        total_types,
        min_dimension,
        max_dimension,
        min_quantity,
        max_quantity,
        container_c,
        container_l,
        container_a
    );

    BagProblemAlgorithm bagProblemAlgorithm;
    for(Instance instance:generated)
    {
        // conversion from vector to package object
        vector<Package> formated_packages;
        vector<vector<int>> unformated_packages = instance.packages;
        for(vector<int> dimensions:unformated_packages)
        {
            formated_packages.push_back(Package(0,dimensions[0], dimensions[1], dimensions[2]));
        }

        int container_volume = instance.a * instance.c * instance.l;
        auto subset = bagProblemAlgorithm.get_maximum_subset(formated_packages, container_volume);
        
        print_subset(subset);
    }

    return 0;
}

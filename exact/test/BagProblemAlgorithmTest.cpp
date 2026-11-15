#include "../src/header/BagProblemAlgorithm.h"
#include "../src/header/InstanceManagerExact.h"
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

    InstanceManagerExact manager(root.string());

    const int 
    instance_quantity = 5,
    total_types = 10,
    min_quantity = 4,
    max_quantity = 10,
    min_dimension = 1,
    max_dimension = 6,
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
        std::cout << "Dimensões {c,l,a} e quantidade:" << '\n';
        // conversion from vector to package object
        vector<Package> packages = instance.packages_exact;
        for(Package p:packages)
        {
            std::cout << p.c << " " << p.l << " " <<  p.a << " " 
            <<  p.quantity <<"\n";
        }

        int container_volume = container_a * container_c * container_l;
        auto subset = bagProblemAlgorithm.get_maximum_subset(packages, container_volume);
        
        std::cout << "Resultado: " << '\n';
        
        print_subset(subset);

        int ocupado = 0;
        
        for(int i=0; i<packages.size(); i++){
            int p_volume = packages[i].a * packages[i].c * packages[i].l;
            ocupado += p_volume * subset[i];
        }
        
        std::cout << "Volume ocupado: " << ocupado << " Volume total: " << container_volume;
        std::cout << '\n';
    }

    return 0;
}

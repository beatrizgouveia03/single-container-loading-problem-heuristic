#include "../headers/InstanceManager.h"
#include "../lib/json.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <random>
#include <string>

InstanceManager::InstanceManager(string _folder){
    folder = _folder;
}

void InstanceManager::generate(int packageNumber, int maxDimension, int minDimension){
    json instance;
    string fileName = "instance_";
    fileName.append(to_string(countFiles(folder) + 1)).append(".json");

    int c, l, a;
    for(int i=0; i<packageNumber; i++){
        c = randomInt(maxDimension, minDimension);
        l = randomInt(maxDimension, minDimension);
        a = randomInt(maxDimension, minDimension);

        instance[i] = {c,l,a};
    }
    

    save(instance, fileName);
}



void InstanceManager::save(json j, string name){
    fs::create_directories(folder);
    ofstream out(fs::path("instances") / name);
    if (!out) {
        std::cerr << "Could not open file for writing\n";
        return;
    } 
    out << j.dump(4);
}

int InstanceManager::randomInt(int min, int max) {
    static std::mt19937 gen{std::random_device{}()};
    std::uniform_int_distribution<int> dist(min, max);
    return dist(gen);
}

int InstanceManager::countFiles(const fs::path& folder) {
    int count = 0;

    if (!fs::exists(folder) || !fs::is_directory(folder)) {
        std::cerr << "Could not open folder for reading\n";
        return 0;
    }

    for (const auto& entry : fs::directory_iterator(folder)) {
        if (entry.is_regular_file()) {
            ++count;
        }
    }
    return count;
}

vector<vector<int>> InstanceManager::readInstance(int instanceIndex){
    string fileName = "instance_" + to_string(instanceIndex) + ".json";
    fs::path path = fs::path(folder) / fileName;

    cout << fileName << "\n";
    ifstream file(path);
    if(!file.is_open()){
        cerr << "Could not open file \n";
        return vector<vector<int>>();
    }

    json j = json::parse(file);
    vector<vector<int>> instanceVector;
    for(const auto& package : j){
        instanceVector.push_back(package.get<vector<int>>());
    }

    return instanceVector;

}
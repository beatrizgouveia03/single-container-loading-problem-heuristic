#include "../headers/InstanceManager.h"
#include "../lib/json.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <regex>
#include <string>
#include <vector>

InstanceManager::InstanceManager(string _folder) { folder = _folder; }

Instance InstanceManager::generate(int packageNumber, int minDimension,
                                   int maxDimension, int containerC,
                                   int containerL, int containerA) {
  if (packageNumber < 1 || minDimension < 1 || maxDimension < minDimension ||
      containerC < 1 || containerL < 1 || containerA < 1) {
    std::cerr << "Could not generate instance: invalid arguments\n";
    return Instance();
  }

  Instance instance;
  instance.id = nextIndex();
  instance.file = (fs::path(folder) / ("instance_" + to_string(instance.id) +
                                       ".json"))
                      .string();
  instance.c = containerC;
  instance.l = containerL;
  instance.a = containerA;

  for (int i = 0; i < packageNumber; i++) {
    instance.packages.push_back({randomInt(minDimension, maxDimension),
                                 randomInt(minDimension, maxDimension),
                                 randomInt(minDimension, maxDimension)});
  }

  json j;
  j["container"] = {containerC, containerL, containerA};
  j["packages"] = instance.packages;
  save(j, "instance_" + to_string(instance.id) + ".json");

  return instance;
}

void InstanceManager::save(json j, string name) {
  fs::create_directories(folder);
  ofstream out(fs::path(folder) / name);
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

int InstanceManager::countFiles(const fs::path &folder) {
  int count = 0;

  if (!fs::exists(folder) || !fs::is_directory(folder)) {
    std::cerr << "Could not open folder for reading\n";
    return 0;
  }

  for (const auto &entry : fs::directory_iterator(folder)) {
    if (entry.is_regular_file()) {
      ++count;
    }
  }
  return count;
}

int InstanceManager::nextIndex() {
  static const std::regex instanceName(R"(^instance_(\d+)\.json$)");

  int maxId = 0;
  if (fs::exists(folder) && fs::is_directory(folder)) {
    for (const auto &entry : fs::directory_iterator(folder)) {
      std::smatch match;
      string name = entry.path().filename().string();
      if (entry.is_regular_file() &&
          std::regex_match(name, match, instanceName)) {
        maxId = std::max(maxId, std::stoi(match[1].str()));
      }
    }
  }
  return maxId + 1;
}

vector<int> InstanceManager::listIds() {
  static const std::regex instanceName(R"(^instance_(\d+)\.json$)");

  vector<int> ids;
  if (fs::exists(folder) && fs::is_directory(folder)) {
    for (const auto &entry : fs::directory_iterator(folder)) {
      std::smatch match;
      string name = entry.path().filename().string();
      if (entry.is_regular_file() &&
          std::regex_match(name, match, instanceName)) {
        ids.push_back(std::stoi(match[1].str()));
      }
    }
  }
  std::sort(ids.begin(), ids.end());
  return ids;
}

Instance InstanceManager::readInstance(int instanceIndex) {
  Instance instance;
  instance.id = instanceIndex;

  string fileName = "instance_" + to_string(instanceIndex) + ".json";
  fs::path path = fs::path(folder) / fileName;
  instance.file = path.string();

  ifstream file(path);
  if (!file.is_open()) {
    cerr << "Could not open file " << path << "\n";
    return Instance();
  }

  json j = json::parse(file, nullptr, false);
  if (j.is_discarded()) {
    cerr << "Invalid JSON in " << path << "\n";
    return Instance();
  }

  // Formato novo: {"container": [C, L, A], "packages": [[c, l, a], ...]}
  if (j.is_object() && j.contains("container") && j.contains("packages")) {
    vector<int> dims = j["container"].get<vector<int>>();
    if (dims.size() != 3) {
      cerr << "Invalid container dimensions in " << path << "\n";
      return Instance();
    }
    instance.c = dims[0];
    instance.l = dims[1];
    instance.a = dims[2];
    for (const auto &package : j["packages"]) {
      instance.packages.push_back(package.get<vector<int>>());
    }
    return instance;
  }

  // Formato antigo: só a lista de pacotes ([[c, l, a], ...]), sem contêiner.
  if (j.is_array()) {
    for (const auto &package : j) {
      instance.packages.push_back(package.get<vector<int>>());
    }
    return instance;
  }

  cerr << "Unknown instance format in " << path << "\n";
  return Instance();
}

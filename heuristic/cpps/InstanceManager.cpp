#include "../headers/InstanceManager.h"
#include "../lib/json.hpp"
#include <algorithm>
#include <cctype>
#include <climits>
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
  return generateAt(folder, nextIndex(folder), packageNumber, minDimension,
                    maxDimension, containerC, containerL, containerA);
}

Instance InstanceManager::generateAt(const fs::path &dir, int id,
                                     int packageNumber, int minDimension,
                                     int maxDimension, int containerC,
                                     int containerL, int containerA,
                                     const string &group, int iteration) {
  if (packageNumber < 1 || minDimension < 1 || maxDimension < minDimension ||
      containerC < 1 || containerL < 1 || containerA < 1) {
    std::cerr << "Could not generate instance: invalid arguments\n";
    return Instance();
  }

  Instance instance;
  instance.id = id;
  instance.file =
      (dir / ("instance_" + to_string(instance.id) + ".json")).string();
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
  if (!group.empty()) {
    j["grupo"] = group;
    j["iteracao"] = iteration;
  }
  j["min"] = minDimension;
  j["max"] = maxDimension;
  saveAt(dir, j, "instance_" + to_string(instance.id) + ".json");

  return instance;
}

vector<Instance> InstanceManager::generateGroup(
    const string &group, int count, int packageNumber, int minDimension,
    int maxDimension, int containerC, int containerL, int containerA) {
  vector<Instance> instances;

  if (!validGroupName(group)) {
    std::cerr << "Nome de grupo inválido: '" << group
              << "' (use letras, números, '_' ou '-').\n";
    return instances;
  }
  if (count < 1) {
    std::cerr << "Could not generate group: count must be >= 1\n";
    return instances;
  }
  if (packageNumber < 1 || minDimension < 1 || maxDimension < minDimension ||
      containerC < 1 || containerL < 1 || containerA < 1) {
    std::cerr << "Could not generate group: invalid arguments\n";
    return instances;
  }

  fs::path dir = groupFolder(group);
  if (!listIds(dir).empty()) {
    std::cerr << "O grupo '" << group << "' já existe em " << dir.string()
              << ".\nEscolha outro nome ou remova a pasta para recriá-lo.\n";
    return instances;
  }

  // As dimensões da última iteração são a base multiplicada por <count>.
  // Conferimos antes de gerar para não estourar int no meio do caminho
  // (Package::volume é int, então o maior pacote precisa caber em int).
  long last = count;
  long last_max_dimension = (long)maxDimension * last;
  if (last_max_dimension > INT_MAX || (long)containerC * last > INT_MAX ||
      (long)containerL * last > INT_MAX || (long)containerA * last > INT_MAX) {
    std::cerr << "Could not generate group: dimensions overflow for " << count
              << " iterations\n";
    return instances;
  }
  if (last_max_dimension > 0 &&
      last_max_dimension >
          (long)INT_MAX / last_max_dimension / last_max_dimension) {
    std::cerr << "Could not generate group: package volume ("
              << last_max_dimension << "^3) overflows int\n";
    return instances;
  }

  fs::create_directories(dir);
  for (int i = 1; i <= count; i++) {
    const int factor = i;
    Instance instance =
        generateAt(dir, i, packageNumber, minDimension * factor,
                   maxDimension * factor, containerC * factor,
                   containerL * factor, containerA * factor, group, i);
    if (instance.packages.empty()) {
      std::cerr << "Falha ao gerar a instância " << i << " do grupo '" << group
                << "'.\n";
      return {};
    }
    instances.push_back(instance);
  }

  // Metadados do grupo (não é uma instância; o leitor ignora este arquivo).
  json meta;
  meta["grupo"] = group;
  meta["instancias"] = count;
  meta["pacotes"] = packageNumber;
  meta["min_base"] = minDimension;
  meta["max_base"] = maxDimension;
  meta["container_base"] = {containerC, containerL, containerA};
  meta["regra_crescimento"] = "iteracao i: pacotes [min*i, max*i], container "
                              "* i nas tres dimensoes";
  saveAt(dir, meta, "grupo.json");

  return instances;
}

void InstanceManager::save(json j, string name) { saveAt(folder, j, name); }

void InstanceManager::saveAt(const fs::path &dir, json j, const string &name) {
  fs::create_directories(dir);
  ofstream out(dir / name);
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

int InstanceManager::countFiles(const fs::path &dir) {
  int count = 0;

  if (!fs::exists(dir) || !fs::is_directory(dir)) {
    std::cerr << "Could not open folder for reading\n";
    return 0;
  }

  for (const auto &entry : fs::directory_iterator(dir)) {
    if (entry.is_regular_file()) {
      ++count;
    }
  }
  return count;
}

int InstanceManager::nextIndex() { return nextIndex(folder); }

int InstanceManager::nextIndex(const fs::path &dir) {
  static const std::regex instanceName(R"(^instance_(\d+)\.json$)");

  int maxId = 0;
  if (fs::exists(dir) && fs::is_directory(dir)) {
    for (const auto &entry : fs::directory_iterator(dir)) {
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

vector<int> InstanceManager::listIds() { return listIds(folder); }

vector<int> InstanceManager::listIds(const fs::path &dir) {
  static const std::regex instanceName(R"(^instance_(\d+)\.json$)");

  vector<int> ids;
  if (fs::exists(dir) && fs::is_directory(dir)) {
    for (const auto &entry : fs::directory_iterator(dir)) {
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
  return readInstance(folder, instanceIndex);
}

Instance InstanceManager::readInstance(const fs::path &dir,
                                       int instanceIndex) {
  Instance instance;
  instance.id = instanceIndex;

  string fileName = "instance_" + to_string(instanceIndex) + ".json";
  fs::path path = dir / fileName;
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

fs::path InstanceManager::groupFolder(const string &group) const {
  return folder / group;
}

vector<string> InstanceManager::listGroups() const {
  vector<string> groups;
  if (!fs::exists(folder) || !fs::is_directory(folder)) {
    return groups;
  }
  for (const auto &entry : fs::directory_iterator(folder)) {
    if (entry.is_directory()) {
      groups.push_back(entry.path().filename().string());
    }
  }
  std::sort(groups.begin(), groups.end());
  return groups;
}

bool InstanceManager::validGroupName(const string &group) {
  if (group.empty()) {
    return false;
  }
  for (char ch : group) {
    unsigned char uc = static_cast<unsigned char>(ch);
    if (!std::isalnum(uc) && ch != '_' && ch != '-') {
      return false;
    }
  }
  return true;
}

#include "header/InstanceManagerExact.h"
#include "../../heuristic/headers/InstanceManager.h"
#include <iostream>
#include <climits>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

vector<Instance> InstanceManagerExact::generateGroup(
    const string &group, int count, int itemTypes, int minDimension,
    int maxDimension, int minQuantity, int maxQuantity, int containerC,
    int containerL, int containerA) 
{
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
  if (itemTypes < 1 || minDimension < 1 || maxDimension < minDimension ||
      minQuantity < 1 || maxQuantity < minQuantity || containerC < 1 ||
      containerL < 1 || containerA < 1) {
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
  // As quantidades não crescem com a iteração, então não entram nesta checagem.
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
    Instance instance = InstanceManagerExact::generateAt(
        dir, i, itemTypes, minDimension * factor, maxDimension * factor,
        minQuantity, maxQuantity, containerC * factor, containerL * factor,
        containerA * factor, group, i);
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
  meta["tipos_de_itens"] = itemTypes;
  meta["min_base"] = minDimension;
  meta["max_base"] = maxDimension;
  meta["min_qtd"] = minQuantity;
  meta["max_qtd"] = maxQuantity;
  meta["container_base"] = {containerC, containerL, containerA};
  meta["regra_crescimento"] = "iteracao i: pacotes [min*i, max*i], container "
                              "* i nas tres dimensoes; quantidades por tipo "
                              "fixas em [min_qtd, max_qtd]";
  saveAt(dir, meta, "grupo.json");

  return instances;
}

Instance InstanceManagerExact::generateAt(const fs::path &dir, int id,
                                     int itemTypes, int minDimension,
                                     int maxDimension, int minQuantity,
                                     int maxQuantity, int containerC,
                                     int containerL, int containerA,
                                     const string &group, int iteration) {
  if (itemTypes < 1 || minDimension < 1 || maxDimension < minDimension ||
      minQuantity < 1 || maxQuantity < minQuantity || containerC < 1 ||
      containerL < 1 || containerA < 1) {
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

  // Cada tipo tem dimensões fixas e é repetido uma quantidade aleatória de vezes.
  json types = json::array();
  for (int t = 0; t < itemTypes; t++) {
    vector<int> dims = {randomInt(minDimension, maxDimension),
                        randomInt(minDimension, maxDimension),
                        randomInt(minDimension, maxDimension)};
    int quantity = randomInt(minQuantity, maxQuantity);

    instance.packages.push_back(dims);
    instance.packages_exact.push_back(Package(0, dims[0], dims[1], dims[2], quantity));
    
    types.push_back({{"dimensoes", dims}, {"quantidade", quantity}});
  }

  json j;
  j["container"] = {containerC, containerL, containerA};
  j["packages"] = instance.packages;
  j["tipos"] = types; // metadado informativo; o leitor ignora
  if (!group.empty()) {
    j["grupo"] = group;
    j["iteracao"] = iteration;
  }
  j["min"] = minDimension;
  j["max"] = maxDimension;
  j["min_qtd"] = minQuantity;
  j["max_qtd"] = maxQuantity;
  saveAt(dir, j, "instance_" + to_string(instance.id) + ".json");

  return instance;
}
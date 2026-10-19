#ifndef INSTANCE_GENERATOR_H
#define INSTANCE_GENERATOR_H

#include "../lib/json.hpp"
#include <filesystem>
#include <string>
#include <vector>

using json = nlohmann::json;
using namespace std;
namespace fs = std::filesystem;

// Uma instância de teste: as dimensões do contêiner e as dimensões de cada
// pacote. Antes só os pacotes eram salvos; agora o contêiner vem junto.
struct Instance {
  int id = 0;                  // identificador (instance_<id>.json)
  string file;                 // caminho do arquivo da instância
  int c = 0, l = 0, a = 0;     // dimensões do contêiner
  vector<vector<int>> packages; // {c, l, a} de cada pacote

  bool has_container() const { return c > 0 && l > 0 && a > 0; }
};

class InstanceManager {
public:
  fs::path folder = "instances";

  InstanceManager(string _folder);

  // Gera <packageNumber> pacotes com dimensões aleatórias entre minDimension e
  // maxDimension e salva a instância junto das dimensões do contêiner.
  Instance generate(int packageNumber, int minDimension, int maxDimension,
                    int containerC, int containerL, int containerA);

  void save(json j, string name);

  int randomInt(int min, int max);

  int countFiles(const fs::path &folder);

  // Próximo índice livre (maior instance_<n>.json existente + 1).
  int nextIndex();

  // Ids de todas as instâncias salvas, em ordem crescente.
  vector<int> listIds();

  Instance readInstance(int instanceIndex);
};

#endif

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

  Instance generate(int packageNumber, int minDimension, int maxDimension,
                    int containerC, int containerL, int containerA);

  vector<Instance> generateGroup(const string &group, int count,
                                 int packageNumber, int minDimension,
                                 int maxDimension, int containerC,
                                 int containerL, int containerA);

  void save(json j, string name);

  int randomInt(int min, int max);

  int countFiles(const fs::path &folder);

  // Próximo índice livre (maior instance_<n>.json existente + 1).
  int nextIndex();
  int nextIndex(const fs::path &dir);

  // Ids de todas as instâncias salvas, em ordem crescente.
  vector<int> listIds();
  vector<int> listIds(const fs::path &dir);

  Instance readInstance(int instanceIndex);
  Instance readInstance(const fs::path &dir, int instanceIndex);

  // Pasta de um grupo (instances/<group>).
  fs::path groupFolder(const string &group) const;

  // Nomes dos grupos (subpastas de instances/), em ordem alfabética.
  vector<string> listGroups() const;

  // Um nome de grupo válido não pode vazar para fora de instances/
  // (letras, números, '_' e '-'; sem acentos, espaços ou separadores).
  static bool validGroupName(const string &group);

private:
  // Salva uma instância em <dir>/instance_<id>.json. group/iteration são
  // gravados como metadados quando informados (grupos).
  Instance generateAt(const fs::path &dir, int id, int packageNumber,
                      int minDimension, int maxDimension, int containerC,
                      int containerL, int containerA, const string &group = "",
                      int iteration = 0);

  void saveAt(const fs::path &dir, json j, const string &name);
};

#endif

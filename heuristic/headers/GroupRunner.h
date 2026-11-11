#ifndef GROUP_RUNNER_H
#define GROUP_RUNNER_H

#include "InstanceManager.h"
#include <filesystem>
#include <string>
#include <vector>

using namespace std;
namespace fs = std::filesystem;

// Desempenho de uma instância do grupo. Ocupação em porcentagem (0..100).
struct InstancePerformance {
  int id = 0;
  int packages = 0;
  int allocated = 0;
  long volume_used = 0;  // volume preenchido (u³)
  long volume_total = 0; // volume do contêiner (u³)
  double occupancy = 0.0;
};

struct GroupRunResult {
  string group;
  fs::path folder;
  fs::path performance_file;
  vector<InstancePerformance> instances;
  double mean_occupancy = 0.0;
  bool ok() const { return !instances.empty(); }
};

class GroupRunner {
public:
  // Executa todas as instâncias do grupo e grava:
  //   saidas/<grupo>/saida_<id>.txt   relatório de cada instância
  //   instances/<grupo>/desempenho.csv volume preenchido por id
  // verbose = false silencia o log detalhado da heurística.
  GroupRunResult run(InstanceManager &manager, const string &group,
                     bool verbose = false);

  // Escreve o CSV de desempenho do grupo:
  //   id,pacotes,alocados,volume_preenchido,volume_total,ocupacao_pct
  static void write_performance_csv(
      const fs::path &path, const vector<InstancePerformance> &instances);
};

#endif

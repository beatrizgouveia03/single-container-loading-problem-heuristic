#include "../headers/GroupRunner.h"
#include "../headers/InstanceManager.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace std;

static int failures = 0;

static void check(bool condition, const string &message) {
  if (!condition) {
    cerr << "ERRO: " << message << "\n";
    ++failures;
  }
}

int main() {
  // Tudo é criado em build/test_instances para não sujar instances/ no git.
  fs::path root = fs::path("build") / "test_instances";
  const string group_name = "_teste_grupo";
  fs::remove_all(root);
  fs::remove_all(fs::path("saidas") / group_name);
  fs::create_directories(root);

  InstanceManager manager(root.string());

  // 1) Instância avulsa no formato novo (com as dimensões do contêiner),
  //    relida do disco para conferir o que foi salvo.
  Instance generated = manager.generate(10, 1, 4, 15, 10, 10);
  cout << "Instância gerada: " << generated.file << "\n";
  check(generated.id == 1, "a primeira instância avulsa deve ter id 1");

  Instance loaded = manager.readInstance(generated.id);
  check(loaded.has_container(), "contêiner não foi lido de volta");
  check(loaded.packages == generated.packages, "os pacotes lidos diferem dos gerados");
  check(loaded.c == 15 && loaded.l == 10 && loaded.a == 10,
        "dimensões do contêiner lidas erradas");

  cout << "Contêiner: " << loaded.c << " x " << loaded.l << " x " << loaded.a
       << "\nPacotes: " << loaded.packages.size() << "\n";

  // 2) Grupo crescente: a instância i usa pacotes em [min*i, max*i] e
  //    contêiner base*i em todas as dimensões.
  const int count = 5;
  const int packages = 8;
  const int min_base = 1, max_base = 3;
  const int base_c = 15, base_l = 10, base_a = 10;

  vector<Instance> group =
      manager.generateGroup(group_name, count, packages, min_base, max_base,
                            base_c, base_l, base_a);
  check((int)group.size() == count, "o grupo deve ter " + to_string(count) +
                                        " instâncias");
  check(fs::exists(manager.groupFolder(group_name) / "grupo.json"),
        "grupo.json não foi salvo");
  check(manager.listIds(manager.groupFolder(group_name)).size() ==
            (size_t)count,
        "quantidade de instance_*.json no grupo");
  check(!fs::exists(manager.groupFolder(group_name) / "instance_6.json"),
        "não deve existir uma instância além de <count>");

  for (int i = 1; i <= count; i++) {
    Instance instance = manager.readInstance(manager.groupFolder(group_name), i);
    check(instance.c == base_c * i && instance.l == base_l * i &&
              instance.a == base_a * i,
          "contêiner da instância " + to_string(i) + " não escala por i");
    check((int)instance.packages.size() == packages,
          "número de pacotes da instância " + to_string(i));
    for (const auto &package : instance.packages) {
      for (int dimension : package) {
        check(dimension >= min_base * i && dimension <= max_base * i,
              "dimensão " + to_string(dimension) + " fora da faixa da iteracao " +
                  to_string(i));
      }
    }
  }

  // Recriar um grupo existente não pode sobrescrever o que já foi gerado.
  vector<Instance> again =
      manager.generateGroup(group_name, 2, 1, 1, 1, 1, 1, 1);
  check(again.empty(), "recriar um grupo existente deve falhar");
  check(!fs::exists(manager.groupFolder(group_name) / "instance_6.json"),
        "o grupo existente não pode ser alterado");

  // Nome de grupo não pode escapar de instances/.
  check(manager
            .generateGroup("../fora", 1, 1, 1, 1, 1, 1, 1)
            .empty(),
        "nome de grupo inválido deve ser recusado");
  check(!fs::exists(fs::path("build") / "fora"),
        "nenhuma pasta pode ser criada fora do grupo");

  // 3) Rodar o grupo inteiro e conferir o arquivo de desempenho.
  GroupRunner runner;
  GroupRunResult result = runner.run(manager, group_name);
  check(result.ok(), "o grupo deve ter sido executado");
  check(result.instances.size() == (size_t)count,
        "todas as instâncias do grupo devem ser executadas");
  check(fs::exists(result.performance_file), "desempenho.csv não foi criado");
  check(fs::exists(fs::path("saidas") / group_name / "saida_1.txt"),
        "relatório da instância 1 não foi criado");

  ifstream csv(result.performance_file);
  string header;
  getline(csv, header);
  check(header ==
            "id,pacotes,alocados,volume_preenchido,volume_total,ocupacao_pct",
        "cabeçalho do CSV de desempenho");

  string line;
  int rows = 0;
  while (getline(csv, line)) {
    if (line.empty()) {
      continue;
    }
    ++rows;

    vector<string> fields;
    stringstream stream(line);
    string field;
    while (getline(stream, field, ',')) {
      fields.push_back(field);
    }
    check(fields.size() == 6, "cada linha do CSV deve ter 6 colunas");
    if (fields.size() != 6) {
      continue;
    }

    int id = stoi(fields[0]);
    check(id == rows, "os ids do CSV devem estar em ordem a partir de 1");

    long used = stol(fields[3]);
    long total = stol(fields[4]);
    double occupancy = stod(fields[5]);
    long expected_total = (long)base_c * id * base_l * id * base_a * id;
    check(total == expected_total,
          "volume total do id " + to_string(id) + " não escala por i^3");
    check(used >= 0 && used <= total,
          "volume usado fora do contêiner no id " + to_string(id));
    check(occupancy >= 0.0 && occupancy <= 100.0,
          "ocupação fora de 0..100 no id " + to_string(id));
  }
  check(rows == count, "o CSV deve ter uma linha por instância");

  fs::remove_all(root);
  fs::remove_all(fs::path("saidas") / group_name);

  if (failures == 0) {
    cout << "OK\n";
    return 0;
  }
  cerr << failures << " verificação(ões) falharam\n";
  return 1;
}

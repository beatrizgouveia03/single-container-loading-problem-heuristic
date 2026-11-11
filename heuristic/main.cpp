// Console de comandos para testar a heurística de carregamento.
//
//   criar <pacotes> <min> <max> <C> <L> <A>   cria uma instância aleatória
//   listar                                    lista as instâncias salvas
//   rodar <id>                                roda a heurística na instância <id>
//   criar-grupo <nome> <n> <pacotes> <min> <max> <C> <L> <A>
//                                             cria <n> instâncias em
//                                             instances/<nome>/ crescendo as
//                                             dimensões por iteração
//   listar-grupos                             lista os grupos de instâncias
//   listar-grupo <nome>                       lista as instâncias do grupo
//   rodar-grupo <nome>                        roda o grupo inteiro e grava o
//                                             desempenho por id
//   demo                                      roda o exemplo embutido (15x10x10)
//   ajuda                                     mostra os comandos
//   sair                                      encerra
//
// Também aceita comandos fora do console: ./exec_trabalho rodar <id>,
// ./exec_trabalho rodar-grupo <nome>, ./exec_trabalho criar-grupo ... ou
// ./exec_trabalho demo (útil para scripts e para o Makefile).

#include "Container.h"
#include "GroupRunner.h"
#include "Heuristic.h"
#include "InstanceManager.h"
#include "Package.h"
#include "Utils.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// ---------------------------------------------------------------------------
// Utilitários da interface
// ---------------------------------------------------------------------------

static vector<string> split(const string &line) {
  istringstream in(line);
  vector<string> tokens;
  string token;
  while (in >> token) {
    tokens.push_back(token);
  }
  return tokens;
}

static bool to_int(const string &text, int &out) {
  try {
    size_t consumed = 0;
    out = stoi(text, &consumed);
    return consumed == text.size();
  } catch (...) {
    return false;
  }
}

static void print_help() {
  cout << "Comandos:\n"
       << "  criar <pacotes> <min> <max> <C> <L> <A>\n"
       << "      Cria uma instância com <pacotes> pacotes de dimensões\n"
       << "      aleatórias entre <min> e <max>, para um contêiner C x L x A.\n"
       << "      Ex.: criar 10 2 5 15 10 10\n"
       << "  listar                    Lista as instâncias salvas.\n"
       << "  rodar <id>                Roda a heurística na instância <id>.\n"
       << "  criar-grupo <nome> <n> <pacotes> <min> <max> <C> <L> <A>\n"
       << "      Cria <n> instâncias em instances/<nome>/; a instância i usa\n"
       << "      pacotes entre <min>*i e <max>*i e contêiner C*i x L*i x A*i.\n"
       << "      Ex.: criar-grupo escala 50 20 1 5 15 10 10\n"
       << "  listar-grupos             Lista os grupos e quantas instâncias têm.\n"
       << "  listar-grupo <nome>       Lista as instâncias de um grupo.\n"
       << "  rodar-grupo <nome>        Roda todas as instâncias do grupo e grava\n"
       << "      instances/<nome>/desempenho.csv (volume por id).\n"
       << "  demo                      Roda o exemplo embutido (15x10x10).\n"
       << "  ajuda                     Mostra esta ajuda.\n"
       << "  sair                      Encerra o programa.\n";
}

// ---------------------------------------------------------------------------
// Comandos
// ---------------------------------------------------------------------------

static void cmd_criar(InstanceManager &manager, const vector<string> &args) {
  if (args.size() != 7) {
    cerr << "Uso: criar <pacotes> <min> <max> <C> <L> <A>\n";
    return;
  }

  int n, min_dim, max_dim, c, l, a;
  if (!to_int(args[1], n) || !to_int(args[2], min_dim) ||
      !to_int(args[3], max_dim) || !to_int(args[4], c) || !to_int(args[5], l) ||
      !to_int(args[6], a)) {
    cerr << "Erro: todos os argumentos devem ser números inteiros.\n";
    return;
  }
  if (n < 1) {
    cerr << "Erro: o número de pacotes deve ser >= 1.\n";
    return;
  }
  if (min_dim < 1 || max_dim < 1) {
    cerr << "Erro: as dimensões dos pacotes devem ser >= 1.\n";
    return;
  }
  if (max_dim < min_dim) {
    cerr << "Erro: <max> deve ser maior ou igual a <min>.\n";
    return;
  }
  if (c < 1 || l < 1 || a < 1) {
    cerr << "Erro: as dimensões do contêiner devem ser >= 1.\n";
    return;
  }

  Instance instance = manager.generate(n, min_dim, max_dim, c, l, a);
  if (instance.packages.empty()) {
    cerr << "Erro ao criar a instância.\n";
    return;
  }

  long vol_container = (long)c * l * a;
  long vol_packages = 0;
  for (const auto &p : instance.packages) {
    vol_packages += (long)p[0] * p[1] * p[2];
  }

  cout << "Instância " << instance.id << " salva em " << instance.file << "\n";
  cout << "  contêiner: " << c << " x " << l << " x " << a << " (volume "
       << vol_container << ")\n";
  cout << "  pacotes:   " << n << " com dimensões entre " << min_dim << " e "
       << max_dim << " (volume total " << vol_packages << ")\n";
  if (vol_packages > vol_container) {
    cout << "  aviso: a soma dos volumes (" << vol_packages
         << ") excede o volume do contêiner (" << vol_container
         << ") — nem todos os pacotes vão caber.\n";
  }
  cout << "Para testar:  rodar " << instance.id << "\n";
}

static void cmd_listar(InstanceManager &manager) {
  vector<int> ids = manager.listIds();
  if (ids.empty()) {
    cout << "Nenhuma instância em " << manager.folder.string() << ".\n";
    return;
  }

  cout << "Instâncias em " << manager.folder.string() << ":\n";
  for (int id : ids) {
    Instance instance = manager.readInstance(id);
    cout << "  [" << id << "] ";
    if (instance.packages.empty() && !instance.has_container()) {
      cout << "(inválida ou ilegível)\n";
      continue;
    }
    if (instance.has_container()) {
      cout << "contêiner " << instance.c << "x" << instance.l << "x"
           << instance.a << " | " << instance.packages.size() << " pacotes";
    } else {
      cout << "formato antigo, sem contêiner | " << instance.packages.size()
           << " pacotes";
    }
    cout << "\n";
  }
}

static int cmd_listar_grupos(InstanceManager &manager) {
  vector<string> groups = manager.listGroups();
  if (groups.empty()) {
    cout << "Nenhum grupo em " << manager.folder.string() << ".\n";
    return 0;
  }

  cout << "Grupos em " << manager.folder.string() << ":\n";
  int printed = 0;
  for (const auto &group : groups) {
    fs::path dir = manager.groupFolder(group);
    vector<int> ids = manager.listIds(dir);
    if (ids.empty()) {
      continue; // pasta que não contém instâncias
    }
    cout << "  " << group << " | " << ids.size() << " instâncias";
    if (fs::exists(dir / "desempenho.csv")) {
      cout << " | desempenho.csv";
    }
    cout << "\n";
    ++printed;
  }
  if (printed == 0) {
    cout << "  (nenhum grupo com instâncias)\n";
  }
  return 0;
}

static int cmd_listar_grupo(InstanceManager &manager,
                            const vector<string> &args) {
  if (args.size() != 2) {
    cerr << "Uso: listar-grupo <nome>\n";
    return 1;
  }

  fs::path dir = manager.groupFolder(args[1]);
  vector<int> ids = manager.listIds(dir);
  if (ids.empty()) {
    cerr << "Nenhuma instância no grupo '" << args[1] << "' (" << dir.string()
         << ").\n";
    return 1;
  }

  cout << "Instâncias do grupo '" << args[1] << "' em " << dir.string()
       << ":\n";
  for (int id : ids) {
    Instance instance = manager.readInstance(dir, id);
    cout << "  [" << id << "] ";
    if (instance.packages.empty() && !instance.has_container()) {
      cout << "(inválida ou ilegível)\n";
      continue;
    }
    if (instance.has_container()) {
      cout << "contêiner " << instance.c << "x" << instance.l << "x"
           << instance.a << " | " << instance.packages.size() << " pacotes";
    } else {
      cout << "formato antigo, sem contêiner | " << instance.packages.size()
           << " pacotes";
    }
    cout << "\n";
  }
  return 0;
}

static int cmd_criar_grupo(InstanceManager &manager,
                           const vector<string> &args) {
  if (args.size() != 9) {
    cerr << "Uso: criar-grupo <nome> <n> <pacotes> <min> <max> <C> <L> <A>\n";
    return 1;
  }

  const string &group = args[1];
  int n_instances, n_packages, min_dim, max_dim, c, l, a;
  if (!to_int(args[2], n_instances) || !to_int(args[3], n_packages) ||
      !to_int(args[4], min_dim) || !to_int(args[5], max_dim) ||
      !to_int(args[6], c) || !to_int(args[7], l) || !to_int(args[8], a)) {
    cerr << "Erro: todos os argumentos após <nome> devem ser inteiros.\n";
    return 1;
  }
  if (n_instances < 1) {
    cerr << "Erro: o número de instâncias deve ser >= 1.\n";
    return 1;
  }
  if (n_packages < 1) {
    cerr << "Erro: o número de pacotes deve ser >= 1.\n";
    return 1;
  }
  if (min_dim < 1 || max_dim < 1) {
    cerr << "Erro: as dimensões dos pacotes devem ser >= 1.\n";
    return 1;
  }
  if (max_dim < min_dim) {
    cerr << "Erro: <max> deve ser maior ou igual a <min>.\n";
    return 1;
  }
  if (c < 1 || l < 1 || a < 1) {
    cerr << "Erro: as dimensões do contêiner devem ser >= 1.\n";
    return 1;
  }

  vector<Instance> generated = manager.generateGroup(
      group, n_instances, n_packages, min_dim, max_dim, c, l, a);
  if (generated.empty()) {
    return 1; // o motivo já foi impresso pelo generateGroup
  }

  long last = n_instances;
  cout << "Grupo '" << group << "' criado em "
       << manager.groupFolder(group).string() << "\n";
  cout << "  instâncias: " << n_instances << ", pacotes por instância: "
       << n_packages << "\n";
  cout << "  regra: na iteração i, pacotes entre " << min_dim << "*i e "
       << max_dim << "*i e contêiner " << c << "*i x " << l << "*i x " << a
       << "*i\n";
  cout << "  primeira: pacotes " << min_dim << ".." << max_dim
       << ", contêiner " << c << "x" << l << "x" << a << "\n";
  cout << "  última:   pacotes " << (long)min_dim * last << ".."
       << (long)max_dim * last << ", contêiner " << (long)c * last << "x"
       << (long)l * last << "x" << (long)a * last << "\n";
  cout << "Para rodar as " << n_instances
       << " de uma vez:  rodar-grupo " << group << "\n";
  return 0;
}

static int cmd_rodar_grupo(InstanceManager &manager,
                           const vector<string> &args) {
  if (args.size() != 2) {
    cerr << "Uso: rodar-grupo <nome>\n";
    return 1;
  }

  GroupRunner runner;
  GroupRunResult result = runner.run(manager, args[1]);
  if (!result.ok()) {
    return 1;
  }

  cout << "\n=== Desempenho do grupo '" << result.group << "' ===\n";
  cout << "Instâncias executadas: " << result.instances.size() << "\n";
  cout << fixed << setprecision(2);
  cout << "Ocupação média: " << result.mean_occupancy << " %\n";
  cout << "Desempenho por id: " << result.performance_file.string() << "\n";
  cout << "Relatórios:        saidas/" << result.group << "/saida_<id>.txt\n";
  cout << "Gráfico:           make grafico GRUPO=" << result.group << "\n";
  return 0;
}

static int cmd_rodar(InstanceManager &manager, const vector<string> &args) {
  if (args.size() != 2) {
    cerr << "Uso: rodar <id>\n";
    return 1;
  }

  int id;
  if (!to_int(args[1], id) || id < 1) {
    cerr << "Erro: <id> deve ser um número inteiro positivo.\n";
    return 1;
  }

  Instance instance = manager.readInstance(id);
  if (instance.packages.empty() && !instance.has_container()) {
    return 1; // o motivo já foi impresso pelo readInstance
  }
  if (!instance.has_container()) {
    cerr << "Erro: a instância " << id << " (" << instance.file
         << ") foi salva no formato antigo, sem as dimensões do contêiner.\n"
         << "Recrie a instância com o comando: criar <pacotes> <min> <max> "
            "<C> <L> <A>\n";
    return 1;
  }

  cout << "=== Instância " << id << " ===\n";
  cout << "Contêiner: " << instance.c << " x " << instance.l << " x "
       << instance.a << "\n";
  cout << "Pacotes:   " << instance.packages.size() << "\n\n";

  Container container(instance.c, instance.l, instance.a);
  vector<Package> packages = Utils::toPackage(instance.packages);

  GreedyHeuristic solver(container, packages);
  solver.execute();

  // O relatório da instância <id> fica em saidas/saida_<id>.txt
  string report = "saidas/saida_" + to_string(id) + ".txt";
  solver.generate_resume(report);

  const Container &result = solver.get_container();
  long vol_total = Utils::containerVolume(result);
  long vol_used = Utils::usedVolume(result);

  cout << "\n=== Resultado ===\n";
  cout << "Alocados: " << result.allocated_packages.size() << " de "
       << packages.size() << " pacotes\n";
  cout << "Ocupação: " << vol_used << "/" << vol_total << " u³ ("
       << fixed << setprecision(1) << (100.0 * vol_used / vol_total) << " %)\n";
  cout << "Relatório: " << report << " (visualize com `make visual ID=" << id
       << "`)\n";
  return 0;
}

// Exemplo embutido (o mesmo do antigo main), usado pelo comando `demo`.
static void run_demo() {
  Container container(15, 10, 10);

  vector<Package> packages = {
      Package(101, 6, 4, 3), Package(102, 3, 3, 3), Package(103, 5, 2, 4),
      Package(104, 4, 4, 4), Package(105, 2, 2, 2), Package(106, 5, 5, 5)};

  GreedyHeuristic solver(container, packages);
  solver.execute();
  solver.generate_resume("saidas/saida_demo.txt");

  cout << "\nExemplo embutido finalizado (saidas/saida_demo.txt).\n";
}

// ---------------------------------------------------------------------------
// Programa principal
// ---------------------------------------------------------------------------

int main(int argc, char *argv[]) {
  InstanceManager manager("instances");

  // Modo não interativo: ./exec_trabalho demo | ./exec_trabalho rodar <id>
  if (argc > 1) {
    vector<string> args(argv + 1, argv + argc);
    if (args[0] == "demo" && args.size() == 1) {
      run_demo();
      return 0;
    }
    if (args[0] == "rodar") {
      return cmd_rodar(manager, args);
    }
    if (args[0] == "criar-grupo") {
      return cmd_criar_grupo(manager, args);
    }
    if (args[0] == "rodar-grupo") {
      return cmd_rodar_grupo(manager, args);
    }
    if (args[0] == "listar-grupos") {
      return cmd_listar_grupos(manager);
    }
    if (args[0] == "listar-grupo") {
      return cmd_listar_grupo(manager, args);
    }
    cerr << "Comando não interativo inválido. Uso:\n"
         << "  ./exec_trabalho demo\n"
         << "  ./exec_trabalho rodar <id>\n"
         << "  ./exec_trabalho criar-grupo <nome> <n> <pacotes> <min> <max> <C> "
            "<L> <A>\n"
         << "  ./exec_trabalho rodar-grupo <nome>\n";
    print_help();
    return 1;
  }

  cout << "=== Single Container Loading — console de testes ===\n";
  cout << "Instâncias em: " << manager.folder.string() << "\n";
  cout << "Digite 'ajuda' para ver os comandos.\n\n";

  string line;
  while (cout << "> " && getline(cin, line)) {
    vector<string> args = split(line);
    if (args.empty()) {
      continue;
    }

    const string &cmd = args[0];
    if (cmd == "sair" || cmd == "exit" || cmd == "quit") {
      break;
    } else if (cmd == "ajuda" || cmd == "help" || cmd == "?") {
      print_help();
    } else if (cmd == "criar") {
      cmd_criar(manager, args);
    } else if (cmd == "listar") {
      cmd_listar(manager);
    } else if (cmd == "rodar") {
      cmd_rodar(manager, args);
    } else if (cmd == "criar-grupo") {
      cmd_criar_grupo(manager, args);
    } else if (cmd == "rodar-grupo") {
      cmd_rodar_grupo(manager, args);
    } else if (cmd == "listar-grupos") {
      cmd_listar_grupos(manager);
    } else if (cmd == "listar-grupo") {
      cmd_listar_grupo(manager, args);
    } else if (cmd == "demo") {
      run_demo();
    } else {
      cerr << "Comando desconhecido: " << cmd << " (digite 'ajuda')\n";
    }
  }

  cout << "\nAté mais!\n";
  return 0;
}

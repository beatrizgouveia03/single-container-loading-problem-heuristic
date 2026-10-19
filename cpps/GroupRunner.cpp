#include "../headers/GroupRunner.h"
#include "../headers/Heuristic.h"
#include "../headers/Utils.h"
#include <fstream>
#include <iomanip>
#include <iostream>

GroupRunResult GroupRunner::run(InstanceManager &manager, const string &group,
                                bool verbose) {
  GroupRunResult result;
  result.group = group;
  result.folder = manager.groupFolder(group);

  vector<int> ids = manager.listIds(result.folder);
  if (ids.empty()) {
    std::cerr << "Nenhuma instância encontrada no grupo '" << group << "' ("
              << result.folder.string() << ").\n";
    return result;
  }

  result.performance_file = result.folder / "desempenho.csv";
  fs::path reports_folder = fs::path("saidas") / group;

  double occupancy_sum = 0.0;
  for (size_t i = 0; i < ids.size(); i++) {
    Instance instance = manager.readInstance(result.folder, ids[i]);
    if (!instance.has_container()) {
      std::cerr << "Ignorando a instância " << ids[i]
                << ": formato antigo/sem contêiner.\n";
      continue;
    }
    if (instance.packages.empty()) {
      std::cerr << "Ignorando a instância " << ids[i]
                << ": nenhum pacote encontrado.\n";
      continue;
    }

    Container container(instance.c, instance.l, instance.a);
    GreedyHeuristic solver(container, Utils::toPackage(instance.packages));
    solver.set_verbose(verbose);
    solver.execute();

    const Container &filled = solver.get_container();

    InstancePerformance performance;
    performance.id = instance.id;
    performance.packages = (int)instance.packages.size();
    performance.allocated = (int)filled.allocated_packages.size();
    performance.volume_used = Utils::usedVolume(filled);
    performance.volume_total = Utils::containerVolume(filled);
    performance.occupancy =
        performance.volume_total > 0
            ? 100.0 * (double)performance.volume_used / performance.volume_total
            : 0.0;

    fs::path report = reports_folder / ("saida_" + to_string(instance.id) +
                                        ".txt");
    solver.generate_resume(report.string());

    std::cout << "[" << std::setw(3) << (i + 1) << "/" << ids.size()
              << "] instância " << std::setw(3) << instance.id << ": "
              << performance.allocated << "/" << performance.packages
              << " pacotes, " << std::fixed << std::setprecision(2)
              << performance.occupancy << " % (" << performance.volume_used
              << "/" << performance.volume_total << " u³)\n";

    occupancy_sum += performance.occupancy;
    result.instances.push_back(performance);
  }

  if (result.instances.empty()) {
    std::cerr << "Nenhuma instância do grupo '" << group
              << "' pôde ser executada.\n";
    return result;
  }

  result.mean_occupancy = occupancy_sum / result.instances.size();
  write_performance_csv(result.performance_file, result.instances);
  return result;
}

void GroupRunner::write_performance_csv(
    const fs::path &path, const vector<InstancePerformance> &instances) {
  if (path.has_parent_path()) {
    fs::create_directories(path.parent_path());
  }

  std::ofstream out(path);
  if (!out) {
    std::cerr << "Erro: não foi possível abrir " << path.string()
              << " para escrita.\n";
    return;
  }

  out << "id,pacotes,alocados,volume_preenchido,volume_total,ocupacao_pct\n";
  out << std::fixed << std::setprecision(2);
  for (const auto &performance : instances) {
    out << performance.id << "," << performance.packages << ","
        << performance.allocated << "," << performance.volume_used << ","
        << performance.volume_total << "," << performance.occupancy << "\n";
  }
}

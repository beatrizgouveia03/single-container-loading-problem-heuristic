#include "Heuristic.h"
#include <algorithm>
#include <fstream>
#include <future>
#include <iostream>

GreedyHeuristic::GreedyHeuristic(Container cont, std::vector<Package> pkgs)
    : container(cont), packages(pkgs) {}

void GreedyHeuristic::execute() {
  std::sort(
      packages.begin(), packages.end(),
      [](const Package &a, const Package &b) { return a.volume > b.volume; });

  for (auto &package : packages) {
    bool item_alocated = false;
    auto orientations = package.get_valid_orientations();

    for (const auto &orientation : orientations) {
      if (item_alocated)
        break;

      if (container.space_available(package, 0, 0, 0, orientation)) {
        package.pos_x = 0;
        package.pos_y = 0;
        package.pos_z = 0;
        package.ori_c = std::get<0>(orientation);
        package.ori_l = std::get<1>(orientation);
        package.ori_a = std::get<2>(orientation);
        package.alocated = true;

        container.alocated_packages.push_back(package);
        item_alocated = true;
      }
    }
  }
}

void GreedyHeuristic::generate_resume(std::string archive_name) {
  std::ofstream archive(archive_name);

  std::cout << "=== RELATÓRIO DO EXPERIMENTO DE CARREGAMENTO ===" << std::endl;
  std::cout << "Dimensões do Contêiner: " << container.c << " x " << container.l
            << " x " << container.a << std::endl;
  std::cout << "Total de itens alocados: " << container.alocated_packages.size()
            << std::endl;
  std::cout << "--------------------------------------------------"
            << std::endl;

  archive << "=== RELATÓRIO DO EXPERIMENTO DE CARREGAMENTO ===\n";
  archive << "Dimensões do Contêiner: " << container.c << " x " << container.l
          << " x " << container.a << "\n";
  archive << "Total de itens alocados: " << container.alocated_packages.size()
          << "\n";
  archive << "--------------------------------------------------\n";

  for (const auto &package : container.alocated_packages) {
    std::string line =
        "Caixa ID: " + std::to_string(package.id) + " | Pos (x,y,z): (" +
        std::to_string(package.pos_x) + ", " + std::to_string(package.pos_y) +
        ", " + std::to_string(package.pos_z) + ")" + " | Dimensões usadas: (" +
        std::to_string(package.ori_c) + ", " + std::to_string(package.ori_l) +
        ", " + std::to_string(package.ori_a) + ")";
    std::cout << line << std::endl;
    archive << line << "\n";
  }

  archive.close();
  std::cout << "\nRelatório salvo com sucesso no arquivo: " << archive_name
            << std::endl;
}

#include "Heuristic.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>

using std::get;
using std::max;
using std::min;

struct LayerCandidate {
  Package pkg; // Package that defines the layer
  std::tuple<int, int, int>
      orientation; // The orientation chosen for the package
  int depth;       // The depth of the layer basead on the package
};

GreedyHeuristic::GreedyHeuristic(Container cont, std::vector<Package> pkgs)
    : container(cont), packages(pkgs) {}

double GreedyHeuristic::cavity_degree(const Package &pkg, int x, int y, int z,
                                      int dx, int dy, int dz) {
  double c_i;  // Cavity Degree variable
  int k_i{0};  // Number of collages
  double ad_i; // Adjacency Degree
  int r_i;     // Collage Ratio
  int d_i;     // Minimal distance from other packages in the container

  // r_i = a_c * a_t (Collage Ratio = Area of the collages / Total Area of the
  // package)
  int a_c{0}; // Area of the collage
  int a_t = 2 * (dx * dy + dy * dz + dz * dx);

  // 1 - Calculate the coordenates for the upper right far corner of the package
  // if included
  int other_x = x + dx, other_y = y + dy, other_z = z + dz;

  // 2 - Calculate the number of collages -
  // 2.1 - Walls of the container
  if (x == 0) {
    k_i++;
    a_c += dy * dz;
  }
  if (y == 0) {
    k_i++;
    a_c += dx * dz;
  }
  if (z == 0) {
    k_i++;
    a_c += dx * dy;
  }
  if (other_x == container.c) {
    k_i++;
    a_c += dy * dz;
  }
  if (other_y == container.l) {
    k_i++;
    a_c += dx * dz;
  }
  if (other_z == container.a) {
    k_i++;
    a_c += dx * dy;
  }

  // 2.2 - Packages already loaded in the container
  for (const auto &pkg : container.allocated_packages) {
    // Calculate the upper left far corner of the package
    int px = pkg.pos_x + pkg.ori_c;
    int py = pkg.pos_y + pkg.ori_l;
    int pz = pkg.pos_z + pkg.ori_a;

    // Test if there is contact in the X
    if (x == px || other_x == pkg.pos_x) {
      if (max(y, pkg.pos_y) < min(other_y, py) &&
          max(z, pkg.pos_z) < min(other_z, pz)) {
        k_i++;
        a_c += (min(other_y, py) - max(y, pkg.pos_y)) *
               (min(other_z, pz) - max(z, pkg.pos_z));
      }
    }

    // Test if there is contact in the Y
    if (y == py || other_y == pkg.pos_y) {
      if (max(x, pkg.pos_x) < min(other_x, px) &&
          max(z, pkg.pos_z) < min(other_z, pz)) {
        k_i++;
        a_c += (min(other_x, px) - max(x, pkg.pos_x)) *
               (min(other_z, pz) - max(z, pkg.pos_z));
      }
    }

    // Test if there is contact in the Z
    if (z == pz || other_z == pkg.pos_z) {
      if (max(y, pkg.pos_y) < min(other_y, py) &&
          max(x, pkg.pos_x) < min(other_x, px)) {
        k_i++;
        a_c += (min(other_x, px) - max(x, pkg.pos_x)) *
               (min(other_y, py) - max(y, pkg.pos_y));
      }
    }
  }

  // 3 - Calculate r_i and d_i
  r_i = a_c / a_t;
  d_i = (k_i > 0) ? 0 : 1;

  // 4 - Calculate the ad_i
  ad_i = std::exp((double)(-d_i) / cbrt((double)dx * dy * dz));

  // 5 - Lastly calculate the c_i
  c_i = 100 * (double)k_i + 10 * ad_i + (double)r_i;

  return c_i;
}

bool comp_candidates(const LayerCandidate &a, LayerCandidate &b) {
  if (a.pkg.volume != b.pkg.volume) {
    return a.pkg.volume > b.pkg.volume;
  }
  return a.depth > b.depth;
}

void GreedyHeuristic::execute() {
  std::vector<Package> unallocated = packages;
  int x_offset = 0;

  while (x_offset < container.c && !unallocated.empty()) {
    // 1 - Find the layer candidates
    std::vector<LayerCandidate> candidates;

    for (auto &pkg : unallocated) {
      for (const auto &ori : pkg.get_valid_orientations()) {
        int dx = get<0>(ori);
        if (container.space_available(pkg, x_offset, 0, 0, ori)) {
          candidates.push_back({pkg, ori, dx});
        }
      }
    }

    // Se não tiver espaço para nenhum dos pacotes restantes ou não tiver mais
    // pacotes a serem alocados encerra
    if (candidates.empty()) {
      break;
    }

    // 2 - Sort the candidates basead on the volume and the depth
    std::sort(candidates.begin(), candidates.end(), comp_candidates);

    std::cout << " Sorted layer candidates: " << std::endl;
    for (const auto c : candidates) {
      std::cout << c.pkg.id << std::endl;
    }

    // 3 - Define the current layer
    LayerCandidate curr_layer = candidates[0];

    // 4 - Fill the layer basead on the cavity degree of the unallocated
    // packages
    std::vector<std::tuple<int, int, int>> corners;

    // 4.1 - Initiate the corners' list with the origin(0,0,0)
    corners.push_back({x_offset, 0, 0});

    // 4.2 - While there is a corner or a package was inserted, get the next
    // corner and calculate the best package to insert
    bool package_inserted = true;
    while (package_inserted && !corners.empty()) {
      package_inserted = false;

      int best_pkg_idx = -1;
      std::tuple<int, int, int> best_ori;
      std::tuple<int, int, int> next_corner = corners[0];
      double max_cavity_degree = -1.0;

      int cx = get<0>(next_corner);
      int cy = get<1>(next_corner);
      int cz = get<2>(next_corner);

      std::cout << " Inserting package in the corner " << cx << " " << cy << " "
                << cz << std::endl;

      for (auto i{0}; i < unallocated.size(); ++i) {
        auto &pkg = unallocated[i];

        std::cout << " Testing package: " << pkg.id << std::endl;
        for (const auto &ori : pkg.get_valid_orientations()) {
          int dx = get<0>(ori);
          int dy = get<1>(ori);
          int dz = get<2>(ori);

          // 4.2.1 - If the box is bigger than the layer discard
          if (cx + dx > x_offset + curr_layer.depth) {
            continue;
          }

          // 4.2.2 - Calculate the cavity degree of the package
          if (container.space_available(pkg, cx, cy, cz, ori)) {
            double cavity = cavity_degree(pkg, cx, cy, cz, dx, dy, dz);

            // 4.2.3 - If necessary update the best package
            if (cavity > max_cavity_degree) {
              max_cavity_degree = cavity;
              best_pkg_idx = (int)i;
              best_ori = ori;
            }
          }
        }
      }

      // 4.3 - Add the best package to the container
      if (best_pkg_idx != -1) {
        Package best_pkg = unallocated[best_pkg_idx];

        best_pkg.pos_x = cx;
        best_pkg.pos_y = cy;
        best_pkg.pos_z = cz;
        best_pkg.ori_c = get<0>(best_ori);
        best_pkg.ori_l = get<1>(best_ori);
        best_pkg.ori_a = get<2>(best_ori);
        best_pkg.allocated = true;

        container.allocated_packages.push_back(best_pkg);
        unallocated.erase(unallocated.begin() + best_pkg_idx);

        // 4.4 - Remove the corners
        corners.erase(std::remove(corners.begin(), corners.end(), next_corner),
                      corners.end());

        // 4.5 - Add the new corners created
        corners.push_back({cx + best_pkg.ori_c, cy, cz});
        corners.push_back({cx, cy + best_pkg.ori_l, cz});
        corners.push_back({cx, cy, cz + best_pkg.ori_a});

        std::cout << " Corners left: " << std::endl;
        for (auto c : corners) {
          std::cout << get<0>(c) << "," << get<1>(c) << "," << get<2>(c)
                    << std::endl;
        }

        package_inserted = true;
      }
    }

    // 5 - Update the depth of the container that is already filled
    x_offset += curr_layer.depth;
  }
}

void GreedyHeuristic::generate_resume(std::string archive_name) {
  std::ofstream archive(archive_name);

  std::cout << "=== RELATÓRIO DO EXPERIMENTO DE CARREGAMENTO ===" << std::endl;
  std::cout << "Dimensões do Contêiner: " << container.c << " x " << container.l
            << " x " << container.a << std::endl;
  std::cout << "Total de itens alocados: "
            << container.allocated_packages.size() << std::endl;
  std::cout << "--------------------------------------------------"
            << std::endl;

  archive << "=== RELATÓRIO DO EXPERIMENTO DE CARREGAMENTO ===\n";
  archive << "Dimensões do Contêiner: " << container.c << " x " << container.l
          << " x " << container.a << "\n";
  archive << "Total de itens alocados: " << container.allocated_packages.size()
          << "\n";
  archive << "--------------------------------------------------\n";

  for (const auto &package : container.allocated_packages) {
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

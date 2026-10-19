#include "../headers/InstanceManager.h"
#include "../headers/Utils.h"
#include <iostream>

int main() {
  InstanceManager instanceManager("instances");

  // Gera uma instância no formato novo (com as dimensões do contêiner) e
  // relê do disco para conferir o que foi salvo.
  Instance generated = instanceManager.generate(10, 1, 4, 15, 10, 10);
  std::cout << "Instância gerada: " << generated.file << "\n";

  Instance loaded = instanceManager.readInstance(generated.id);
  if (!loaded.has_container()) {
    std::cerr << "ERRO: contêiner não foi lido de volta\n";
    return 1;
  }
  if (loaded.packages != generated.packages) {
    std::cerr << "ERRO: os pacotes lidos diferem dos gerados\n";
    return 1;
  }

  std::cout << "Contêiner: " << loaded.c << " x " << loaded.l << " x "
            << loaded.a << "\n";
  std::cout << "Pacotes: " << loaded.packages.size() << "\n";
  Utils::printInstance(loaded.packages);

  std::cout << "OK\n";
  return 0;
}

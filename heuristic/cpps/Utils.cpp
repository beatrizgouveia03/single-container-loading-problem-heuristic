#include "../headers/Utils.h"
#include <iostream>
using namespace std;

void Utils::printInstance(vector<vector<int>> instance) {
  int count = 1;
  for (vector<int> line : instance) {
    cout << "pacote " << count << "\n";
    count++;
    for (int element : line) {
      cout << element << " ";
    }
    cout << "\n";
  }
}

vector<Package> Utils::toPackage(vector<vector<int>> instance){
  int count = 1;
  vector<Package> packages;
  for(auto package: instance){
    Package p(count, package[0], package[1], package[2]);
    packages.push_back(p);
    count ++;
  }

  return packages;
}

long Utils::containerVolume(const Container &container) {
  return (long)container.c * container.l * container.a;
}

long Utils::usedVolume(const Container &container) {
  long volume = 0;
  for (const auto &package : container.allocated_packages) {
    int c = package.ori_c > 0 ? package.ori_c : package.c;
    int l = package.ori_l > 0 ? package.ori_l : package.l;
    int a = package.ori_a > 0 ? package.ori_a : package.a;
    volume += (long)c * l * a;
  }
  return volume;
}

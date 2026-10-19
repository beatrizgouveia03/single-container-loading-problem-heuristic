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

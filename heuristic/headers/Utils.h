#ifndef UTILS_H
#define UTILS_H

#include "../headers/Container.h"
#include "../headers/Package.h"
#include <iostream>
#include <vector>

using namespace std;
class Utils {
public:
  void static printInstance(vector<vector<int>> instance);
  vector<Package> static toPackage(vector<vector<int>>);

  // Volume total do contêiner em unidades cúbicas.
  long static containerVolume(const Container &container);

  // Volume efetivamente ocupado pelas caixas alocadas.
  long static usedVolume(const Container &container);
};

#endif

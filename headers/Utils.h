#include <iostream>
#include <vector>
#include "../headers/Package.h"

using namespace std;
class Utils {
public:
  void static printInstance(vector<vector<int>> instance);
  vector<Package> static toPackage(vector<vector<int>>);
};

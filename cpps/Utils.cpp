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

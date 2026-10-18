#include "../headers/InstanceManager.h"

int main(){
    InstanceManager instanceManager("instances");

    instanceManager.generate(10, 5, 80);
    return 0;
}
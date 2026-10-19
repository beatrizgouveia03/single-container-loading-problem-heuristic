#include "../headers/InstanceManager.h"
#include "../headers/Utils.h"
int main(){
    InstanceManager instanceManager("instances");

    //instanceManager.generate(10, 5, 80);
    Utils::printInstance(instanceManager.readInstance(1));

    return 0;
}
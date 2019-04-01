#include "inet/icancloud/Architecture/NodeComponents/Hardware/CPUs/CPUcores/ICore.h"

namespace inet {

namespace icancloud {


ICore::~ICore(){

}

void ICore::initialize(int stage){

    HWEnergyInterface::initialize(stage);
    if (stage == INITSTAGE_PHYSICAL_ENVIRONMENT){
        cModule* mod;
        mod = getParentModule()->getSubmodule ("eController");

        cpuController = check_and_cast<CPUController*> (mod);
        cpuController->registerCore(this, getIndex());
    }

}

void ICore::e_changeState (const string & energyState){
    cpuController->e_changeState(energyState.c_str(), getIndex());
}

int ICore::e_getStatesSize (){
    return cpuController->e_getStatesSize(getIndex());
}

} // namespace icancloud
} // namespace inet

#include "inet/icancloud/Architecture/NodeComponents/Hardware/CPUs/CPUController/CPUController.h"

namespace inet {

namespace icancloud {


Define_Module (CPUController);

cGate* CPUController::getOutGate (cMessage *msg){
    return nullptr;
}

CPUController::~CPUController(){
}


void CPUController::initialize(int stage) {

    HWEnergyInterface::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        complementaryCores.clear();

        energyIndepentCores = par("indepentCores").boolValue();
        numCPUs = par("numCPUs").intValue();

        for (int i = 0; i < numCPUs; i++)
            complementaryCores.push_back(nullptr);

        setNumberOfComponents(numCPUs);
    }
}


void CPUController::finish(){

}

void CPUController::registerCore (ICore* core, int coreIndex){

   (*(complementaryCores.begin()+coreIndex)) = core;

}


void CPUController::e_changeState (const string & state, unsigned int componentIndex){

    string actual_state;

    if (energyIndepentCores){
        actual_state = e_getActualState(componentIndex);
//        if (strcmp(actual_state.c_str(), state.c_str()) != 0)
            HWEnergyInterface::e_changeState(state,componentIndex);
    }else {
        unsigned int numcpus = (unsigned int) numCPUs;
        unsigned int i;

        for (i = 0; i < numcpus; i ++){
                HWEnergyInterface::e_changeState(state,componentIndex);
        }
    }
}


string CPUController::e_getActualState (unsigned int componentIndex){
    return HWEnergyInterface::e_getActualState(componentIndex);
}


simtime_t CPUController::e_getStateTime (const string &state, unsigned int componentIndex){
    return HWEnergyInterface::e_getStateTime(state, componentIndex);
}

string CPUController::e_getStateName (int statePosition, unsigned int componentIndex){
    return HWEnergyInterface::e_getStateName(statePosition, componentIndex);
}

int CPUController::e_getStatesSize(unsigned int componentIndex){
    return HWEnergyInterface::e_getStatesSize(componentIndex);
}

void CPUController::changeDeviceState (const string &state, unsigned int componentIndex){
    showErrorMessage("CPUController::changeDeviceState does not has to be invoked ...");
}


void CPUController::changeState (const string & energyState, unsigned int componentIndex){
    showErrorMessage("CPUController::changeState does not has to be invoked ...");
}

} // namespace icancloud
} // namespace inet

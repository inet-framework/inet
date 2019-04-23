//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "inet/icancloud/EnergySystem/EnergyMeter/EnergyMeterController/EnergyMeterController.h"

namespace inet {

namespace icancloud {


Define_Module(EnergyMeterController);

EnergyMeterController::~EnergyMeterController() {
	// TODO Auto-generated destructor stub
}

void EnergyMeterController::finish(){
    cpu = nullptr;
    memory = nullptr;
    storage = nullptr;
    network = nullptr;
    psu = nullptr;
}

void EnergyMeterController::initialize(){
    cpu = nullptr;
    memory = nullptr;
    storage = nullptr;
    network = nullptr;
    psu = nullptr;
}


void EnergyMeterController::init(){

    activeEnergyMeter = getParentModule()->par("activeEnergyMeter").boolValue();

    cpu = dynamic_cast <AbstractMeterUnit*>(getParentModule()->getSubmodule("cpuMeter")->getSubmodule("core"));

    memory = dynamic_cast <AbstractMeterUnit*>(getParentModule()->getSubmodule("memoryMeter")->getSubmodule("core"));

    storage = dynamic_cast <AbstractMeterUnit*>(getParentModule()->getSubmodule("storageMeter")->getSubmodule("core"));

    network = dynamic_cast <AbstractMeterUnit*>(getParentModule()->getSubmodule("networkMeter")->getSubmodule("core"));

    psu = dynamic_cast <AbstractPSU*>(getParentModule()->getParentModule()->getSubmodule("psu"));
}

void EnergyMeterController::registerMemorization(bool memo){
    memorization = memo;
    cpu ->activeMemorization(memorization);
    memory ->activeMemorization(memorization);
    storage ->activeMemorization(memorization);
    network ->activeMemorization(memorization);

}

void EnergyMeterController::activateMeters(){
    cpu ->activateMeter();
    memory ->activateMeter();
    storage ->activateMeter();
    network ->activateMeter();
}

void EnergyMeterController::loadMemo(MemoSupport* c,MemoSupport* m,MemoSupport* s,MemoSupport* n){
    cpu ->loadMemo(c);
    memory ->loadMemo(m);
    storage ->loadMemo(s);
    network ->loadMemo(n);
}

double EnergyMeterController::cpuInstantConsumption(string state, int partIndex){
   return cpu->getInstantConsumption(state,partIndex);
}

double EnergyMeterController::getCPUEnergyConsumed(int partIndex){
    return cpu->getEnergyConsumed(partIndex);
}

double EnergyMeterController::getMemoryInstantConsumption(string state,int partIndex){
    return memory->getInstantConsumption(state,partIndex);
}

double EnergyMeterController::getMemoryEnergyConsumed(int partIndex){
    return memory->getEnergyConsumed(partIndex);
}

double EnergyMeterController::getNICInstantConsumption(string state,int partIndex){
    return network->getInstantConsumption(state,partIndex);
}

double EnergyMeterController::getNICEnergyConsumed(int partIndex){
    return network->getEnergyConsumed(partIndex);
}

double EnergyMeterController::getStorageInstantConsumption(string state,int partIndex){
    return storage->getInstantConsumption(state,partIndex);
}

double EnergyMeterController::getStorageEnergyConsumed(int partIndex){
    return storage->getEnergyConsumed(partIndex);
}

double EnergyMeterController::getPSUConsumptionLoss(){
    return psu->getConsumptionLoss();
}

double EnergyMeterController::getPSUEnergyLoss(){
    return psu->getEnergyLoss();
}

double EnergyMeterController::getNodeInstantConsumption(){
    return psu->getNodeConsumption();
}

double EnergyMeterController::getNodeEnergyConsumed(){
    return psu->getNodeEnergyConsumed();
}

double EnergyMeterController::getNodeSubsystemsConsumption(){
    return psu->getNodeSubsystemsConsumption();

}

void EnergyMeterController::switchOnPSU (){
    psu->switchOn();
}


void EnergyMeterController::switchOffPSU (){
    psu->switchOff();
}

} // namespace icancloud
} // namespace inet

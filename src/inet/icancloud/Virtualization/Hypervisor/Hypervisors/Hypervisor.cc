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

#include "inet/icancloud/Virtualization/Hypervisor/Hypervisors/Hypervisor.h"

namespace inet {

namespace icancloud {


Define_Module(Hypervisor);

Hypervisor::~Hypervisor() {
    // TODO Auto-generated destructor stub
}

void Hypervisor::initialize(){
    cModule* mod;

    mod = this->getSubmodule("cpuManager");
    cpuM = check_and_cast<H_CPUManager_Base*>(mod);

    mod = this->getSubmodule("memoryManager");
    memM = check_and_cast<H_MemoryManager_Base*>(mod);

    mod = this->getSubmodule("netManager")->getSubmodule("netManager");
    netM = check_and_cast<H_NETManager_Base*>(mod);

    mod = this->getSubmodule("storageManager")->getSubmodule("storageManager");
    storageM = check_and_cast<H_StorageManager_Base*>(mod);

}

void Hypervisor::finish(){

}

void Hypervisor::handleMessage(cMessage* msg){
   throw cRuntimeError ("Hypervisor::handleMessage -> Hypervisor module (cover) should not receive messages\n");
   delete(msg);
}

void Hypervisor::setVM (cGate** iGateCPU,
        cGate** oGateCPU,
        cGate* iGateMemI,
        cGate* oGateMemI,
        cGate* iGateMemO,
        cGate* oGateMemO,
        cGate* iGateNet,
        cGate* oGateNet,
        cGate* iGateStorage,
        cGate* oGateStorage,
        int numCores, string virtualIP, int requestedMemoryKB, int requestedStorageKB, int uId, int pId){

    cpuM->setVM(oGateCPU, iGateCPU, numCores, uId, pId);

    netM->setVM(oGateNet, iGateNet, uId, pId, virtualIP, 1);

    storageM->setVM(oGateStorage, iGateStorage, uId, pId, requestedStorageKB);

    memM->setVM(oGateMemI, oGateMemO, iGateMemI, iGateMemO, uId, pId, requestedMemoryKB);

}

void Hypervisor::freeResources (int uId, int pId){
    cpuM->freeVM(uId, pId);
    memM->freeVM(uId, pId);
    netM->freeVM(uId, pId);
    storageM->freeVM(uId, pId);
}


} // namespace icancloud
} // namespace inet

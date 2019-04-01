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

#include "inet/icancloud/Architecture/Node/NodeVL/NodeVL.h"

namespace inet {

namespace icancloud {


Define_Module(NodeVL);


NodeVL::~NodeVL() {
    instancedVMs.clear();
}

void NodeVL::initialize(int stage){
    Node::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        instancedVMs.clear();


}

void NodeVL::finish(){
    instancedVMs.clear();
    Node::finish();
}

void NodeVL::freeResources (int pId, int uId){
    hypervisor->freeResources(pId,uId);
    if (getNumOfLinkedVMs() == 0) AbstractNode::freeResources();
}

bool NodeVL::testLinkVM (int vmCPUs, int vmMemory, int vmStorage, int vmNetIF, string vmTypeID, int uId, int pId){

    // Define ..
        bool ok;

    // Init ..
        ok = false;

    // Begin ..

     if ((getFreeMemory() > vmMemory) && (getFreeStorage() > vmStorage) && (!getIP().empty())) {
         // The maximum number of processes that allow running at node (vms maybe?)
         if  (getNumProcessesRunning() < 1000){
             ok = true;
             os->memDecrease(vmMemory);
             os->storageDecrease(vmStorage);

         }
     }
     return ok;
}

void NodeVL::linkVM (cGate** iGateCPU,cGate** oGateCPU,
        cGate* iGateMemI,
        cGate* oGateMemI,
        cGate* iGateMemO,
        cGate* oGateMemO,
        cGate* iGateNet,cGate* oGateNet,
        cGate* iGateStorage,cGate* oGateStorage,
        int numCores, string virtualIP,  int vmMemory, int vmStorage, int uId, int pId){

    VMID* vmIdentifier;

    vmIdentifier = new VMID();
    vmIdentifier->initialize(uId,pId);
    instancedVMs.insert(instancedVMs.end(), vmIdentifier);

        hypervisor->setVM(iGateCPU,oGateCPU,iGateMemI,oGateMemI,iGateMemO,oGateMemO,iGateNet,oGateNet,iGateStorage,oGateStorage, numCores, virtualIP, vmMemory, vmStorage, uId,pId);

}

void NodeVL::unlinkVM (int vmMemory, int vmNumCores, int vmStorage, string virtualIP, int uId, int pId){

	// Define ..
		vector<VMID*>::iterator vmIT;
		std::ostringstream hypervisorID;

	// Begin ..
		os->memIncrease(vmMemory);
		os->storageIncrease(vmStorage);

		for (vmIT = instancedVMs.begin(); vmIT < instancedVMs.end(); vmIT++){

			if (((*vmIT)->getUser() == uId) && ((*vmIT)->getVMID() == pId) ){

					hypervisor->freeResources(uId, pId);
					instancedVMs.erase(vmIT);
			}
		}

        if ((getState()  == MACHINE_STATE_RUNNING) && (instancedVMs.size() == 0))
            changeState(MACHINE_STATE_IDLE);

}

int NodeVL::getNumOfLinkedVMs ()
{
    return instancedVMs.size();
}

void NodeVL::setVMInstance(VM* vmPtr){

    vector<VMID*>::iterator it;
    bool found = false;

    for (it = instancedVMs.begin(); (it < instancedVMs.end() && (!found)); it++){
            if (
                    ((*it)->getVMID() == vmPtr->getPid()) &&
                    ((*it)->getUser() == vmPtr->getUid())
                ){
                (*it)->setVM(vmPtr);
            }
    }

	// Found the vm instanced into the vector

		if (getState() == MACHINE_STATE_IDLE)
		    changeState(MACHINE_STATE_RUNNING);


}

VM* NodeVL::getVMInstance (int pId, int uId){

    VM* vm = nullptr;
    vector<VMID*>::iterator it;
    bool found = false;

    for (it = instancedVMs.begin(); (it < instancedVMs.end() && (!found)); it++){
            if ((*it)->getVMID(),uId){
                vm = (*it)->getVM();
                found = true;
            }
    }
    return vm;
};

void NodeVL::initNode (){

    cModule* hypervisorMod;
    string ipNode;
    // Init ..

    try{
        hypervisorMod = getSubmodule("hypervisor");
        hypervisor = dynamic_cast <Hypervisor*>  (hypervisorMod);

        //get the ip of the Node
        Node::initNode ();


    }catch (exception& e){
        throw cRuntimeError("NodeVL::initNode -> can not initialize the node module!...");
    }
}

cModule* NodeVL::getHypervisor(){
    cModule* mod;
    mod = dynamic_cast<cModule*>(hypervisor);
    return mod;
};

void NodeVL::notifyManager (Packet *pkt){
    AbstractCloudManager* manager;

    manager = dynamic_cast <AbstractCloudManager*> (managerPtr);
    if (manager == nullptr) throw cRuntimeError ("NodeVL::notifyVMConnectionsClosed -> Manager can not be casted\n");

    manager->notifyManager(pkt);
};


void NodeVL::setManager(icancloud_Base* manager){
    managerPtr = dynamic_cast <AbstractCloudManager*> (manager);
    if (managerPtr == nullptr) throw cRuntimeError ("NodeVL::notifyVMConnectionsClosed -> Manager can not be casted\n");
};

void NodeVL::turnOn (){

    // Decrease the overhead of the hypervisor at system
    double memoryOverhead_MB = hypervisor->getMemoryOverhead();
    os->memDecrease(memoryOverhead_MB * 1024);

    //turn on the node
    Node::turnOn();

}

void NodeVL::turnOff (){

    Node::turnOff();
}

} // namespace icancloud
} // namespace inet

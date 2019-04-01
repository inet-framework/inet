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

#include "inet/icancloud/Virtualization/Hypervisor/HypervisorManagers/H_CPUManager/H_CPUManager_Base.h"

namespace inet {

namespace icancloud {


H_CPUManager_Base::~H_CPUManager_Base() {
    numCPUs = 0;
    nodeGate = 0;
    toNodeCPU = nullptr;
    fromNodeCPU = nullptr;
    toVMCPU = nullptr;
    fromVMCPU = nullptr;

}

void H_CPUManager_Base::initialize(int stage) {
    icancloud_Base::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        std::ostringstream osStream;
        unsigned int i;

        // Get module parameters
        numCPUs = par("numCPUs");

        // Init the gate IDs to/from CPU
        toNodeCPU = new cGate*[numCPUs];
        fromNodeCPU = new cGate*[numCPUs];

        for (i = 0; i < numCPUs; i++) {
            toNodeCPU[i] = gate("toNodeCPU", i);
            fromNodeCPU[i] = gate("fromNodeCPU", i);
        }

        // Init the gates IDs to/from Vms
        toVMCPU = new cGateManager(this);
        fromVMCPU = new cGateManager(this);

        // Init the id from the node at position 0
        nodeGate = 0;
        toVMCPU->linkGate("toVMCPU", nodeGate);
        fromVMCPU->linkGate("fromVMCPU", nodeGate);

        vms.clear();

        // Virtualization overhead
        computing_overhead = par("computingOverhead").doubleValue();

        for (i = 0; i < numCPUs; i++) {
            overhead* oh = new overhead();
            oh->msg = nullptr;
            overheadStructure.push_back(oh);
        }
    }
}

void H_CPUManager_Base::finish(){
	vms.clear();
	icancloud_Base::finish();
}

cGate* H_CPUManager_Base::getOutGate (cMessage *msg){

		// If msg arrive from VM
		if (msg->arrivedOn("fromVMCPU"))
			return (gate("toVMCPU", msg->getArrivalGate()->getIndex()));

		// If msg arrive from VM
		else if (msg->arrivedOn("fromNodeCPU"))
		    return (gate("toNodeCPU", msg->getArrivalGate()->getIndex()));

		// If gate not found!
		else
		    return nullptr;
}

void H_CPUManager_Base::processSelfMessage (cMessage *msg){

    cancelAndDelete (msg);

}

void H_CPUManager_Base::processRequestMessage (Packet *pktCpu){

	int i;
	int changeStateCpuSize;
	int operation;
	vector<int> gates;

	const auto &sm = pktCpu->peekAtFront<icancloud_Message>();
	const auto &sm_cpu = CHK(dynamicPtrCast <const icancloud_App_CPU_Message>(sm));

	// The operation arrives from a VM
	if (pktCpu->arrivedOn("fromVMCPU")) {

	    operation = sm_cpu->getOperation();

	    // If operation is aim to change the energy state
		if (operation == SM_CHANGE_CPU_STATE){

			changeStateCpuSize = sm_cpu->get_component_to_change_size();

			for (i = 0; i < changeStateCpuSize; i++){

			    auto sm_cpu2 = makeShared<icancloud_App_CPU_Message>();
			    sm_cpu2->setChangingState(sm_cpu->getChangingState());
			    sm_cpu2->setOperation(sm_cpu->getOperation());
				int changeStateCpuIndex = sm_cpu->get_component_to_change(i);
				auto pkt2 = new Packet("icancloud_App_CPU_Message");
				pkt2->insertAtFront(sm_cpu2);
				sendRequestMessage(pkt2, toNodeCPU[changeStateCpuIndex]);
			}
			delete (pktCpu);
		}
		// If operation is aimed to perform a computing block from a VM
		else {
			schedulingCPU(pktCpu);
		}

	// If operation arrives from the cpumodule
	}
	else {
	    throw cRuntimeError ("H_CPUManager_Base::processRequestMessage-> CPU can not request for anything .. \n");
	}
}

void H_CPUManager_Base::sendRequestToCore(Packet* pkt, cGate* gate, int gateIdx){

    double overheadMIs;
    simtime_t overheadTime;

    pkt->trim();
    const auto &sm = CHK(pkt->peekAtFront<icancloud_Message>());
    if (sm == nullptr)
        throw cRuntimeError("packet doesn't include icancloud_Message header");

    if (computing_overhead != 0.0){
        auto pkt2 = pkt->dup();
        pkt2->trimFront();
        auto sm_cpu = pkt2->removeAtFront<icancloud_App_CPU_Message>();
        if (sm_cpu == nullptr)
            throw cRuntimeError("packet doesn't include icancloud_App_CPU_Message header");

        sm_cpu->setOperation(VM_OVERHEAD);

        overheadMIs = (sm_cpu->getTotalMIs() * computing_overhead / 100);
        sm_cpu->setTotalMIs(overheadMIs);

        overheadTime = (sm_cpu->getCpuTime() * computing_overhead / 100).dbl();
        sm_cpu->setCpuTime(overheadTime);

        pkt2->insertAtFront(sm_cpu);
        overheadStructure[gateIdx]->msg = pkt2;
    }
    sendRequestMessage(pkt, gate);
}


void H_CPUManager_Base::processResponseMessage (Packet *pkt){

    // Process the response message to the application that request it.

    int coreIdx = pkt->getArrivalGate()->getIndex();

    const auto &sm = pkt->peekAtFront<icancloud_Message>();

    if ((sm->getOperation() != VM_OVERHEAD) && (computing_overhead != 0)){
        // allocate the incomming message to be sent when the overhead arrives
        auto msg = overheadStructure[coreIdx]->msg;
        overheadStructure[coreIdx]->msg = pkt;
        sendRequestMessage(msg, toNodeCPU[coreIdx]);

    }
    else if (sm->getOperation() == VM_OVERHEAD){
        // get the saved processed message to send it
        processHardwareResponse(pkt);
        auto msg = overheadStructure[coreIdx]->msg;
        delete(pkt);
        overheadStructure[coreIdx]->msg = nullptr;
        sendResponseMessage(msg);
    }
    else{
        // no overhead
        processHardwareResponse(pkt);

        if (computing_overhead != 0){
            auto msg = overheadStructure[coreIdx]->msg;
            delete(msg);
            overheadStructure[coreIdx]->msg = nullptr;
        }
        sendResponseMessage(pkt);
    }
}

void H_CPUManager_Base::setVM (cGate** oGates, cGate** iGates, int numCores, int uId, int pId){


    int idxToVM;
    int idxFromVM;

    // Initialize control structure at node
        vmControl* control;
        control = new vmControl();
        control->gates.clear();
        control->uId = uId;
        control->pId = pId;

    // Connect to output gates

    for (int i = 0; i < (int)numCores; i++){
        idxToVM = toVMCPU->newGate("toVMCPU");
        toVMCPU->connectOut(iGates[i],idxToVM);
        control->gates.push_back(idxToVM);

    }

    // Connect to input gates

    for (int i = 0; i < (int)numCores; i++){
        idxFromVM = fromVMCPU->newGate("fromVMCPU");
        fromVMCPU->connectIn(oGates[i],idxFromVM);
    }

    vms.push_back(control);

}

void H_CPUManager_Base::freeVM (int uId, int pId){

    bool found = false;
    vmControl* control;
    int gateIdx;
    int gatesSize;

    for (int i = 0; (i < (int)vms.size()) && (!found); i++){
        control = (*(vms.begin() + i));
        if ((control->pId == pId) && (control->uId == uId)){
            gatesSize = (int)(control->gates.size());
            for (int j = 0; j < gatesSize;j++){
                gateIdx = (*(control->gates.begin() + j));
                fromVMCPU->freeGate (gateIdx);
                toVMCPU->freeGate (gateIdx);
            }
            vms.erase(vms.begin()+i);
            found = true;
        }
    }

    if (!found) throw cRuntimeError ("H_CPUManager_Base::freeVM--> (id = %i) not exists at hypervisor\n", pId);
}

} // namespace icancloud
} // namespace inet

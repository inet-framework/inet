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

#include "H_MEM_SCHED_FIFO.h"

namespace inet {

namespace icancloud {


Define_Module(H_MEM_SCHED_FIFO);

H_MEM_SCHED_FIFO::~H_MEM_SCHED_FIFO() {
}

void H_MEM_SCHED_FIFO::initialize(int stage) {

    H_MemoryManager_Base::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        memoryCells.clear();
        totalMemory_Blocks = requestedSizeToBlocks((int) memorySize_MB * 1024); // memory in KB
        memoryAvailable_Blocks = totalMemory_Blocks;

        memoryCell* cell = new memoryCell();
        cell->uId = 0;
        cell->pId = 0;
        cell->vmGate = nodeGate;
        cell->vmTotalBlocks_KB = totalMemory_Blocks;
        cell->remainingBlocks_KB = totalMemory_Blocks;

        memoryCells.push_back(cell);
    }
}

void H_MEM_SCHED_FIFO::finish(){
    H_MemoryManager_Base::finish();

}

void H_MEM_SCHED_FIFO::schedulingMemory(Packet *pkt){

    memoryCell* cell;
    bool found = false;

    int requiredBlocks;
    //icancloud_App_MEM_Message* sm_mem;

    int temporalBlocksQuantity = 0;

    const auto  &sm = CHK(pkt->peekAtFront<icancloud_Message>());
    int operation = sm->getOperation();

    // The operation come from the physical resources
        if (pkt->arrivedOn("fromNodeMemoryO") || pkt->arrivedOn("fromNodeMemoryI"))

            sendMemoryMessage(pkt);

        // The operation is a remote operation. So it will go to the OS
        else if (sm->getRemoteOperation()){

            sendRequestMessage(pkt, toVMMemoryI->getGate(nodeGate));

        }
        else if (operation == SM_MEM_ALLOCATE) {

            const auto &sm_mem = CHK(dynamicPtrCast <const icancloud_App_MEM_Message> (sm));

            // Memory account
            requiredBlocks = requestedSizeToBlocks(sm_mem->getMemSize());

            // Get the memory cell

            for (int i = 0; (i < (int)memoryCells.size()) && (!found); i++){
                cell = (*(memoryCells.begin() + i));
                if ((cell->uId == sm->getUid()) && (cell->pId == sm->getPid())){
                    found = true;
                    temporalBlocksQuantity = cell->remainingBlocks_KB - requiredBlocks;
                    if (cell->remainingBlocks_KB < 0) cell->remainingBlocks_KB = 0;
                }
            }

            if (!found)
                throw cRuntimeError ("H_MEM_SCHED_FIFO::schedulingMemory->the user:%i with vm id:%i not found..\n",sm->getUid(), sm->getPid());

            if (temporalBlocksQuantity <= 0){ // Not enough memory

                showDebugMessage ("Not enough memory!. Free memory blocks: %d - Requested blocks: %d",  cell->remainingBlocks_KB, requiredBlocks);
                // Cast!
                auto sm_memAux = pkt->removeAtFront<icancloud_App_MEM_Message>();
                sm_memAux->setResult (SM_NOT_ENOUGH_MEMORY);
                // Response message
                sm_memAux->setIsResponse(true);
                pkt->insertAtFront(sm_memAux);

                // Send message back!
                sendResponseMessage (pkt);

            }else if (temporalBlocksQuantity >= 0){  // Decrement the memory in the Hypervisor

                cell->remainingBlocks_KB = temporalBlocksQuantity;
                sendMemoryMessage(pkt);

            } else { // The size is 0!

                sendRequestMessage(pkt, toVMMemoryO->getGate(pkt->getArrivalGate()->getIndex()));
            }

        } else if (operation == SM_MEM_RELEASE) {

            //sm_mem = dynamic_cast <icancloud_App_MEM_Message*> (msg);
            const auto &sm_mem = CHK(dynamicPtrCast<const icancloud_App_MEM_Message> (sm));

            requiredBlocks = requestedSizeToBlocks(sm_mem->getMemSize());

            for (int i = 0; (i < (int)memoryCells.size()) && (!found); i++){
                cell = (*(memoryCells.begin() + i));
                if ((cell->uId == sm->getUid()) && (cell->pId) == (cell->pId)){
                    found = true;
                    cell->remainingBlocks_KB += requiredBlocks;
                    if (cell->remainingBlocks_KB > cell->vmTotalBlocks_KB) cell->remainingBlocks_KB = cell->vmTotalBlocks_KB;
                    sendMemoryMessage(pkt);
                }
            }

            if (!found)
                throw cRuntimeError ("H_MEM_SCHED_FIFO::schedulingMemory->the user%i with vm id:%i not found..\n",sm->getUid(), sm->getPid());

        }

        else{

            //Get the gate and the index of the arrival msg
            sendMemoryMessage(pkt);
        }

}


int H_MEM_SCHED_FIFO::setVM (cGate* oGateI, cGate* oGateO, cGate* iGateI, cGate* iGateO, int uId, int pId, int requestedMemory_KB){

    int gateidx = H_MemoryManager_Base::setVM(oGateI, oGateO, iGateI, iGateO, requestedMemory_KB, uId, pId);

    memoryCell* cell = new memoryCell();
    cell->uId = uId;
    cell->pId = pId;
    cell->vmGate = gateidx;
    cell->vmTotalBlocks_KB = requestedSizeToBlocks(requestedMemory_KB);
    cell->remainingBlocks_KB = cell->vmTotalBlocks_KB;
    memoryCells.push_back(cell);

    memoryAvailable_Blocks -= cell->vmTotalBlocks_KB;

    return 0;

}

void H_MEM_SCHED_FIFO::freeVM(int uId, int pId){

    memoryCell* cell;
    bool found = false;

    for (int i = 0; (i < (int)memoryCells.size()) && (!found); i++){
        cell = (*(memoryCells.begin() + i));
        if ((cell->uId == uId) && ((cell->pId) == pId)){
            found = true;
            memoryAvailable_Blocks += (cell->vmTotalBlocks_KB);
            memoryCells.erase(memoryCells.begin() + i);
        }
    }

    H_MemoryManager_Base::freeVM(uId, pId);

}

int H_MEM_SCHED_FIFO::getVMGateIdx(int uId, int pId){

    memoryCell* cell;
    bool found = false;
    int gateIdx = -1;

    for (int i = 0; (i < (int)memoryCells.size()) && (!found); i++){
        cell = (*(memoryCells.begin() + i));
        if ((cell->uId == uId) && ((cell->pId) == pId)){
            found = true;
            gateIdx = cell->vmGate;

        }
    }

    return gateIdx;
}

void H_MEM_SCHED_FIFO::printCells(string methodName){
    memoryCell* cell;

    for (int i = 0; (i < (int)memoryCells.size()); i++){
        cell = (*(memoryCells.begin() + i));

        printf("H_MEM_SCHED_FIFO::printCells [%s] -->cell[%i] - uId-%i pId-%i gate-%i TotalBlocks-%li remainingBlocks-%li\n",methodName.c_str(), i, cell->uId, cell->pId, cell->vmGate, cell->vmTotalBlocks_KB, cell->remainingBlocks_KB);

    }
}

double H_MEM_SCHED_FIFO::getVMMemoryOccupation_MB(int uId, int pId){

    memoryCell* cell;
    bool found = false;
    int size = -1;

    for (int i = 0; (i < (int)memoryCells.size()) && (!found); i++){
        cell = (*(memoryCells.begin() + i));
        if ((cell->uId == uId) && ((cell->pId) == pId)){
            found = true;
            size =  (((cell->vmTotalBlocks_KB - cell->remainingBlocks_KB) * blockSize_KB) / 1024);

        }
    }

    return size;
}

} // namespace icancloud
} // namespace inet

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

#include "H_STORAGE_SCHED_FIFO.h"

namespace inet {

namespace icancloud {


Define_Module(H_STORAGE_SCHED_FIFO);

H_STORAGE_SCHED_FIFO::~H_STORAGE_SCHED_FIFO() {

}

void H_STORAGE_SCHED_FIFO::initialize(int stage){

    H_StorageManager_Base::initialize(stage);

}

void H_STORAGE_SCHED_FIFO::finish(){
    H_StorageManager_Base::finish();

}

void H_STORAGE_SCHED_FIFO::schedulingStorage(Packet *pkt){

    //icancloud_App_IO_Message* sm;

    const auto &sm = pkt->peekAtFront<icancloud_Message>();

    int operation = sm->getOperation();

    if (pkt->arrivedOn("fromNET_Manager")){

        const auto &sm_io =  CHK(dynamicPtrCast<const icancloud_App_IO_Message> (sm));

        if (operation == SM_DELETE_USER_FS) {

            sendRequestMessage(pkt, toNodeStorageServers[0]);
        }
        else if (sm_io->getRemoteOperation()){

            int gate = getCellGate(sm->getUid(), sm_io->getPid());

            if (sm_io->getRemoteOperation()){
                // The request is a remote operation to be resolved in the target node!
                sendRequestMessage(pkt,to_storageCell->getGate(gate));
            } else {
                // The request is a operation to be resolved in this node!
                showErrorMessage("An IO message arrives from H_NET_MANAGER to H_STORAGE_MANAGER and it is not a remote operation.\nProbably in the node target?");
            }

       }
        else {
               // The request is a operation to be resolved in this node!
               showErrorMessage("An IO message arrives from H_NET_MANAGER to H_STORAGE_MANAGER and it is not a remote operation.\nProbably in the node target?");

       }
    }
    else if (pkt->arrivedOn("fromVMStorageServers")){

        //sm =  check_and_cast <icancloud_App_IO_Message*>  (msg);
        const auto &sm_io =  CHK(dynamicPtrCast<const icancloud_App_IO_Message> (sm));

        long int ok;
        if (sm->getOperation() == SM_WRITE_FILE){
            ok = updateOperation(sm_io->getUid(), sm->getPid(), sm_io->getSize()/1024, true);
            if (ok < 0) showDebugMessage("H_STORAGE_SCHED_FIFO::schedulingStorage->user has not got enough disk space..\n");
        }
        else if (sm->getOperation() == SM_DELETE_FILE){
            ok = updateOperation(sm_io->getUid(), sm_io->getPid(), sm_io->getSize()/1024, false);
        }

        sendRequestMessage(pkt, toNodeStorageServers[0]);
    }
    else if (pkt->arrivedOn("from_storageCell"))
        sendRequestMessage(pkt, toNET_Manager);
    else
        throw cRuntimeError ("Error in H_StorageManager::processRequestMessage manager. Unknown gate\n");

}






} // namespace icancloud
} // namespace inet

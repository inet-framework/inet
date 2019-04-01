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

#include "H_NET_SCHED_FIFO.h"

namespace inet {

namespace icancloud {


Define_Module(H_NET_SCHED_FIFO);

H_NET_SCHED_FIFO::~H_NET_SCHED_FIFO() {
}

void H_NET_SCHED_FIFO::initialize(int stage){
    H_NETManager_Base::initialize(stage);

}

void H_NET_SCHED_FIFO::finish(){
    H_NETManager_Base::finish();
}

void H_NET_SCHED_FIFO::schedulingNET(Packet *pktSm) {
    Enter_Method_Silent();
    int vm_gate;
    int operation;
    int decision;
    unsigned int i;

    string virtual_destinationIP, virtual_localIP;
    string destinationIP, localIP;
    string ipNode;
    string emptyString = "";
    int userID;
    int vmID;

    const auto &sm = pktSm->peekAtFront<icancloud_Message>();
    const auto & sm_net = dynamicPtrCast<const icancloud_App_NET_Message>(sm);
    const auto & sm_io = dynamicPtrCast<const icancloud_App_IO_Message>(sm);
    const auto & sm_mpi = dynamicPtrCast<const icancloud_MPI_Message>(sm);

    Packet *smNet = nullptr;
    if (sm_net)
        smNet = pktSm;
    Packet *smIo = nullptr;
    if (sm_io)
        smIo = pktSm;
    Packet *smMpi = nullptr;
    if (sm_mpi)
        smMpi = pktSm;

//    icancloud_App_NET_Message* sm_net;
//    icancloud_App_IO_Message *sm_io;
//    icancloud_MPI_Message* sm_mpi;
//    sm_net = dynamic_cast <icancloud_App_NET_Message*> (sm);
//    sm_io = dynamic_cast<icancloud_App_IO_Message *>(sm);
//    sm_mpi = dynamic_cast<icancloud_MPI_Message *>(sm);

    vector<Packet*> sm_close;

    // Init ..


    userID = sm->getUid();
    vmID = sm->getPid();
    operation = sm->getOperation();

    sm_close.clear();

    // The message came from a node application ..
    if (pktSm->arrivedOn("fromNodeNet")) {

        if (sm_net != nullptr) {

            // the message came from the node
            if (userID == 0) {

                sendRequestMessage(smNet, toVMNet->getGate(nodeGate));

                // the message came from a vm
            } else {

                localNetManager->manage_receiveMessage(pktSm);

                vm_gate = getGateByVIP(sm_net->getDestinationIP(), sm_net->getUid());

                sendRequestMessage(smNet, toVMNet->getGate(vm_gate));

            }

        } else {
            throw cRuntimeError ("unknown message arrival. Is not a network message\n");
        }

    }

    // The message came from a vm application ..
    else if (pktSm->arrivedOn("fromVMNet")) {

        if (operation == SM_LISTEN_CONNECTION) {

            localNetManager->manage_listen(pktSm);
            sendRequestMessage(pktSm, toNodeNet);

        }
        else if (operation == SM_CREATE_CONNECTION) {

            decision = localNetManager->manage_createConnection(pktSm);

            CHK(sm_net);
            // If everything is ok..
            if (decision == 0) {
                sendRequestMessage(smNet, toNodeNet);
            } else {
                enqueuePendingMessage(smNet);
            }

        }

        else if (operation == SM_CLOSE_VM_CONNECTIONS) {

            sm_close = localNetManager->manage_close_connections(vmID, userID);

            for (i = 0; i < sm_close.size(); i++) {
                sendRequestMessage((*(sm_close.begin()+i)), toNodeNet);
            }

            // To distinguish between when the message appears and when it has been proccessed.
            pktSm->trimFront();
            auto smAux = pktSm->removeAtFront<icancloud_Message>();
            smAux->setIsResponse(true);
            pktSm->insertAtFront(smAux);
            // Notify to the cloud manager
            sendRequestMessage(smNet, toVMNet->getGate(nodeGate));

        }
        else if (operation == SM_CLOSE_CONNECTION) {

            localNetManager->manage_close_single_connection(pktSm);

            sendRequestMessage(pktSm, toNodeNet);

        }

        //Check arrival of a MPI message
        else {

            // At the node host, from the
            if ((sm_io != nullptr) && (sm_io->getRemoteOperation())) {
                sendRequestMessage(pktSm, toHStorageManager);
            }

            // A message from the app to the net
            else if (sm_net != nullptr) {

                // MPI message!
                if ((sm->getOperation() == MPI_SEND) ||
                (sm->getOperation() == MPI_RECV) ||
                (sm->getOperation() == MPI_BARRIER_UP) ||
                (sm->getOperation() == MPI_BARRIER_DOWN) ||
                (sm->getOperation() == MPI_BCAST) ||
                (sm->getOperation() == MPI_SCATTER) ||
                (sm->getOperation() == MPI_GATHER)) {

                    if (strcmp(sm_mpi->getDestinationIP(),emptyString.c_str()) != 0) {
                        localNetManager->manage_sendMessage(smMpi);
                    }

                    sendRequestMessage(smMpi, toNodeNet);
                }

                // No MPI message...
                else {

                    if (strcmp(sm_net->getDestinationIP(),emptyString.c_str()) != 0) {
                        localNetManager->manage_sendMessage(smNet);
                    }

                    // Send the message to the net
                    sendRequestMessage(smNet, toNodeNet);
                }
            }
            else {
                showErrorMessage("Error. H_NET_Manager -> processRequestMessage, does not recognize the message. Operation: %i\n", sm->getOperation());
            }
        }
    }

}

void H_NET_SCHED_FIFO::processResponseMessage(Packet *msg){
    H_NETManager_Base::processResponseMessage(msg);
}


} // namespace icancloud
} // namespace inet

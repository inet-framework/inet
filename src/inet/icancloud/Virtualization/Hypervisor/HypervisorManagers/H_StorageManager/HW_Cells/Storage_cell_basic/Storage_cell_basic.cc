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

#include "Storage_cell_basic.h"

namespace inet {

namespace icancloud {


Define_Module(Storage_cell_basic);

#define PVFS_AVG 0.00045
#define PVFS_STD_DEV 0.0003

Storage_cell_basic::~Storage_cell_basic() {
    if (pendingMessage)
        delete(pendingMessage);

}

void Storage_cell_basic::initialize(int stage){
    AbstractStorageCell::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        replicas.clear();
    }
}

void Storage_cell_basic::processSelfMessage (cMessage *msg){

	// Is a pending message?
	if (!strcmp (msg->getName(), SM_LATENCY_MESSAGE.c_str())){

	    cancelAndDelete(msg);
	    latencyMessage = nullptr;

		// There is a pending message...
		if (pendingMessage != nullptr){

		    const auto &sm = pendingMessage->peekAtFront<icancloud_Message>();
			if ((DEBUG_DETAILED_Parallel_FS) && (DEBUG_Parallel_FS))
				showDebugMessage ("End of PFS processing! Pending request:%s ", sm->contentsToString(DEBUG_MSG_Parallel_FS).c_str());

			processRequestMessage(pendingMessage);

		}

		// No pending message waiting to be sent...
		else
			showErrorMessage ("There is no pending message waiting to be sent! :(");
	}

	else
		showErrorMessage ("Unknown self message [%s]", msg->getName());

}

void Storage_cell_basic::processRequestMessage (Packet *pktSm){

		Abstract_Remote_FS* remote_storage_cell;
		//icancloud_Message *subRequest;
		//icancloud_App_IO_Message* sm_io;
        //icancloud_App_IO_Message* sm_sub_req;
		//icancloud_App_IO_Message* subrequest_to_process;
		//icancloud_App_NET_Message* sm_net;

		const auto &sm = pktSm->peekAtFront<icancloud_Message>();

		double currentLatency;
		bool isInitialMsg;

		vector<int> vm_gates;


		const auto &sm_net = dynamicPtrCast<const icancloud_App_NET_Message>(sm);

		// Configuration message
		if (sm_net != nullptr){
		    if (sm_net->getOperation() == SM_SET_HBS_TO_REMOTE){
		        setRemoteData(pktSm);
		    }
		}
		// Operation message
		else{

            // Set the size in the message for the remote system
		    auto  sm_io = pktSm->removeAtFront<icancloud_App_IO_Message>();
            //sm_io = check_and_cast <icancloud_App_IO_Message*> (sm);
            sm_io->setNfs_requestSize_KB(sm_io->getSize());
            pktSm->insertAtFront(sm_io);
            remote_storage_cell = getRemoteStorage_byPosition(0);

            if (remote_storage_cell == nullptr){
                showErrorMessage ("BS_cell_basic::processRequestMessage->Remote storage cell not initialized!");
            }
            else if (!remote_storage_cell->isActive()){
                remote_storage_cell->enqueueRequest(pktSm);
            }
            else {

                // Generate the corresponding subrequests.

                isInitialMsg = remote_storage_cell->splitRequest (pktSm);

                auto subrequest_to_process = remote_storage_cell->hasMoreMessages();

                // PVFS
                if (subrequest_to_process != nullptr){

                    // Send..

                    // Currently processing?
                    if (latencyMessage == nullptr){
                        latencyMessage = new cMessage (SM_LATENCY_MESSAGE.c_str());
                    }
                    if (!latencyMessage->isScheduled()){

                        // Pop subrequest!
                            subrequest_to_process = remote_storage_cell->popSubRequest();

                            // if it is nullptr, there is no subrequest to be processed
                            if (subrequest_to_process != nullptr){
                                pendingMessage = subrequest_to_process;

                                currentLatency = fabs (normal(PVFS_AVG, PVFS_STD_DEV));
                                scheduleAt (currentLatency+simTime(), latencyMessage);
                            }
                    }
                }
                // NFS
                if (!isInitialMsg) {

                    auto subRequest = remote_storage_cell->popNextSubRequest (pktSm);

                    // There is a subRequest!
                    if (subRequest != nullptr){
                        // Cast subRequest!
                        const auto &sm_sub_req = subRequest->peekAtFront<icancloud_App_IO_Message>();
                        if (sm_sub_req == nullptr)
                            throw cRuntimeError("Header error");
                        //sm_sub_req = check_and_cast<icancloud_App_IO_Message *>(subRequest);

                        // Send the message to the server
                        sendRemoteRequest (subRequest, remote_storage_cell);

                    } else {

                        // Send the message to the server
                        sendRemoteRequest (pktSm, remote_storage_cell);

                    }
                }
            }
		}
}


void Storage_cell_basic::processResponseMessage (Packet *pktSm){

	//icancloud_App_IO_Message *requestMsg;
	//icancloud_App_IO_Message *nextSubRequest;
	//icancloud_Message *enqueuedRequest;
	//cMessage *parentRequest;
	//cMessage *parentRequest_test;
	Abstract_Remote_FS* remote_storage_cell;
	//icancloud_App_IO_Message* sm_io;
	//icancloud_App_NET_Message* sm_net;

	Packet *pktIo = nullptr;
	Packet *pktNet = nullptr;


	const auto &sm = pktSm->peekAtFront<icancloud_Message>();
    // check and cast the sm_io
    const auto &sm_io = dynamicPtrCast <const icancloud_App_IO_Message> (sm);
    const auto &sm_net = dynamicPtrCast <const icancloud_App_NET_Message> (sm);

    if (sm_net)
        pktNet = pktSm;
    if (sm_io)
        pktIo = pktSm;

    if (sm_net != nullptr)
        setRemoteData(pktNet);
    else{
        // Parent request
        auto parentRequest = sm_io->getParentRequest();
        // Get remote storage cell
        remote_storage_cell = getRemoteStorage(sm_io->getNfs_destAddress(),
                sm_io->getNfs_destPort(),
                sm_io->getNfs_connectionID(),
                sm_io->getNfs_type());
            // SubRequest arrives...
        remote_storage_cell->arrivesSubRequest (pktIo, parentRequest);
        // If all request have arrived... erase request and send back the response!
        auto parentRequest_test = remote_storage_cell->arrivesAllSubRequest(parentRequest);
        if (parentRequest_test != nullptr){
            // Casting!
            auto requestMsg = parentRequest->removeAtFront<icancloud_App_IO_Message>();
            //requestMsg = check_and_cast <icancloud_App_IO_Message*> (parentRequest);
            // Removes the request object!
            remote_storage_cell->removeRequest (parentRequest);
            // Now is a Response Message
            requestMsg->setIsResponse (true);
            // Update the mesage length!
            requestMsg->updateLength ();
            // Sends the message
            parentRequest->insertAtFront(requestMsg);
            sendResponseMessage (parentRequest);
        }
        // Send the next subRequest to server!
        else{
            auto enqueuedRequest = remote_storage_cell->popNextSubRequest (parentRequest);
            if (enqueuedRequest != nullptr){
                const auto &nextSubRequest = enqueuedRequest->peekAtFront<icancloud_App_IO_Message>();
                if (nextSubRequest == nullptr)
                    throw cRuntimeError("Header error");
                //nextSubRequest = check_and_cast <icancloud_App_IO_Message*> (enqueuedRequest);
                sendRemoteRequest (enqueuedRequest, remote_storage_cell);
            }
        }
    }
}


bool Storage_cell_basic::setRemoteData (Packet *pktSmNet){

    //icancloud_App_NET_Message* sm_net
    const auto &sm_net = pktSmNet->peekAtFront<icancloud_App_NET_Message>();
    int pId =  sm_net->getPid();
    int uId =  sm_net->getUid();
    string destAddress = sm_net->getVirtual_destinationIP();
    int destPort = sm_net->getVirtual_destinationPort();
    int connectionID = -1;
    string netType =   "INET";
    string fsType = sm_net->getFsType();
    int numTargetNode =  sm_net->getTargetPosition();
    // Define ..
    Abstract_Remote_FS* remote_cell;
    bool ok = false;
    bool found = false;
    std::ostringstream fs_type_modified;
    int position;
    replica_t* replica;
    // Initialize ..
    remote_cell = nullptr;
    fs_type = fsType;
    position = -1;

	// remote cell creation
    position = getRemoteCellPosition (pId);
    if (fsType == FS_NFS){
        // NFS Exists.. a second app from a VM is trying to open the same connection
        if (position != -1){
            remote_cell = getRemoteStorage_byPosition(position);
            if (!remote_cell->existsConnection(destAddress, destPort)){
                active_remote_storage(sm_net->getPid(), sm_net->getDestinationIP(), sm_net->getDestinationPort(), sm_net->getConnectionId(), "INET");
            }
            auto sm_netAux =  pktSmNet->removeAtFront<icancloud_App_NET_Message>();
            sm_netAux->setIsResponse(true);
            pktSmNet->insertAtFront(sm_netAux);
            sendResponseMessage(pktSmNet);
            // Nueva VM con NFS
            }
        else{
            remote_cell = new NFS_Storage_Cell();

            remote_cell->setNumServersForFS(numTargetNode);
            remote_cell->setCellID(pId);
            remote_cell->setConnection(destAddress,destPort, netType, connectionID);
            remote_cell->setDataSize_KB(get_nfs_requestSize_KB());
            insert_remote_storage_cells(remote_cell);
            // Send the request to create the connection..
            auto sm_netAux =  pktSmNet->removeAtFront<icancloud_App_NET_Message>();
            sm_netAux->setOperation(SM_VM_REQUEST_CONNECTION_TO_STORAGE);
            pktSmNet->insertAtFront(sm_netAux);
            sendRequestMessage(pktSmNet, to_H_StorageManager);
        }
        ok = true;
    }
    else if (fsType == FS_PFS){
        // PFS Exists.. a second app from a VM is trying to open the same connection
        if (position != -1){
            remote_cell = getRemoteStorage_byPosition(position);
            if (remote_cell->existsConnection(destAddress, destPort)){
                for (int p = 0; (p < (int)replicas.size()) && (!found); p++){
                    replica = (*(replicas.begin() + p));
                    // The operation is from the app commID, but it is a replica
                    if (replica->commId == sm_net->getCommId()){
                        int k = replica->machinesleft;
                        replica->machinesleft = k - 1;
                        found = true;
                        // The repeated operation has finalized..
                        if ((replica->machinesleft == 0) && (remote_cell->isActive())){
                            auto sm_netAux =  pktSmNet->removeAtFront<icancloud_App_NET_Message>();
                            sm_netAux->setOperation(SM_VM_REQUEST_CONNECTION_TO_STORAGE);
                            sm_netAux->setIsResponse(true);
                            pktSmNet->insertAtFront(sm_netAux);
                            sendResponseMessage(pktSmNet);
                        }
                    }
                }
                // Creates the new sentry
                if (!found){
                    replica = new replica_t();
                    replica->commId = sm_net->getCommId();
                    replica->uId = sm_net->getUid();
                    replica->pId = sm_net->getPid();
                    replica->machinesleft = numTargetNode-1;
                    auto sm_netAux =  pktSmNet->removeAtFront<icancloud_App_NET_Message>();
                    sm_netAux->setIsResponse(true);
                    pktSmNet->insertAtFront(sm_netAux);

                    replica->sms = pktSmNet;
                    replicas.push_back(replica);
                }
            }
            // The connection does not exists, so create it..
            else {
                if (sm_net->getIsResponse() ){
                    remote_cell = getRemoteStorage_byPosition(position);
                    if (active_remote_storage(sm_net->getPid(), sm_net->getDestinationIP(), sm_net->getDestinationPort(), sm_net->getConnectionId(), "INET")){
                        // Send the request to create the connection..
                        auto sm_netAux =  pktSmNet->removeAtFront<icancloud_App_NET_Message>();
                        sm_netAux->setIsResponse(true);
                        pktSmNet->insertAtFront(sm_netAux);
                        sendResponseMessage(pktSmNet);
                        for (int p = 0; (p < (int)replicas.size()) && (!found);){
                            replica = (*(replicas.begin() + p));
                            if  ((replica->uId == uId) && (replica->pId == pId)){
                                sendResponseMessage(replica->sms);
                                replicas.erase(replicas.begin() + p);
                            }
                            else {
                                p++;
                            }
                        }
                    }
                    else
                        delete(pktSmNet);
                }
                else{
                    // 2nd to 'n'nd - Other requests for file systems with more than 1 target node
                    remote_cell = getRemoteStorage_byPosition(position);
                    remote_cell -> setConnection(destAddress, destPort, netType, connectionID);
                    // Send the request to create the connection..
                    auto sm_netAux =  pktSmNet->removeAtFront<icancloud_App_NET_Message>();
                    sm_netAux->setOperation(SM_VM_REQUEST_CONNECTION_TO_STORAGE);
                    pktSmNet->insertAtFront(sm_netAux);
                    sendRequestMessage(pktSmNet, to_H_StorageManager);
                }
            }
        }
        // Arrival of a new connection.. the first connection
        else{
            remote_cell = new PFS_Storage_Cell(pfs_strideSize_KB);
            remote_cell->setNumServersForFS(numTargetNode);
            remote_cell->setCellID(pId);
            remote_cell->setConnection(destAddress,destPort, netType, connectionID);
            remote_cell->setDataSize_KB(get_nfs_requestSize_KB());
            insert_remote_storage_cells(remote_cell);
            // Send the request to create the connection..

            auto sm_netAux =  pktSmNet->removeAtFront<icancloud_App_NET_Message>();
            //sm_net->setOperation(SM_VM_REQUEST_CONNECTION_TO_STORAGE);
            sm_netAux->setOperation(SM_VM_REQUEST_CONNECTION_TO_STORAGE);
            pktSmNet->insertAtFront(sm_netAux);
            sendRequestMessage(pktSmNet, to_H_StorageManager);
        }
        ok = true;
    }
    else {
        throw cRuntimeError(" Storage_cell_basic::setRemoteData ->unknown fs type.\n");
    }
    return ok;
}


} // namespace icancloud
} // namespace inet

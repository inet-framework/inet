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

#include "inet/icancloud/Virtualization/Hypervisor/HypervisorManagers/H_StorageManager/H_StorageManager_Base.h"

namespace inet {

namespace icancloud {


H_StorageManager_Base::~H_StorageManager_Base() {
    vmIDs.clear();
}

void H_StorageManager_Base::initialize(int stage) {

    icancloud_Base::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        std::ostringstream osStream;
        int i;

        // Initialize
        numStorageServers = 0;
        nodeGate = 0;

        fromVMStorageServers = nullptr;
        toVMStorageServers = nullptr;
        from_storageCell = nullptr;
        to_storageCell = nullptr;
        toNodeStorageServers = nullptr;
        fromNodeStorageServers = nullptr;
        toNET_Manager = nullptr;
        fromNET_Manager = nullptr;
        vmIDs.clear();

        // Get module parameters
        numStorageServers = par("numStorageServers");

        // Init state to idle!

        from_storageCell = new cGateManager(this);
        to_storageCell = new cGateManager(this);

        fromVMStorageServers = new cGateManager(this);
        toVMStorageServers = new cGateManager(this);

        for (i = 0; i < numStorageServers; i++) {

            fromVMStorageServers->linkGate("fromVMStorageServers", i);
            toVMStorageServers->linkGate("toVMStorageServers", i);
        }

        /////////////////////////////////////
        originalStorage = getParentModule()->getSubmodule("storage_cell");

        // Connect the gates gates
        int idxFromCell = from_storageCell->newGate("from_storageCell");
        int idxToCell = to_storageCell->newGate("to_storageCell");

        to_storageCell->connectOut(
                originalStorage->gate("from_H_StorageManager"), idxToCell);
        from_storageCell->connectIn(
                originalStorage->gate("to_H_StorageManager"), idxFromCell);

        ///////////////////////////////////////////

        setControlStructure(0, 0, nodeGate,
                this->getParentModule()->getParentModule()->par("storageSize_GB").intValue() * 1024 * 1024);

        toNodeStorageServers = new cGate*[numStorageServers];
        fromNodeStorageServers = new cGate*[numStorageServers];

        for (i = 0; i < numStorageServers; i++) {
            toNodeStorageServers[i] = gate("toNodeStorageServers", i);
            fromNodeStorageServers[i] = gate("fromNodeStorageServers", i);
        }

        toNET_Manager = gate("toNET_Manager");
        fromNET_Manager = gate("fromNET_Manager");

        overheadStructure.clear();

        io_overhead = par("ioOverhead");
    }

}

void H_StorageManager_Base::finish(){
	icancloud_Base::finish();
}

cGate* H_StorageManager_Base::getOutGate (cMessage *msg){

		// If msg arrive from VM
		if (msg->arrivedOn("fromVMStorageServers"))
		    return (gate("toVMStorageServers", msg->getArrivalGate()->getIndex()));

		// If msg arrive from NODE
		else if (msg->arrivedOn("fromNodeStorageServers"))
		    return (gate("toNodeStorageServers", msg->getArrivalGate()->getIndex()));

		// If msg arrive from storageCell
		else if (msg->arrivedOn("from_storageCell"))
            return (gate("to_storageCell", msg->getArrivalGate()->getIndex()));

		// If msg arrive from network Manager
		else if (msg->arrivedOn("fromNET_Manager"))
			return (gate("toNET_Manager"));


	// If gate not found!
	return nullptr;
}

void H_StorageManager_Base::processSelfMessage (cMessage *msg){


    //icancloud_Message* sm;

    cancelEvent (msg);

    auto pkt = check_and_cast<Packet*>(msg);
    const auto &sm = pkt->peekAtFront<icancloud_Message>();
    if (sm == nullptr)
        throw cRuntimeError("Header error");


    //sm = dynamic_cast<icancloud_Message*>(msg);
    sendResponseMessage(pkt);


}

void H_StorageManager_Base::processRequestMessage(Packet *pktSm) {
    // Define ..
    int operation;
    int i;
    int changeStateDisksSize;
    int changeStateDiskIndex;

    //icancloud_App_IO_Message* msg;
    //icancloud_BlockList_Message *sm_storage;

    const auto &sm = pktSm->peekAtFront<icancloud_Message>();
    string destAddress;
    string type;

    //vector<Abstract_Remote_FS*>::iterator cellsIt;

    // Initialize ..
    operation = sm->getOperation();

    // Begin ..

    if (pktSm->arrivedOn("fromNodeStorageServers")) {
        throw cRuntimeError(
                "Error in H_StorageManager_Base::processRequestMessage manager. There is a msg from fromNodeStorageServers\n");
    }

    // The message came from the virtual machine
    else if (pktSm->arrivedOn("fromVMStorageServers")) {

        // If the message is to change the state of the disk..
        if (operation == SM_CHANGE_DISK_STATE) {
            const auto &sm_storage = CHK(dynamicPtrCast<const icancloud_BlockList_Message>(sm));

            changeStateDisksSize = sm_storage->get_component_to_change_size();

            for (i = 0; i < changeStateDisksSize; i++) {
                auto msg_io = makeShared<icancloud_App_IO_Message>();
                msg_io->setChangingState(sm_storage->getChangingState());
                msg_io->setOperation(sm_storage->getOperation());
                auto pktIo = new Packet("icancloud_App_IO_Message");
                pktIo->insertAtFront(msg_io);

                changeStateDiskIndex = sm_storage->get_component_to_change(i);

                sendRequestMessage(pktIo,
                        toNodeStorageServers[changeStateDiskIndex]);
            }

            delete (pktSm);

            // If the message is a disk operation ..
        } else {

            //////////////
            // Overhead //

            if (io_overhead != 0.0) {
                overhead* ov;
                ov = new overhead();
                ov->msg = pktSm;
                ov->timeStamp = simTime();
                overheadStructure.push_back(ov);
            }
            //////////////

            schedulingStorage(pktSm);
        }

    }
    // If the message is to set new remote operations ..
    else if (pktSm->arrivedOn("fromNET_Manager")) {

        if ((operation == ALLOCATE_MIGRATION_DATA)
                || (operation == SET_MIGRATION_CONNECTIONS)
                || (operation == GET_MIGRATION_CONNECTIONS)
                || (operation == GET_MIGRATION_DATA)) {

            processMigrationMessage(pktSm);
        }
        else if (operation == SM_SET_HBS_TO_REMOTE) {
            setRemoteStorage(pktSm);

        }
        else
            schedulingStorage(pktSm);
    }
    else if (pktSm->arrivedOn("from_storageCell")) {
        // The message
        if (operation == SM_SET_HBS_TO_REMOTE) {
            sendRequestMessage(pktSm, toNET_Manager);
        }
        // A complete operation have been performed
        else if (operation == SM_UNBLOCK_HBS_TO_REMOTE) {
            sendRequestMessage(pktSm, toVMStorageServers->getGate(nodeGate));
        } else
            schedulingStorage(pktSm);
    }

    // the message came from the node
    else {
        throw cRuntimeError(
                "Error in H_StorageManager_Base::processRequestMessage ->unknown arrival gate\n");
    }

}

void H_StorageManager_Base::processResponseMessage(Packet *pktSm) {

    bool found = false;
    //cMessage* msg;

    const auto &sm = pktSm->peekAtFront<icancloud_Message>();

    if (sm->getResult() == STORAGE_DATA) {
        delete (pktSm);
    }
    else {
        if (pktSm->arrivedOn("fromNET_Manager")) {
            processIOCallResponse (pktSm);
        }
        else {
            if (io_overhead != 0.0) {

                for (int i = 0; (i < (int) overheadStructure.size()) && !found; i++) {
                    overhead* ov = (*(overheadStructure.begin() + i));
                    if (ov->msg->getId() == pktSm->getId()) {
                        found = true;
                        //msg = check_and_cast<cMessage*>(sm);
                        scheduleAt(simTime() + ((simTime() - ov->timeStamp) * io_overhead), pktSm);
                        overheadStructure.erase(overheadStructure.begin() + i);
                    }
                }
            }
            if (!found)
                sendResponseMessage (pktSm);
        }
    }
}

void H_StorageManager_Base::processIOCallResponse(Packet *pktSm) {

    int operation;

    // Cast to icancloud_App_NET_Message
    const auto &sm = pktSm->peekAtFront<icancloud_Message>();
    operation = sm->getOperation();

    // IO call response...

    if (operation == REACTIVATE_REMOTE_CONNECTIONS) {
//					hypervisor->activate_migration_remote_storage_cell(sm->getVmID());
        sendResponseMessage(pktSm);
    }
    else {

        sendResponseMessage(pktSm);
    }
}

cModule* H_StorageManager_Base::createStorageCell(int uId, int pId, int64_t storageSizeGB) {

    // Define ...
    cModule *cloneStorage;
    cModuleType *modType;
    std::ostringstream storagePath;
    int i, numParameters;

    // Init ..

    storagePath << originalStorage->getNedTypeName();

    // Create the app module
    modType = cModuleType::get(storagePath.str().c_str());

    // Create the app into the user module
    cloneStorage = modType->create(storagePath.str().c_str(), getParentModule());

    // Configure the main parameters
    numParameters = originalStorage->getNumParams();
    for (i = 0; i < numParameters; i++) {
        cloneStorage->par(i) = originalStorage->par(i);
    }

    cloneStorage->par("storageSizeGB").setIntValue(storageSizeGB);
    cloneStorage->par("uId").setIntValue(uId);
    cloneStorage->par("pId").setIntValue(pId);

    cloneStorage->setName("storage_cell");

    // Finalize and build the module
    cloneStorage->finalizeParameters();
    cloneStorage->buildInside();
    // Call initialize
    cloneStorage->callInitialize();

    // Connect the gates gates

    int idxFromCell = from_storageCell->newGate("from_storageCell");
    int idxToCell = to_storageCell->newGate("to_storageCell");

    to_storageCell->connectOut(cloneStorage->gate("from_H_StorageManager"), idxToCell);
    from_storageCell->connectIn(cloneStorage->gate("to_H_StorageManager"), idxFromCell);

    return cloneStorage;

}

void H_StorageManager_Base::setRemoteStorage(Packet* pktSmIo) {

    bool found = false;
    int i;
    int gate;
    const auto &sm = pktSmIo->peekAtFront<icancloud_Message>();

    for (i = 0; (i < (int) vmIDs.size()) && (!found); i++) {
        if ((((*(vmIDs.begin() + i))->uId == sm->getUid()))
                && (((*(vmIDs.begin() + i))->pId == sm->getPid()))) {
            gate = (*(vmIDs.begin() + i))->gate;
            found = true;
        }
    }

    if (!found) {
        throw cRuntimeError(
                "H_StorageManager_Base::setRemoteStorage->The storage cell has not been created properly .. or it is not at this node\n");
    } else
        sendRequestMessage(pktSmIo, to_storageCell->getGate(gate));
}

int H_StorageManager_Base::setControlStructure(int uId, int pId, int gate, int64_t storageSizeKB) {
    storageControl* control = new storageControl();
    control->uId = uId;
    control->pId = pId;
    control->gate = gate;
    control->storageSizeKB = storageSizeKB;
    vmIDs.push_back(control);

    return vmIDs.size() - 1;
}

long int H_StorageManager_Base::updateOperation(int uId, int pId, int64_t sizeKB, bool decrease) {

    bool found = false;
    int i;
    long int size;
    storageControl* nodeControl = (*(vmIDs.begin()));
    storageControl* vmControl;

    for (i = 0; (i < (int) vmIDs.size()) && (!found); i++) {
        vmControl = (*(vmIDs.begin() + i));
        if (((vmControl->uId == uId)) && ((vmControl->pId == pId))) {
            if (decrease) {
                // the node system
                nodeControl->storageSizeKB -= sizeKB;
                if (nodeControl->storageSizeKB < 0) {
                    nodeControl->storageSizeKB += sizeKB;
                }
                // the vm system
                vmControl->storageSizeKB -= sizeKB;
                size = vmControl->storageSizeKB;
                if (vmControl->storageSizeKB < 0) {
                    nodeControl->storageSizeKB += sizeKB;
                    size = -1;
                }
            }
            else {
                // The node system
                nodeControl->storageSizeKB += sizeKB;
                // The vm system
                vmControl->storageSizeKB += sizeKB;
                size = vmControl->storageSizeKB;
            }
            found = true;
        }
    }
    return size;

}

int H_StorageManager_Base::getCellGate(int uId, int pId) {

    bool found = false;
    int gate = -1;

    for (int i = 0; (i < (int) vmIDs.size()) && (!found); i++) {
        if (((*(vmIDs.begin() + i))->uId == uId)
                && ((*(vmIDs.begin() + i))->pId == pId)) {
            found = true;
            gate = (*(vmIDs.begin() + i))->gate;
        }
    }
    return gate;
}

void H_StorageManager_Base::setVM(cGate* oGate, cGate* iGate, int uId, int pId, int requestedStorageGB) {

    cModule* cell;

    cell = createStorageCell(uId, pId, requestedStorageGB);

    // Connect to input gates
    int idxFromVM = fromVMStorageServers->newGate("fromVMStorageServers");
    fromVMStorageServers->connectIn(oGate, idxFromVM);

    int idxToVM = toVMStorageServers->newGate("toVMStorageServers");
    toVMStorageServers->connectOut(iGate, idxToVM);

    // build the control structure
    int i = setControlStructure(uId, pId, idxFromVM, requestedStorageGB);
    // link the cmodule to the structure
    (*(vmIDs.begin() + i))->remoteStorage = cell;

}

void H_StorageManager_Base::freeVM(int uId, int pId) {
    // disconnect the main gates between vm and storage manager
    bool found = false;
    int gateIdx = -1;

    for (int i = 0; (i < (int) vmIDs.size()) && (!found); i++) {
        if (((*(vmIDs.begin() + i))->uId == uId)
                && ((*(vmIDs.begin() + i))->pId == pId)) {

            //Delete the gates from the structure
            gateIdx = ((*(vmIDs.begin() + i))->gate);

            toVMStorageServers->freeGate(gateIdx);
            fromVMStorageServers->freeGate(gateIdx);

            /*
             *  (gateIdx + 1)- numStorageServers is because the Node only has 1 remote storage cell
             */
            from_storageCell->freeGate((gateIdx + 1) - numStorageServers);
            to_storageCell->freeGate((gateIdx + 1) - numStorageServers);

            // Disconnect physically the gates from the remote cell
            for (int k = 0;
                    k < (*(vmIDs.begin() + i))->remoteStorage->gateCount();
                    k++) {
                (*(vmIDs.begin() + i))->remoteStorage->gateByOrdinal(k)->disconnect();
            }

            // Call finalize
            (*(vmIDs.begin() + i))->remoteStorage->callFinish();
            (*(vmIDs.begin() + i))->remoteStorage->deleteModule();
            // delete the structure
            vmIDs.erase(vmIDs.begin() + i);
            found = true;
        }
    }

    if (!found)
        throw cRuntimeError(
                "H_CPUManager_Base::freeVM--> uId - %i ; pId - %i not exists at hypervisor\n",
                uId, pId);

}

Packet* H_StorageManager_Base::migrationToDiskContents(Packet* pktMi) {

    //icancloud_BlockList_Message *newMessage;
    icancloud_File newFile;

    //TCPCommand *controlOld;

    const auto &sm = pktMi->peekAtFront<icancloud_Migration_Message>();

    //icancloud_Migration_Message* sm
    unsigned int i;

    // Create a new message!
    auto newMessage = makeShared<icancloud_BlockList_Message>();
    // Base parameters...
    newMessage->setOperation(sm->getOperation());
    newMessage->setIsResponse(sm->getIsResponse());
    newMessage->setRemoteOperation(sm->getRemoteOperation());
    newMessage->setConnectionId(sm->getConnectionId());
    newMessage->setCommId(sm->getCommId());
    newMessage->setSourceId(sm->getSourceId());
    newMessage->setNextModuleIndex(sm->getNextModuleIndex());
    newMessage->setResult(sm->getResult());
    newMessage->setUid(sm->getUid());
    newMessage->setPid(sm->getPid());
    newMessage->setParentRequest(sm->getParentRequest());
    newMessage->setOffset(0);

    if (sm->getDiskSizeKB() > 0) {
        newMessage->setOperation(SM_WRITE_FILE);
        newMessage->setChunkLength(B(sm->getDiskSizeKB()));
        newMessage->setSize(sm->getDiskSizeKB() / KB);
    }
    else if (sm->getDiskSizeKB() < 0) {
        newMessage->setOperation(SM_DELETE_FILE);
        newMessage->setChunkLength(B(abs(sm->getDiskSizeKB())));
        newMessage->setSize(abs(sm->getDiskSizeKB()) / KB);
    }
    else {
        newMessage->setOperation(SM_READ_FILE);
        newMessage->setChunkLength(B(0));
        newMessage->setSize(0);
    }

    // Calculate info for new file
    newFile.setFileName("test");
    newFile.setFileSize(sm->getDiskSizeKB());

    newMessage->setFile(newFile);

    // Reserve memory to trace!
    newMessage->setTraceArraySize(sm->getTraceArraySize());
    // Copy trace!
    for (i = 0; i < (sm->getTraceArraySize()); i++) {
        newMessage->addNodeTrace(sm->getHostName(i), sm->getNodeTrace(i));
    }

    auto pktNew = new Packet("icancloud_BlockList_Message");
    pktNew->copyTags(*pktMi);

    // Copy the control info, if exists!
    if (pktMi->getControlInfo() != nullptr) {
        pktNew->setControlInfo(pktMi->getControlInfo()->dup());
    }
    pktNew->insertAtFront(newMessage);
    return (pktNew);
}


void H_StorageManager_Base::processMigrationReconnections(Packet *pkt){
//
//
//    const auto & sm_migration = pkt->peekAtFront<icancloud_Migration_Message>();
//	if (DEBUG_H_STORAGEMANAGER_BASE) printf ("\n Method[H_StorageManager_Base Application]: --------> processMigrationReconnections:\n");
//
//	// Define ..
//		icancloud_App_NET_Message *sm_net;
//
//		vector<Remote_Storage_Cell*> remote_storage_cells_migrated;
//		Remote_Storage_Cell* remote_storage_cell;
//		int vmIndex;
//		int pending_reconnections;
//
//	// Initialize ..
//
//		pending_reconnections = 0;
//
//	// Begin ..
//
//		if (sm_migration->getRemoteStorage()){
//
////			remote_storage_cells_migrated = sm_migration->getRemote_Storage_cells_vector();
//
//			if (remote_storage_cells_migrated.size() != 0){
//
//				for (int i = 0; i < remote_storage_cells_migrated.size(); i++){
//
//					remote_storage_cell = (*remote_storage_cells_migrated.begin()+i);
//
//					remote_storage_cell->setVmGate(vmIndex);
//					// Creates the message
//					sm_net = new icancloud_App_NET_Message ();
//					sm_net->setOperation (REACTIVATE_REMOTE_CONNECTIONS);
//
//					// Set parameters
//					sm_net->setDestinationIP (remote_storage_cell->getDestAddress().c_str());
//					sm_net->setDestinationPort (remote_storage_cell->getDestPort());
////					sm_net->setLocalIP (remote_storage_cell->getLocalAddress().c_str());
////					sm_net->setLocalPort (remote_storage_cell->getLocalPort());
//					sm_net->setConnectionType (remote_storage_cell->getNetType().c_str());
//
//					sm_net->setVmID(sm_migration->getVmID());
//
//					sendRequestMessage (sm_net, toNET_Manager);
//				}
//
//			}
//
//			sm_migration->setIsResponse(true);
//			sm_migration->updateLength();
//
//		}

}

void H_StorageManager_Base::processMigrationMessage(Packet *pkt){
//
//      auto sm = pkt->removeAtFront<icancloud_Message>();
//
//                // The remote storage app check this value to set the connections in the block or not ..
//                    sm->setResult(STORAGE_DATA);
//                    schedulingDecision = MIGRATION_RETURN;
//
//                // Cast to a sm migration message
//                    sm_migration = check_and_cast <icancloud_Migration_Message*> (sm);
//                    operation = sm_migration->getOperation();
//
//                // Obtain all the sm_migration parameters from the hypervisor
//                    hypervisor -> schedulingStorage(sm_migration, false);
//
//                if (operation == SET_MIGRATION_CONNECTIONS){
//
//                    if (sm_migration->getRemoteStorage()){
//                        cell->setRemoteStorage(true);
//                        cell->setStandardRequests(sm_migration->getStandardRequests());
//                        cell->setDeleteRequests(sm_migration->getDeleteRequests());
//                        cell->setRemoteStorageUsed(sm_migration->getRemoteStorageUsedKB());
//
//                        remote_storage_cells_migrated = sm_migration->getRemote_Storage_cells_vector();
//
//                        if (remote_storage_cells_migrated.size() != 0){
//
//                            cell->set_num_waiting_connections(remote_storage_cells_migrated.size());
//                            cell->set_remote_storage_vector(remote_storage_cells_migrated);
//
//                        }
//                    }
//
//                    processMigrationReconnections(sm_migration);
//
//                    // This message contains the connections for the remote storage app block ..
//                        sm->setResult(STORAGE_CONNECTIONS);
//
//                } else if (operation == GET_MIGRATION_DATA) {
//                    sm_migration->setRemoteStorage(cell->hasRemoteStorage());
//                    sm_migration->setDeleteRequests(cell->getDeleteRequests());
//                    sm_migration->setStandardRequests(cell->getDeleteRequests());
//                    sm_migration->setRemoteStorageUsedKB(cell->getRemoteStorageUsed());
//
//                    sm_migration->setDiskSizeKB((cell->getTotalStorageSize()-cell->getRemainingStorageSize()) * blockSize_KB * 1024);
//                    cell->freeRemainingVMStorage();
//
//                    remote_storage_cells_migrated = cell->get_remote_storage_vector();
//                    sm_migration->setRemote_Storage_cells_vector(remote_storage_cells_migrated);
//
//                } else if (operation == GET_MIGRATION_CONNECTIONS) {
//
//                    sm_migration->setRemoteStorage(cell->hasRemoteStorage());
//
//                    sm_migration->setDeleteRequests(cell->getDeleteRequests());
//                    cell->initDeleteRequests();
//
//                    sm_migration->setStandardRequests(cell->getDeleteRequests());
//                    cell->initStandardRequests();
//
//                    sm_migration->setRemoteStorageUsedKB(cell->getRemoteStorageUsed());
//
//                    remote_storage_cells_migrated = cell->get_remote_storage_vector();
//
//                    sm_migration->setRemote_Storage_cells_vector(remote_storage_cells_migrated);
//
//                    sm_migration->setDiskSizeKB((cell->getTotalStorageSize()-cell->getRemainingStorageSize()) * blockSize_KB * 1024);
//                    cell->freeRemainingVMStorage();
//
//                    // Create app io message from sm_migration
//                        sm_storage = migrationToDiskContents(sm_migration);
//
//                    if (sm_migration->getDiskSizeKB() == 0){
//
//                        delete(sm_storage);
//                    }else {
//
//                        // Modify the contents with data of the sm_migration obtained in the last scheduling storage!
//                            sm_storage->setOperation(SM_READ_FILE);
//                            sendRequestMessage(sm_storage, toNodeStorageServers[0]);
//                    }
//                }
//
//                // Return the migration message with the contents!
//                sm_migration->setIsResponse(true);
//                sendResponseMessage(sm_migration);

}

} // namespace icancloud
} // namespace inet

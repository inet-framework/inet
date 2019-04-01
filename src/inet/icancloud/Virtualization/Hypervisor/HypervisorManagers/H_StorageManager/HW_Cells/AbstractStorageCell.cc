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

#include "inet/icancloud/Virtualization/Hypervisor/HypervisorManagers/H_StorageManager/HW_Cells/AbstractStorageCell.h"

namespace inet {

namespace icancloud {


AbstractStorageCell::AbstractStorageCell() {

	virtualIP = "";

	vmTotalBlocks_KB = 0;
	remainingBlocks_KB = 0;
	remoteStorage = false;

    numStandardRequests = 0;
    numDeleteRequests = 0;
    remoteStorageUsed = 0;

    from_H_StorageManager = nullptr;
    to_H_StorageManager = nullptr;
    nfs_requestSize_KB = 0;
    pfs_strideSize_KB = 0;

    storageSizeGB = 0;
    uId = 0;

}

void AbstractStorageCell::init(int newNodeGate) {

	numStandardRequests = 0;
	numDeleteRequests = 0;
	remoteStorageUsed = 0;

}

AbstractStorageCell::~AbstractStorageCell() {

  for (int i = 0; i < (int)remote_storage_cells.size();)
      remote_storage_cells.erase(remote_storage_cells.begin());

      from_H_StorageManager = nullptr;
      to_H_StorageManager = nullptr;


}

void AbstractStorageCell::finish() {

    icancloud_Base::finish();

}

void AbstractStorageCell::initialize(int stage) {

    icancloud_Base::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {

        std::ostringstream osStream;

        virtualIP = "";

        vmTotalBlocks_KB = 0;
        remainingBlocks_KB = 0;
        remoteStorage = false;
        remote_storage_cells.clear();

        // Get module parameters
        fs_type = "";

        // Storage size GB
        storageSizeGB = par("storageSizeGB").intValue();
        nfs_requestSize_KB = par("requestSize_KB");
        pfs_strideSize_KB = par("strideSize_KB");

        // Init state to idle!

        to_H_StorageManager = gate("to_H_StorageManager");
        from_H_StorageManager = gate("from_H_StorageManager");
    }

}

cGate* AbstractStorageCell::getOutGate (cMessage *msg){

	if (msg->arrivedOn("from_H_StorageManager")){
		return (gate("to_H_StorageManager"));
	}
	else {
		return nullptr;
	}

}

void AbstractStorageCell::setVirtualIP(string vip){
	virtualIP = vip;
}

void AbstractStorageCell::setTotalStorageSize(int blocks_KB){
	vmTotalBlocks_KB = blocks_KB;
	remainingBlocks_KB = blocks_KB;
}

void AbstractStorageCell::incTotalStorageSize(int blocks_KB){
	int diffTotalRemaining;

	diffTotalRemaining = vmTotalBlocks_KB - remainingBlocks_KB;

	vmTotalBlocks_KB += blocks_KB;

	remainingBlocks_KB = vmTotalBlocks_KB - diffTotalRemaining;

}

void AbstractStorageCell::setRemainingStorageSize(int blocks_KB){
	remainingBlocks_KB = blocks_KB;
}

void AbstractStorageCell::setRemoteStorage (bool remote){
	remoteStorage = remote;
}


void AbstractStorageCell::setCell (int uid, string vip, int diskSize){

	uId = uid;
	virtualIP = vip;

	vmTotalBlocks_KB = diskSize;
	remainingBlocks_KB = diskSize;

}

bool AbstractStorageCell::active_remote_storage(int pId, string destAddress, int destPort, int connectionID, string netType){


	Abstract_Remote_FS* cell;

	bool global_activation = false;
	int position = -1;

	cell = getRemoteStorage (destAddress, destPort, -1, netType);
	position = cell->getServerCellPosition (destAddress, destPort, -1, netType);

	if (cell == nullptr){
		printf("Error at Storage_Cell with parameters: %s:%i, connectionID:%i. The remote_storage_cell not exists.\n",destAddress.c_str(), destPort, connectionID);
	}

	cell->setConnectionId(connectionID, position);

    if (dec_num_waiting_connections(pId)){
		cell->activeCell();
		global_activation = true;

		cell->initializeCell();
	}

	return global_activation;

}

string AbstractStorageCell::getVirtualIP(){
	return virtualIP;
}

int AbstractStorageCell::getTotalStorageSize(){
	return vmTotalBlocks_KB;
}

int AbstractStorageCell::getRemainingStorageSize(){
	return remainingBlocks_KB;
}

bool AbstractStorageCell::hasRemoteStorage (){
	return remoteStorage;
}

Abstract_Remote_FS* AbstractStorageCell::getRemoteStorage (string destAddress, int destPort,
									  int connectionID, string netType){
	//Define ..
		Abstract_Remote_FS* cell;
		vector <Abstract_Remote_FS*>::iterator cellIT;
		int position;

	//Init ..
		cell = nullptr;
		position = -1;

	// Find the cell with cell_id identifier
		for (cellIT = remote_storage_cells.begin(); cellIT < remote_storage_cells.end(); cellIT++){

			position = (*cellIT)->getServerCellPosition(destAddress, destPort, connectionID, netType);

			if (position != -1){
				if ((*cellIT)->equalCell(position, destAddress, destPort, connectionID, netType)){
					cell = (*cellIT);
					break;
				}
			}

		}

	return cell;
}

Abstract_Remote_FS* AbstractStorageCell::getRemoteStorage_byPosition (unsigned int position) {

	//Define ..
		Abstract_Remote_FS* cell;

	//Init ..
		cell = nullptr;

		if ((position > remote_storage_cells.size()) || (position < 0)){
			showErrorMessage("position at Storage_cell::getRemoteStorage_byPosition is incorrect: %i", position);
		}

		cell = (*(remote_storage_cells.begin() + position));
	return cell;

}


bool AbstractStorageCell::existsRemoteCell (string destAddress, int destPort,
								int serverID, string netType){

	//Define ..
		bool found;
		vector <Abstract_Remote_FS*>::iterator cellIT;
		int position = -1;
	//Init ..
		found = false;

	// Find the cell with cell_id identifier
	for (cellIT = remote_storage_cells.begin(); cellIT < remote_storage_cells.end(); cellIT++){

		position = (*cellIT)->getServerCellPosition(destAddress, destPort, serverID, netType);

		if (position == -1){
			found = false;
		}
		else {
			found = true;
			break;
		}
	}

	return found;

}

int AbstractStorageCell::getRemoteCellPosition (int pId){
	//Define ..
		bool found;
		vector <Abstract_Remote_FS*>::iterator cellIT;
		int i;
		int position;
	//Init ..
		i = 0;
		position = -1;
		found = false;

	// Find the cell with cell_id identifier
	cellIT = remote_storage_cells.begin();

	while ((cellIT < remote_storage_cells.end()) && (!found)){

		if (((*cellIT)->getCellID()) == pId){
			found = true;
			position = i;
		}
		else {
			i++;
			cellIT++;
		}
	}

	return position;
}

bool AbstractStorageCell::isActiveRemoteCell (string destAddress, int destPort, int serverID, string netType){

	//Define ..
		bool found;
		vector <Abstract_Remote_FS*>::iterator cellIT;
		int position;

	//Init ..
		found = false;
		position = -1;

	// Find the cell with cell_id identifier
	for (cellIT = remote_storage_cells.begin(); cellIT < remote_storage_cells.end(); cellIT++){

		position = (*cellIT)->getServerCellPosition(destAddress, destPort, serverID, netType);

		if (position == -1) showErrorMessage("Storage_cell::isActiveRemoteCell->remote storage cell not found!");

		if ((*cellIT)->equalCell(position, destAddress, destPort, serverID, netType)){
			found = (*cellIT)->isActive();
			break;
		}
	}

	return found;

}

int AbstractStorageCell::incRemainingVMStorage(int size){

	remainingBlocks_KB += size;

	return remainingBlocks_KB;

}

int AbstractStorageCell::decRemainingVMStorage(int size){
	remainingBlocks_KB -= size;

	return remainingBlocks_KB;

}

void AbstractStorageCell::freeRemainingVMStorage(){
	remainingBlocks_KB = vmTotalBlocks_KB;
}

//Remote storage values

int AbstractStorageCell::getStandardRequests(){
	return numStandardRequests;
}

int AbstractStorageCell::getDeleteRequests(){
	return numDeleteRequests;
}

void AbstractStorageCell::initStandardRequests(){
	numStandardRequests = 0;
}

void AbstractStorageCell::initDeleteRequests(){
	numDeleteRequests = 0;
}

void AbstractStorageCell::setStandardRequests(int number){
	numStandardRequests = number;
}

void AbstractStorageCell::setDeleteRequests(int number){
	numDeleteRequests = number;
}

void AbstractStorageCell::setRemoteStorageUsed(int size){
	remoteStorageUsed = size;
}

int AbstractStorageCell::getRemoteStorageUsed(){
	return remoteStorageUsed;
}

void AbstractStorageCell::incStandardRequests(){
	numStandardRequests++;
}

void AbstractStorageCell::incDeleteRequests(){
	numDeleteRequests--;
}

void AbstractStorageCell::incRemoteStorageUsed(int size){
	remoteStorageUsed +=size;
}

void AbstractStorageCell::decRemoteStorageUsed(int size){
	remoteStorageUsed -= size;
}

// -----------------------------------------------

// For migrating vms

int AbstractStorageCell::get_remote_storage_cells_size(){
	return remote_storage_cells.size();
}

Abstract_Remote_FS* AbstractStorageCell::get_remote_storage_cell (int index){
	return remote_storage_cells[index];
}

vector<Abstract_Remote_FS*> AbstractStorageCell::get_remote_storage_vector(){
	return remote_storage_cells;
}

void AbstractStorageCell::initialize_remote_storage_vector(){
	remote_storage_cells.clear();
}

void AbstractStorageCell::set_remote_storage_vector(vector<Abstract_Remote_FS*> cells){
	vector<Abstract_Remote_FS*>::iterator remote_cell_it;

	for (remote_cell_it = cells.begin(); remote_cell_it < cells.end(); remote_cell_it++){
		(*remote_cell_it)->deactiveCell();
		remote_storage_cells.insert(remote_storage_cells.end(), (*remote_cell_it));
	}
}

void AbstractStorageCell::set_num_waiting_connections (int num, int id){
    bool found = false;
    for (int i = 0; (i < (int)remote_storage_cells.size()) && (!found); i++){
        if ((*(remote_storage_cells.begin() + i))->getCellID() == id){
            (*(remote_storage_cells.begin() + i))->setNumServersForFS(num);
            found = true;
        }
    }

}

bool AbstractStorageCell::dec_num_waiting_connections (int pId){

    vector<Abstract_Remote_FS*>::iterator remote_cell_it;
    bool cellActive = false;

    for (remote_cell_it = remote_storage_cells.begin(); remote_cell_it < remote_storage_cells.end(); remote_cell_it++){
        if ((*remote_cell_it)->getCellID() == pId){
            if ((*remote_cell_it)->allServersSetted()){
                (*remote_cell_it)->activeCell();
                cellActive = true;
            }
            break;

        }
    }

    return cellActive;
}

void AbstractStorageCell::set_maxConnections (int pId, int num){
    vector<Abstract_Remote_FS*>::iterator remote_cell_it;

        for (remote_cell_it = remote_storage_cells.begin(); remote_cell_it < remote_storage_cells.end(); remote_cell_it++){
            if ((*remote_cell_it)->getCellID() == pId){
                (*remote_cell_it)->setNumServersForFS(num);
                break;
            }
        }

}

int AbstractStorageCell::get_maxConnections (int pId){
   // vector<Abstract_Remote_FS*>::iterator remote_cell_it;
    int maxCon = 0;

    for (auto remote_cell_it = remote_storage_cells.begin(); remote_cell_it < remote_storage_cells.end(); remote_cell_it++){
        if ((*remote_cell_it)->getCellID() == pId){
            maxCon = (*remote_cell_it)->getNumServers();
            break;
        }
    }

    return maxCon;
}

int AbstractStorageCell::get_nfs_requestSize_KB(){
	return nfs_requestSize_KB;
}

int AbstractStorageCell::get_pfs_strideSize_KB(){
	return pfs_strideSize_KB;

}

void AbstractStorageCell::insert_remote_storage_cells(Abstract_Remote_FS* cell){
	remote_storage_cells.insert(remote_storage_cells.end(), cell);
}

void AbstractStorageCell::sendRemoteRequest (Packet* pktSubRequest, Abstract_Remote_FS* cell){

    Enter_Method_Silent();

    int serverID_position;

 	serverID_position = cell->getServerCellPosition(pktSubRequest);

//	if (serverID_position == -1) {
	    serverID_position = cell->getServerToSend(pktSubRequest);
	    if (serverID_position < 0)
	        showErrorMessage("STORAGE_cell_basic::sendRemoteRequest -> error getting the position of the remote server at remote cell");
//	}
	pktSubRequest->trimFront();
	auto subRequest = pktSubRequest->removeAtFront<icancloud_App_IO_Message>();
	subRequest->setConnectionId(cell->getConnectionId(serverID_position));
	subRequest->setNfs_destAddress(cell->getDestAddress(serverID_position).c_str());
	subRequest->setNfs_destPort(cell->getDestPort(serverID_position));
	subRequest->setNfs_requestSize_KB(cell->getDataSize_KB());
	subRequest->setNfs_id(serverID_position);
	subRequest->setNfs_type(cell->getNetType().c_str());
	subRequest->setNfs_connectionID(cell->getConnectionId(serverID_position));

	// If trace is empty, add current hostName, module and request number
	if (subRequest->isTraceEmpty()){
		subRequest->addNodeToTrace (getHostName());
		pktSubRequest->insertAtFront(subRequest);
		updateMessageTrace (pktSubRequest);
	}
	else
	    pktSubRequest->insertAtFront(subRequest);

	// Send the message!
	send(pktSubRequest, to_H_StorageManager);


}

void AbstractStorageCell::connectGates(cGate* oGate, cGate* iGate){
    if (!from_H_StorageManager->isConnectedInside()){
        from_H_StorageManager->disconnect();
        oGate->connectTo(from_H_StorageManager);
    }

    if (!to_H_StorageManager->isConnectedOutside()){
        to_H_StorageManager->disconnect();
        to_H_StorageManager->connectTo(iGate);
    }
}

} // namespace icancloud
} // namespace inet

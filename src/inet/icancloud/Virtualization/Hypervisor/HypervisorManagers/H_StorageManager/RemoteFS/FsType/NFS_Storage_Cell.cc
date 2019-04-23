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

#include "inet/icancloud/Virtualization/Hypervisor/HypervisorManagers/H_StorageManager/RemoteFS/FsType/NFS_Storage_Cell.h"

namespace inet {

namespace icancloud {


NFS_Storage_Cell::NFS_Storage_Cell(){
	requests_queue.clear();
	active = false;

	pId = -1;
    SMS_nfs = nullptr;

	netType = "";

	connections.clear();
}

NFS_Storage_Cell::~NFS_Storage_Cell() {
	// TODO Auto-generated destructor stub
	// Remove all requests
	SMS_nfs->clear();
	delete (SMS_nfs);
}

int NFS_Storage_Cell::getServerToSend(Packet* sm){
	return 0;
}

Packet* NFS_Storage_Cell::hasMoreMessages(){
	return nullptr;
}

void NFS_Storage_Cell::initializeCell() {
	// TODO Auto-generated constructor stub
		SMS_nfs = new SMS_NFS (getDataSize_KB());
}

bool NFS_Storage_Cell::splitRequest (cMessage *msg){

	SMS_nfs->splitRequest (msg);

	return false;
}

Packet* NFS_Storage_Cell::popSubRequest (){

	string msgLine;

	msgLine = "NFS_Storage_Cell::popSubRequest can not be invoked, and it happens!..\n. Aborting simulation ..";

	throw cRuntimeError(msgLine.c_str());
}

Packet* NFS_Storage_Cell::popNextSubRequest (cMessage* parentRequest){

	return  SMS_nfs->popNextSubRequest (check_and_cast<Packet *>(parentRequest));
}

void NFS_Storage_Cell::arrivesSubRequest (cMessage* subRequest, cMessage* parentRequest){
	SMS_nfs->arrivesSubRequest (check_and_cast<Packet *>(subRequest), check_and_cast<Packet *>(parentRequest));
}

cMessage* NFS_Storage_Cell::arrivesAllSubRequest (cMessage* request){
	cMessage* parentRequest;
	if (SMS_nfs->arrivesAllSubRequest (check_and_cast<Packet *>(request))){
		parentRequest = request;
	} else {
		parentRequest = nullptr;
	}
	return parentRequest;

}

bool NFS_Storage_Cell::removeRequest (cMessage* request){
	SMS_nfs->removeRequest (check_and_cast<Packet *>(request));

	bool result = false;

	if (requests_queue.size() == 0){
		result = true;
	}

	return result;
}


} // namespace icancloud
} // namespace inet

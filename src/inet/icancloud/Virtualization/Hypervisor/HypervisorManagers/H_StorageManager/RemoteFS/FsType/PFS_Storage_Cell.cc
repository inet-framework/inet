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

#include "inet/icancloud/Virtualization/Hypervisor/HypervisorManagers/H_StorageManager/RemoteFS/FsType/PFS_Storage_Cell.h"

namespace inet {

namespace icancloud {


PFS_Storage_Cell::PFS_Storage_Cell(){
	requests_queue.clear();
	active = false;
	strideSize = -1;
	pId = -1;
	netType = "";
	SMS_pfs = nullptr;

	connections.clear();
}

PFS_Storage_Cell::PFS_Storage_Cell(int n_strideSizeKB){
	requests_queue.clear();
	active = false;

	pId = -1;	netType = "";

	strideSize = n_strideSizeKB;
    SMS_pfs = nullptr;
	connections.clear();

}

PFS_Storage_Cell::~PFS_Storage_Cell() {

	SMS_pfs->clear();
	delete (SMS_pfs);

}

int PFS_Storage_Cell::getServerToSend(Packet* pktSm){
    const auto &sm = pktSm->peekAtFront<icancloud_Message>();
	return sm->getConnectionId();
}

Packet* PFS_Storage_Cell::hasMoreMessages(){
	return SMS_pfs->getFirstSubRequest();
}

void PFS_Storage_Cell::initializeCell() {

    SMS_pfs = new SMS_PFS (strideSize, connections.size());
}

bool PFS_Storage_Cell::splitRequest (cMessage *msg){

	//icancloud_App_IO_Message* sm_io;
	bool system;

	auto pkt = check_and_cast<Packet *>(msg);
	const auto &sm_io = pkt->peekAtFront<icancloud_App_IO_Message>();

	//const auto &sm_io = check_and_cast <icancloud_App_IO_Message*> (msg);

	if (sm_io->getNfs_connectionID() == -1){
		system = true;
		SMS_pfs->splitRequest (pkt);
	}
	else {
		system = false;
	}
	return system;
}

Packet* PFS_Storage_Cell::popSubRequest (){
	return SMS_pfs->popSubRequest();
}

Packet* PFS_Storage_Cell::popNextSubRequest (cMessage* parentRequest){

	//return  SMS_pfs->popNextSubRequest (parentRequest);
    return  nullptr;
}

void PFS_Storage_Cell::arrivesSubRequest (cMessage* subRequest, cMessage* parentRequest){
    SMS_pfs->arrivesSubRequest (check_and_cast<Packet *>(subRequest), check_and_cast<Packet *>(parentRequest));
}

cMessage* PFS_Storage_Cell::arrivesAllSubRequest (cMessage* request){

    auto pkt = check_and_cast<Packet *>(request);
    const auto &sm_io = pkt->peekAtFront<icancloud_App_IO_Message>();
    if (sm_io == nullptr)
        throw cRuntimeError("Header not found");

    Packet *parentRequest_io = nullptr;
	//icancloud_App_IO_Message* parentRequest_io;
	//icancloud_App_IO_Message* sm_io;

	//sm_io = check_and_cast <icancloud_App_IO_Message*> (request);

	if (SMS_pfs->arrivesAllSubRequest (pkt)){
	    parentRequest_io = pkt;
	}
	return parentRequest_io;
}

bool PFS_Storage_Cell::removeRequest (cMessage* request){
	SMS_pfs->removeRequest (check_and_cast<Packet *>(request));

	bool result = false;

	if (requests_queue.size() == 0){
		result = true;
	}

	return result;
}



} // namespace icancloud
} // namespace inet

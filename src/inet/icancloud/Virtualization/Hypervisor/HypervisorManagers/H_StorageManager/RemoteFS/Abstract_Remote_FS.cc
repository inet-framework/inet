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

#include "inet/icancloud/Virtualization/Hypervisor/HypervisorManagers/H_StorageManager/RemoteFS/Abstract_Remote_FS.h"

namespace inet {

namespace icancloud {

using namespace inet;
Abstract_Remote_FS::Abstract_Remote_FS(){
	active = false;
	numServersForFS = 0;
	numSettingServers = 0;
	dataSize_KB = 0;
	pId = -1;
	connections.clear();
	netType = "INET";
}

Abstract_Remote_FS::~Abstract_Remote_FS() {
    connections.clear();

}


void Abstract_Remote_FS::clean_cell(){

    connections.clear();

    while (requests_queue.size() != 0)
    	delete((*(requests_queue.begin())));


}

void Abstract_Remote_FS::setCellID (int id){
	pId = id;
}

int Abstract_Remote_FS::getCellID (){
	return pId;
}

void Abstract_Remote_FS::setConnection(string destAddress_var, int destPort, string netType_, int connectionId){
	connect* cn;

	cn = new connect();
	cn->destAddress = destAddress_var;
	cn->destPort = destPort;
	netType = netType_;
	cn->connectionID = connectionId;

	connections.push_back(cn);
	numSettingServers++;

}

int Abstract_Remote_FS::getConnectionId(int index_connection){
	return (*(connections.begin()+index_connection))->connectionID;
}

string Abstract_Remote_FS::getDestAddress (int index_connection){
	return (*(connections.begin()+index_connection))->destAddress;
}

int Abstract_Remote_FS::getDestPort (int index_connection){
	return (*(connections.begin()+index_connection))->destPort;
}

string Abstract_Remote_FS::getNetType (){
	return netType;
}

void Abstract_Remote_FS::setConnectionId(int connectionId,int index_connection){
    numSettingServers--;
	(*(connections.begin()+index_connection))->connectionID = connectionId;
}

void Abstract_Remote_FS::setDataSize_KB (int requestSize_KB_var){
	dataSize_KB = requestSize_KB_var;
}

int Abstract_Remote_FS::getDataSize_KB (){
	return dataSize_KB;
}

void Abstract_Remote_FS::activeCell (){
	active = true;
}

void Abstract_Remote_FS::deactiveCell (){
	active = false;
}

bool Abstract_Remote_FS::isActive (){
	return active;
}


int Abstract_Remote_FS::getServerCellPosition (string destAddress_, int destPort_, int connectionID_, string netType_){

	int position = 0;
	bool found = false;

	while ((!found) && (position < ((int) connections.size()))){
		if (equalCell (position, destAddress_, destPort_, connectionID_, netType_)){
			found = true;
		} else {
			position++;
		}
	}

	if (!found) position = -1;

	return position;

}

int Abstract_Remote_FS::getServerCellPosition (Packet* sm){

 //TODO: I only allow 1 different type of remote filesystem per VM.
    return 0;

}

bool Abstract_Remote_FS::existsConnection (string destAddress_, int destPort_){
    connection* cn;
    bool found = false;

    for (int i = 0; (i < (int)connections.size()) && (!found); i++){
        cn = (*(connections.begin()+i));
       if ( (strcmp (cn->destAddress.c_str(), destAddress_.c_str()) == 0) &&
            (cn->destPort == destPort_)){
           found = true;
       }
    }

    return found;
}


bool Abstract_Remote_FS::equalCell (int serverCellPosition, string destAddress_, int destPort_,
						 int connectionID_, string netType_){
	connection* cn;

	cn = (*(connections.begin()+serverCellPosition));

    return ( (strcmp (cn->destAddress.c_str(), destAddress_.c_str()) == 0) &&
		     (cn->destPort == destPort_) &&
		     (strcmp (netType.c_str(), netType_.c_str()) == 0) &&
		     (cn->connectionID == connectionID_)
		   );

}

void Abstract_Remote_FS::enqueueRequest(Packet* request){
	requests_queue.insert(requests_queue.end(), request);
}

Packet* Abstract_Remote_FS::popEnqueueRequest(){
	//icancloud_Message* msg;

	auto msg = (*requests_queue.begin());
	requests_queue.erase(requests_queue.begin());

	return msg;
}

bool Abstract_Remote_FS::hasPendingRequests(){
	return (requests_queue.size()>0);
}

int Abstract_Remote_FS::getPendingRequestsSize (){
	return requests_queue.size();
}

} // namespace icancloud
} // namespace inet

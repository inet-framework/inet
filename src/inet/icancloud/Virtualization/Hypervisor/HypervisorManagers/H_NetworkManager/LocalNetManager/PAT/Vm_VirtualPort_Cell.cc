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

#include "Vm_VirtualPort_Cell.h"

namespace inet {

namespace icancloud {


Vm_VirtualPort_Cell::Vm_VirtualPort_Cell() {
	vmID = -1;
	ports.clear();
	vmIP.clear();
}

Vm_VirtualPort_Cell::~Vm_VirtualPort_Cell() {
	// TODO Auto-generated destructor stub
}

void Vm_VirtualPort_Cell::setVMID (int id){
	vmID = id;
}

void Vm_VirtualPort_Cell::setVMIP (string ip){
	vmIP = ip;
}

void Vm_VirtualPort_Cell::setPort(int realPort, int virtualPort){
	port_structure* port;

	port = new port_structure();

	port->rPort = realPort;
	port->vPort = virtualPort;
	port->connectionID = -1;

	ports.push_back(port);
}

void Vm_VirtualPort_Cell::setConnectionID (int realPort, int connID){
	// Define ..
		vector<port_structure*>::iterator it;
		bool found;

	// Initialize ..
		found = false;

	// Get the port
	for(it = ports.begin(); (it < ports.end()) && (!found); it++){
		if ((*it)->rPort == realPort){
			(*it)->connectionID = connID;
			found = true;
		}
	}
}

int Vm_VirtualPort_Cell::getVMID (){
	return vmID;
}

string Vm_VirtualPort_Cell::getVMIP(){
	return vmIP;
}

int Vm_VirtualPort_Cell::getVirtualPort (int realPort){

	// Define ..
		vector<port_structure*>::iterator it;
		int port;
		bool found;

	// Initialize ..
		found = false;
		port = -1;

	// Get the port
	for(it = ports.begin(); (it < ports.end()) && (!found); it++){
		if ((*it)->rPort == realPort){
			port = (*it)->vPort;
			found = true;
		}
	}

	return port;
}

int Vm_VirtualPort_Cell::getRealPort (int virtualPort){

	// Define ..
		vector<port_structure*>::iterator it;
		int port;
		bool found;

	// Initialize ..
		found = false;
		port = -1;

	// Get the port
	for(it = ports.begin(); (it < ports.end()) && (!found); it++){
		if ((*it)->vPort == virtualPort){
			port = (*it)->rPort;
			found = true;
		}
	}

	return port;
}

int Vm_VirtualPort_Cell::getConnectionID (int virtualPort){

	// Define ..
		vector<port_structure*>::iterator it;
		int connId;
		bool found;

	// Initialize ..
		found = false;

	// Get the port
	for(it = ports.begin(); (it < ports.end()) && (!found); it++){
		if ((*it)->vPort == virtualPort){
			connId = (*it)->connectionID;
			found = true;
		}
	}

	return connId;

}

vector<int> Vm_VirtualPort_Cell::getAllRPorts (){
	// Define ..
		vector<port_structure*>::iterator it;
		vector<int> rPorts;
		rPorts.clear();

	// Initialize ..
		rPorts.clear();

	// Get the port
	for(it = ports.begin(); it < ports.end(); it++){
		if  ((*(it))->connectionID > -1)
			rPorts.push_back((*it)->rPort);
	}

	return rPorts;
}

vector<int> Vm_VirtualPort_Cell::getAllConnectionIDs (){
	// Define ..
		vector<port_structure*>::iterator it;
		vector<int> connections_id;
		int con;

	// Initialize ..
		connections_id.clear();
		con = -1;

	// Get the port
	for(it = ports.begin(); it < ports.end(); it++){

		con = (*it)->connectionID;

		if  (con > -1)
			connections_id.push_back(con);
	}

	return connections_id;
}

void Vm_VirtualPort_Cell::deletePort_byConnectionID (int connID){
	// Define ..
		vector<port_structure*>::iterator it;
		bool found;

	// Initialize ..
		found = false;

	// Get the port
	for(it = ports.begin(); (it < ports.end()) && (!found); it++){
		if ((*it)->connectionID == connID){
			ports.erase(it);
			found = true;
		}
	}
}

void Vm_VirtualPort_Cell::deletePort_byRPort (int realPort){

	// Define ..
		vector<port_structure*>::iterator it;
		bool found;

	// Initialize ..
		found = false;


	// Get the port
	for(it = ports.begin(); (it < ports.end()) && (!found); it++){
		if ((*it)->rPort == realPort){
			ports.erase(it);
			found = true;
		}
	}
}



void Vm_VirtualPort_Cell::deletePort_byVPort (int virtualPort){

	// Define ..
		vector<port_structure*>::iterator it;
		bool found;

	// Initialize ..
		found = false;


	// Get the port
	for(it = ports.begin(); (it < ports.end()) && (!found); it++){
		if ((*it)->vPort == virtualPort){
			ports.erase(it);
			found = true;
		}
	}
}




} // namespace icancloud
} // namespace inet

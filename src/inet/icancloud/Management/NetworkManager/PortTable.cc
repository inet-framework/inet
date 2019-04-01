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

#include "inet/icancloud/Management/NetworkManager/PortTable.h"

namespace inet {

namespace icancloud {


PortTable::PortTable() {
	// TODO Auto-generated constructor stub
	ipNode.clear();
	virtualPorts.clear();
}

PortTable::~PortTable() {
	// TODO Auto-generated destructor stub
}

void PortTable::initialize(string ip_node){
	ipNode = ip_node;
}

string PortTable::getIPNode(){
	return ipNode;
}

void PortTable::registerPort(int rPort, int vPort, int user, int vmID, int operation){

	int i;
	bool found = false;
	int virtualPort;
	realPorts* real_port;
	ports* port;
	int vpSize = virtualPorts.size();

	for (i = 0; (i < vpSize) && (!found); ){

		virtualPort = (*(virtualPorts.begin()+i))->vPort;

		if (virtualPort == vPort){

			real_port = new realPorts();
			real_port->vmID = vmID;
			real_port->user = user;
			real_port->rPort = rPort;
			real_port->operation = operation;

			(*(virtualPorts.begin()+i))->rPorts.push_back(real_port);

			found = true;
		} else
			i++;
	}

	if (!found){
		real_port = new realPorts();
		real_port->vmID = vmID;
		real_port->user = user;
		real_port->rPort = rPort;
		real_port->operation = operation;

		port = new ports();
		port->vPort = vPort;
		port->rPorts.clear();
		port->rPorts.push_back(real_port);

		virtualPorts.push_back(port);
	}
}

int PortTable::getRealPort (int vPort, int vmID, int user){

	int i, j;
	bool found = false;
	int virtualPort;
	int realPort = PORT_NOT_FOUND;
	int user_;
	int vmID_;
	ports* port;
	realPorts* rPort;
    int vpSize = virtualPorts.size();
    int rpSize;

	for (i = 0; (i < vpSize) && (!found); ){

		virtualPort = (*(virtualPorts.begin()+i))->vPort;

		if (virtualPort == vPort){

			port = (*(virtualPorts.begin()+i));
			rpSize = (port->rPorts.size());

			for (j = 0; (j < rpSize) && (!found) ; ){

				rPort = (*(port->rPorts.begin() + j));
				vmID_ = rPort->vmID;
				user_ = rPort->user;

				if ((user_ == user) && (vmID_ == vmID)){
					realPort = rPort->rPort;
					found = true;
				}
				else
					j++;

			}

			if (!found) i++;

		} else
			i++;
	}

	return realPort;
}

int PortTable::getOpenRealPort (int vPort, int user, int vmID){

	int i, j;
	bool found = false;
	int virtualPort;
	int realPort = PORT_NOT_FOUND;
	int vmID_;
	int user_;
	ports* port;
	realPorts* rPort;
    int vpSize = virtualPorts.size();
    int rpSize;

	for (i = 0; (i < vpSize) && (!found); ){

		virtualPort = (*(virtualPorts.begin()+i))->vPort;

		if (virtualPort == vPort){

			port = (*(virtualPorts.begin()+i));
			rpSize = port->rPorts.size();
			for (j = 0; (j < rpSize) && (!found) ; ){

				rPort = (*(port->rPorts.begin() + j));
				vmID_ = rPort->vmID;
				user_ = rPort->user;

				if ((vmID_ == vmID) && (user_ == user) ){

					found = true;

					if ((rPort->operation) == PORT_OPEN){
						realPort = rPort->rPort;
					}
					else{
						realPort = PORT_NOT_FOUND;
					}

				}
				else
					j++;

			}

			found = true;

		} else
			i++;
	}

	return realPort;
}

void PortTable::freeVMPorts (int vmID, int user){

    int i, j;
	int vmID_;
	int user_;
	ports* port;
	realPorts* rPort;
    int vpSize = virtualPorts.size();
    int rpSize;

	for (i = 0; i < vpSize; ){

		port = (*(virtualPorts.begin()+i));
		rpSize = port->rPorts.size();
		for (j = 0; j < rpSize ; ){

			rPort = (*(port->rPorts.begin() + j));
			vmID_ = rPort->vmID;
			user_ = rPort->user;

			if ((vmID_ == vmID) && (user_ == user) ){

				port->rPorts.erase(port->rPorts.begin() + j);
				rpSize = port->rPorts.size();
			}
			else
				j++;

		}

		i++;
	}
}

int PortTable::freePort (int vmID, int user, int virtualPort){
	int i, j;
	int virtual_port;
	int real_port;
	bool found = false;
	int vmID_;
	int user_;

	ports* port;
	realPorts* rPort;
	real_port = -1;
    int vpSize = virtualPorts.size();
    int rpSize;

	for (i = 0; i < vpSize; ){

		port = (*(virtualPorts.begin()+i));
		virtual_port = port->vPort;

		if (virtual_port == virtualPort){

		    rpSize = (port->rPorts.size());

			for (j = 0; (j < rpSize) && (!found); ){

				rPort = (*(port->rPorts.begin() + j));
				vmID_ = rPort->vmID;
				user_ = rPort->user;

				if ((vmID_ == vmID) && (user_ == user)){

					real_port = rPort->rPort;
					port->rPorts.erase(port->rPorts.begin() + j);
					found = true;

				}
				else
					j++;
			}
		} else {
			i++;
		}

	}

	if (!found) real_port = -1;

	return real_port;
}

} // namespace icancloud
} // namespace inet

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

#include "inet/icancloud/Management/NetworkManager/NetworkManager.h"

namespace inet {

namespace icancloud {


Define_Module (NetworkManager);

NetworkManager::~NetworkManager() {
    for (int i =0; i < (int)ipsCloud.size(); i++){
        (*(ipsCloud.begin() + i))->virtualIPCell.clear();
        (*(ipsCloud.begin() + i))->holesIP.clear();
    }

    ports_per_node.clear();
    ipsCloud.clear();
}

void NetworkManager::initialize(int stage) {
    icancloud_Base::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        ipsCloud.clear();
        ipBasis = "";
        ports_per_node.clear();
    }
}


void NetworkManager::finish(){

	// Finish the super-class
	icancloud_Base::finish();
}


cGate* NetworkManager::getOutGate (cMessage *msg){
    return nullptr;
}


void NetworkManager::processSelfMessage (cMessage *msg){
}


void NetworkManager::processRequestMessage (Packet *pkt){
}


void NetworkManager::processResponseMessage (Packet *pkt){

}


void NetworkManager::createNewUser (int newUserID){

	ipsUserSet* element;

	element = new ipsUserSet();

	element->userID = newUserID;
	element->holesIP.clear();
	element->virtualIPCell.clear();
	element->lastIP = ipBasis;

	ipsCloud.insert(ipsCloud.end(), element);

}

void NetworkManager::deleteUser (int userID){

	vector <ipsUserSet*>::iterator it;
	string virtualIP;

	ipsCloud.begin();
	for (it = ipsCloud.begin(); it < ipsCloud.end(); it++){
		if (((*it)->userID) == userID){
			ipsCloud.erase(it);
			break;
		}
	}

}

void NetworkManager::setIPBasis(string ip){
	ipBasis = ip;
}

string NetworkManager::getIPBasis(){
	return ipBasis;
}

string NetworkManager::allocateVirtualIP (string ipNode, int userID, int vmID){

	vector <ipsUserSet*>::iterator it;
	VirtualIPCell* ipcell;
	string virtualIP;
	ipcell = new VirtualIPCell();

	for (it = ipsCloud.begin(); it < ipsCloud.end(); it++){

		if (((*it)->userID) == userID){

			ipcell->setIPNode(ipNode);
			virtualIP = newVIP((*it));
			ipcell->setVirtualIP(virtualIP);
			ipcell->setVMID(vmID);
			(*it)->virtualIPCell.insert((*it)->virtualIPCell.end(),ipcell);
			break;

		}
	}

	return virtualIP;

}

void NetworkManager::deleteVirtualIP_by_VIP(string virtualIP, int userID){
	vector <ipsUserSet*>::iterator it;
	vector <VirtualIPCell*>::iterator itStr;
	bool found;
	string vIP;
	string hole;

	found = false;

	for (it = ipsCloud.begin(); it < ipsCloud.end(); it++){

		if (((*it)->userID) == userID){

			for (itStr = (*it)->virtualIPCell.begin(); itStr < (*it)->virtualIPCell.begin(); itStr++){

				vIP = (*itStr)->getVirtualIP().c_str();

				if ( strcmp( virtualIP.c_str(), vIP.c_str()  ) == 0 )  {

					hole = (*itStr)->getHole(ipBasis);
					(*it)->virtualIPCell.erase(itStr);
					(*it)->holesIP.insert((*it)->holesIP.end(), hole);
					found = true;
					break;

				}
			}

		}

		if (found) break;
	}
}

void NetworkManager::deleteVirtualIP_by_VMID(int vmID, int userID){

	vector <ipsUserSet*>::iterator it;
	vector <VirtualIPCell*>::iterator itStr;
	bool found;
	int vm_id;
	string hole;

	found = false;

	for (it = ipsCloud.begin(); (it < ipsCloud.end()) && (!found); it++){

		if (((*it)->userID) == userID){

			for (itStr = (*it)->virtualIPCell.begin(); (itStr < (*it)->virtualIPCell.end()) && (!found); itStr++){

				vm_id = (*itStr)->getVMID();

				if (vm_id == vmID){
					hole = (*itStr)->getHole(ipBasis);
					(*it)->virtualIPCell.erase(itStr);
					(*it)->holesIP.insert((*it)->holesIP.end(), hole);
					found = true;

				}
			}

		}

	}
}

string NetworkManager::searchNodeIP (string virtualIP, int userID){

	//Define ..
		vector <ipsUserSet*>::iterator it;
		vector <VirtualIPCell*>::iterator itStr;
		bool found;
		string ipFound;
		string user;
		string vIP;
	    std::string userID_filtered;

	// Initialize ..
		found = false;
		ipFound.clear();


	for (it = ipsCloud.begin(); (it < ipsCloud.end()) && (!found); ){

		if (((*it)->userID) == userID){

			for (itStr = (*it)->virtualIPCell.begin(); (itStr < (*it)->virtualIPCell.end()) && (!found); ){

				vIP = (*itStr)->getVirtualIP().c_str();

				if ( strcmp( virtualIP.c_str(), vIP.c_str()  ) == 0 )  {

					ipFound = (*itStr)->getIPNode();
					found = true;
				}
				else
					itStr++;
			}

			found = true;
		}
		else
			it++;
	}

	return ipFound;

}

string NetworkManager::changeNodeIP (string virtualIP, int userID, string nodeIP){
	vector <ipsUserSet*>::iterator it;
	vector <VirtualIPCell*>::iterator itStr;
	bool found;
	string ipFound;
	string vIP;

	found = false;
	ipFound = "";

	for (it = ipsCloud.begin(); (it < ipsCloud.end()) && (!found); ){

	    if (((*it)->userID) == userID){

			for (itStr = (*it)->virtualIPCell.begin(); (itStr < (*it)->virtualIPCell.end()) && (!found); ){

				vIP = (*itStr)->getVirtualIP().c_str();

				if ( strcmp( virtualIP.c_str(), vIP.c_str()  ) == 0 )  {

					(*itStr)->setIPNode(nodeIP.c_str());
					found = true;
					break;

				}
				else
					itStr++;
			}

			found = true;
		}
		else
			it++;
	}

	if (!found) vIP = "";
 	return vIP;
}

bool NetworkManager::checkVMIP (string virtualIP, string nodeIP, int userID){

	//Define ..
		vector <ipsUserSet*>::iterator it;
		vector <VirtualIPCell*>::iterator itStr;
		bool found;
		string nIP;
		string vIP;

	// Initialize ..
		found = false;

	for (it = ipsCloud.begin(); it < ipsCloud.end(); it++){

		if (((*it)->userID) == userID){

			for (itStr = (*it)->virtualIPCell.begin(); itStr < (*it)->virtualIPCell.end(); itStr++){

				vIP = (*itStr)->getVirtualIP().c_str();
				nIP = (*itStr)->getIPNode();

				if ( (strcmp( virtualIP.c_str(), vIP.c_str()  ) == 0 ) &&
						(strcmp( nodeIP.c_str(), nIP.c_str()  ) == 0 )){

					found = true;
					break;

				}
			}

		}

		if (found) break;
	}

	return found;

}

int NetworkManager::getUserIPSize(){
	return ipsCloud.size();
}

string NetworkManager::searchVMIP (int userID, int vmID){

	//Define ..
		vector <ipsUserSet*>::iterator it;
		vector <VirtualIPCell*>::iterator itStr;
		bool found;
		string ipFound;

	// Initialize ..
		found = false;
		ipFound = "";

	for (it = ipsCloud.begin(); (it < ipsCloud.end()) && (!found); ){

		if (((*it)->userID) == userID){

			for (itStr = (*it)->virtualIPCell.begin(); (itStr < (*it)->virtualIPCell.end()) && (!found); ){

				if (((*itStr)->getVMID()) == vmID){

					ipFound = (*itStr)->getVirtualIP();
					found = true;

				}
				else
					itStr++;
			}

			found = true;
		}
		else
			it++;
	}

	return ipFound;

}

// ------------------- Operations with port table -----------------------

void NetworkManager::registerNode (string nodeIP){

	// A ordered insertion
	PortTable* port_table;

	port_table = new PortTable();

	port_table->initialize(nodeIP);

	// Insert in order ..
	int i = 0;
	bool found = false;
	int size = ports_per_node.size();

	for (i = 0; (i < size) && (!found);){
		if (!isMajorIP(nodeIP.c_str(),(*(ports_per_node.begin()+i))->getIPNode().c_str())){
			ports_per_node.insert(ports_per_node.begin()+i,port_table);
			found = true;
		}else
			i++;
	}

	// This case means that ports_per_node is empty or the new node has the biggest ip..
	if (!found){
		ports_per_node.push_back(port_table);
	}

}

void NetworkManager::registerPort(int user, string ipNode, int vmID, int realPort, int virtualPort, int operation){

	PortTable* ports;
	ports = getNodeFromPortsTable(ipNode.c_str());

	ports->registerPort(realPort, virtualPort, user, vmID, operation);

}

int NetworkManager::getRealPort(string destinationIP, int virtualPort, int destinationUser, int vmID){
	PortTable* ports;
	int realPort;

	ports = getNodeFromPortsTable(destinationIP.c_str());

	realPort = ports->getRealPort(virtualPort, vmID, destinationUser);

	return realPort;
}

void NetworkManager::freeAllPortsOfVM (string ipNode, int vmID, int user){

	PortTable* node;

	node = getNodeFromPortsTable(ipNode.c_str());

	node->freeVMPorts (vmID, user);
}

void NetworkManager::freePortOfVM (string ipNode, int vmID, int user, int virtualPort){

	PortTable* node;

	node = getNodeFromPortsTable(ipNode.c_str());

	node->freePort (vmID, user, virtualPort);
}

int NetworkManager::getVMid (string virtualIP, int uId){

    //Define ..
        vector <ipsUserSet*>::iterator it;
        vector <VirtualIPCell*>::iterator itStr;
        bool found;
        int pIdFound;

    // Initialize ..
        found = false;
        pIdFound = -1;

    for (it = ipsCloud.begin(); (it < ipsCloud.end()) && (!found); ){

        if (((*it)->userID) == uId){

            for (itStr = (*it)->virtualIPCell.begin(); (itStr < (*it)->virtualIPCell.end()) && (!found); ){

                if (strcmp ( (*itStr)->getVirtualIP().c_str(), virtualIP.c_str() ) == 0){

                    pIdFound = (*itStr)->getVMID();
                    found = true;

                }
                else
                    itStr++;
            }

            found = true;
        }
        else
            it++;
    }

    return pIdFound;

}
// --------------------------- Private operations ---------------------------

string NetworkManager::newVIP(ipsUserSet* user){

	string ip;
	VirtualIPs* vip;

	// First if there is any hole to set a virtual machine
	if (user->holesIP.size() != 0){
		ip = *(user->holesIP.begin());
		user->holesIP.erase(user->holesIP.begin());

	// second, if it is the first virtual machine to allocate
	} else if (strcmp (ipBasis.c_str(), user->lastIP.c_str()) == 0){
		vip = new VirtualIPs();
		vip->setVirtualIP(ipBasis);
		ip = vip->incVirtualIP();
		user->lastIP = ip;

	// third, it the ip is the next to assign
	} else {
		vip = new VirtualIPs();
		vip->setVirtualIP(user->lastIP);
		ip = vip->incVirtualIP();
		user->lastIP = ip;
	}

	return ip;
}

bool NetworkManager::isMajorIP(string nodeIP_A, string nodeIP_B){
	int first_A;
	int second_A;
	int third_A;
	int fourth_A;
	int first_B;
	int second_B;
	int third_B;
	int fourth_B;
	bool isMajor = false;

	sscanf (nodeIP_A.c_str(), "%i.%i.%i.%i", &first_A,&second_A,&third_A,&fourth_A);
	sscanf (nodeIP_B.c_str(), "%i.%i.%i.%i", &first_B,&second_B,&third_B,&fourth_B);

	if ((first_A >= first_B) &&
		(second_A >= second_B) &&
		(third_A >= third_B) &&
		(fourth_A >= fourth_B) ){
		isMajor = true;
	}

	return isMajor;

}

PortTable*  NetworkManager::getNodeFromPortsTable(string nodeIP){

	bool found;
	PortTable* element;
	string testingIP;
	string watchdog;
	int lower_index;
	int higher_index;
	int medium_index;
	int i;
	int counter = 0;

	found = false;
	watchdog = "";

	if (ports_per_node.size() == 1){
		element = (*ports_per_node.begin());
		found = true;
	} else {
		lower_index = 0;
		higher_index = ports_per_node.size();

		for (i = lower_index; (i < higher_index) && (!found);){
		    counter++;
			if (higher_index == lower_index){
				found = true;
				testingIP = (*(ports_per_node.begin()+(lower_index)))->getIPNode().c_str();
			} else {
				medium_index = (higher_index-lower_index) / 2;
				testingIP = (*(ports_per_node.begin()+(medium_index + lower_index)))->getIPNode().c_str();

				if (strcmp(testingIP.c_str(), nodeIP.c_str()) == 0){
					found = true;
					element = (*(ports_per_node.begin()+(medium_index + lower_index)));
				}
				else if (isMajorIP(testingIP.c_str(),nodeIP.c_str())){
					higher_index = higher_index - medium_index;
				} else {
					lower_index = lower_index + medium_index;
				}
			}
			if (counter == (int)ports_per_node.size()) break;
		}

		if (!found){
		    for (int i = 0; i < (int)ports_per_node.size(); i++){
		        if (strcmp((*(ports_per_node.begin() + i))->getIPNode().c_str(), nodeIP.c_str()) == 0){
		            found = true;
                    element = (*(ports_per_node.begin() + i));
		        }
		    }
		}


	}

	if (!found) showErrorMessage("NetworkManager::getNodeFromPortsTable -> The ip %s is not in the table of nodes!", nodeIP.c_str());
	return element;

}



} // namespace icancloud
} // namespace inet

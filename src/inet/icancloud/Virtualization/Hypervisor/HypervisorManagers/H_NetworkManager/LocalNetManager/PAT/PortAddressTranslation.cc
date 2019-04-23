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

#include "inet/icancloud/Virtualization/Hypervisor/HypervisorManagers/H_NetworkManager/LocalNetManager/PAT/PortAddressTranslation.h"

namespace inet {

namespace icancloud {


PortAddressTranslation::PortAddressTranslation() {

	portPtr = REGISTERED_INITIAL_PORT;
	user_vm_ports.clear();
	portHoles.clear();

	for (int i = 0; i < LAST_PORT; i++){
		freePorts[i] = NOT_STABLISHED;
	}
}

PortAddressTranslation::~PortAddressTranslation() {
	// TODO Auto-generated destructor stub
}

void PortAddressTranslation::pat_initialize(const L3Address &nodeIP){

	User_VirtualPort_Cell* user;
	Vm_VirtualPort_Cell* vm;

	// Search the user into the structure
		user = searchUser(0);

		// If the user does not exists, create it ..
			if (user != nullptr){
				deleteUser (0);
			}

			user = newUser(0);

	// the node is defined as an vm..
		vm = user->newVM(0);
		vm->setVMIP(nodeIP.toIpv4().str(false).c_str());

}

void PortAddressTranslation::portForwarding(Packet *pkt) {

    // Define ..
    //icancloud_Message *sm;
    int operation;

    // Cast!
    pkt->trimFront();
    auto sm = CHK(pkt->removeAtFront<icancloud_Message>());
    //	sm = check_and_cast<icancloud_Message *>(msg);

    operation = sm->getOperation();
    // Established connection message...
    if (!strcmp(pkt->getName(), "ESTABLISHED")) {
        pat_connectionStablished(sm);
    }
    // Closing connection message ..
    else if (!strcmp(pkt->getName(), "PEER_CLOSED")) {
        //			¿?
    }
    // Finished connection message ..
    else if (!strcmp(pkt->getName(), "CLOSED")) {
        //			¿?
    }
//		else if (operation == "CLOSE_VM"){
//
//			pat_closeVM (sm);
//
//		}

    // Not an ESTABLISHED message message...
    else {
        // Request!
        if (!sm->getIsResponse()) {
            // Create a new connection... client-side
            if ((operation == SM_LISTEN_CONNECTION)
                    || (operation == SM_MIGRATION_REQUEST_LISTEN)) {
            }

            // Send data...
            else if ((operation == SM_OPEN_FILE) || (operation == SM_CLOSE_FILE)
                    || (operation == SM_READ_FILE)
                    || (operation == SM_WRITE_FILE)
                    || (operation == SM_CREATE_FILE)
                    || (operation == SM_DELETE_FILE)
                    || (operation == SM_SEND_DATA_NET)
                    || (operation == SM_ITERATIVE_PRECOPY)
                    || (operation == SM_STOP_AND_DOWN_VM)
                    || (operation == SM_VM_ACTIVATION)
                    || (operation == MPI_SEND) || (operation == MPI_RECV)
                    || (operation == MPI_BARRIER_UP)
                    || (operation == MPI_BARRIER_DOWN)
                    || (operation == MPI_BCAST) || (operation == MPI_SCATTER)
                    || (operation == MPI_GATHER)) {
                pat_sendMessage(sm);
            }

            // Close connection...
            else if (operation == SM_CLOSE_CONNECTION) {

                pat_closeConnection(sm);

            }

            // Operation does not change the port...
            else {
//					nullptr;
            }
        }
        // The message is a response
        else {
            pat_receiveMessage(sm);
        }
    }
    pkt->insertAtFront(sm);
}

void PortAddressTranslation::pat_createVM (int userID, int vmID, string vmIP){
	User_VirtualPort_Cell* user;
	Vm_VirtualPort_Cell* vm;

	// Search the user into the structure
		user = searchUser(userID);

	// If the user does not exists, create it ..
		if (user == nullptr){
			user = newUser(userID);
		}

	// Search vm..
		vm = user->searchVM(vmID);

	// initialize the vm..
		if (vm == nullptr){
			vm = user->newVM(vmID);
			vm->setVMIP(vmIP.c_str());
		} else if (vm->getVMIP() != vmIP){
			throw cRuntimeError("PortAddressTranslation::pat_createVM error!. vmID:%i have different ips!!",userID);
		} else {
		    // Another job has been instanced in a vm..
		}
}

User_VirtualPort_Cell* PortAddressTranslation::searchUser (int uId){

	// Define ..
		bool found;
		unsigned int i;
		User_VirtualPort_Cell* user_cell;

	// Init ..
		found = false;
		user_cell = nullptr;

	for (i = 0; (i < user_vm_ports.size()) && (!found);){

		if ((*(user_vm_ports.begin()+i))->getUserID() == uId){
			found = true;
			user_cell = (*(user_vm_ports.begin()+i));
		}else {
			i++;
		}
	}

	return user_cell;

}

void PortAddressTranslation::deleteUser (int uId){
	// Define ..
		bool found;
		unsigned int i;

	// Init ..
		found = false;

	for (i = 0; (i < user_vm_ports.size()) && (!found);){

		if ((*(user_vm_ports.begin()+i))->getUserID() == uId){
			found = true;
			user_vm_ports.erase(user_vm_ports.begin()+i);
		}else {
			i++;
		}
	}

}

User_VirtualPort_Cell* PortAddressTranslation::newUser(int uId){

	User_VirtualPort_Cell* user;

	user = new User_VirtualPort_Cell();

	user->setUserID(uId);

	user_vm_ports.push_back(user);

	return user;

}

int PortAddressTranslation::getNewPort(){
	int port;

	if (portHoles.size() != 0){

		port = (*portHoles.begin());
		portHoles.erase(portHoles.begin());

	} else {
		// Throws an exception to stop the execution!
		if ((portPtr == LAST_PORT) && (portHoles.size() == 0)) throw cRuntimeError("portManager::getNewPort error. There is no free ports!");

		while (freePorts[portPtr]){
			portPtr++;
		}

		port = portPtr;

		portPtr++;

	}

	freePorts[port] = STABLISHED;

	return port;
}

void PortAddressTranslation::freePort(int port){

	// Mark as free the cell into the free port
	freePorts[port] = NOT_STABLISHED;

	// push it into the port holes!
	portHoles.push_back(port);

}

int PortAddressTranslation::pat_createListen(int uId, int pId, int virtualPort){

	// Define ..
		User_VirtualPort_Cell* user;
		Vm_VirtualPort_Cell* vm;
		int realPort;

	// Search the user into the structure
		user = searchUser(uId);

		// If the user does not exists, create it ..
			if (user == nullptr){
				user = newUser(uId);
			}

	// Search the vm
		vm = user->searchVM(pId);

		// If the vm does not exists, create it ..
			if (vm == nullptr){
				vm = user->newVM(pId);
			}

	// Port forwarding
		realPort = getNewPort();

		vm->setPort(realPort, virtualPort);

		return realPort;
}

void PortAddressTranslation::pat_arrivesIncomingConnection(Ptr<icancloud_Message> &sm){

	// Define ..
		//icancloud_App_NET_Message* sm_net;
		User_VirtualPort_Cell* user;
		Vm_VirtualPort_Cell* vm;
		int uId = sm->getUid();
		int pId = sm->getPid();
		int realPort;
		int virtualPort;

	// Init ..
		auto sm_net = CHK(dynamicPtrCast<icancloud_App_NET_Message> (sm));

	// Search the user into the structure
		user = searchUser(uId);

		// If the user does not exists, create it ..
			if (user == nullptr) throw cRuntimeError("portManager::pat_connectionStablished error. There is no listen connection for the user %i!\n", uId);

	// Search the vm
		vm = user->searchVM(pId);

		// If the vm does not exists, create it ..
			if (vm == nullptr)throw cRuntimeError("portManager::pat_connectionStablished error. There is no vm connection for the incoming message %s!\n", pId);

	//Allocate the connectionID
		vm->setConnectionID(sm_net->getLocalPort(), sm_net->getConnectionId());

	// Port forwarding
		virtualPort = sm_net->getLocalPort();
		realPort = vm->getRealPort(virtualPort);

		sm_net->setLocalPort(realPort);
		sm_net->setVirtual_localPort(virtualPort);
}

int PortAddressTranslation::pat_connectionStablished(Ptr<icancloud_Message> &sm){

	// Define ..
		//icancloud_App_NET_Message* sm_net;
		User_VirtualPort_Cell* user;
		Vm_VirtualPort_Cell* vm;
        int uId = sm->getUid();
        int pId = sm->getPid();
		int realPort;


	// Init ..
	//	sm_net = check_and_cast <icancloud_App_NET_Message*> (sm);
		auto sm_net = CHK(dynamicPtrCast<icancloud_App_NET_Message> (sm));

	// Search the user into the structure
		user = searchUser(uId);

		// If the user does not exists, create it ..
			if (user == nullptr){
				user = newUser(uId);
			}

	// Search the vm
		vm = user->searchVM(pId);

		// If the vm does not exists, create it ..
			if (vm == nullptr){
				vm = user->newVM(pId);
			}

	//Allocate the connectionID
		realPort = sm_net->getLocalPort();

		freePorts[realPort] = STABLISHED;

		vm->setPort(realPort, realPort);
		vm->setConnectionID(realPort, sm_net->getConnectionId());

	return realPort;

}

void PortAddressTranslation::pat_receiveMessage(Ptr<icancloud_Message> &sm){

	// Define ..
		//	icancloud_App_NET_Message* sm_net;
			User_VirtualPort_Cell* user;
			Vm_VirtualPort_Cell* vm;
	        int uId = sm->getUid();
	        int pId = sm->getPid();
			int realPort;
			int virtualPort;

		// Init ..
		//	sm_net = check_and_cast <icancloud_App_NET_Message*> (sm);
			auto sm_net = CHK(dynamicPtrCast<icancloud_App_NET_Message> (sm));

		// Search the user into the structure
			user = searchUser(uId);

			// If the user does not exists, create it ..
				if (user == nullptr) throw cRuntimeError("portManager::pat_connectionStablished error. There is no listen connection for the user %i!\n", uId);

		// Search the vm
			vm = user->searchVM(pId);

			// If the vm does not exists, create it ..
				if (vm == nullptr)throw cRuntimeError("portManager::pat_connectionStablished error. There is no vm connection for the incoming message %i!\n", pId);


		// PAT
			//Get the virtual and real ports
				realPort = sm_net->getDestinationPort();
				virtualPort = vm->getVirtualPort(realPort);

			// Reallocate in the message the ports
				sm_net->setDestinationPort(virtualPort);
				sm_net->setVirtual_destinationPort(realPort);

}

void PortAddressTranslation::pat_sendMessage(Ptr<icancloud_Message> &sm) {
    // Define ..
    //icancloud_App_NET_Message* sm_net;
    User_VirtualPort_Cell* user;
    Vm_VirtualPort_Cell* vm;
    int uId = sm->getUid();
    int pId = sm->getPid();
    int realPort;
    int virtualPort;

    // Init ..
    //	sm_net = check_and_cast <icancloud_App_NET_Message*> (sm);
    auto sm_net = CHK(dynamicPtrCast<icancloud_App_NET_Message>(sm));

    // Search the user into the structure
    user = searchUser(uId);

    // If the user does not exists, create it ..
    if (user == nullptr)
        throw cRuntimeError(
                "portManager::pat_connectionStablished error. There is no listen connection for the user %s!\n",
                uId);

    // Search the vm
    vm = user->searchVM(pId);

    // If the vm does not exists, create it ..
    if (vm == nullptr)
        throw cRuntimeError(
                "portManager::pat_connectionStablished error. There is no vm connection for the incoming message %s!\n",
                pId);

    // PAT
    //Get the virtual and real ports
    virtualPort = sm_net->getLocalPort();
    realPort = vm->getRealPort(virtualPort);

    // Reallocate in the message the ports
    sm_net->setLocalPort(realPort);
    sm_net->setVirtual_localPort(virtualPort);
}

int PortAddressTranslation::pat_closeConnection(Ptr<icancloud_Message> &sm) {
    // Define ..
    //icancloud_App_NET_Message* sm_net;
    User_VirtualPort_Cell* user;
    Vm_VirtualPort_Cell* vm;
    int uId = sm->getUid();
    int pId = sm->getPid();
    int realPort;
    int virtualPort;
    int connID;

    // TODO: COMPROBAR QUE SE DEVULEVE BIEN EL VALOR

    // Init ..
    //sm_net = check_and_cast <icancloud_App_NET_Message*> (sm);
    auto sm_net = CHK(dynamicPtrCast<icancloud_App_NET_Message>(sm));

    // Search the user into the structure
    user = searchUser(uId);

    // If the user does not exists, create it ..
    if (user == nullptr)
        throw cRuntimeError(
                "portManager::pat_connectionStablished error. There is no listen connection for the user %s!\n",
                uId);

    // Search the vm
    vm = user->searchVM(pId);

    // If the vm does not exists, create it ..
    if (vm == nullptr)
        throw cRuntimeError(
                "portManager::pat_connectionStablished error. There is no vm connection for the incoming message %s!\n",
                pId);

    // PAT
    //Get the virtual port assigned during the connection
    virtualPort = sm_net->getLocalPort();

    connID = vm->getConnectionID(virtualPort);

    if (connID != sm->getConnectionId())
        throw cRuntimeError(
                "portManager::pat_closeConnection error. Connection id doesn't match %i != %i to free ports!\n",
                connID, sm->getConnectionId());

    vm->deletePort_byConnectionID(connID);

    // If the port is busy by other user, get a new virtual port
    realPort = vm->getRealPort(virtualPort);

    freePort(realPort);
    portHoles.push_back(realPort);

    sm_net->setLocalPort(realPort);
    sm_net->setVirtual_localPort(virtualPort);

    return realPort;

}

vector<int> PortAddressTranslation::pat_closeVM(int uId, int pId) {

    // Define ..
    User_VirtualPort_Cell* user;
    Vm_VirtualPort_Cell* vm;
    vector<int> rPorts;
    vector<int> connIds;
    bool connectedPorts;

    // Init ..
    rPorts.clear();
    connIds.clear();
    connectedPorts = false;

    // Search the user into the structure
    user = searchUser(uId);

    // If the user does not exists, create it ..
    if (user == nullptr)
        throw cRuntimeError(
                "portManager::pat_closeConnection error. There is no user %i - vm %i to free ports!\n",
                uId, pId);

    // Search the vm
    vm = user->searchVM(pId);

    // If the vm does not exists, create it ..
    if (vm == nullptr)
        printf(
                "portManager::pat_closeConnection error. There is no open ports for %i to free ports!\n",
                pId);
    else {
        // PAT
        //Get the virtual port assigned during the connection
        rPorts = vm->getAllRPorts();

        connectedPorts = (rPorts.size() != 0);

        while (rPorts.size() != 0) {
            freePort((*(rPorts.begin())));
            rPorts.erase(rPorts.begin());
        }

        // Get the connIDs
        if (connectedPorts)
            connIds = vm->getAllConnectionIDs();

        // erase the vm
        user->eraseVM(pId);

        // If the user has not more vms, erase the entry
        if (user->getVM_Size() == 0) {
            deleteUser(uId);
        }
    }

    return connIds;
}

} // namespace icancloud
} // namespace inet

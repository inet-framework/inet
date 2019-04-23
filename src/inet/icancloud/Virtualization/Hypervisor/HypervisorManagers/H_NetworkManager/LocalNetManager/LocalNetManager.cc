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
#include "inet/applications/common/SocketTag_m.h"
#include "inet/icancloud/Virtualization/Hypervisor/HypervisorManagers/H_NetworkManager/LocalNetManager/LocalNetManager.h"

namespace inet {

namespace icancloud {


Define_Module(LocalNetManager);

LocalNetManager::~LocalNetManager() {
	// TODO Auto-generated destructor stub
}

void LocalNetManager::initialize(int stage) {
    icancloud_Base::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        pat = new PortAddressTranslation();
        ip_LocalNode.reset();
    }
}

void LocalNetManager::finish() {
}

cGate* LocalNetManager::getOutGate (cMessage *msg){
    return nullptr;
}

void LocalNetManager::processSelfMessage (cMessage *msg){

}

void LocalNetManager::processRequestMessage (Packet *){

}

void LocalNetManager::processResponseMessage (Packet *){

}

void LocalNetManager::initializePAT (const L3Address &nodeIP){
	ip_LocalNode = nodeIP;
	pat->pat_initialize(nodeIP);

	cModule* networkmanager_mod;

	networkmanager_mod =  getParentModule()->getParentModule()->getParentModule()->getParentModule()->getSubmodule("networkManager");
	netManagerPtr = check_and_cast <NetworkManager*> (networkmanager_mod);

	if (netManagerPtr == nullptr){
		showErrorMessage("LocalNetManager::initializePAT --> Error. Net Manager Pointer is nullptr .. ");
	}

}

void LocalNetManager::createVM(Packet* pkt){

	// Define ..
		//icancloud_App_NET_Message* sm_net;
    const auto &sm = pkt->peekAtFront<icancloud_Message>();
	// Init ..
    const auto &sm_net = CHK(dynamicPtrCast<const icancloud_App_NET_Message> (sm));
    if (sm_net == nullptr)
        throw cRuntimeError("Header error");

	// create the user into the structure
		pat->pat_createVM(sm->getUid(), sm->getPid(), sm_net->getLocalIP());
}

void LocalNetManager::manage_listen(Packet* pkt){

	//icancloud_App_NET_Message* sm_net;
	int realPort;
	int virtualPort;
	pkt->trimFront();
	auto sm = pkt->removeAtFront<icancloud_Message>();
	auto sm_net = CHK(dynamicPtrCast<icancloud_App_NET_Message> (sm));
    if (sm_net == nullptr)
        throw cRuntimeError("Header error");

	//sm_net = dynamic_cast <icancloud_App_NET_Message*> (sm);

	virtualPort = sm_net->getLocalPort();
	realPort = pat->pat_createListen(sm->getUid(), sm->getPid(), virtualPort);

	sm_net->setLocalPort(realPort);
	sm_net->setVirtual_localPort(virtualPort);
	pkt->insertAtFront(sm);

	netManagerPtr->registerPort(sm->getUid(), ip_LocalNode.toIpv4().str(false), sm->getPid(), realPort, virtualPort, LISTEN);
}

int LocalNetManager::manage_create_storage_Connection(Packet* pkt) {

    //icancloud_App_NET_Message* sm_net;
    int realPort;
    int virtualPort;
    string destinationIP, localIP;
    int decision;

    //sm_net = dynamic_cast <icancloud_App_NET_Message*> (sm);

    pkt->trimFront();
    auto sm = pkt->removeAtFront<icancloud_Message>();
    auto sm_net = CHK(dynamicPtrCast<icancloud_App_NET_Message>(sm));
    if (sm_net == nullptr)
        throw cRuntimeError("Header error");

    // The ips are correct by definition from the RemoteStorageApp..
    destinationIP = sm_net->getDestinationIP();
    localIP = sm_net->getLocalIP();
    sm_net->setVirtual_user(sm_net->getUid());


    // Get the ports for the connection..
    // Get the port from the message (virtual) and set it in its field
    virtualPort = sm_net->getDestinationPort();

    // Get the virtual port
    realPort = netManagerPtr->getRealPort(destinationIP, virtualPort, sm_net->getUid(), sm_net->getPid());

    // The port is not found .. maybe it will be open soon? ..
    if (realPort == PORT_NOT_FOUND) {
        decision = -1;
    }
    // if the port is listening ..
    else {
        // The port that user believe that is open
        sm_net->setVirtual_destinationPort(virtualPort);
        // The real open port waiting ..
        sm_net->setDestinationPort(realPort);
        // node IP
        sm_net->setLocalIP(ip_LocalNode.toIpv4().str(false).c_str());
        sm_net->setVirtual_destinationIP(destinationIP.c_str());
        // VM ip
        sm_net->setVirtual_localIP(localIP.c_str());

        decision = 0;
    }

    pkt->insertAtFront(sm_net);
    return decision;
}


int LocalNetManager::manage_createConnection(Packet* pkt){

	//icancloud_App_NET_Message* sm_net;
	string virtual_destinationIP, virtual_localIP;
	string destinationIP, localIP;
	int realDestinationPort;
	int virtualDestinationPort;
	int decision;

//	sm_net = dynamic_cast <icancloud_App_NET_Message*> (sm);
	pkt->trimFront();
    auto sm = pkt->removeAtFront<icancloud_Message>();
    auto sm_net = CHK(dynamicPtrCast<icancloud_App_NET_Message> (sm));
    if (sm_net == nullptr)
        throw cRuntimeError("Header error");


	// Get the destinationIP (vm) and the local ip (vm)
    virtual_destinationIP = sm_net->getDestinationIP();
    virtual_localIP = sm_net->getLocalIP();

    sm_net->setVirtual_user(sm_net->getUid());

    // Get the port from the message (virtual) and set it in its field
    virtualDestinationPort = sm_net->getDestinationPort();
    destinationIP = netManagerPtr->searchNodeIP(virtual_destinationIP, sm_net->getUid());

    if (destinationIP.empty()){
        showErrorMessage("LocalNetManager::manage_createConnection-> destination IP for the UserPid(%i) - vmID: %i and virtual destination IP: %s is nullptr", sm->getUid(), sm->getPid(), virtual_destinationIP.c_str());
    }

	// Get the virtual port
    realDestinationPort = netManagerPtr->getRealPort (destinationIP, virtualDestinationPort, sm_net->getUid(), netManagerPtr->getVMid(virtual_destinationIP,sm_net->getUid()));

	// The port is not found .. maybe it will be open soon? ..
    if (realDestinationPort == PORT_NOT_FOUND){
        decision = -1;
    }
    // if the port is listening ..
    else {
        // Set the virtual ip's in the message
        sm_net->setVirtual_destinationIP(virtual_destinationIP.c_str());
        sm_net->setVirtual_localIP(virtual_localIP.c_str());
        // Set real destination and local ips of the nodes
        sm_net->setLocalIP(ip_LocalNode.toIpv4().str(false).c_str());
        sm_net->setDestinationIP(destinationIP.c_str());
        // Set the ports for the connection..
        sm_net->setVirtual_destinationPort(virtualDestinationPort);
        sm_net->setDestinationPort(realDestinationPort);

		decision = 0;
    }
    pkt->insertAtFront(sm_net);
    return decision;
}

void LocalNetManager::connectionStablished(Packet * pkt) {

    //icancloud_App_NET_Message* sm_net;
    int localPort;
    int virtual_localPort;
    int virtual_destinationPort;
    int destinationPort;

    int vmID;
    int uId;

    string virtual_destinationIP, destination_ip;
    string virtual_localIP, localIP;
    pkt->trimFront();
    auto sm = pkt->removeAtFront<icancloud_Message>();
    auto sm_net = CHK(dynamicPtrCast<icancloud_App_NET_Message>(sm));
    if (sm_net == nullptr)
        throw cRuntimeError("Header error");

    // Init ..
    //	sm_net = dynamic_cast <icancloud_App_NET_Message*> (sm);

    // Get the id's
    vmID = sm_net->getUid();
    uId = sm_net->getPid();

    // Get the ips
    virtual_destinationIP = sm_net->getVirtual_destinationIP();
    destination_ip = sm_net->getDestinationIP();
    virtual_localIP = sm_net->getVirtual_localIP();
    localIP = sm_net->getLocalIP();

    // Get the port from the message (virtual) and set it in its field
    localPort = sm_net->getLocalPort();
    virtual_destinationPort = sm_net->getVirtual_destinationPort();
    destinationPort = sm_net->getDestinationPort();

    // Set the virtual destination ip
    sm_net->setDestinationIP(virtual_destinationIP.c_str());
    sm_net->setVirtual_destinationIP(destination_ip.c_str());

    // Set the virtual local ip of the virtual machine
    sm_net->setLocalIP(virtual_localIP.c_str());
    sm_net->setVirtual_localIP(localIP.c_str());

    // Set the connection stablished in the port address translator of the local node
    virtual_localPort = pat->pat_connectionStablished(sm);

    // Register the port in the net manager for possible ask for the port state by other nodes
    netManagerPtr->registerPort(uId, ip_LocalNode.toIpv4().str(false), vmID, localPort,
            virtual_localPort, CONNECT);

    // Set the virtual local port as the new port for the user.
    sm_net->setLocalPort(virtual_localPort);
    sm_net->setVirtual_localPort(localPort);

    // Exchange the destination ports
    sm_net->setVirtual_destinationPort(destinationPort);
    sm_net->setDestinationPort(virtual_destinationPort);

    pkt->insertAtFront(sm_net);
}

vector<Packet*> LocalNetManager::manage_close_connections(
        int uId, int pId) {

    //icancloud_App_NET_Message *sm_close_connection;
    string userID;
    vector<int> connectionIDs;
    unsigned int i;
    vector<Packet *> sm_vector;

    // Init ..
    connectionIDs.clear();
    sm_vector.clear();

    // Delete the vm from the virtual manager ipUserSet
    netManagerPtr->deleteVirtualIP_by_VMID(pId, uId);

    // Delete port entries associated to the vm from the virtual manager port table.
    netManagerPtr->freeAllPortsOfVM(ip_LocalNode.toIpv4().str(false).c_str(), pId, uId);

    // get all the connectionIDs to close the connections from the Local net manager (PAT)..
    connectionIDs = getConnectionsIDs(uId, pId);

    for (i = 0; i < connectionIDs.size(); i++) {
        // Build the message for closing connection (node host)
        auto sm_close_connection = makeShared<icancloud_App_NET_Message>();

        sm_close_connection->setOperation(SM_CLOSE_CONNECTION);
        sm_close_connection->setUid(uId);
        sm_close_connection->setPid(pId);
        sm_close_connection->setVirtual_user(uId);
        sm_close_connection->setConnectionId((*(connectionIDs.begin() + i)));

        auto pktClose = new Packet("icancloud_App_NET_Message");
        pktClose->insertAtFront(sm_close_connection);

        pktClose->setKind(TCP_C_CLOSE);
        //Build the parameters for close the socket

        TcpCommand *cmd = new TcpCommand();
        //cmd->setConnId((*(connectionIDs.begin() + i)));
        pktClose->addTagIfAbsent<SocketReq>()->setSocketId((*(connectionIDs.begin() + i)));

        pktClose->setControlInfo(cmd);
        sm_vector.push_back(pktClose);
    }

    return sm_vector;
}

void LocalNetManager::manage_receiveMessage(Packet *pkt){

	//icancloud_App_NET_Message* sm_net;
	string virtual_destinationIP, virtual_localIP;
	int realDestinationPort;
	int virtualDestinationPort;
	pkt->trimFront();
    auto sm = pkt->removeAtFront<icancloud_Message>();
    auto sm_net = CHK(dynamicPtrCast<icancloud_App_NET_Message>(sm));
    if (sm_net == nullptr)
        throw cRuntimeError("Header error");

	//sm_net = dynamic_cast <icancloud_App_NET_Message> (sm);

	virtual_destinationIP = sm_net->getVirtual_destinationIP();
	sm_net->setDestinationIP(virtual_destinationIP.c_str());

	virtual_localIP = sm_net->getVirtual_localIP();
	sm_net->setLocalIP(virtual_localIP.c_str());

	virtualDestinationPort = sm_net->getDestinationPort();
	realDestinationPort = sm_net->getVirtual_destinationPort();
	sm_net->setVirtual_destinationPort (virtualDestinationPort);
	sm_net->setDestinationPort(realDestinationPort);

	pkt->insertAtFront(sm_net);

}

void LocalNetManager::manage_sendMessage(Packet* pkt){

	//icancloud_App_NET_Message* sm_net;
	//icancloud_MPI_Message* sm_mpi;
	string virtual_destinationIP, virtual_localIP;
	string destinationIP, localIP;
	int realDestinationPort;
	int virtualDestinationPort;

    auto sm = pkt->removeAtFront<icancloud_Message>();
    auto sm_net = dynamicPtrCast<icancloud_App_NET_Message>(sm);
    auto sm_mpi = dynamicPtrCast<icancloud_MPI_Message>(sm);

	//sm_mpi = dynamic_cast<icancloud_MPI_Message *>(sm);
	//sm_net = dynamic_cast <icancloud_App_NET_Message*> (sm);

	// Process a MPI message
	if (sm_mpi != nullptr){

	    // Get the destinationIP (vm) and the local ip (vm)

        if (sm_mpi->getVirtual_user() == -1){
            sm_mpi->setVirtual_user(sm_mpi->getUid());
        }

        // Get the port from the message (virtual) and set it in its field
        virtualDestinationPort = sm_mpi->getDestinationPort();
        virtual_destinationIP = sm_mpi->getVirtual_destinationIP();
        virtual_localIP = sm_mpi->getLocalIP();

        destinationIP = netManagerPtr->searchNodeIP(virtual_destinationIP.c_str(), sm_mpi->getUid());

	            //      if (destinationIP.empty()){
	            //          showErrorMessage("LocalNetManager::sendMessage-> destination IP for the vmID: %s and virtual destination IP: %s is nullptr", sm->getVmID(), virtual_destinationIP.c_str());
	            //      }

        // Get the virtual port
        realDestinationPort = netManagerPtr->getRealPort (destinationIP, virtualDestinationPort, sm_mpi->getUid(), sm_net->getPid());

        // Set the virtual ip's in the message
        sm_mpi->setVirtual_destinationIP(virtual_destinationIP.c_str());
        sm_mpi->setVirtual_localIP(virtual_localIP.c_str());

        // Set real destination and local ips of the nodes
        sm_mpi->setLocalIP(ip_LocalNode.toIpv4().str(false).c_str());
        sm_mpi->setDestinationIP(destinationIP.c_str());

        // Set the ports for the connection..
        sm_mpi->setVirtual_destinationPort(virtualDestinationPort);
        sm_mpi->setDestinationPort(realDestinationPort);
        pkt->insertAtFront(sm_mpi);
	}

	// Process a NET message
	else{
            // Get the destinationIP (vm) and the local ip (vm)
            virtual_destinationIP = sm_net->getDestinationIP();
            virtual_localIP = sm_net->getLocalIP();

            sm_net->setVirtual_user(sm->getUid());

            // Get the port from the message (virtual) and set it in its field
            virtualDestinationPort = sm_net->getDestinationPort();

            destinationIP = netManagerPtr->searchNodeIP(virtual_destinationIP, sm_net->getUid());

            // Get the virtual port
            realDestinationPort = netManagerPtr->getRealPort (destinationIP, virtualDestinationPort, sm_net->getUid(), sm_net->getPid());

            // Set the virtual ip's in the message
            sm_net->setVirtual_destinationIP(virtual_destinationIP.c_str());
            sm_net->setVirtual_localIP(virtual_localIP.c_str());

            // Set real destination and local ips of the nodes
            sm_net->setLocalIP(ip_LocalNode.toIpv4().str(false).c_str());
            sm_net->setDestinationIP(destinationIP.c_str());

            // Set the ports for the connection..
            sm_net->setVirtual_destinationPort(virtualDestinationPort);
            sm_net->setDestinationPort(realDestinationPort);
            pkt->insertAtFront(sm_net);
	}

}

void LocalNetManager::manage_close_single_connection(Packet *pkt){

	//icancloud_App_NET_Message *sm_close_connection;

	int userID;
	int vmID;

	// Init ..
	pkt->trimFront();
    auto sm = pkt->removeAtFront<icancloud_Message>();
    auto sm_close_connection = CHK(dynamicPtrCast<icancloud_App_NET_Message>(sm));
    if (sm_close_connection == nullptr)
        throw cRuntimeError("Header error");

    //sm_close_connection = check_and_cast<icancloud_App_NET_Message*>(sm);

    // Get the user id from the vmID
    vmID = sm->getPid();
    userID = sm->getUid();

    // get the virtual IP from the Local net manager (PAT)..
    pat->pat_closeConnection(sm);

    // Delete the port entry associated to the network manager port table.
    netManagerPtr->freePortOfVM(ip_LocalNode.toIpv4().str(false), userID, vmID,  sm_close_connection->getLocalPort());

    pkt->setKind(TCP_C_CLOSE);
    //Build the parameters for close the socket

    if (pkt->getControlInfo())
        delete pkt->removeControlInfo();

    TcpCommand *cmd = new TcpCommand();
    //cmd->setConnId((*(connectionIDs.begin() + i)));
    pkt->addTagIfAbsent<SocketReq>()->setSocketId(sm_close_connection->getConnectionId());
    pkt->setControlInfo(cmd);

    pkt->insertAtFront(sm_close_connection);
    //Build the parameters for close the socket
}

vector<int> LocalNetManager::getConnectionsIDs(int uId, int pId){

	//vector<User_VirtualPort_Cell*>::iterator it;
	vector<int> connectionIDs;

	connectionIDs = pat->pat_closeVM(uId, pId);

	return connectionIDs;
}

} // namespace icancloud
} // namespace inet

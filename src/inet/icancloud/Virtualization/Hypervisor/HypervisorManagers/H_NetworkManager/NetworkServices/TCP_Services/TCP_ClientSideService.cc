#include "inet/icancloud/Virtualization/Hypervisor/HypervisorManagers/H_NetworkManager/NetworkServices/TCP_Services/TCP_ClientSideService.h"
#include "inet/icancloud/Virtualization/Hypervisor/HypervisorManagers/H_NetworkManager/NetworkServices/NetworkService/NetworkService.h"

namespace inet {

namespace icancloud {


TCP_ClientSideService::TCP_ClientSideService(string newLocalIP,						 
											cGate* toTCP,
											NetworkService *netService){
    localIP = newLocalIP;    
    outGate_TCP = toTCP;
    networkService = netService;
}


TCP_ClientSideService::~TCP_ClientSideService(){
	socketMap.deleteSockets ();
	connections.clear();
}


void TCP_ClientSideService::createConnection(Packet  *pktSm){

	clientTCP_Connector newConnection;
	//icancloud_App_NET_Message *sm_net;

	const auto &sm = pktSm->peekAtFront<icancloud_Message>();
	const auto &sm_net = dynamicPtrCast<const icancloud_App_NET_Message>(sm);
	// Cast to icancloud_App_NET_Message
	//sm_net = dynamic_cast<icancloud_App_NET_Message *>(sm);
	// Wrong message?
	if (sm_net == nullptr)
	    networkService->showErrorMessage ("[createConnection] Error while casting to icancloud_App_NET_Message!");
	
		
	// Attach the message to the corresponding connection
	newConnection.msg = pktSm;
	// Create the socket
	newConnection.socket = new TcpSocket();
	//newConnection.socket->setDataTransferMode(TCP_TRANSFER_OBJECT);
	newConnection.socket->bind(*(localIP.c_str()) ? L3Address(localIP.c_str()) : L3Address(), -1);
	newConnection.socket->setCallback(this);
	newConnection.socket->setOutputGate(outGate_TCP);
	//newConnection.socket->renewSocket();
	// Add new socket to socketMap
	socketMap.addSocket (newConnection.socket);
	// Add new connection to vector
	connections.push_back (newConnection);
	// Debug...
	if (DEBUG_TCP_Service_Client)
	    networkService->showDebugMessage ("[createConnection] local IP:%s ---> %s:%d. %s",
	            localIP.c_str(),
	            sm_net->getDestinationIP(),
	            sm_net->getDestinationPort(),
	            sm_net->contentsToString(DEBUG_TCP_Service_MSG_Client).c_str());
	// Establish Connection
	newConnection.socket->connect(L3AddressResolver().resolve(sm_net->getDestinationIP()), sm_net->getDestinationPort());
}

void TCP_ClientSideService::sendPacketToServer(Packet *pktSm) {

    int index;
    const auto &sm = pktSm->peekAtFront<icancloud_Message>();
    // Search for the connection...

    index = searchConnectionByConnId(sm->getConnectionId());

    // Connection not found?
    if (index == NOT_FOUND)
        networkService->showErrorMessage(
                "[sendPacketToServer] Socket not found!");

    else {

        if (DEBUG_TCP_Service_Client)
            networkService->showDebugMessage(
                    "[sendPacketToServer] Sending message to %s:%d. %s",
                    connections[index].socket->getRemoteAddress().str().c_str(),
                    connections[index].socket->getRemotePort(),
                    sm->contentsToString(DEBUG_TCP_Service_MSG_Client).c_str());

        connections[index].socket->send(pktSm);
    }
} 




void TCP_ClientSideService::closeConnection(Packet *pktSm) {

    inet::TcpSocket *socket;

    // Search for the socket
    socket = check_and_cast<TcpSocket *>(socketMap.findSocketFor(pktSm));
    const auto &sm = pktSm->peekAtFront<icancloud_Message>();
    if (!socket)
        networkService->showErrorMessage(
                "[closeConnection] Socket not found to send the message: %s\n",
                sm->contentsToString(true).c_str());
    else {

        if (DEBUG_TCP_Service_Client)
            networkService->showDebugMessage("Closing connection %s:%d. %s",
                    socket->getRemoteAddress().str().c_str(),
                    socket->getRemotePort(),
                    sm->contentsToString(DEBUG_TCP_Service_MSG_Client).c_str());
        socket->close();

        auto socketAux = socketMap.removeSocket(socket);

        if (socketAux != socket)
            throw cRuntimeError("Socket not found");

        delete (socket);
    }

    delete (pktSm);
}

int TCP_ClientSideService::searchConnectionByConnId(int connId) {

    int result;
    bool found;
    unsigned int i;

    // Init
    found = false;
    i = 0;

    // Search for the connection...
    while ((!found) && (i < connections.size())) {
        if (connections[i].socket->getSocketId() == connId)
            found = true;
        else
            i++;
    }

    // Set the result
    if (found)
        result = i;
    else
        result = NOT_FOUND;

    return result;
}

TcpSocket* TCP_ClientSideService::getInvolvedSocket(cMessage *msg) {
    return (dynamic_cast<TcpSocket *>(socketMap.findSocketFor(msg)));
}

void TCP_ClientSideService::socketEstablished(TcpSocket *socket) {

    int index;
    //icancloud_App_NET_Message *sm_net;

    int connId = socket->getSocketId();
    // Search for the connection...
    index = searchConnectionByConnId(connId);

    // Connection not found?
    if (index == NOT_FOUND)
        networkService->showErrorMessage(
                "[socketEstablished] Socket not found!");

//	 	// Retry
//	 	else if (connId < 0){
//
//	 		// Cast
//			sm_net = dynamic_cast<icancloud_App_NET_Message *>(connections[index].msg);
//
//			// Try again!ç
//			connections[index].socket->renewSocket();
//	 		connections[index].socket->connect(IPAddressResolver().resolve(sm_net->getDestinationIP()), sm_net->getDestinationPort());
//
//	 	}

    // All OK!
    else {

        // Cast
        auto pktSm = connections[index].msg;
        pktSm->trimFront();
        auto sm_net = pktSm->removeAtFront<icancloud_App_NET_Message>();
        //sm  = dynamic_cast<icancloud_App_NET_Message *>(connections[index].msg);

        // Detach
        connections[index].msg = nullptr;

        // update msg status
        sm_net->setIsResponse(true);
        sm_net->setConnectionId(connId);
        sm_net->setLocalIP(
                connections[index].socket->getLocalAddress().str().c_str());
        sm_net->setLocalPort(connections[index].socket->getLocalPort());

        // debug...
        if (DEBUG_TCP_Service_Client)
            networkService->showDebugMessage(
                    "[socketEstablished-client] %s Connection established with %s:%d",
                    localIP.c_str(),
                    connections[index].socket->getRemoteAddress().str().c_str(),
                    connections[index].socket->getRemotePort());

        pktSm->insertAtFront(sm_net);

        // Send back the response
        networkService->sendResponseMessage(pktSm);
    }
}

void TCP_ClientSideService::socketDataArrived(TcpSocket* socket, Packet *packet, bool urgent) {

    //icancloud_Message *sm;
    int index;

    const auto &sm = packet->peekAtFront<icancloud_Message>();

    // Casting
    //sm = dynamic_cast<icancloud_Message *>(msg);

    // Show debug?
    if (DEBUG_TCP_Service_Client) {

        index = searchConnectionByConnId(socket->getSocketId());

        // Connection not found?
        if (index == NOT_FOUND)
            networkService->showErrorMessage(
                    "[socketDataArrived] Socket not found!");

        networkService->showDebugMessage("Arrived data from %s:%d. %s",
                connections[index].socket->getRemoteAddress().str().c_str(),
                connections[index].socket->getRemotePort(),
                sm->contentsToString(DEBUG_TCP_Service_MSG_Client).c_str());
    }

    // Send back the message...
    networkService->sendResponseMessage(packet);
}

void TCP_ClientSideService::socketPeerClosed(TcpSocket *socket){
    if (DEBUG_TCP_Service_Client)
        networkService->showDebugMessage("Socket Peer closed");
}

void TCP_ClientSideService::socketClosed(TcpSocket *socket) {
    if (DEBUG_TCP_Service_Client)
        networkService->showDebugMessage("Socket closed");
}

void TCP_ClientSideService::socketFailure(TcpSocket *socket, int code) {
    if (DEBUG_TCP_Service_Client)
        networkService->showDebugMessage("Socket Failure");
}

void TCP_ClientSideService::printSocketInfo(inet::TcpSocket *socket) {

    printf("Socket info. ConnId:%d\n", socket->getSocketId());
    printf(" - Source IP:%s   Source Port:%d",
            socket->getLocalAddress().str().c_str(), socket->getLocalPort());
    printf(" - Destination IP:%s - Destination Port:%d\n",
            socket->getRemoteAddress().str().c_str(), socket->getRemotePort());
}


} // namespace icancloud
} // namespace inet

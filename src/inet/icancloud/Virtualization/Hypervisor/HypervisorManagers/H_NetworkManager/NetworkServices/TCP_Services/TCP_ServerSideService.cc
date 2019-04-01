#include "inet/icancloud/Virtualization/Hypervisor/HypervisorManagers/H_NetworkManager/NetworkServices/TCP_Services/TCP_ServerSideService.h"
#include "inet/icancloud/Virtualization/Hypervisor/HypervisorManagers/H_NetworkManager/NetworkServices/NetworkService/NetworkService.h"
#include "inet/applications/common/SocketTag_m.h"

namespace inet {

namespace icancloud {


TCP_ServerSideService::TCP_ServerSideService(string newLocalIP,			    										    						
				    						cGate* toTCP,
				    						cGate* toicancloudAPI,
				    						NetworkService *netService){

		// Init...
		localIP = newLocalIP;	    	    
	    outGate_TCP = toTCP;
	    outGate_icancloudAPI = toicancloudAPI;
	    networkService = netService;	    	    
}


TCP_ServerSideService::~TCP_ServerSideService(){
	socketMap.deleteSockets ();		
}


void TCP_ServerSideService::newListenConnection(Packet *pkt) {

    inet::TcpSocket *newSocket;
    //icancloud_App_NET_Message *sm_net;
    serverTCP_Connector newConnection;

    const auto &sm = pkt->peekAtFront<icancloud_Message>();
    const auto & sm_net = dynamicPtrCast<const icancloud_App_NET_Message>(sm);

    // Cast to icancloud_App_NET_Message
    //	sm_net = dynamic_cast<icancloud_App_NET_Message *>(sm);

    // Wrong message?
    if (sm_net == nullptr)
        networkService->showErrorMessage(
                "[TCP_ServerSideService::newListenConnection] Error while casting to icancloud_App_NET_Message!");

    // Exists a previous connection listening at the same port
    if (existsConnection(sm_net->getLocalPort()))
        networkService->showErrorMessage(
                "[TCP_ServerSideService::newListenConnection] This connection already exists. Port:%d!",
                sm_net->getLocalPort());

    // Connection entry
    newConnection.port = sm_net->getLocalPort();
    newConnection.appIndex = sm_net->getNextModuleIndex();
    connections.push_back(newConnection);

    // Create a new socket
    newSocket = new inet::TcpSocket();

    // Listen...

    newSocket->setOutputGate(outGate_TCP);
    //newSocket->setDataTransferMode(TCP_TRANSFER_OBJECT);
    newSocket->bind(*(localIP.c_str()) ? L3Address(localIP.c_str()) : L3Address(),
            sm_net->getLocalPort());
    newSocket->setCallback(this);
    newSocket->listen();

    // Debug...
    if (DEBUG_TCP_Service_Server)
        networkService->showDebugMessage(
                "[TCP_ServerSideService] New connection for App[%d] is listening on %s:%d",
                sm_net->getNextModuleIndex(), localIP.c_str(),
                sm_net->getLocalPort());

    // Delete msg
    delete (pkt);
}

void TCP_ServerSideService::arrivesIncommingConnection(cMessage *msg) {

    TcpSocket *socket;

    // Search an existing connection...
    socket = dynamic_cast<TcpSocket *>(socketMap.findSocketFor(msg));

    // Connection does not exist. Create a new one!
    if (!socket) {

        // Create a new socket
        socket = new TcpSocket(msg);
        socket->setOutputGate(outGate_TCP);
        //socket->setDataTransferMode(TCP_TRANSFER_OBJECT);
        socket->setCallback(this);
        socketMap.addSocket(socket);

        if (DEBUG_TCP_Service_Server)
            networkService->showDebugMessage(
                    "[TCP_ServerSideService] Arrives an incoming connection from %s:%d to local connection %s:%d with connId = %d",
                    socket->getRemoteAddress().str().c_str(),
                    socket->getRemotePort(),
                    socket->getLocalAddress().str().c_str(),
                    socket->getLocalPort(), socket->getSocketId());

        // Process current operation!
        socket->processMessage(msg);
    } else
        networkService->showErrorMessage(
                "[TCP_ServerSideService::arrivesIncommingConnection] Connection already exists. ConnId =  %d",
                socket->getSocketId());
}

void TCP_ServerSideService::sendPacketToClient(Packet  *pktSm) {

    inet::TcpSocket *socket;

    // Search for the socket
    socket = dynamic_cast<TcpSocket *>(socketMap.findSocketFor(pktSm));
    const auto &sm = pktSm->peekAtFront<icancloud_Message>();
    if (!socket)
        networkService->showErrorMessage(
                "[sendPacketToClient] Socket not found to send the message: %s\n",
                sm->contentsToString(true).c_str());

    else {

        if (DEBUG_TCP_Service_Server)
            networkService->showDebugMessage(
                    "Sending message to client %s:%d. %s",
                    socket->getRemoteAddress().str().c_str(),
                    socket->getRemotePort(),
                    sm->contentsToString(DEBUG_TCP_Service_MSG_Server).c_str());

        if (pktSm->getControlInfo())
            delete pktSm->removeControlInfo();

//        TcpSendCommand *cmd = new TCPSendCommand();
//        cmd->setConnId(socket->getSocketId());
//        pktSm->setControlInfo(cmd);
        pktSm->addTagIfAbsent<SocketReq>()->setSocketId(socket->getSocketId());
        pktSm->setKind(TCP_C_SEND);
        networkService->sendResponseMessage(pktSm);

    }
}

inet::TcpSocket* TCP_ServerSideService::getInvolvedSocket(cMessage *msg) {

    return (dynamic_cast<TcpSocket *>(socketMap.findSocketFor(msg)));
}

void TCP_ServerSideService::socketDataArrived(TcpSocket* socket, Packet *packet, bool urgent) {

    //icancloud_Message *sm;
    int appIndex;

    // Casting
    packet->trimFront();
    const auto sm = packet->removeAtFront<icancloud_Message>();

    if (sm == nullptr)
        networkService->showErrorMessage(
                "[socketDataArrived] Error while casting to icancloud_Message!");

    // Get involved socket
    auto socketAux = socketMap.findSocketFor(packet);

    if (socketAux == nullptr)
        networkService->showErrorMessage(
                "[socketDataArrived] Socket not found. %s!",
                sm->contentsToString(true).c_str());

    // Search for the server App index
    appIndex = getAppIndexFromPort(socket->getLocalPort());

    if (DEBUG_TCP_Service_Server)
        networkService->showDebugMessage(
                "[socketDataArrived] Server Index:%d\n", appIndex);

    if (appIndex == NOT_FOUND)
        networkService->showErrorMessage(
                "[socketDataArrived] Port not found. %d!",
                socket->getLocalPort());

    // Set server index in app vector!
    sm->setNextModuleIndex(appIndex);

    if (DEBUG_TCP_Service_Server)
        networkService->showDebugMessage(
                "[socketDataArrived] Arrived data from %s:%d. Server Index:%d. %s",
                socket->getRemoteAddress().str().c_str(),
                socket->getRemotePort(), appIndex,
                sm->contentsToString(DEBUG_TCP_Service_MSG_Server).c_str());

    // Send message to Service Redirector
    packet->insertAtFront(sm);
    networkService->sendRequestMessage(packet, outGate_icancloudAPI);
}

void TCP_ServerSideService::socketEstablished(TcpSocket *socket) {

    if (DEBUG_TCP_Service_Server)
        networkService->showDebugMessage(
                "[socketEstablished-server] Socket establised with connId = %d",
                socket->getSocketId());
}

void TCP_ServerSideService::socketPeerClosed(TcpSocket *socket) {

    if (DEBUG_TCP_Service_Server)
        networkService->showDebugMessage("Socket Peer Closed with connId = %d",
                socket->getSocketId());
}

void TCP_ServerSideService::socketClosed(TcpSocket *socket) {

    if (DEBUG_TCP_Service_Server)
        networkService->showDebugMessage("Socket Closed with connId = %d",
                socket->getSocketId());
}

void TCP_ServerSideService::socketFailure(TcpSocket *socket, int code) {

    if (DEBUG_TCP_Service_Server)
        networkService->showDebugMessage("Socket Failure with connId = %d",
                socket->getSocketId());
}

void TCP_ServerSideService::socketStatusArrived(TcpSocket *socket, TcpStatusInfo *status) {

    if (DEBUG_TCP_Service_Server)
        networkService->showDebugMessage(
                "Socket Status Arrived with connId = %d", socket->getSocketId());
}

bool TCP_ServerSideService::existsConnection(int port) {

    bool found;
    unsigned int i;

    // init
    found = false;
    i = 0;

    // Search for the connection...
    while ((!found) && (i < connections.size())) {

        if (connections[i].port == port)
            found = true;
        else
            i++;
    }

    return found;
}

int TCP_ServerSideService::getAppIndexFromPort(int port) {

    bool found;
    unsigned int i;
    int result;

    // init
    result = NOT_FOUND;
    found = false;
    i = 0;

    // Search for the connection...
    while ((!found) && (i < connections.size())) {

        if (connections[i].port == port)
            found = true;
        else
            i++;
    }

    // Assign appIndex...
    if (found) {
        result = connections[i].appIndex;

        if (DEBUG_TCP_Service_Server)
            networkService->showDebugMessage(
                    "[TCP_server:getAppIndexFromPort] found AppIndex %d for local port %d",
                    result, port);
    } else {
        if (DEBUG_TCP_Service_Server)
            networkService->showDebugMessage(
                    "[TCP_server:getAppIndexFromPort] Not found AppIndex for local port %d",
                    port);
    }

    return result;
}

void TCP_ServerSideService::closeConnectionReceived(cMessage *sm) {

    inet::TcpSocket *socket;

    // Search for the socket
    socket = dynamic_cast<TcpSocket *>(socketMap.findSocketFor(sm));

    if (!socket) {
        delete (sm);
    } else {
        socket->close();
        auto socketAux = socketMap.removeSocket(socket);
        delete (socketAux);
        delete (sm);
    }

}


} // namespace icancloud
} // namespace inet

#include "inet/icancloud/Virtualization/Hypervisor/HypervisorManagers/H_NetworkManager/NetworkServices/NetworkService/NetworkService.h"
#include "inet/icancloud/Virtualization/Hypervisor/HypervisorManagers/H_NetworkManager/NetworkServices/TCP_Services/TCP_ClientSideService.h"
#include "inet/icancloud/Virtualization/Hypervisor/HypervisorManagers/H_NetworkManager/NetworkServices/TCP_Services/TCP_ServerSideService.h"

namespace inet {

namespace icancloud {


Define_Module (NetworkService);

NetworkService::~NetworkService(){

}


void NetworkService::initialize(int stage) {
    HWEnergyInterface::initialize (stage);
    if (stage == INITSTAGE_LOCAL) {
        std::ostringstream osStream;

        // Set the moduleIdName
        osStream << "NetworkService." << getId();
        moduleIdName = osStream.str();

        // Init the super-class

        // Module parameters
        localIP = (const char*) par("localIP");

        // Module gates
        fromNetManagerGate = gate("fromNetManager");
        fromNetTCPGate = gate("fromNetTCP");
        toNetManagerGate = gate("toNetManager");
        toNetTCPGate = gate("toNetTCP");

        // Service objects
        clientTCP_Services = new TCP_ClientSideService(localIP, toNetTCPGate,
                this);
        serverTCP_Services = new TCP_ServerSideService(localIP, toNetTCPGate,
                toNetManagerGate, this);

        nodeState = MACHINE_STATE_OFF;

        sm_vector.clear();
        lastOp = 0;
    }

}


void NetworkService::finish(){

    sm_vector.clear();
    lastOp = 0;

	// Finish the super-class
    HWEnergyInterface::finish();
}


void NetworkService::handleMessage(cMessage *msg) {

    // If msg is a Self Message...
    if (msg->isSelfMessage())
        processSelfMessage(msg);

    // Not a self message...
    else {

        // TODO: HANDLE MESSAGES TYPES

        // Established connection message...
        if (!strcmp(msg->getName(), "ESTABLISHED")) {
//			    delete(msg);
            receivedEstablishedConnection(msg);
        }

        // Closing connection message ..
        else if (!strcmp(msg->getName(), "PEER_CLOSED")) {
            serverTCP_Services->closeConnectionReceived(msg);
        }
        // Finished connection message ..
        else if (!strcmp(msg->getName(), "CLOSED")) {
            delete (msg);
        }

        // Not an ESTABLISHED message message...
        else {

            // Cast!
            auto pkt = check_and_cast<Packet *>(msg);
            const auto &sm = pkt->peekAtFront<icancloud_Message>();
            //sm = check_and_cast<icancloud_Message *>(msg);

            // Request!
            if (!sm->getIsResponse()) {

                // Update message trace
                updateMessageTrace(pkt);

                // Insert into queue
                queue.insert(pkt);

                // If not processing any request...
                processCurrentRequestMessage();
            }

            // Response message!
            else
                processResponseMessage(pkt);
        }
    }
}

cGate* NetworkService::getOutGate(cMessage *msg) {

    // If msg arrive from Service Redirector
    if (msg->getArrivalGate() == fromNetManagerGate) {
        if (gate("toNetManager")->getNextGate()->isConnected()) {
            return (toNetManagerGate);
        }
    }

    // If msg arrive from Service Redirector
    else if (msg->getArrivalGate() == fromNetTCPGate) {
        if (gate("toNetTCP")->getNextGate()->isConnected()) {
            return (toNetTCPGate);
        }
    }

    // If gate not found!
    return nullptr;
}


void NetworkService::processSelfMessage (cMessage *msg){


    if ((!strcmp (msg->getName(), "enqueued")) && ((sm_vector.size() > 0))){

        cancelEvent(msg);

        //icancloud_Message* sm;

        Packet *pkt = sm_vector.front();
        // Load the first message
        //sm = (*(sm_vector.begin()));
        const auto &sm = pkt->peekAtFront<icancloud_Message>();
        if (sm == nullptr)
            throw cRuntimeError("Header Error");

        // send the message
        clientTCP_Services->sendPacketToServer(pkt);

        // erase the message from queue
        sm_vector.erase(sm_vector.begin());

        if (sm_vector.size() > 0){
            cMessage* waitToExecuteMsg = new cMessage ("enqueued");
            scheduleAt (simTime() + 0.5, waitToExecuteMsg);
        }
        else
            // Clean the lock of last op
            lastOp = 0;
    }
    delete(msg);
}


void NetworkService::processRequestMessage(Packet *pktSm) {

    inet::TcpSocket *socket;
    int operation;
    // Msg cames from Hypervisor ...

    const auto &sm = pktSm->peekAtFront<icancloud_Message>();

    if (pktSm->getArrivalGate() == fromNetManagerGate) {

        if (DEBUG_Network_Service)
            showDebugMessage(
                    "[processRequestMessage] from Service Redirector. %s",
                    sm->contentsToString(DEBUG_MSG_Network_Service).c_str());

        // Create a new connection... client-side
        operation = sm->getOperation();
        if (operation == SM_CREATE_CONNECTION) {

            clientTCP_Services->createConnection(pktSm);
        }

        // Create a listen connection... server-side
        else if (operation == SM_LISTEN_CONNECTION) {
            serverTCP_Services->newListenConnection(pktSm);
        }

        // Send data...
        else if ((operation == SM_OPEN_FILE) || (operation == SM_CLOSE_FILE)
                || (operation == SM_READ_FILE) || (operation == SM_WRITE_FILE)
                || (operation == SM_CREATE_FILE)
                || (operation == SM_DELETE_FILE)
                || (operation == SM_SEND_DATA_NET)) {

            clientTCP_Services->sendPacketToServer(pktSm);
        }

        // Remote Storage Calls
        else if ((operation == SM_VM_REQUEST_CONNECTION_TO_STORAGE)
                || (operation == SM_NODE_REQUEST_CONNECTION_TO_MIGRATE)) {

            clientTCP_Services->createConnection(pktSm);
        }

        else if (operation == SM_MIGRATION_REQUEST_LISTEN) {

            serverTCP_Services->newListenConnection(pktSm);

        }

        // Migration calls...
        else if ((operation == SM_ITERATIVE_PRECOPY)
                || (operation == SM_STOP_AND_DOWN_VM)
                || (operation == SM_VM_ACTIVATION)) {

            clientTCP_Services->sendPacketToServer(pktSm);

        }

        // Close connection...
        else if (operation == SM_CLOSE_CONNECTION) {

            clientTCP_Services->closeConnection(pktSm);

        }

        // MPI Calls
        else if ((operation == MPI_SEND) || (operation == MPI_RECV)
                || (operation == MPI_BARRIER_UP)
                || (operation == MPI_BARRIER_DOWN) || (operation == MPI_BCAST)
                || (operation == MPI_SCATTER) || (operation == MPI_GATHER)) {

            clientTCP_Services->sendPacketToServer(pktSm);

        }

        // Change state
        else if (operation == SM_CHANGE_NET_STATE) {
            // change the state of the network
            changeDeviceState(sm->getChangingState().c_str());

            delete (pktSm);

        }
        // Wrong operation...
        else {
            showErrorMessage("Wrong request operation... %s",
                    sm->contentsToString(true).c_str());
        }
    }

    // Msg cames from TCP Network
    else if (pktSm->getArrivalGate() == fromNetTCPGate) {

        if (DEBUG_Network_Service)
            showDebugMessage("[processRequestMessage] from TCP Network. %s",
                    sm->contentsToString(DEBUG_MSG_Network_Service).c_str());

        // Seach the involved socket... server...
        socket = serverTCP_Services->getInvolvedSocket(pktSm);

        // Receiving data...
        if (socket != nullptr) {
            socket->processMessage(pktSm);
        }

        // No socket found!
        else
            showErrorMessage("[processRequestMessage] No socket found!. %s",
                    sm->contentsToString(true).c_str());
    }
}


void NetworkService::processResponseMessage(Packet * pktSm) {

    inet::TcpSocket *socket;
    const auto &sm = pktSm->peekAtFront<icancloud_Message>();

    // Msg cames from Service Redirector...
    if (pktSm->getArrivalGate() == fromNetManagerGate) {

        if (DEBUG_Network_Service)
            showDebugMessage(
                    "[processResponseMessage] from Service Redirector. %s",
                    sm->contentsToString(DEBUG_MSG_Network_Service).c_str());

        socket = serverTCP_Services->getInvolvedSocket(pktSm);

        // Sending data to corresponding client...
        if (socket != nullptr) {
            serverTCP_Services->sendPacketToClient(pktSm);
        }

        // Not socket found!
        else {
            showErrorMessage("[processResponseMessage] Socket not found... %s",
                    sm->contentsToString(true).c_str());
        }
    }

    // Msg cames from TCP Network
    else if (pktSm->getArrivalGate() == fromNetTCPGate) {

        if (DEBUG_Network_Service)
            showDebugMessage("[processResponseMessage] from TCP Network. %s",
                    sm->contentsToString(DEBUG_MSG_Network_Service).c_str());

        socket = clientTCP_Services->getInvolvedSocket(pktSm);

        // Sending data to corresponding application...
        if (socket != nullptr) {
            socket->processMessage(pktSm);
        }

        // Not socket found!
        else {
            showErrorMessage("[processResponseMessage] Socket not found... %s",
                    sm->contentsToString(true).c_str());
        }
    }
    changeState(NETWORK_OFF);
}


void NetworkService::receivedEstablishedConnection(cMessage *msg) {

    TcpSocket *socket;

    socket = clientTCP_Services->getInvolvedSocket(msg);

    // Establishing connection... (client)
    if (socket != nullptr)
        socket->processMessage(msg);

    // Establishing connection... (server)
    else
        serverTCP_Services->arrivesIncommingConnection(msg);
}


void NetworkService::changeDeviceState(const string & state, unsigned componentIndex) {

    if (state == MACHINE_STATE_IDLE) {
        nodeState = MACHINE_STATE_IDLE;
        changeState(NETWORK_ON);

    }
    else if (state == MACHINE_STATE_RUNNING) {
        nodeState = MACHINE_STATE_RUNNING;
        changeState(NETWORK_ON);

    }
    else if (state == MACHINE_STATE_OFF) {
        nodeState = MACHINE_STATE_OFF;
        changeState(NETWORK_OFF);
    }
}

void NetworkService::changeState (const string & energyState,unsigned componentIndex){

	if (nodeState == MACHINE_STATE_OFF) {
	    e_changeState (NETWORK_OFF);
		return;
	}

	e_changeState (energyState);

}


} // namespace icancloud
} // namespace inet

#include "inet/icancloud/Applications/Libraries_API/API_OS/API_OS.h"
#include "inet/icancloud/Base/include/Constants.h"

namespace inet {

namespace icancloud {


API_OS::~API_OS(){
	
	connections.clear();
}

void API_OS::initialize(int stage){
	
	// Init the super-class
	icancloud_Base::initialize(stage);

}


void API_OS::finish(){

	// Finish the super-class
	icancloud_Base::finish();
}

void API_OS::startExecution(){

    fromOSGate = gate ("fromOS");
    toOSGate = gate ("toOS");
}



cGate* API_OS::getOutGate (cMessage *msg){

	// If msg arrive from IOR
	if (msg->getArrivalGate()==fromOSGate){
		if (toOSGate->getNextGate()->isConnected()){
			return (toOSGate);
		}
	}

	// If gate not found!
	return nullptr;
}


// ---------------------------------- I/O ---------------------------------- //
						

void API_OS::icancloud_request_open(const char* fileName)
{

    // Creates the message
    auto sm_io = makeShared<icancloud_App_IO_Message>();
    sm_io->setOperation(SM_OPEN_FILE);

    // Set the corresponding parameters
    sm_io->setFileName(fileName);

    // Update message length
    sm_io->updateLength();
    //CloudManagerBase* cloudManagerPtr;
    if (DEBUG_AppSystem)
        showDebugMessage("[AppSystemRequests] Open file %s",
                sm_io->contentsToString(DEBUG_MSG_AppSystem).c_str());

    // Send the request to the Operating System
    auto pkt = new Packet("icancloud_App_IO_Message");
    pkt->insertAtFront(sm_io);
    sendRequestMessage(pkt, toOSGate);
}

void API_OS::icancloud_request_close(const char* fileName)
{

    // Creates the message
    auto sm_io = makeShared<icancloud_App_IO_Message>();

    sm_io->setOperation(SM_CLOSE_FILE);

    // Set the corresponding parameters
    sm_io->setFileName(fileName);

    // Update message length
    sm_io->updateLength();

    if (DEBUG_AppSystem)
        showDebugMessage("[AppSystemRequests] Close file %s",
                sm_io->contentsToString(DEBUG_MSG_AppSystem).c_str());

    // Send the request to the Operating System
    auto pkt = new Packet("icancloud_App_IO_Message");
    pkt->insertAtFront(sm_io);
    sendRequestMessage(pkt, toOSGate);
}
		
		
void API_OS::icancloud_request_read(const char* fileName, unsigned int offset, unsigned int size)
{

    // Creates the message
    auto sm_io = makeShared<icancloud_App_IO_Message>();
    sm_io->setOperation(SM_READ_FILE);

    // Set the corresponding parameters
    sm_io->setFileName(fileName);
    sm_io->setOffset(offset);
    sm_io->setSize(size);

    // Update message length
    sm_io->updateLength();

    if (DEBUG_AppSystem)
        showDebugMessage("[AppSystemRequests] Read data %s",
                sm_io->contentsToString(DEBUG_MSG_AppSystem).c_str());

    // Send the request to the Operating System
    auto pkt = new Packet("icancloud_App_IO_Message");
    pkt->insertAtFront(sm_io);
    sendRequestMessage(pkt, toOSGate);
}

void API_OS::icancloud_request_write(const char* fileName, unsigned int offset, unsigned int size)
{

    // Creates the message
    auto sm_io = makeShared<icancloud_App_IO_Message>();
    sm_io->setOperation(SM_WRITE_FILE);

    // Set the corresponding parameters
    sm_io->setFileName(fileName);
    sm_io->setOffset(offset);
    sm_io->setSize(size);

    // Update message length
    sm_io->updateLength();

    if (DEBUG_AppSystem)
        showDebugMessage("[AppSystemRequests] Write data %s",
                sm_io->contentsToString(DEBUG_MSG_AppSystem).c_str());

    // Send the request to the Operating System
    auto pkt = new Packet("icancloud_App_IO_Message");
    pkt->insertAtFront(sm_io);
    sendRequestMessage(pkt, toOSGate);
}

void API_OS::icancloud_request_create(const char* fileName)
{

    // Creates the message
    auto sm_io = makeShared<icancloud_App_IO_Message>();
    sm_io->setOperation(SM_CREATE_FILE);

    // Set the corresponding parameters
    sm_io->setFileName(fileName);

    // Update message length
    sm_io->updateLength();

    if (DEBUG_AppSystem)
        showDebugMessage("[AppSystemRequests] Create file %s",
                sm_io->contentsToString(DEBUG_MSG_AppSystem).c_str());

    // Send the request to the Operating System
    auto pkt = new Packet("icancloud_App_IO_Message");
    pkt->insertAtFront(sm_io);

    sendRequestMessage(pkt, toOSGate);
}

void API_OS::icancloud_request_delete(const char* fileName)
{
    // Creates the message
    auto sm_io = makeShared<icancloud_App_IO_Message>();
    sm_io->setOperation(SM_DELETE_FILE);

    // Set the corresponding parameters
    sm_io->setFileName(fileName);

    // Update message length
    sm_io->updateLength();

    if (DEBUG_AppSystem)
        showDebugMessage("[AppSystemRequests] Delete file %s",
                sm_io->contentsToString(DEBUG_MSG_AppSystem).c_str());

    // Send the request to the Operating System
    auto pkt = new Packet("icancloud_App_IO_Message");
    pkt->insertAtFront(sm_io);

    sendRequestMessage(pkt, toOSGate);
}

void API_OS::icancloud_request_changeState_IO(string newState, vector<int> devicesIndexToChange) {

    unsigned int i;
    vector<int>::iterator componentIndex;

    // Creates the message
    auto sm_io = makeShared<icancloud_App_IO_Message>();
    sm_io->setOperation(SM_CHANGE_DISK_STATE);

    // Set the corresponding parameters
    sm_io->setChangingState(newState);

    for (i = 0; i < devicesIndexToChange.size(); i++) {
        componentIndex = devicesIndexToChange.begin() + i;
        sm_io->add_component_index_To_change_state((*componentIndex));
    }
    // Update message length
    //sm_io->updateLength ();

    if (DEBUG_AppSystem)
        showDebugMessage("[AppSystemRequests] Changing disk state to %s\n",
                newState.c_str());

    // Send the request to the Operating System
    auto pkt = new Packet("icancloud_App_IO_Message");
    pkt->insertAtFront(sm_io);
    sendRequestMessage(pkt, toOSGate);
}

// ----------------- CPU calls ----------------- //

void API_OS::icancloud_request_cpu(unsigned int MIs)
{

    // Creates the message
    auto sm_cpu = makeShared<icancloud_App_CPU_Message>();
    sm_cpu->setOperation(SM_CPU_EXEC);

    // Set the corresponding parameters
    sm_cpu->setTotalMIs(MIs);
    sm_cpu->setRemainingMIs(MIs);
    sm_cpu->setCpuTime(0.000000);

    // Update message length
    sm_cpu->updateLength();

    if (DEBUG_AppSystem)
        showDebugMessage("[AppSystemRequests] Computing (CPU) %s",
                sm_cpu->contentsToString(DEBUG_MSG_AppSystem).c_str());

    // Send the request to the Operating System
    auto pkt = new Packet("icancloud_App_CPU_Message");
    pkt->insertAtFront(sm_cpu);
    sendRequestMessage(pkt, toOSGate);
}

void API_OS::icancloud_request_cpuTime(simtime_t cpuTime)
{


    // Creates the message
    auto sm_cpu = makeShared<icancloud_App_CPU_Message>();
    sm_cpu->setOperation(SM_CPU_EXEC);

    // Set the corresponding parameters
    sm_cpu->setTotalMIs(0);
    sm_cpu->setRemainingMIs(0);
    sm_cpu->setCpuTime(cpuTime);

    // Update message length
    sm_cpu->updateLength();

    if (DEBUG_AppSystem)
        showDebugMessage("[AppSystemRequests] Computing (CPU) %s",
                sm_cpu->contentsToString(DEBUG_MSG_AppSystem).c_str());

    // Send the request to the Operating System
    auto pkt = new Packet("icancloud_App_CPU_Message");
    pkt->insertAtFront(sm_cpu);
    sendRequestMessage(pkt, toOSGate);
}

void API_OS::icancloud_request_changeState_cpu(string newState, vector<int> devicesIndexToChange)
{
    unsigned int i;
    vector<int>::iterator componentIndex;

    // Creates the message
    auto sm_cpu = makeShared<icancloud_App_CPU_Message>();
    sm_cpu->setOperation(SM_CHANGE_CPU_STATE);

    // Set the corresponding parameters
    sm_cpu->setChangingState(newState);

    for (i = 0; i < devicesIndexToChange.size(); i++) {
        componentIndex = devicesIndexToChange.begin() + i;
        sm_cpu->add_component_index_To_change_state((*componentIndex));
    }
    // Update message length
    sm_cpu->updateLength();

    //CloudManagerBase* cloudManagerPtr;
    if (DEBUG_AppSystem)
        showDebugMessage("[AppSystemRequests] Changing CPU state to %s\n",
                newState.c_str());

    // Send the request to the Operating System
    auto pkt = new Packet("icancloud_App_CPU_Message");
    pkt->insertAtFront(sm_cpu);

    sendRequestMessage(pkt, toOSGate);
}

// ----------------- Memory calls ----------------- //

void API_OS::icancloud_request_allocMemory(unsigned int memorySize, unsigned int region)
{
    // Creates the message
    auto sm_mem = makeShared<icancloud_App_MEM_Message>();
    sm_mem->setOperation(SM_MEM_ALLOCATE);

    // Set the corresponding parameters
    sm_mem->setMemSize(memorySize);
    sm_mem->setRegion(region);

    // Update message length
    sm_mem->updateLength();

    if (DEBUG_AppSystem)
        showDebugMessage("[AppSystemRequests] Allocating memory (MEM) %s",
                sm_mem->contentsToString(DEBUG_MSG_AppSystem).c_str());

    // Send the request to the Operating System
    auto pkt = new Packet("icancloud_App_MEM_Message");
    pkt->insertAtFront(sm_mem);

    sendRequestMessage(pkt, toOSGate);
}

void API_OS::icancloud_request_freeMemory(unsigned int memorySize, unsigned int region)
{

    // Creates the message
    auto sm_mem = makeShared<icancloud_App_MEM_Message>();
    sm_mem->setOperation(SM_MEM_RELEASE);

    // Set the corresponding parameters
    sm_mem->setMemSize(memorySize);
    sm_mem->setRegion(region);

    // Update message length
    sm_mem->updateLength();

    if (DEBUG_AppSystem)
        showDebugMessage("[AppSystemRequests] Allocating memory (MEM) %s",
                sm_mem->contentsToString(DEBUG_MSG_AppSystem).c_str());

    // Send the request to the Operating System
    auto pkt = new Packet("icancloud_App_MEM_Message");
    pkt->insertAtFront(sm_mem);

    sendRequestMessage(pkt, toOSGate);
}

void API_OS::icancloud_request_changeState_memory(string newState) {

     // Creates the message
    auto sm_mem = makeShared<icancloud_App_MEM_Message>();
    sm_mem->setOperation(SM_CHANGE_MEMORY_STATE);

    // Set the corresponding parameters
    sm_mem->setChangingState(newState);

    // Update message length
    sm_mem->updateLength();
    //CloudManagerBase* cloudManagerPtr;
    if (DEBUG_AppSystem)
        showDebugMessage("[AppSystemRequests] Changing memory state to %s\n",
                newState.c_str());

    // Send the request to the Operating System
    auto pkt = new Packet("icancloud_App_MEM_Message");
    pkt->insertAtFront(sm_mem);

    sendRequestMessage(pkt, toOSGate);
}

// ----------------- Network calls ----------------- // 

void API_OS::icancloud_request_createConnection(string destAddress, int destPort, int id, int destUserName)
{

    struct icancloud_App_Connector newConnection;		// Connection struct
    // Configure the new connection
    newConnection.destAddress = destAddress;
    newConnection.destPort = destPort;
    newConnection.localAddress = "Address not set yet!";
    newConnection.localPort = 0;
    newConnection.connectionType = "INET";
    newConnection.id = id;
    newConnection.connectionId = EMPTY;

    // Creates the message
    auto sm_net = makeShared<icancloud_App_NET_Message>();
    sm_net->setOperation(SM_CREATE_CONNECTION);

    // Set parameters
    sm_net->setDestinationIP(destAddress.c_str());
    sm_net->setDestinationPort(destPort);
    sm_net->setConnectionType("INET");
    sm_net->setCommId(id);

    // Set the destination user
    sm_net->setVirtual_user(destUserName);

    // Add the connection to the connection vector
    connections.push_back(newConnection);

    if (DEBUG_AppSystem)
        showDebugMessage("[AppSystemRequests] Creating connection %s",
                sm_net->contentsToString(DEBUG_MSG_AppSystem).c_str());

    // Send the request message to Network Service module
    auto pkt = new Packet("icancloud_App_MEM_Message");
    pkt->insertAtFront(sm_net);
    sendRequestMessage(pkt, toOSGate);
}

void API_OS::icancloud_request_createConnection(string destAddress, int destPort, int id)
{

    struct icancloud_App_Connector newConnection;		// Connection struct

    // Configure the new connection
    newConnection.destAddress = destAddress;
    newConnection.destPort = destPort;
    newConnection.localAddress = "Address not set yet!";
    newConnection.localPort = 0;
    newConnection.connectionType = "INET";
    newConnection.id = id;
    newConnection.connectionId = EMPTY;

    // Creates the message
    auto sm_net = makeShared<icancloud_App_NET_Message>();
    sm_net->setOperation(SM_CREATE_CONNECTION);

    // Set parameters
    sm_net->setDestinationIP(destAddress.c_str());
    sm_net->setDestinationPort(destPort);
    sm_net->setConnectionType("INET");
    sm_net->setCommId(id);

    // Set the destination user
    sm_net->setVirtual_user(-1);

    // Add the connection to the connection vector
    connections.push_back(newConnection);

    if (DEBUG_AppSystem)
        showDebugMessage("[AppSystemRequests] Creating connection %s",
                sm_net->contentsToString(DEBUG_MSG_AppSystem).c_str());

    // Send the request message to Network Service module
    auto pkt = new Packet("icancloud_App_MEM_Message");
    pkt->insertAtFront(sm_net);
    sendRequestMessage(pkt, toOSGate);
}

void API_OS::icancloud_request_createListenConnection(int localPort)
{

    struct icancloud_App_Connector newConnection;		// Connection struct

    // Configure the new connection
    newConnection.destAddress = "Address not set yet!";
    newConnection.destPort = 0;
    newConnection.localAddress = "Address not set yet!";
    newConnection.localPort = localPort;
    newConnection.connectionType = "INET";
    newConnection.connectionId = EMPTY;

    // Creates the message
    auto sm_net = makeShared<icancloud_App_NET_Message>();
    sm_net->setOperation(SM_LISTEN_CONNECTION);

    // Set parameters
    sm_net->setLocalPort(localPort);
    sm_net->setConnectionType("INET");
    sm_net->setNextModuleIndex(getParentModule()->getIndex());

    // Add the connection to the connection vector
    connections.push_back(newConnection);

    if (DEBUG_AppSystem)
        showDebugMessage("[AppSystemRequests] Listen connection %s",
                sm_net->contentsToString(DEBUG_MSG_AppSystem).c_str());

    // Send the request message to Network Service module
    auto pkt = new Packet("icancloud_App_MEM_Message");
    pkt->insertAtFront(sm_net);
    sendRequestMessage(pkt, toOSGate);
}

void API_OS::icancloud_request_sendDataToNetwork(Packet *pkt, int id) {

    int index;
    // Search connection...
    index = searchConnectionById(id);
    // Error?
    if (index == NOT_FOUND) {
        showErrorMessage("Connection with ID=%d Not found", id);
    }
    else {
        pkt->trimFront();
        auto  sm = pkt->removeAtFront<icancloud_Message>();
        auto sm_net = CHK(dynamicPtrCast<icancloud_App_NET_Message>(sm));

        sm->setConnectionId(connections[index].connectionId);

        sm_net->setDestinationIP(connections[index].destAddress.c_str());
        sm_net->setDestinationPort(connections[index].destPort);
        sm_net->setConnectionType(connections[index].connectionType.c_str());
        sm_net->setLocalPort(connections[index].localPort);
        sm_net->setLocalIP(connections[index].localAddress.c_str());
        if (DEBUG_AppSystem)
            showDebugMessage("[AppSystemRequests] Send data to server %s",
                    sm->contentsToString(DEBUG_MSG_AppSystem).c_str());
        // Send to OS
        pkt->insertAtFront(sm);
        sendRequestMessage(pkt, toOSGate);
    }
}

void API_OS::icancloud_request_closeConnection(int localPort, int id) {

    struct icancloud_App_Connector newConnection;		// Connection struct

    // Configure the new connection
    newConnection.destAddress = "Address not set yet!";
    newConnection.destPort = 0;
    newConnection.localAddress = "Address not set yet!";
    newConnection.localPort = localPort;
    newConnection.connectionType = "INET";
    newConnection.connectionId = id;

    // Creates the message
    auto sm_net = makeShared<icancloud_App_NET_Message>();
    sm_net->setOperation(SM_LISTEN_CONNECTION);

    // Set parameters
    sm_net->setLocalPort(localPort);
    sm_net->setConnectionId(id);
    sm_net->setConnectionType("INET");
    sm_net->setNextModuleIndex(getParentModule()->getIndex());

    // Add the connection to the connection vector
    connections.push_back(newConnection);

    if (DEBUG_AppSystem)
        showDebugMessage("[AppSystemRequests] Listen connection %s", sm_net->contentsToString(DEBUG_MSG_AppSystem).c_str());


    // Send the request message to Network Service module
    auto pkt = new Packet("icancloud_App_NET_Message");
    pkt->insertAtFront(sm_net);
    sendRequestMessage(pkt, toOSGate);
}

void API_OS::setEstablishedConnection(Packet *pkt) {

    int index;
    int connectionId;

    pkt->trim();
    const auto &sm = pkt->peekAtFront<icancloud_Message>();

    // Cast
    const auto &sm_net = CHK(dynamicPtrCast<const icancloud_App_NET_Message>(sm));

    // Get result!
    connectionId = sm_net->getConnectionId();

    index = searchConnectionById(sm_net->getCommId());

    // Error?
    if (index == NOT_FOUND) {
        printf("%s\n", connectionsToString(true).c_str());
        showErrorMessage("Server with ID=%d Not found", sm_net->getCommId());
    }
    else {
        connections[index].connectionId = connectionId;
        connections[index].localAddress = sm_net->getLocalIP();
        connections[index].localPort = sm_net->getLocalPort();
    }

    if (DEBUG_AppSystem)
        showDebugMessage(
                "[AppSystemRequests] Set established connection: localAddress:%s - destinationAddress:%s - localPort:%d - destinationPort:%d - type:%s - Id:%d - connId:%d",
                connections[index].localAddress.c_str(),
                connections[index].destAddress.c_str(),
                connections[index].localPort, connections[index].destPort,
                connections[index].connectionType.c_str(),
                connections[index].id, connections[index].connectionId);

}

void API_OS::icancloud_request_changeState_network(string newState, vector<int> devicesIndexToChange) {

    unsigned int i;
    vector<int>::iterator componentIndex;

    // Creates the message
    auto sm_net = makeShared<icancloud_App_NET_Message>();
    sm_net->setOperation(SM_CHANGE_NET_STATE);

    // Set the corresponding parameters
    sm_net->setChangingState(newState);

    for (i = 0; i < devicesIndexToChange.size(); i++) {
        componentIndex = devicesIndexToChange.begin() + i;
        sm_net->add_component_index_To_change_state((*componentIndex));
    }

    if (DEBUG_AppSystem)
        showDebugMessage("[AppSystemRequests] Changing network state to %s\n",
                newState.c_str());

    // Send the request message to Network Service module
    auto pkt = new Packet("icancloud_App_NET_Message");
    pkt->insertAtFront(sm_net);
    sendRequestMessage(pkt, toOSGate);
}

// ----------------- Util calls ----------------- //

int API_OS::searchConnectionById(int id) {

    unsigned int index;
    bool found;

    // Init...
    found = false;
    index = 0;

    // Searching...
    while ((!found) && (index < connections.size())) {
        if (id == connections[index].id)
            found = true;
        else
            index++;
    }

    // Return the result...
    if (found)
        return index;
    else
        return NOT_FOUND;
}

string API_OS::connectionsToString(bool printConnections) {

    std::ostringstream osStream;
    unsigned int i;

    osStream.str("");

    // Connection list enable?
    if (printConnections) {

        osStream << "Communication list..." << endl;

        for (i = 0; i < connections.size(); i++) {
            osStream << "  Connection[" << i << "] localAddress:"
                    << connections[i].localAddress << "  destinationAddress:"
                    << connections[i].destAddress << endl << "  localPort:"
                    << connections[i].localPort << "  destinationPort:"
                    << connections[i].destPort << endl << "  type:"
                    << connections[i].connectionType << "  serverID:"
                    << connections[i].id << "  connecDescriptor:"
                    << connections[i].connectionId << endl << endl;
        }
    }

    return osStream.str();
}

} // namespace icancloud
} // namespace inet

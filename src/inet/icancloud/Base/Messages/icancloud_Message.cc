#include "inet/icancloud/Base/Messages/icancloud_Message_m.h"
#include "inet/icancloud/Base/Messages/icancloud_Message.h"

namespace inet {

namespace icancloud {

using namespace omnetpp;

Register_Class(icancloud_Message);


TraceComponent& TraceComponent::operator=(const TraceComponent& other){
	moduleID = other.moduleID;
	gateID = other.gateID;
	reqSequence = other.reqSequence;

    return *this;
}


TraceComponent::TraceComponent(int newModule, int newGate) :
  moduleID(newModule),
  gateID(newGate){
}


TraceComponent::TraceComponent(int newModule, int newGate, reqNum_t newReq) :
  moduleID(newModule),
  gateID(newGate){
  reqSequence.push_front (newReq);
}

icancloud_Message::~icancloud_Message(){

	unsigned int module;
	unsigned int node;

		// For each node...
		for (node=0; node<trace.size(); node++){

			// For each module
			for (module=0; module<trace[node].second.size(); module++){

				// Removes all lists belonging to current module
				trace[node].second[module].reqSequence.clear();
			}

			// Removes current node vector
			trace[node].second.clear();
		}

		// Removes completlety the trace!
		trace.clear();
}


icancloud_Message::icancloud_Message(){

    setChunkLength(B(MSG_INITIAL_LENGTH));
	//setName ("icancloud_Message");
	parentRequest = nullptr;
	state = "";
	change_states_index.clear();

}


icancloud_Message::icancloud_Message(const icancloud_Message& other) {

	operator=(other);
	setChunkLength(B(MSG_INITIAL_LENGTH));
	//setName ("icancloud_Message");
	parentRequest = nullptr;
	state = "";
	change_states_index.clear();

}


icancloud_Message& icancloud_Message::operator=(const icancloud_Message& other){

	icancloud_Message_Base::operator=(other);

	return *this;
}


icancloud_Message *icancloud_Message::dup() const{

	//vector <pair <string, vector<TraceComponent> > >::iterator trace_it;
	icancloud_Message *newMessage;
	//TCPCommand *controlNew;
	//TCPCommand *controlOld;

	unsigned int i;

		// Create a new message!
		newMessage = new icancloud_Message();

		// Base parameters...
		newMessage->setOperation (getOperation());
		newMessage->setIsResponse (getIsResponse());
		newMessage->setRemoteOperation (getRemoteOperation());
		newMessage->setConnectionId (getConnectionId());
		newMessage->setSourceId (getSourceId());
		newMessage->setNextModuleIndex (getNextModuleIndex());
		newMessage->setResult (getResult());
		newMessage->setCommId(getCommId());
        newMessage->setUid(getUid());
        newMessage->setPid(getPid());
		newMessage->setChunkLength (getChunkLength());
		newMessage->setParentRequest (getParentRequest());

		newMessage->setChangingState(state);
		for (i=0; i< change_states_index.size(); i++){
			newMessage->add_component_index_To_change_state(change_states_index[i]);
		}

		// Copy the control info, if exists!
		/*if (getControlInfo() != nullptr){
			controlOld = check_and_cast<TCPCommand *>(getControlInfo());
			controlNew = new TCPCommand();
			controlNew = controlOld->dup();
			newMessage->setControlInfo (controlNew);
		}*/

		// Reserve memory to trace!
		newMessage->setTraceArraySize (getTraceArraySize());

		// Copy trace!
		for (i=0; i<trace.size(); i++){
			newMessage->addNodeTrace (trace[i].first, trace[i].second);
		}

	return (newMessage);
}


void icancloud_Message::setTraceArraySize(size_t size){
	trace.reserve (size);
}


size_t icancloud_Message::getTraceArraySize() const{
	return trace.size();
}


TraceComponent& icancloud_Message::getTrace(size_t k) const{
//    TraceComponent* trace;
//    trace = nullptr;
//    return *trace;

    return const_cast<TraceComponent&>(trace[k].second.front());
}


void icancloud_Message::setTrace(size_t k, const TraceComponent& trace_var){
}


void icancloud_Message::insertTrace(const TraceComponent& trace) {
}

void icancloud_Message::insertTrace(size_t k, const TraceComponent& trace) {

}

void icancloud_Message::eraseTrace(size_t k) {
    trace.erase(trace.begin()+k);
}

void icancloud_Message::setChangingState (string newState){
	state = newState;
}


string icancloud_Message::getChangingState () const {
	return state;
}


void icancloud_Message::add_component_index_To_change_state (int componentIndex){
	change_states_index.insert (change_states_index.end(),componentIndex);
}


int icancloud_Message::get_component_to_change_size () const {
	return change_states_index.size();
}


int icancloud_Message::get_component_to_change (int componentsPosition) const {
	return (*change_states_index.begin() + componentsPosition);
}


void icancloud_Message::addModuleToTrace (int module, int gate, reqNum_t currentRequest){

	struct TraceComponent currentModule(module,gate);

		// Check if trace is empty!
		if (trace.empty())
			throw cRuntimeError ("Trace is empty!");

		currentModule.reqSequence.push_back(currentRequest);

		// Update current module trace
		trace.back().second.push_back(currentModule);
}


void icancloud_Message::removeLastModuleFromTrace (){

		// Check if trace is empty!
		if (trace.empty())
			throw cRuntimeError ("Trace is empty!");

		// Removes last module from trace!
		trace.back().second.pop_back();
}


void icancloud_Message::addNodeToTrace (string node){

	vector<TraceComponent> currentModuleTrace;

		// Add a new node trace
		trace.push_back (make_pair(node, currentModuleTrace));
}


void icancloud_Message::removeLastNodeFromTrace (){

		// Check if trace is empty!
		if (trace.empty())
			throw cRuntimeError ("Trace is empty!");

		// Removes last node trace
		trace.pop_back();
}


void icancloud_Message::addRequestToTrace (int request){

		// Check if trace is empty!
		if (trace.empty())
			throw cRuntimeError ("Trace is empty!");

		// Add a request number to last module trace
		trace.back().second.back().reqSequence.push_back (request);
}


void icancloud_Message::removeLastRequestFromTrace (){

		// Check if trace is empty!
		if (trace.empty())
			throw cRuntimeError ("Trace is empty!");

		// Add a request number to last module trace
		trace.back().second.back().reqSequence.pop_back ();
}


int icancloud_Message::getLastGateId () const {

		// Check if trace is empty!
		if (trace.empty())
			throw cRuntimeError ("Trace is empty!");


	// Update current module trace
	return trace.back().second.back().gateID;
}


int icancloud_Message::getLastModuleId () const {

		// Check if trace is empty!
		if (trace.empty())
			throw cRuntimeError ("Trace is empty!");


	// Update current module trace
	return trace.back().second.back().moduleID;
}


int icancloud_Message::getCurrentRequest () const {

		// Check if trace is empty!
		if (trace.empty())
			throw cRuntimeError ("Trace is empty!");


	// Update current module trace
	return trace.back().second.back().reqSequence.back();
}


inet::Packet* icancloud_Message::getParentRequest () const{
	return parentRequest;
}


void icancloud_Message::setParentRequest (inet::Packet * parent){
	parentRequest = parent;
}


string icancloud_Message::traceToString () const {

	std::ostringstream traceLine;
	unsigned int currentNodeTrace;
	unsigned int currentModuleTrace;
	//std::list <reqNum_t>::iterator listIterator;


		// Clear the string buffer
		traceLine.str("");

		// Walk through node traces
		for (currentNodeTrace=0; currentNodeTrace<trace.size(); currentNodeTrace++){

			// If not first element, add a separator!
			if (currentNodeTrace!=0)
				traceLine << "|";

			// Get current hostName
			traceLine << trace[currentNodeTrace].first << "@";

			// Walk through module traces
			for (currentModuleTrace=0; currentModuleTrace<trace[currentNodeTrace].second.size(); currentModuleTrace++){

				// If not first element, add a separator!
				if (currentModuleTrace!=0)
					traceLine << "/";

				// Add module ID
				traceLine << trace[currentNodeTrace].second[currentModuleTrace].moduleID << ",";

				// Add gate ID
				traceLine << trace[currentNodeTrace].second[currentModuleTrace].gateID << ":";

					// Walk through request list
					for(auto listIterator = trace[currentNodeTrace].second[currentModuleTrace].reqSequence.begin();
						listIterator != trace[currentNodeTrace].second[currentModuleTrace].reqSequence.end();
						++listIterator){

						// If not the first element, add a separator!
						if (listIterator!=trace[currentNodeTrace].second[currentModuleTrace].reqSequence.begin())
							traceLine << ".";

						traceLine << *listIterator;
					}
			}
		}

	return traceLine.str();
}


string icancloud_Message::contentsToString (bool printContents) const {

	std::ostringstream osStream;	

	if (printContents){

		osStream  << "icancloud Message contents" << endl;

		osStream  << " - name:" <<  getName() << endl;
		osStream  << " - operation:" <<  operationToString(getOperation()) << endl;
		osStream  << " - length:" <<  B(this->getChunkLength()).get() << " bytes" << endl;
		osStream  << " - isResponse:" <<  getIsResponse_string() << endl;
		osStream  << " - remoteOperation:" <<  getRemoteOperation_string() << endl;
		osStream  << " - connectionId:" <<  getConnectionId() << endl;
		osStream  << " - commId:" <<  getCommId() << endl;
		osStream  << " - sourceProcess ID:" <<  getSourceId() << endl;
		osStream  << " - result:" <<  getResult() << endl;
		osStream  << " - UserID:" <<  getUid() << endl;
		osStream  << " - ProcessID:" <<  getPid() << endl;
		
		if (PRINT_SM_TRACE)		
			osStream  << " - trace:" <<  traceToString() << endl;
	}

	return osStream.str();
}


string icancloud_Message::operationToString (unsigned int operation) const {

	string result;
	int op = operation;

		if (op == NULL_OPERATION)
			result = "Operation not set!";

		else if (op == SM_OPEN_FILE)
			result = "SM_OPEN_FILE";

		else if (op == SM_CLOSE_FILE)
			result = "SM_CLOSE_FILE";

		else if (op == SM_READ_FILE)
			result = "SM_READ_FILE";

		else if (op == SM_WRITE_FILE)
			result = "SM_WRITE_FILE";

		else if (op == SM_CREATE_FILE)
			result = "SM_CREATE_FILE";

		else if (op == SM_DELETE_FILE)
			result = "SM_DELETE_FILE";		
		
		else if (op == SM_CPU_EXEC)
			result = "SM_CPU_EXEC";
		
		else if (op == SM_CREATE_CONNECTION)
			result = "SM_CREATE_CONNECTION";
		
		else if (op == SM_LISTEN_CONNECTION)
			result = "SM_LISTEN_CONNECTION";
		
		else if (op == SM_SEND_DATA_NET)
			result = "SM_SEND_DATA_NET";			
			
		else if (op == SM_MEM_ALLOCATE)
			result = "SM_MEM_ALLOCATE";
			
		else if (op == SM_MEM_RELEASE)
			result = "SM_MEM_RELEASE";

        // MPI operations

            else if (operation == MPI_SEND)
                    result = "MPI_SEND";

            else if (operation == MPI_RECV)
                    result = "MPI_RECV";

            else if (operation == MPI_BARRIER)
                    result = "MPI_BARRIER";

            else if (operation == MPI_BARRIER_UP)
                    result = "MPI_BARRIER_UP";

            else if (operation == MPI_BARRIER_DOWN)
                    result = "MPI_BARRIER_DOWN";

            else if (operation == MPI_BCAST)
                    result = "MPI_BCAST";

            else if (operation == MPI_SCATTER)
                    result = "MPI_SCATTER";

            else if (operation == MPI_GATHER)
                    result = "MPI_GATHER";

            else if (operation == MPI_FILE_OPEN)
                    result = "MPI_FILE_OPEN";

            else if (operation == MPI_FILE_CLOSE)
                    result = "MPI_FILE_CLOSE";

            else if (operation == MPI_FILE_CREATE)
                    result = "MPI_FILE_CREATE";

            else if (operation == MPI_FILE_DELETE)
                    result = "MPI_FILE_DELETE";

            else if (operation == MPI_FILE_READ)
                    result = "MPI_FILE_READ";

            else if (operation == MPI_FILE_WRITE)
                    result = "MPI_FILE_WRITE";

            else
                    result = "Unknown operation!";


	return result;
}


string icancloud_Message::operationToString() const {

	return operationToString(getOperation());
}


string icancloud_Message::getIsResponse_string() const {
	
	string result;
	
		if (getIsResponse())
			result = "true";
		else
			result = "false";
	
	return result;
}


string icancloud_Message::getRemoteOperation_string() const {
	
	string result;
	
		if (getRemoteOperation())
			result = "true";
		else
			result = "false";
	
	return result;
}


bool icancloud_Message::isTraceEmpty() const {
	return (trace.empty());
}


bool icancloud_Message::fitWithParent() const {

    bool fit = false;
    size_t dot, twoDots;

    //icancloud_Message *parent;
    string childTrace;
    string parentTrace;

    // Init...		fit = false;
    auto pktParent = getParentRequest();
    const auto &parent =  pktParent->peekAtFront<icancloud_Message>();
    //parent = check_and_cast<icancloud_Message*>(getParentRequest());

    childTrace = traceToString();
    parentTrace = parent->traceToString();

    // Search for last dot!
    dot = childTrace.find_last_of(".");
    twoDots = childTrace.find_last_of(":");

    // if found... remove last sequence number!
    if ((dot != string::npos) && (twoDots != string::npos)) {

        // last dot behind ':'
        if (dot > twoDots) {

            childTrace = childTrace.substr(0, dot);
            if (childTrace.compare(parentTrace) == 0)
                fit = true;
        }
    }

    return fit;
}


bool icancloud_Message::fitWithParent(inet::Packet *parentMsg) {

    bool fit;
    size_t dot, twoDots;

    //icancloud_Message *parent;
    string childTrace;
    string parentTrace;
    // Init...
    fit = false;
    const auto &parent = parentMsg->peekAtFront<icancloud_Message>();
    //parent = check_and_cast<icancloud_Message*>(parentMsg);
    childTrace = traceToString();
    parentTrace = parent->traceToString();

    // Search for last dot!
    dot = childTrace.find_last_of(".");
    twoDots = childTrace.find_last_of(":");

    // if found... remove last sequence number!
    if ((dot != string::npos) && (twoDots != string::npos)) {

        // last dot behind ':'
        if (dot > twoDots) {

            childTrace = childTrace.substr(0, dot);

            if (childTrace.compare(parentTrace) == 0)
                fit = true;
        }
    }

    return fit;
}

void icancloud_Message::addNodeTrace(string host,
        vector<TraceComponent> nodeTrace) {

    vector<TraceComponent>::iterator component_it;
    std::list<reqNum_t>::iterator sequence_it;
    struct TraceComponent currentComponent(0, 0);
    vector<TraceComponent> currentNodeTrace;

    // Copy current node trace
    for (component_it = nodeTrace.begin(); component_it != nodeTrace.end();
            component_it++) {

        currentComponent.moduleID = (*component_it).moduleID;
        currentComponent.gateID = (*component_it).gateID;
        currentComponent.reqSequence.clear();

        // Copy the sequence list
        for (sequence_it = (*component_it).reqSequence.begin();
                sequence_it != (*component_it).reqSequence.end();
                sequence_it++) {

            currentComponent.reqSequence.push_back(*sequence_it);
        }

        // Add current module to node trace
        currentNodeTrace.push_back(currentComponent);
    }

    // Add a new node trace
    trace.push_back(make_pair(host, currentNodeTrace));
}

string icancloud_Message::getHostName(int k) const {
    return trace[k].first;
}

vector<TraceComponent> icancloud_Message::getNodeTrace(int k) const {
    return trace[k].second;
}

void icancloud_Message::parsimPack(cCommBuffer *b) const {

    int numNodes;
    int numModules;
    int numRequests;
    int currentNode, currentModule;
    unsigned long parentPointer;
    //std::list <reqNum_t>::iterator listIterator;

    // Pack icancloud_Message_Base
    icancloud_Message_Base::parsimPack(b);

    // Pack parent message
    memcpy(&parentPointer, &parentRequest, 4);
    b->pack(parentPointer);

    // Get and pack the number of nodes...
    numNodes = trace.size();
    b->pack(numNodes);

    if (DEBUG_PACKING)
        printf("packing numNodes = %d\n", numNodes);

    // For each node...
    for (currentNode = 0; currentNode < numNodes; currentNode++) {

        // Pack the hostname!
        b->pack(trace[currentNode].first.c_str());

        if (DEBUG_PACKING)
            printf("packing hostName [%d] = %s\n", currentNode,
                    trace[currentNode].first.c_str());

        // Get and pack the number of modules in current node
        numModules = trace[currentNode].second.size();
        b->pack(numModules);

        if (DEBUG_PACKING)
            printf("packing numModules = %d\n", numModules);

        // For each module...
        for (currentModule = 0; currentModule < numModules; currentModule++) {

            // Pack the moduleID and gateID
            b->pack(trace[currentNode].second[currentModule].moduleID);
            b->pack(trace[currentNode].second[currentModule].gateID);

            if (DEBUG_PACKING)
                printf("unpacking moduleID = %d   gateID = %d\n",
                        trace[currentNode].second[currentModule].moduleID,
                        trace[currentNode].second[currentModule].gateID);

            // Get and pack the number of requests
            numRequests =
                    trace[currentNode].second[currentModule].reqSequence.size();
            b->pack(numRequests);

            if (DEBUG_PACKING)
                printf("packing numRequests = %d\n", numRequests);

            // For each request...
            for (auto listIterator = trace[currentNode].second[currentModule].reqSequence.begin();
                    listIterator != trace[currentNode].second[currentModule].reqSequence.end();
                    ++listIterator) {
                b->pack(*listIterator);
                if (DEBUG_PACKING)
                    printf("packing request = %lu\n", *listIterator);
            }
        }
    }
}

void icancloud_Message::parsimUnpack(cCommBuffer *b) {

    int numNodes;
    int numModules;
    int numRequests;
    char* hostname;
    int moduleID;
    int gateID;
    reqNum_t req;
    string nodeName;
    unsigned long parentPointer;

    int &numNodesRef = numNodes;
    int &numModulesRef = numModules;
    int &numRequestsRef = numRequests;
    char *&hostnameRef = hostname;
    int &moduleIDRef = moduleID;
    int &gateIDRef = gateID;
    reqNum_t &reqRef = req;
    unsigned long &parentPointerRef = parentPointer;

    int currentNode, currentModule, currentRequest;
    std::list<reqNum_t>::iterator listIterator;

    // Unpack icancloud_Message_Base
    icancloud_Message_Base::parsimUnpack(b);

    // Unpack parent message
    b->unpack(parentPointerRef);
    memcpy(&parentRequest, &parentPointer, 4);

    // Init...
    hostname = (char*) malloc(NAME_SIZE);

    // unpack the number of nodes...
    b->unpack(numNodesRef);

    if (DEBUG_PACKING)
        printf("unpacking numNodes = %d\n", numNodes);

    // For each node...
    for (currentNode = 0; currentNode < numNodes; currentNode++) {

        // Unpack the hostname!
        memset(hostname, 0, NAME_SIZE);
        b->unpack(hostnameRef);

        // Add a new node!
        nodeName = hostname;
        addNodeToTrace(nodeName);

        if (DEBUG_PACKING)
            printf("unpacking hostName [%d] = %s\n", currentNode,
                    nodeName.c_str());

        // Gunpack the number of modules in current node
        b->unpack(numModulesRef);

        if (DEBUG_PACKING)
            printf("unpacking numModules = %d\n", numModules);

        // For each module...
        for (currentModule = 0; currentModule < numModules; currentModule++) {

            // unpack the moduleID and gateID
            b->unpack(moduleIDRef);
            b->unpack(gateIDRef);

            if (DEBUG_PACKING)
                printf("unpacking moduleID = %d   gateID = %d\n", moduleID,
                        gateID);

            // Get and pack the number of requests
            b->unpack(numRequestsRef);

            if (DEBUG_PACKING)
                printf("unpacking numRequests = %d\n", numRequests);

            // For each request...
            for (currentRequest = 0; currentRequest < numRequests;
                    currentRequest++) {

                // unpack current request
                b->unpack(reqRef);

                if (DEBUG_PACKING)
                    printf("unpacking request = %lu\n", req);

                // add a new module
                if (currentRequest == 0) {
                    addModuleToTrace(moduleID, gateID, req);
                }

                // add a new request
                else {
                    addRequestToTrace(req);
                }
            }
        }
    }
}





} // namespace icancloud
} // namespace inet

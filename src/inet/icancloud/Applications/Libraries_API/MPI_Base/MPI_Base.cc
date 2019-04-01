#include "inet/icancloud/Applications/Libraries_API/MPI_Base/MPI_Base.h"

namespace inet {

namespace icancloud {



MPI_Base::~MPI_Base(){
//	cancelAndDelete (computingDelayMessage);
}


void MPI_Base::initialize(int stage){

    UserJob::initialize(stage);
}

void MPI_Base::startExecution (){

	std::ostringstream osStream;

		// Get module parameters...
		allToAllConnections = par ("allToAllConnections");

		myRank = par ("myRank");

		connectionDelay_s = par ("connectionDelay_s");
		connectionDelay_s += SimTime();

		startDelay_s = par ("startDelay_s");
		startDelay_s  += SimTime();
		
		workersSet = par("workersSet");

		runStartTime = time(nullptr);

		if ((connectionDelay_s < SimTime()) || (startDelay_s < SimTime()))
			showErrorMessage ("Error! parameter startDelay_s and connectionDelay_s must be > actual SimTime()");

		// Check delays!
		if (startDelay_s <= connectionDelay_s){
			showErrorMessage ("Error! parameter startDelay_s must be > connectionDelay_s");
		}

		// Check
		if (workersSet < 0){
			showErrorMessage ("Param workersSet must be > 0!");
		}
		
		// Create the computing delay message
		computingDelayMessage = new cMessage ("computing-message");
		
		// Create MPI calls objects
		currentMPICall = new MPI_Call();
		currentMPICall->clear();		
		lastSynchroMPICall = new MPI_Call();
		lastSynchroMPICall->clear();

		// Init barriers...
		barriers = new int [numProcesses];
		initBarriers ();		

    UserJob::startExecution();

    Enter_Method_Silent();
    // Create SM_WAIT_TO_EXECUTE and wait startDelay to sent it!
    cMessage *waitToExecuteMsg = new cMessage (SM_SUPER.c_str());
    scheduleAt (simTime(), waitToExecuteMsg);

}


void MPI_Base::processSelfMessage (cMessage *msg){

    if (!strcmp (msg->getName(), SM_SUPER.c_str())){

    // Delete msg!
        cancelAndDelete (msg);

    // Parse machine file
        parseMachinesFile ();

        // Create WAIT_TO_CONNECT_MESSAGE and wait connectionDelay to sent it!
        cMessage *waitToConnectMsg = new cMessage (SM_WAIT_TO_CONNECT.c_str());
        scheduleAt (simTime() + connectionDelay_s, waitToConnectMsg);

        // Create SM_WAIT_TO_EXECUTE and wait startDelay to sent it!
        cMessage *waitToExecuteMsg = new cMessage (SM_WAIT_TO_EXECUTE.c_str());
        scheduleAt (simTime() + startDelay_s, waitToExecuteMsg);
    }
    else
            showErrorMessage ("Unknown self message [%s]", msg->getName());
}

void MPI_Base::finish(){
	
    UserJob::finish();
}


void MPI_Base::processRequestMessage(Packet *pkt) {

    // Try to cast to icancloud_App_NET_Message
    pkt->trimFront();
    auto sm = pkt->removeAtFront<icancloud_Message>();
    auto sm_mpi = dynamicPtrCast<icancloud_MPI_Message>(sm);

    // NET call response...
    if (sm_mpi != nullptr) {
        pkt->insertAtFront(sm_mpi);
        processMPICallRequest(pkt);
    }

    else
        showErrorMessage("Unknown message type as request :%s",
                sm->contentsToString(true).c_str());
}


void MPI_Base::processMPICallRequest(Packet *pkt) {

    if (DEBUG_MPIApp) {
        pkt->trimFront();
        auto sm_mpi = pkt->removeAtFront<icancloud_MPI_Message>();
        showDebugMessage("[MPI_Base] Rank %d. Incoming call %s", myRank,
                sm_mpi->contentsToString(DEBUG_MSG_MPIApp).c_str());
        pkt->insertAtFront(sm_mpi);
    }

    const auto &  sm_mpi = pkt->peekAtFront<icancloud_MPI_Message>();

    // MPI_Incoming call... enqueue and process...
    // MPI_SEND, MPI_SCATTER or MPI_BCAST
    if ((sm_mpi->getOperation() == MPI_SEND)
            || (sm_mpi->getOperation() == MPI_SCATTER)
            || (sm_mpi->getOperation() == MPI_BCAST)) {

        // Inserts a new MPI call into queue
        insertMPICall(pkt);

        // Free mem.
        delete (pkt);
        // Synchrionize with current request!
        synchronize(false);
    }

    // MPI_BARRIER_DOWN
    else if (sm_mpi->getOperation() == MPI_BARRIER_DOWN) {
        mpi_barrier_down(pkt);
    }

    // MPI_BARRIER_UP
    else if (sm_mpi->getOperation() == MPI_BARRIER_UP) {
        mpi_barrier_up();
        delete (pkt);
    }

    // MPI_GATHER
    else if (sm_mpi->getOperation() == MPI_GATHER) {
        mpi_gather_arrives(pkt);
        delete (pkt);
    }

    else {
        auto sm_mpi = pkt->removeAtFront<icancloud_MPI_Message>();
        showErrorMessage("Unknown MPI operation :%s",
                sm_mpi->contentsToString(true).c_str());
    }
}


void MPI_Base::parseMachinesFile (){

	unsigned int i;
	unsigned int master;
    mpiCfg = userPtr->getMPIEnv();

    if (mpiCfg != nullptr){

        if (DEBUG_MPIApp)
                   showDebugMessage ("Show MPI configuration [%d]: %s", myRank, mpiCfg->toString().c_str());

        // No servers file!
        if (mpiCfg->getNumProcesses() == 0)
            showErrorMessage ("There is no processes in MPI Trace player machine !");

        // Wrong number
        else if (workersSet > mpiCfg->getNumProcesses()){
            showErrorMessage ("Param workersSet must be less than total number of processes!");
        }

        else{

            // Number of processes...
            numProcesses = mpiCfg->getNumProcesses();

            // For each process..
            for (i=0; i<mpiCfg->getNumProcesses(); i++){

                // Own process... listen connections
                if (myRank == mpiCfg->getRank(i)){

                    if (DEBUG_MPIApp)
                        showDebugMessage ("Process %d listening connection on %s:%i\n", myRank, mpiCfg->getHostName(i).c_str(), mpiCfg->getPort(i));

                    icancloud_request_createListenConnection (mpiCfg->getPort(i));
                }

                // Not own process!
                else{

                    // There is no set of workers. Only one master process!
                    if (workersSet == 0){

                        // Establish connection with all processes
                        if (allToAllConnections){
                            addNewConnection (mpiCfg->getHostName(i), mpiCfg->getPort(i), mpiCfg->getRank(i));
                        }

                        // Add the corresponding connection
                        //else if ((mpiCfg->getRank(i) == MPI_MASTER_RANK) || (myRank == MPI_MASTER_RANK))
                            //addNewConnection (mpiCfg->getHostName(i), mpiCfg->getPort(i), mpiCfg->getRank(i));
                    }

                    // There are workers set!
                    else{

                        // Wrong number
                        if ((mpiCfg->getNumProcesses()%workersSet) != 0){
                            showErrorMessage ("Param workersSet must be multiple of total number of processes!");
                        }

                        // Establish a new connection
                        master = getMyMaster(myRank);
                        if ((master == mpiCfg->getRank(i)) || (isThisWorkerInMyRange(myRank, mpiCfg->getRank(i))))
                            addNewConnection (mpiCfg->getHostName(i), mpiCfg->getPort(i), mpiCfg->getRank(i));
                    }
                }
            }
        }
    }

    else
        showErrorMessage ("nullptr object CfgTracePlayerMPI");
}


bool MPI_Base::isMaster (int processRank){

	if (workersSet == 0){

		if (processRank == MPI_MASTER_RANK)
			return true;
		else
			return false;
	}

	else{

		if ((processRank % workersSet) == 0)
			return true;
		else
			return false;
	}
}


int MPI_Base::getMyMaster (int processRank){

	if (workersSet == 0)
		return MPI_MASTER_RANK;
	else
		return (processRank/workersSet) * workersSet;
}


bool MPI_Base::isThisWorkerInMyRange (int master, int processRank){

    int operation;

	if (workersSet == 0)
		return true;

	else{

		if (isMaster(master)){

		    operation = master+workersSet;

			if ((processRank > master) &&  (processRank < operation))
				return true;
			else
				return false;
		}
		else
			return false;
	}
}


void MPI_Base::mpi_send (unsigned int rankReceiver, int bufferSize){

		// Sets currentMPICall records...
		currentMPICall->setCall (MPI_SEND);
		currentMPICall->setSender (myRank);
		currentMPICall->setReceiver (rankReceiver);
		currentMPICall->setBufferSize (bufferSize);
		
		// Debug...
		if (DEBUG_MPIApp)
			showDebugMessage ("[MPI_Base] Rank %d executing MPI_SEND... %s ", myRank, currentMPICall->toString().c_str());

		// Create a message and add the corresponding parameters
		const auto & sm_mpi = makeShared<icancloud_MPI_Message>();
		sm_mpi->setIsResponse (false);
		sm_mpi->setRemoteOperation (false);
		sm_mpi->setOperation (currentMPICall->getCall());		
		sm_mpi->setSourceRank (myRank);
		sm_mpi->setDestRank (rankReceiver);
		sm_mpi->setSize (bufferSize);

		if (isProcessInThisNode(rankReceiver)){
			//printf ("----- Process %d sending to process %d in the same node\n", myRank, rankReceiver);
			sm_mpi->setChunkLength(B(MSG_INITIAL_LENGTH));
		}
		else{
			//printf ("----- Process %d sending to process %d in ANOTHER node\n", myRank, rankReceiver);
		    sm_mpi->setChunkLength(B(MSG_INITIAL_LENGTH + bufferSize));
		}

		// Send...
		auto pkt = new Packet("icancloud_MPI_Message");
		pkt->insertAtFront(sm_mpi);
		icancloud_request_sendDataToNetwork (pkt, rankReceiver);
		
		// Next call...
		currentMPICall->clear();		
}


bool MPI_Base::isProcessInThisNode(unsigned int processID) {

    cModule* currentApp;
//	int currentIndex;
    bool found;
    unsigned int currentRank;

    // Init
    found = false;

    for (cModule::SubmoduleIterator i(getParentModule()); !i.end(); i++) {
        currentApp = *i;

        if (strcmp(currentApp->getFullName(), "app") == 0) // if submod() is in the same vector as this module
                {

            if (currentApp->hasPar("myRank")) {

                currentRank = currentApp->par("myRank");

                if (processID == currentRank)
                    found = true;
            }

        }
    }

    return found;
}



void MPI_Base::mpi_recv (unsigned int rankSender, int bufferSize){

	// Sets currentMPICall records...
	currentMPICall->setCall (MPI_RECV);
	currentMPICall->setSender (rankSender);
	currentMPICall->setReceiver (myRank);
	currentMPICall->setBufferSize (bufferSize);
		
	// Debug...
	if (DEBUG_MPIApp)
		showDebugMessage ("[MPI_Base] Rank %d executing MPI_RECV... %s ", myRank, currentMPICall->toString().c_str());

	// Synchronize with the corresponding send
	synchronize(false);
}

void MPI_Base::mpi_irecv (unsigned int rankSender, int bufferSize){

    // Sets currentMPICall records...
    currentMPICall->setCall (MPI_RECV);
    currentMPICall->setSender (rankSender);
    currentMPICall->setReceiver (myRank);
    currentMPICall->setBufferSize (bufferSize);

    // Debug...
    if (DEBUG_MPIApp)
        showDebugMessage ("[MPI_Base] Rank %d executing MPI_RECV... %s ", myRank, currentMPICall->toString().c_str());

    // Synchronize with the corresponding send
    synchronize(true);
}


void MPI_Base::mpi_barrier (){

		// Sets currentMPICall records...
		currentMPICall->setCall (MPI_BARRIER);
		currentMPICall->setSender (myRank);
		currentMPICall->setReceiver (MPI_MASTER_RANK);
		
		// Debug...
		if (DEBUG_MPIApp)
			showDebugMessage ("[MPI_Base] Rank %d executing BARRIER... %s ", myRank, currentMPICall->toString().c_str());

		// MASTER does Barrier Down!
		if (myRank == MPI_MASTER_RANK){
			barriers [MPI_MASTER_RANK] = MPI_BARRIER_DOWN;
			checkBarriers();
		}

		// A slave process makes barrier down!
		else{

			// Create a message and add the corresponding parameters
			auto sm_mpi = makeShared<icancloud_MPI_Message>();
			sm_mpi->setIsResponse (false);
			sm_mpi->setRemoteOperation (true);
			sm_mpi->setChunkLength(B(MSG_INITIAL_LENGTH));
			sm_mpi->setOperation (MPI_BARRIER_DOWN);		
			sm_mpi->setSourceRank (myRank);
			sm_mpi->setDestRank (MPI_MASTER_RANK);

			// Send...
	        auto pkt = new Packet("icancloud_MPI_Message");
	        pkt->insertAtFront(sm_mpi);
			icancloud_request_sendDataToNetwork (pkt, MPI_MASTER_RANK);
		}
}


void MPI_Base::mpi_barrier_up (){
	
	if (DEBUG_MPIApp)
		showDebugMessage ("Process [%d] BARRIER_UP!", myRank);							

	// Init currentMPICall records...
	currentMPICall->clear();

	// Continue execution
	continueExecution ();
}


void MPI_Base::mpi_barrier_down(Packet *pkt) {

    int senderProcess;

    const auto &sm_mpi = pkt->peekAtFront<icancloud_MPI_Message>();

    // Sender
    senderProcess = sm_mpi->getSourceRank();
    delete (pkt);

    // Check process rank!
    if ((senderProcess >= ((int) numProcesses)) || (senderProcess < 0))
        showErrorMessage("Wrong process rank number: %d", senderProcess);

    // Checks barriers
    barriers[senderProcess] = MPI_BARRIER_DOWN;

    if (DEBUG_MPIApp)
        showDebugMessage("Master process [%d] BARRIER_DOWN process [%d]!",
                myRank, senderProcess);

    checkBarriers();
}


void MPI_Base::initBarriers() {

    unsigned int i;

    for (i = 0; i < numProcesses; i++)
        barriers[i] = MPI_BARRIER_UP;
}

void MPI_Base::checkBarriers() {

    int currentProcess;
    bool allDown;

    // Init...
    allDown = true;
    currentProcess = 0;

    // Check if all process made BARRIER_DOWN
    while ((currentProcess < ((int) numProcesses)) && (allDown)) {

        if (barriers[currentProcess] == MPI_BARRIER_UP)
            allDown = false;
        else
            currentProcess++;
    }

    // Up barriers...
    if (allDown) {

        if (DEBUG_MPIApp)
            showDebugMessage("Master process has received all BARRIERS!");

        // Re-Init barriers!
        initBarriers();

        // Broadcast ACKs
        for (currentProcess = 1; currentProcess < ((int) numProcesses);
                currentProcess++) {

            // Creates BARRIER_UP message
            auto sm_mpi = makeShared<icancloud_MPI_Message>();
            sm_mpi->setIsResponse(false);
            sm_mpi->setRemoteOperation(false);
            sm_mpi->setChunkLength(B(MSG_INITIAL_LENGTH));
            sm_mpi->setOperation(MPI_BARRIER_UP);
            sm_mpi->setSourceRank(MPI_MASTER_RANK);
            sm_mpi->setDestRank(currentProcess);

            // Search the process's socket!
            auto pkt = new Packet("icancloud_MPI_Message");
            pkt->insertAtFront(sm_mpi);
            icancloud_request_sendDataToNetwork(pkt, currentProcess);
        }

        // Init currentMPICall records...
        currentMPICall->clear();

        // Continue execution
        continueExecution();
    }
}


void MPI_Base::mpi_bcast(unsigned int root, int bufferSize) {

    unsigned int i;
    unsigned int maxBcastProcesses;

    // Sets currentMPICall records...
    currentMPICall->setCall(MPI_BCAST);
    currentMPICall->setSender(root);
    currentMPICall->setRoot(root);
    currentMPICall->setReceiver(myRank);
    currentMPICall->setBufferSize(bufferSize);

    // Debug...
    if (DEBUG_MPIApp)
        showDebugMessage("[MPI_Base] Rank %d executing BCAST... %s ", myRank,
                currentMPICall->toString().c_str());

    maxBcastProcesses = (calculateBcastMax() * 2) - 1;
    //maxBcastProcesses = numProcesses/2;

    // If I am root... send messages to each process (bcast)
    if (root == myRank) {

        for (i = 0; i < numProcesses; i++) {

            // Send to all process except myself!
            if (i != myRank) {

                // Create a message and add the corresponding parameters
                auto sm_mpi = makeShared<icancloud_MPI_Message>();
                sm_mpi->setIsResponse(false);
                sm_mpi->setRemoteOperation(false);

                if (i < maxBcastProcesses)
                    sm_mpi->setChunkLength(B(MSG_INITIAL_LENGTH + bufferSize));
                else
                    sm_mpi->setChunkLength(B(MSG_INITIAL_LENGTH));

                sm_mpi->setOperation(MPI_BCAST);
                sm_mpi->setSourceRank(root);
                sm_mpi->setDestRank(i);
                sm_mpi->setSize(currentMPICall->getBufferSize());

                // Send...
                auto pkt = new Packet("icancloud_MPI_Message");
                pkt->insertAtFront(sm_mpi);
                icancloud_request_sendDataToNetwork(pkt, i);
            }
        }

        // Next call...
        currentMPICall->clear();
        continueExecution();
    }

    // Slave process wait to recieve scatter messages!
    else
        synchronize(false);
}


void MPI_Base::mpi_scatter(unsigned int root, int bufferSize) {

    unsigned int i;
    // Sets currentMPICall records...
    currentMPICall->setCall(MPI_SCATTER);
    currentMPICall->setSender(root);
    currentMPICall->setRoot(root);
    currentMPICall->setReceiver(myRank);
    currentMPICall->setBufferSize(bufferSize);

    // Debug...
    if (DEBUG_MPIApp)
        showDebugMessage("[MPI_Base] Rank %d executing SCATTER... %s ", myRank,
                currentMPICall->toString().c_str());

    // If I am root... send messages to each process (scatter)
    if (root == myRank) {

        for (i = 0; i < numProcesses; i++) {

            // Send to all process except myself!
            if (i != myRank) {

                // Create a message and add the corresponding parameters
                auto sm_mpi = makeShared<icancloud_MPI_Message>();
                sm_mpi->setOperation(MPI_SCATTER);
                sm_mpi->setIsResponse(false);
                sm_mpi->setRemoteOperation(false);
                sm_mpi->setChunkLength(B(MSG_INITIAL_LENGTH + bufferSize));
                sm_mpi->setSourceRank(root);
                sm_mpi->setDestRank(i);
                sm_mpi->setSize(currentMPICall->getBufferSize());

                // Send...
                auto pkt = new Packet("icancloud_MPI_Message");
                pkt->insertAtFront(sm_mpi);
                icancloud_request_sendDataToNetwork(pkt, i);
            }
        }

        // Next call...
        currentMPICall->clear();
        continueExecution();
    }

    // Slave process wait to recieve scatter messages!
    else
        synchronize(false);
}


void MPI_Base::mpi_gather(unsigned int root, int bufferSize) {

    // Sets currentMPICall records...
    currentMPICall->setCall(MPI_GATHER);
    currentMPICall->setSender(myRank);
    currentMPICall->setReceiver(root);
    currentMPICall->setRoot(root);
    currentMPICall->setBufferSize(bufferSize);
    currentMPICall->setPendingACKs(numProcesses - 1);

    // Debug...
    if (DEBUG_MPIApp)
        showDebugMessage("[MPI_Base] Rank %d executing GATHER... %s ", myRank,
                currentMPICall->toString().c_str());

    // If I am not root... send messages to root (gather)
    if (root != myRank) {

        // Create a message and add the corresponding parameters
        auto sm_mpi = makeShared<icancloud_MPI_Message>();
        sm_mpi->setOperation(MPI_GATHER);
        sm_mpi->setIsResponse(false);
        sm_mpi->setRemoteOperation(false);
        sm_mpi->setChunkLength(B(MSG_INITIAL_LENGTH + bufferSize));
        sm_mpi->setSourceRank(myRank);
        sm_mpi->setDestRank(root);
        sm_mpi->setSize(currentMPICall->getBufferSize());

        // Send...
        auto pkt = new Packet("icancloud_MPI_Message");
        pkt->insertAtFront(sm_mpi);
        icancloud_request_sendDataToNetwork(pkt, root);

        // next call
        currentMPICall->clear();
        continueExecution();
    }
}

void MPI_Base::mpi_gather_arrives(Packet *pkt)
{

    const auto & sm = pkt->peekAtFront<icancloud_Message>();
    if (sm == nullptr)
        throw cRuntimeError("Incorrect header");
    // Checks if waiting for GATHER Ack
    if (currentMPICall->getCall() != MPI_GATHER)
        showErrorMessage("Expected MPI_Gather! Received:%s",
                callToString(currentMPICall->getCall()).c_str());

    // Update pending Acks
    currentMPICall->arrivesACK();

    // If there is no pending ACKs... process next MPI Call..
    if (currentMPICall->getPendingACKs() == 0) {

        // Debug...
        if (DEBUG_MPIApp)
            showDebugMessage("[MPI_Base] Rank [%u] All GATHER received...",
                    myRank);

        currentMPICall->clear();
        continueExecution();
    }
}

void MPI_Base::synchronize(bool asyn) {

    MPI_Call *searchedCall;

    unsigned int searchCall;
    unsigned int searchSender;
    int searchSize;

    // Init...
    searchCall = MPI_NO_VALUE;
    searchSender = MPI_NO_VALUE;
    searchSize = 0;
    searchedCall = nullptr;

    // This process has not executed the corresponding operation yet!
    if (currentMPICall->getCall() == MPI_NO_VALUE) {
        return;
    }

    // Current MPI Call is MPI_RECV... search a MPI_SEND!
    else if (currentMPICall->getCall() == MPI_RECV) {
        searchCall = MPI_SEND;
        searchSender = currentMPICall->getSender();
        searchSize = currentMPICall->getBufferSize();
    }

    // Current MPI Call is MPI_BCAST!
    else if (currentMPICall->getCall() == MPI_BCAST) {
        searchCall = MPI_BCAST;
        searchSender = currentMPICall->getRoot();
        searchSize = currentMPICall->getBufferSize();
    }

    // Current MPI Call is MPI_SCATTER
    else if (currentMPICall->getCall() == MPI_SCATTER) {
        searchCall = MPI_SCATTER;
        searchSender = currentMPICall->getSender();
        searchSize = currentMPICall->getBufferSize();
    }

    // No synch call!
    //else
    //showErrorMessage ("Unknown call for synchronizing!!! %s", callToString (currentMPICall->getCall()).c_str());

    // Debug!
    if (DEBUG_MPIApp)
        showDebugMessage(
                "Process [%d] Synch! Searching %s - Sender:%u - Size:%d. MPI Call list:%s",
                myRank, callToString(searchCall).c_str(), searchSender,
                searchSize, incomingMPICallsToString().c_str());

    // Search for the incoming call...
    searchedCall = searchMPICall(searchCall, searchSender, searchSize);

    // MPI incoming call found!
    if (searchedCall != nullptr) {

        if (DEBUG_MPIApp)
            showDebugMessage("Process [%d] Synch found!!! %s with incoming %s!",
                    myRank, callToString(currentMPICall->getCall()).c_str(),
                    callToString(searchedCall->getCall()).c_str());

        // Set the last synchronized call
        if (currentMPICall->getSender() == MPI_ANY_SENDER)
            currentMPICall->setSender(searchedCall->getSender());

        currentMPICall->copyMPICall(lastSynchroMPICall);

        // Clean MPI Call
        currentMPICall->clear();

        // Delete current incoming call
        deleteMPICall(searchedCall);

        // Continue execution
        if (!asyn)
            continueExecution();
    }
}

void MPI_Base::insertMPICall(Packet *pkt) {

    MPI_Call *newCall;
    // Create new call
    const auto & sm_mpi = pkt->peekAtFront<icancloud_MPI_Message>();

    newCall = new MPI_Call();
    newCall->setCall(sm_mpi->getOperation());
    newCall->setSender(sm_mpi->getSourceRank());
    newCall->setReceiver(myRank);
    newCall->setFileName(sm_mpi->getFileName());
    newCall->setOffset(sm_mpi->getOffset());
    newCall->setBufferSize(sm_mpi->getSize());
    if (DEBUG_MPIApp)
        showDebugMessage("[MPI_Base] Inserting call in queue of rank %d %s",
                myRank, newCall->toString().c_str());
    // If collective call... set root process!
    if ((newCall->getCall() == MPI_SCATTER)
            || (newCall->getCall() == MPI_BCAST))
        newCall->setRoot(sm_mpi->getSourceRank());
    // Insert on list
    mpiCallList.push_back(newCall);
}

MPI_Call* MPI_Base::searchMPICall(unsigned int call, unsigned int sender,
        int size) {

    MPI_Call *auxCall;
    bool found;
    list<MPI_Call*>::iterator listIterator;

    // Init...
    found = false;
    auxCall = nullptr;

    // Search the corresponding call...
    for (listIterator = mpiCallList.begin();
            ((listIterator != mpiCallList.end()) && (!found)); listIterator++) {

        // Any sender?
        if (sender == MPI_ANY_SENDER) {

            if (((*listIterator)->getCall() == call)
                    && ((*listIterator)->getBufferSize() == size)) {
                found = true;
                auxCall = *listIterator;
            }
        }

        // Specific sender!
        else {

            if (((*listIterator)->getCall() == call)
                    && ((*listIterator)->getSender() == sender)
                    && ((*listIterator)->getBufferSize() == size)) {
                found = true;
                auxCall = *listIterator;
            }
        }
    }

    return auxCall;
}

void MPI_Base::deleteMPICall(MPI_Call* removedCall) {

    mpiCallList.remove(removedCall);
}

string MPI_Base::incomingMPICallsToString() {

    std::ostringstream osStream;
    list<MPI_Call*>::iterator listIterator;

    if (mpiCallList.empty())
        osStream << "Empty!";

    // Search the corresponding call...
    for (listIterator = mpiCallList.begin();
            (listIterator != mpiCallList.end()); listIterator++) {

        osStream << endl;
        osStream << (*listIterator)->toString();
        osStream << endl;
    }

    return osStream.str();
}

string MPI_Base::callToString(int call) {

    string result;

    if (call == MPI_NO_VALUE)
        result = "MPI_NO_VALUE";
    else if (call == MPI_BARRIER_UP)
        result = "MPI_BARRIER_UP";
    else if (call == MPI_BARRIER_DOWN)
        result = "MPI_BARRIER_DOWN";
    else if (call == MPI_SEND)
        result = "MPI_SEND";
    else if (call == MPI_RECV)
        result = "MPI_RECV";
    else if (call == MPI_BARRIER)
        result = "MPI_BARRIER";
    else if (call == MPI_BCAST)
        result = "MPI_BCAST";
    else if (call == MPI_SCATTER)
        result = "MPI_SCATTER";
    else if (call == MPI_GATHER)
        result = "MPI_GATHER";
    else if (call == MPI_FILE_OPEN)
        result = "MPI_FILE_OPEN";
    else if (call == MPI_FILE_CLOSE)
        result = "MPI_FILE_CLOSE";
    else if (call == MPI_FILE_DELETE)
        result = "MPI_FILE_DELETE";
    else if (call == MPI_FILE_READ)
        result = "MPI_FILE_READ";
    else if (call == MPI_FILE_WRITE)
        result = "MPI_FILE_WRITE";
    else if (call == MPI_FILE_CREATE)
        result = "MPI_FILE_CREATE";
    else
        result = "UNKNOWN_MPI_CALL";

    return result;
}

void MPI_Base::addNewConnection(string hostName, unsigned int port,
        unsigned int rank) {

    mpiConnector newMpiConnector;

    // Congigure the new connection
    newMpiConnector.destAddress = hostName;
    newMpiConnector.destPort = port;
    newMpiConnector.rank = rank;

    printf("[%d] Adding a connection to rank [%d]\n", myRank, rank);

    // Add the connection to the connection vector
    mpiConnections.push_back(newMpiConnector);
}

void MPI_Base::establishAllConnections() {

    unsigned int i;

    // Establish all connections
    for (i = 0; i < mpiConnections.size(); i++) {

        if (DEBUG_MPIApp)
            showDebugMessage("Process %d creating remote connection with %s:%d",
                    myRank, mpiConnections[i].destAddress.c_str(),
                    mpiConnections[i].destPort);

        icancloud_request_createConnection(mpiConnections[i].destAddress,
                mpiConnections[i].destPort, mpiConnections[i].rank);
    }
}

string MPI_Base::mpiCommunicationsToString() {

    std::ostringstream osStream;
    unsigned int i;

    osStream.str("");

    // Connection list enable?
    osStream << "Communications configured for processing with rank [" << myRank
            << "]" << endl;

    for (i = 0; i < mpiConnections.size(); i++) {
        osStream << "  Connection[" << i << "] IP:"
                << mpiConnections[i].destAddress << "  Port:"
                << mpiConnections[i].destPort << "  dest Rank:"
                << mpiConnections[i].rank << endl;
    }

    return osStream.str();
}

unsigned int MPI_Base::calculateBcastMax() {

    int maxProcesses;
    int i;
    bool found;

    found = false;
    maxProcesses = 0;
    i = 1;

    while ((!found) && (i <= ((int) numProcesses))) {

        maxProcesses = (int) pow(2, i);

        if (maxProcesses >= ((int) numProcesses))
            found = true;
        else
            i++;
    }

    if (!found)
        showErrorMessage("Number not found!!!");

    return i;
}

MPI_Call* MPI_Base::getLastSynchronizedCall() {

    return (lastSynchroMPICall);
}

} // namespace icancloud
} // namespace inet

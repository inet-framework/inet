#include "inet/icancloud/Applications/Apps/ApplicationHTC/ApplicationHTC.h"
#include <strings.h>

namespace inet {

namespace icancloud {


#define HTCAPP_INIT 0
#define HTCAPP_OPENING_FILE 1
#define HTCAPP_READING_FILE 2
#define HTCAPP_PROCESSING 3
#define HTCAPP_CREATING_FILE 4
#define HTCAPP_WRITTING 5
#define HTCAPP_FINISH 6
#define RESULTS_FILE_SUFFIX "./results_"

Define_Module(ApplicationHTC);


ApplicationHTC::~ApplicationHTC(){
}

void ApplicationHTC::initialize(int stage){
    MPI_Base::initialize(stage);
}

void ApplicationHTC::startExecution(){

	std::ostringstream osStream;	
	

		// Init the super-class
        MPI_Base::startExecution ();
		
		// Set the moduleIdName
		osStream << "ApplicationHTC." << getId();
		moduleIdName = osStream.str();
		
		// Get parameters from module
		startDelay_s = par ("startDelay_s");
		numProcessedFiles = par ("numProcessedFiles");
		fileSize_MB = par ("fileSize_MB");
		resultsSize_MB = par ("resultsSize_MB");
		cpuMIs = par ("cpuMIs");	
		fileNameSuffix = (const char*) par("fileNameSuffix");
		currentFile = 0;
		
		if (numProcessedFiles == 0)
			showErrorMessage ("Number of processed files must be greater than 0!");
		
		// Init state
		currentState = HTCAPP_INIT;	
	    std::ostringstream osStream2;
	    osStream2 << "Simulation time:rank[" << myRank <<  "] ";
        jobResults->newJobResultSet(osStream2.str().c_str());
		// Create SM_WAIT_TO_EXECUTE and wait startDelay to sent it!
		cMessage *waitToExecuteMsg = new cMessage (SM_WAIT_TO_EXECUTE.c_str());
	    scheduleAt (simTime() + startDelay_s, waitToExecuteMsg);	
}

void ApplicationHTC::finish(){

	// Finish the super-class
    MPI_Base::finish();
}

void ApplicationHTC::processSelfMessage (cMessage *msg){
		
	// Execute!!!
	if (!strcmp (msg->getName(), SM_WAIT_TO_EXECUTE.c_str())){

		// Delete msg!
		cancelAndDelete (msg);

		// Starting time...
		simStartTime = simTime();
		runStartTime = time (nullptr);

		// Start execution...
		continueExecution();
	}

	else
		showErrorMessage ("Unknown self message [%s]", msg->getName());
}

void ApplicationHTC::processRequestMessage (Packet *){
	showErrorMessage ("This module does not recieve request messages!");
}

void ApplicationHTC::processResponseMessage(Packet *pkt) {


    pkt->trim();
    auto sm = pkt->removeAtFront<icancloud_Message>();
    // Try to cast to icancloud_App_NET_Message
    auto sm_net = dynamicPtrCast<icancloud_App_NET_Message>(sm);

    // NET call response...
    if (sm_net != nullptr) {
        pkt->insertAtFront(sm_net);
        processNetCallResponse(pkt);
    }

    // Try to cast to icancloud_App_IO_Message
    else {

        // Try to cast to icancloud_App_IO_Message
        auto sm_io = dynamicPtrCast<icancloud_App_IO_Message>(sm);

        // NET call response...
        if (sm_io != nullptr) {
            pkt->insertAtFront(sm_io);
            processIOCallResponse(pkt);
        }

        // Try to cast to icancloud_App_CPU_Message
        else {

            // Try to cast to icancloud_App_CPU_Message
            auto sm_cpu = dynamicPtrCast<icancloud_App_CPU_Message>(sm);

            // NET call response...
            if (sm_cpu != nullptr) {
                pkt->insertAtFront(sm_cpu);
                processCPUCallResponse(pkt);
            }

            // Unknown message type
            else {
                showErrorMessage("Unknown message type as response :%s",
                        sm->contentsToString(true).c_str());
            }
        }
    }
}

void ApplicationHTC::continueExecution (){

    executeNextState ();
}

void ApplicationHTC::executeNextState (){
	
	// There are more pending files to be processed...
	if (currentFile < numProcessedFiles){
	
		// Init state... read data!
		if (currentState == HTCAPP_INIT){
			currentState = HTCAPP_OPENING_FILE;								
			openFile();						
		}
		
		// Open finished! Start reading data...
		else if (currentState == HTCAPP_OPENING_FILE){
			currentState = HTCAPP_READING_FILE;								
			readInputData();								
		}
		
		// Read finished! Start processing data...
		else if (currentState == HTCAPP_READING_FILE){
			currentState = HTCAPP_PROCESSING;								
			processingCPU();								
		}
		
		// Read finished! Start processing data...
		else if (currentState == HTCAPP_PROCESSING){
			processNextFile();							
		}	
	}	
	
	// All files have been processed...
	else{
	
		if (currentState == HTCAPP_CREATING_FILE){
			currentState = HTCAPP_WRITTING;
			
			if (DEBUG_ApplicationHTC)
				showDebugMessage ("App(%d): Creating results file [%s].", myRank, getResultsFileName().c_str());		
			
			icancloud_request_create (getResultsFileName().c_str());
		}
		
		else if (currentState == HTCAPP_WRITTING){
			currentState = HTCAPP_FINISH;
			writtingData();
		}
		
		else if (currentState == HTCAPP_FINISH){
			showResults();
		}
	}				
}

void ApplicationHTC::processNextFile(){
	
	currentFile++;
	
	// All files have been processed
	if (currentFile >= numProcessedFiles){
	
		if (resultsSize_MB > 0)
			currentState = HTCAPP_CREATING_FILE;
		else
			currentState = HTCAPP_FINISH;
	}
	// There are more files unprocessed
	else
		currentState = HTCAPP_INIT;	
	
	continueExecution();
}

void ApplicationHTC::openFile(){
	
	string fileName;
	
		// Get current file name
		fileName = getCurrentFileName (currentFile);
		
		if (DEBUG_ApplicationHTC)
			showDebugMessage ("App(%d): Opening file [%s]. Current File number:%d", myRank, fileName.c_str(), currentFile);
		
		// Open the file!
		icancloud_request_open (fileName.c_str());
}

void ApplicationHTC::readInputData(){

	string fileName;
	unsigned int dataSize;
	
		// Get current file name
		fileName = getCurrentFileName (currentFile);
		
		// Calculate data size
		dataSize = fileSize_MB * MB;
		
		if (DEBUG_ApplicationHTC)
			showDebugMessage ("App(%d): Reading file [%s]. [%d bytes] Current File number:%d", myRank, fileName.c_str(), dataSize, currentFile);										
		
		// Reading data for each worker process
		icancloud_request_read (fileName.c_str(), 0, dataSize);
}

void ApplicationHTC::processingCPU(){
	
	if (DEBUG_ApplicationHTC)
		showDebugMessage ("App(%d): Processing %d MIs...", myRank, cpuMIs);	
	
	icancloud_request_cpu (cpuMIs);
}

void ApplicationHTC::writtingData(){	
	
	string fileName;
	unsigned int dataSize;
	
		
		// Create the file name		
		fileName = getResultsFileName().c_str();
			
		// Calculate data size
		dataSize = resultsSize_MB * MB;
		
		if (DEBUG_ApplicationHTC)
			showDebugMessage ("App(%d): Writting file [%s]. [%d bytes]", myRank, fileName.c_str(), dataSize);										
		
		// Reading data for each worker process
		icancloud_request_write (fileName.c_str(), 0, dataSize);
}

void ApplicationHTC::processNetCallResponse (Packet *pkt){

	// Create connection response...
    const auto & responseMsg = pkt->peekAtFront<icancloud_App_NET_Message>();
	if (responseMsg->getOperation () == SM_CREATE_CONNECTION){

		// Set the established connection.
		setEstablishedConnection (pkt);
		delete (pkt);
	}

	else {
	    pkt->trim();
	    auto responseMsg = pkt->removeAtFront<icancloud_App_NET_Message>();
		showErrorMessage ("Unknown NET call response :%s", responseMsg->contentsToString(true).c_str());
	}
}

void ApplicationHTC::processIOCallResponse(Packet *pkt) {

    bool correctResult;
    std::ostringstream osStream;

    const auto &responseMsg = pkt->peekAtFront<icancloud_App_IO_Message>();
    // Init...
    correctResult = false;

    // Open response!
    if (responseMsg->getOperation() == SM_OPEN_FILE) {

        // All ok!
        if (responseMsg->getResult() == icancloud_OK) {
            correctResult = true;
        }

        // File not found!
        else if (responseMsg->getResult() == icancloud_FILE_NOT_FOUND) {
            osStream << "File not found!";
            correctResult = false;
        }

        // Unknown result!
        else {
            osStream << "Unknown result value:" << responseMsg->getResult();
            correctResult = false;
        }
    }

    // Check the results...
    else if (responseMsg->getOperation() == SM_READ_FILE) {

        // Check state!
        if (currentState != HTCAPP_READING_FILE)
            throw new cRuntimeError(
                    "State error! Must be HPCAPP_MASTER_READING\n");

        // All ok!
        if (responseMsg->getResult() == icancloud_OK) {
            correctResult = true;
        }

        // File not found!
        else if (responseMsg->getResult() == icancloud_FILE_NOT_FOUND) {
            osStream << "File not found!";
            correctResult = false;
        }

        // File not found!
        else if (responseMsg->getResult() == icancloud_DATA_OUT_OF_BOUNDS) {
            osStream << "Request out of bounds!";
            correctResult = false;
        }

        // Unknown result!
        else {
            osStream << "Unknown result value:" << responseMsg->getResult();
            correctResult = false;
        }
    }

    // Write response!
    else if (responseMsg->getOperation() == SM_WRITE_FILE) {

        // All ok!
        if (responseMsg->getResult() == icancloud_OK) {
            correctResult = true;
        }

        // File not found!
        else if (responseMsg->getResult() == icancloud_FILE_NOT_FOUND) {
            osStream << "File not found!";
            correctResult = false;
        }

        // File not found!
        else if (responseMsg->getResult() == icancloud_DISK_FULL) {
            osStream << "Disk full!";
            correctResult = false;
        }

        // Unknown result!
        else {
            osStream << "Unknown result value:" << responseMsg->getResult();
            correctResult = false;
        }
    }

    // Create response!
    else if (responseMsg->getOperation() == SM_CREATE_FILE) {

        // All ok!
        if (responseMsg->getResult() == icancloud_OK) {
            correctResult = true;
        }

        // File not found!
        else if (responseMsg->getResult() == icancloud_DISK_FULL) {
            osStream << "Disk full!";
            correctResult = false;
        }

        // Unknown result!
        else {
            osStream << "Unknown result value:" << responseMsg->getResult();
            correctResult = false;
        }
    }

    else {
        pkt->trim();
        auto responseMsg = pkt->removeAtFront<icancloud_App_IO_Message>();
        // Error in response message...
        showErrorMessage("Error! Unknown IO operation:%s. %s",
                osStream.str().c_str(),
                responseMsg->contentsToString(DEBUG_MSG_ApplicationHTC).c_str());
    }

    // Error in response message?
    if (!correctResult) {
        pkt->trim();
        auto responseMsg = pkt->removeAtFront<icancloud_App_IO_Message>();
        showErrorMessage("Error in response message:%s. %s",
                osStream.str().c_str(),
                responseMsg->contentsToString(DEBUG_MSG_ApplicationHTC).c_str());

        throw new cRuntimeError("Error in response message\n");
    }

    // All OK!
    else {
        continueExecution();
    }

    delete (pkt);
}

void ApplicationHTC::processCPUCallResponse (Packet *pkt){
		
	delete (pkt);
	continueExecution ();
}

string ApplicationHTC::getCurrentFileName (unsigned int currentFileNumber){

	string currentFileName;
	char fileName [NAME_SIZE];
	int number;
	// Init...
	
	memset(fileName,0,sizeof(fileName));
	// Create the file name
	number = (myRank*numProcessedFiles) + currentFile;
	sprintf (fileName, "%s%d", fileNameSuffix.c_str(), number);

	currentFileName = fileName;
		
	return fileName;
}

string ApplicationHTC::getResultsFileName() {

    string currentFileName;
    char fileName[NAME_SIZE];

    // Init...
    memset(fileName, 0,sizeof(fileName));

    // Create the file name
    sprintf(fileName, "%s%d", RESULTS_FILE_SUFFIX, myRank);

    currentFileName = fileName;

    return fileName;
}

void ApplicationHTC::showResults (){

    std::ostringstream osStream;

	// End time
    simEndTime = simTime();
    runEndTime = time (nullptr);      					
    osStream << (simEndTime - simStartTime).dbl();
    jobResults->setJobResult(0,osStream.str());
    
    showResultMessage ("App [%s] - Rank:%d", moduleIdName.c_str(), myRank);
    showResultMessage ("Simulation time:%f - Real execution time:%f", (simEndTime-simStartTime).dbl(), (difftime (runEndTime,runStartTime)));
    
    userPtr->notify_UserJobHasFinished(this);
}



} // namespace icancloud
} // namespace inet

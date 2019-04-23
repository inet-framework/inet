
#include "inet/icancloud/Applications/Apps/ServerApplication/ServerApplication.h"

namespace inet {

namespace icancloud {

#define INPUT_FILE "/web.html"
#define OUTPUT_FILE "/log.dat"
#define MAX_FILE_SIZE 2000000000
#define SM_WAIT_TO_EVENT  "Wait_To_Event"

Define_Module(ServerApplication);



ServerApplication::~ServerApplication(){
}

void ServerApplication::initialize(int stage) {

    // Init the super-class
    UserJob::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {

        std::ostringstream osStream;
        timeoutEvent = nullptr;
        timeout = 1.0;

        // Set the moduleIdName
        osStream << "ServerApplication." << getId();
        moduleIdName = osStream.str();

        // App Module parameters
        startDelay = par("startDelay");
        inputSize = par("inputSize");
        MIs = par("MIs");
        hitsPerHour = par("hitsPerHour").intValue();
        uptimeLimit = par("uptimeLimit").intValue();
        intervalHit = (3600.0 / (double) hitsPerHour);

        pendingHits = 0;

        if (uptimeLimit != 0)
            uptimeLimit = uptimeLimit + simTime().dbl();

        // Service times
        total_service_IO = 0.0;
        total_service_CPU = 0.0;
        startServiceIO = 0.0;
        endServiceIO = 0.0;
        startServiceCPU = 0.0;
        endServiceCPU = 0.0;
        readOffset = 0;

        // Boolean variables
        executeCPU = executeRead = false;

        // Assign names to the results
        jobResults->newJobResultSet("totalIO");
        jobResults->newJobResultSet("totalCPU");
        jobResults->newJobResultSet("Real run-time");
        jobResults->newJobResultSet("Simulation time");
    }

}

void ServerApplication::startExecution (){

    API_OS::startExecution();
	// Create SM_WAIT_TO_EXECUTE message for delaying the execution of this application
    // Initialize ..
    newIntervalEvent = new cMessage ("intervalEvent");
	cMessage *waitToExecuteMsg = new cMessage (SM_WAIT_TO_EXECUTE.c_str());
	scheduleAt (simTime()+startDelay, waitToExecuteMsg);
}

void ServerApplication::finish(){

	// Finish the super-class
	UserJob::finish();

}

void ServerApplication::processSelfMessage (cMessage *msg){

    SimTime nextEvent;

		if (!strcmp (msg->getName(), SM_WAIT_TO_EXECUTE.c_str())){

			// Starting time...
			simStartTime = simTime();
			runStartTime = time (nullptr);
			cancelAndDelete(msg);
			// Init...
			scheduleAt (simTime()+ intervalHit, newIntervalEvent);

		} else if (!strcmp (msg->getName(), "intervalEvent")){

           newHit();

           // A set of hits each second
           if ((uptimeLimit >= simTime().dbl()) || (uptimeLimit == 0)){
               cancelEvent(msg);
               scheduleAt (simTime()+ intervalHit, newIntervalEvent);
           } else {
               uptimeLimit = -1;
               cancelAndDelete(msg);
           }

        }else{

			showErrorMessage ("Unknown self message [%s]", msg->getName());
		    cancelAndDelete(msg);
        }


}

void ServerApplication::processRequestMessage (Packet *pkt){

}

void ServerApplication::processResponseMessage(Packet *pkt) {

    bool isError;
    std::ostringstream osStream;
    int operation;
    pkt->trim();
    auto sm = pkt->removeAtFront<icancloud_Message>();

    // Init...
    operation = sm->getOperation();
    auto sm_io = dynamicPtrCast<icancloud_App_IO_Message>(sm);
    auto sm_cpu = dynamicPtrCast<icancloud_App_CPU_Message>(sm);
    isError = false;

    // IO Message?
    if (sm_io != nullptr) {

        // Get time!
        endServiceIO = simTime();

        // Read response!
        if (operation == SM_READ_FILE) {
            // All ok!
            if (sm_io->getResult() == icancloud_OK) {
                executeCPU = true;
                executeRead = false;
            }

            // File not found!
            else if (sm_io->getResult() == icancloud_FILE_NOT_FOUND) {
                osStream << "File not found!";
                isError = true;
            }

            // File not found!
            else if (sm_io->getResult() == icancloud_DATA_OUT_OF_BOUNDS) {
                executeCPU = true;
            }

            // Unknown result!
            else {
                osStream << "Unknown result value:" << sm_io->getResult();
                isError = true;
            }
        }

        // Unknown I/O operation
        else {
            osStream << "Unknown received response message";
            isError = true;
        }

        // Increase total time for I/O
        total_service_IO += (endServiceIO - startServiceIO);

    }

    // Response came from CPU system
    else if (sm_cpu != nullptr) {

        // Get time!
        endServiceCPU = simTime();

        // CPU!
        if (operation == SM_CPU_EXEC) {

            pendingHits--;
            if ((pendingHits == 0) && (uptimeLimit == -1)) {
                printResults();
            }
        }

        // Unknown CPU operation
        else {
            osStream << "Unknown received response message";
            isError = true;
        }

        // Increase total time for I/O
        total_service_CPU += (endServiceCPU - startServiceCPU);
    }

    // Wrong response message!
    else {

        osStream << "Unknown received response message";
        isError = true;
    }

    // Error?
    if (isError) {

        showErrorMessage("Error in response message:%s. %s",
                osStream.str().c_str(), sm_io->contentsToString(true).c_str());
    }

    // CPU?
    else if (executeCPU) {

        // Execute CPU!
        executeCPUrequest();
    }

    // IO?
    else if (executeRead) {
        serveWebCode();
    }

    // Inconsistency error!
    else
        showErrorMessage("Inconsistency error!!!! :%s. %s",
                osStream.str().c_str(), sm->contentsToString(true).c_str());

    pkt->insertAtFront(sm);
    delete (pkt);
} 


void ServerApplication::changeState(string newState){

}


void ServerApplication::newHit(){
    pendingHits++;
    startServiceIO = simTime();
    serveWebCode ();
}


void ServerApplication::serveWebCode(){
Enter_Method_Silent();
	// Reset timer!
	startServiceIO = simTime();

	// Executes read operation
		if ((readOffset+(inputSize*MB))>=MAX_FILE_SIZE)
			readOffset = 0;

		if (DEBUG_Application)
			showDebugMessage ("Executing (Read) Offset:%d; dataSize:%d", readOffset,  inputSize*MB);

		icancloud_request_read (INPUT_FILE, readOffset, inputSize*KB);
		readOffset += (inputSize*KB);

}


void ServerApplication::executeCPUrequest(){
    Enter_Method_Silent();

	// Debug?
	if (DEBUG_Application)
		showDebugMessage ("Executing (CPU) MIs:%d", MIs);

	// Reset timer!
	startServiceCPU = simTime ();
	icancloud_request_cpu (MIs);
}


void ServerApplication::printResults (){

	std::ostringstream osStream;

	//Init..
		simEndTime = simTime();
		runEndTime = time (nullptr);

		showResultMessage ("App [%s] - Simulation time:%f - Real execution time:%f - IO:%f  CPU:%f",
		                           moduleIdName.c_str(),
		                           (simEndTime-simStartTime).dbl(),
		                           (difftime (runEndTime,runStartTime)),
		                           total_service_IO.dbl(),
		                           total_service_CPU.dbl());

    //Assign values to the results
        osStream <<  total_service_IO.dbl();
        jobResults->setJobResult(0, osStream.str());
        osStream.str("");

        osStream <<  total_service_CPU.dbl();
        jobResults->setJobResult(1, osStream.str());
        osStream.str("");

        osStream <<  difftime (runEndTime,runStartTime);
        jobResults->setJobResult(2, osStream.str());
        osStream.str("");

        osStream << (simEndTime - simStartTime).dbl();
        jobResults->setJobResult(3, osStream.str());

        addResults(jobResults);
    //Send results list to the cloudManager
        userPtr->notify_UserJobHasFinished(this);

}


} // namespace icancloud
} // namespace inet

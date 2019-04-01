#include "inet/icancloud/Applications/Apps/ApplicationCheckpoint/ApplicationCheckpoint.h"

namespace inet {

namespace icancloud {


#define APP_HPC_RESULTS_FILENAME "/fileResults"

#define HPC_CHECKPOINT_INIT 0
#define HPC_CHECKPOINT_PROCESSING 1
#define HPC_CHECKPOINT_WRITTING 2
#define HPC_CHECKPOINT_WRITE_PARTS 3

Define_Module(ApplicationCheckpoint);

ApplicationCheckpoint::~ApplicationCheckpoint(){
}

void ApplicationCheckpoint::initialize(int stage) {

    MPI_Base::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        std::ostringstream osStream;

        // Init number of worker processes per master process
        workersSet = 0;

        // Init the super-class

        // Set the moduleIdName
        osStream << "ApplicationCheckpoint." << getId();
        moduleIdName = osStream.str();

        // Get parameters from module
        numIterations = par("numIterations");
        dataToWrite_KB = par("dataToWrite_KB");
        sliceCPU = par("sliceCPU");
        currentIteration = 1;
        currentOffset = 0;
        workersSet = 0;

        totalIO = ioStart = ioEnd = 0.0;
        totalCPU = cpuStart = cpuEnd = 0.0;

        sprintf(outFileName, "%s_%d.dat", APP_HPC_RESULTS_FILENAME, myRank);

        // Init state
        currentState = HPC_CHECKPOINT_INIT;

        // Show info?
        showStartedModule(" %s ", mpiCommunicationsToString().c_str());

        // Assign names to the results
        jobResults->newJobResultSet("moduleIdName");
        jobResults->newJobResultSet("Rank");
        jobResults->newJobResultSet("totalIO");
        jobResults->newJobResultSet("totalCPU");
        jobResults->newJobResultSet("Real run-time");
        jobResults->newJobResultSet("Simulation time");
    }
}

void ApplicationCheckpoint::finish(){

	// Finish the super-class
	MPI_Base::finish();
}

void ApplicationCheckpoint::processSelfMessage (cMessage *msg){

    // Execute!!!
           if (!strcmp (msg->getName(), SM_WAIT_TO_EXECUTE.c_str())){

                   // Delete msg!
                   cancelAndDelete (msg);

                   // Starting time...
                   simStartTime = simTime();
                   runStartTime = time (nullptr);

                   // Execute!
                   calculateNextState ();
           }

           // Establish connection with server...
           else if (!strcmp (msg->getName(), SM_WAIT_TO_CONNECT.c_str())){

                   // Delete message
                   cancelAndDelete (msg);

                   // Establish all connections...
                   establishAllConnections();
           }

           else
                   showErrorMessage ("Unknown self message [%s]", msg->getName());


}

void ApplicationCheckpoint::processResponseMessage (Packet *pkt){


    pkt->trimFront();
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

void ApplicationCheckpoint::calculateNextState (){


	// Init state... read data!
	if (currentState == HPC_CHECKPOINT_INIT){

		totalResponses = 1;
		ioStart = simTime();

		if (DEBUG_ApplicationCheckpoint)
			showDebugMessage ("Process %d - Creating results file %s!", myRank, outFileName);

		currentState = HPC_CHECKPOINT_PROCESSING;
		icancloud_request_create (outFileName);
	}

	// Processing
	else if (currentState == HPC_CHECKPOINT_PROCESSING){
		
		ioEnd = simTime();
		totalIO = totalIO + (ioEnd - ioStart);

		if (DEBUG_ApplicationCheckpoint)
			showDebugMessage ("[It:%d] Process %d - State:%s", currentIteration, myRank, stateToString(currentState).c_str());

		currentState = HPC_CHECKPOINT_WRITTING;
		totalResponses = HPC_CHECKPOINT_WRITE_PARTS;

		cpuStart = simTime();

		processingCPU();
	}
			
	// Writting results...
	else if (currentState == HPC_CHECKPOINT_WRITTING){
		
		// There are pending requests...
		if (totalResponses > 0){

			if (totalResponses == HPC_CHECKPOINT_WRITE_PARTS){

				cpuEnd = simTime();
				totalCPU = totalCPU + (cpuEnd - cpuStart);

				ioStart = simTime();
				writtingData();
			}

			else{
				writtingData();
			}
		}
		
		// All requests have been performed
		else{

			ioEnd = simTime();
			totalIO = totalIO + (ioEnd - ioStart);

			currentState = HPC_CHECKPOINT_PROCESSING;
			nextIteration();
		}			
	}
}

void ApplicationCheckpoint::nextIteration (){
	
	// Next iteration
	currentIteration++;

	printf ("-----> Processing iteration: %d - %d\n", currentIteration, numIterations);
			
	// All iterations complete?
	if (currentIteration > numIterations){

		showResults();	

		//if (myRank == MPI_MASTER_RANK)
			//endSimulation();
	}
	else		
		calculateNextState ();	
}

void ApplicationCheckpoint::continueExecution (){
		
	calculateNextState ();
}

void ApplicationCheckpoint::processingCPU(){
	
	if (DEBUG_ApplicationCheckpoint)
		showDebugMessage ("Process %d - Processing...", myRank);	
	
	icancloud_request_cpu (sliceCPU);
}

void ApplicationCheckpoint::writtingData(){


	unsigned int dataSize;

		dataSize = (dataToWrite_KB*KB) / HPC_CHECKPOINT_WRITE_PARTS;
		
	if (DEBUG_ApplicationCheckpoint)
		showDebugMessage ("Process %d - Writting file %s - Offset:%u - Size:%u", myRank, outFileName, currentOffset, dataSize);

	// Reading data for each worker process
	icancloud_request_write (outFileName, currentOffset, dataSize);

	// Calculating next offset
	if ((currentOffset+dataSize) > 3*GB)
		currentOffset = 0;
	else
		currentOffset += dataSize;
}

void ApplicationCheckpoint::processNetCallResponse (Packet *pkt)
{


    const auto & responseMsg = pkt->peekAtFront<icancloud_App_NET_Message>();

    int operation;

    operation = responseMsg->getOperation ();

	// Create connection response...
	if (operation == SM_CREATE_CONNECTION){

		// Set the established connection.
		setEstablishedConnection (pkt);
		delete (pkt);
	}

	else {
	    pkt->trimFront();
	    auto responseMsg = pkt->removeAtFront<icancloud_App_IO_Message>();
		showErrorMessage ("Unknown NET call response :%s", responseMsg->contentsToString(true).c_str());
	}
}

void ApplicationCheckpoint::processIOCallResponse (Packet *pkt){
	
	bool correctResult;
	std::ostringstream osStream;
    int operation;

    const auto & responseMsg = pkt->peekAtFront<icancloud_App_IO_Message>();


		// Init...
		correctResult = false;		
	    operation = responseMsg->getOperation ();


		if (operation == SM_WRITE_FILE){
			
			// Check state!
			if (currentState != HPC_CHECKPOINT_WRITTING)
				throw new cRuntimeError("State error! Must be HPCAPP_MASTER_WRITTING\n");

			// All ok!
			if (responseMsg->getResult() == icancloud_OK){
				correctResult = true;
			}

			// File not found!
			else if (responseMsg->getResult() == icancloud_FILE_NOT_FOUND){
				osStream << "File not found!";
				correctResult = false;
			}

			// File not found!
			else if (responseMsg->getResult() == icancloud_DISK_FULL){
				osStream << "Disk full!";
				correctResult = false;
			}

			// Unknown result!
			else{
				osStream << "Unknown result value:" << responseMsg->getResult();
				correctResult = false;
			}
		}
		
		// Create response!
		else if (operation == SM_CREATE_FILE){
			
			// All ok!
			if (responseMsg->getResult() == icancloud_OK){
				correctResult = true;
			}
			
			// File not found!
			else if (responseMsg->getResult() == icancloud_DISK_FULL){
				osStream << "Disk full!";
				correctResult = false;
			}		
			
			// Unknown result!
			else{
				osStream << "Unknown result value:" << responseMsg->getResult();
				correctResult = false;
			}		
		}
		
		else{
			
			// Error in response message...
		    pkt->trimFront();
		    auto responseMsg = pkt->removeAtFront<icancloud_App_IO_Message>();
			showErrorMessage ("Error! Unknown IO operation:%s. %s",
							  osStream.str().c_str(),
							  responseMsg->contentsToString(DEBUG_MSG_ApplicationCheckpoint).c_str());
		}
		
		
		// Error in response message?
		if (!correctResult){
		    pkt->trimFront();
		    auto responseMsg = pkt->removeAtFront<icancloud_App_IO_Message>();
			showErrorMessage ("Error in response message:%s. %s",
							  osStream.str().c_str(),
							  responseMsg->contentsToString(DEBUG_MSG_ApplicationCheckpoint).c_str());
		
			throw new cRuntimeError("Error in response message\n");
		}
		
		// All OK!				  
		else{		
			totalResponses--;			
			
			if (DEBUG_ApplicationCheckpoint)
				showDebugMessage ("Process %d - Arrives I/O response. Left:%d", myRank, totalResponses);			
			
			// Error in synchronization phase?
			if (totalResponses < 0)
				throw new cRuntimeError("Synchronization error!\n");		
		
			calculateNextState ();
		}
		
	//delete (responseMsg);
}

void ApplicationCheckpoint::processCPUCallResponse (Packet * pkt){

	//delete (responseMsg);
	calculateNextState ();	
}

string ApplicationCheckpoint::stateToString(int state){
	
	string stringState;	
	
		if (state == HPC_CHECKPOINT_INIT)
			stringState = "HPCAPP_INIT";

		else if (state == HPC_CHECKPOINT_PROCESSING)
			stringState = "HPC_CHECKPOINT_PROCESSING";
			
		else if (state == HPC_CHECKPOINT_WRITTING)
			stringState = "HPC_CHECKPOINT_WRITTING";

		else stringState = "Unknown state";		
	
			
	return stringState;
}

void ApplicationCheckpoint::showResults (){
    
    showResultMessage ("App [%s] - Rank:%d - Simulation time:%s - Real execution time:%f - IO:%s  CPU:%s",
						moduleIdName.c_str(), myRank,
						(simEndTime-simStartTime).str().c_str(), difftime (runEndTime,runStartTime),
						totalIO.str().c_str(),
						totalCPU.str().c_str());

	std::ostringstream osStream;

	//Init..


	// End time
		simEndTime = simTime();
		runEndTime = time (nullptr);

    //Assign values to the results

		osStream <<  moduleIdName.c_str();
		jobResults->setJobResult(0,osStream.str());
		osStream.str("");

		osStream <<  myRank;
		jobResults->setJobResult(1, osStream.str());
		osStream.str("");

		osStream <<  totalIO.dbl();
		jobResults->setJobResult(2, osStream.str());
		osStream.str("");

		osStream <<  totalCPU.dbl();
		jobResults->setJobResult(3, osStream.str());
		osStream.str("");

		osStream <<  difftime (runEndTime,runStartTime);
		jobResults->setJobResult(4, osStream.str());
		osStream.str("");

		osStream << (simEndTime - simStartTime).dbl();
		jobResults->setJobResult(5, osStream.str());

		addResults(jobResults);
	//Send results list to the cloudManager
		userPtr->notify_UserJobHasFinished(this);

	// Create SM_WAIT_TO_SCHEDULER message for delaying the execution of this application
		timeoutEvent = new cMessage (SM_WAIT_TO_SCHEDULER.c_str());
		scheduleAt (simTime(), timeoutEvent);

}



} // namespace icancloud
} // namespace inet

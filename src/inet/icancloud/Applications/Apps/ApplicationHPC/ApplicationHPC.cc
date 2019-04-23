#include "inet/icancloud/Applications/Apps/ApplicationHPC/ApplicationHPC.h"

namespace inet {

namespace icancloud {


#define CHECK_MEMORY_HPC_APP 0


#define APP_HPC_INPUT_FILENAME "/fileInput"
#define APP_HPC_RESULTS_FILENAME "/fileResults"

#define HPCAPP_INIT 0
#define HPCAPP_MASTER_READING 1
#define HPCAPP_MASTER_DELIVERING 2
#define HPCAPP_MASTER_WAITING_RESULTS 3
#define HPCAPP_MASTER_WRITTING 4

#define HPCAPP_WORKER_RECEIVING 11
#define HPCAPP_WORKER_READING 12
#define HPCAPP_WORKER_ALLOCATING 13
#define HPCAPP_WORKER_PROCESSING 14
#define HPCAPP_WORKER_WRITTING 15
#define HPCAPP_WORKER_SENDING_BACK 16

#define METADATA_MSG_SIZE 64
#define HPCAPP_MAX_SIZE_PER_INPUT_FILE (2000000*KB)


Define_Module(ApplicationHPC);

ApplicationHPC::~ApplicationHPC(){
}


void ApplicationHPC::getRAM(bool isInit){

    FILE* stream;
    FILE* streamOut;
    int bufsize = 1000;
    char outputFile[100];

		// Open mem file
		stream = fopen("/proc/meminfo", "r" );

		// Output file name
		if (isInit)
			sprintf (outputFile, "memInit_%d.txt", myRank);
		else
			sprintf (outputFile, "memEnd_%d.txt", myRank);

		streamOut = fopen(outputFile, "w+" );

		// Read data
		while(!feof(stream) && !ferror(stream)){
			char buf[bufsize];
			fwrite (buf, 1, bufsize, streamOut);
		}

		fclose (stream);
		fclose (streamOut);
}


void ApplicationHPC::initialize(int stage){

    // Init the super-class
    MPI_Base::initialize(stage);

}


void ApplicationHPC::finish(){

	// Finish the super-class
	MPI_Base::finish();
	

}

void ApplicationHPC::startExecution(){

  // Define ..
    std::ostringstream osStream;
        // Get the workers per set
        workersSet = par ("workersSet");

        CfgMPI *mpiCfg = userPtr->getMPIEnv();
        numProcesses = mpiCfg->getNumProcesses();

        MPI_Base::startExecution ();

        // Set the moduleIdName
        osStream << "ApplicationHPC." << getId();
        moduleIdName = osStream.str();

            numIterations = par ("numIterations");
            sliceToWorkers_KB = par ("sliceToWorkers_KB");
            sliceToMaster_KB = par ("sliceToMaster_KB");
            sliceCPU = par ("sliceCPU");
            workersRead = par ("workersRead");
            workersWrite = par ("workersWrite");
            currentIteration = 1;
            offsetRead = offsetWrite = 0;
            createdResultsFile = false;

            // Calculate the dataSet size
            dataSet_total_KB = sliceToWorkers_KB * numProcesses * numIterations;
            dataResult_total_KB = sliceToMaster_KB * numProcesses * numIterations;

            // Output file!
            if (workersRead){

                previousData_input_KB = myRank * sliceToWorkers_KB * numIterations;
                previousData_output_KB = myRank * sliceToMaster_KB * numIterations;

                currentInputFile = previousData_input_KB / (HPCAPP_MAX_SIZE_PER_INPUT_FILE / KB);
                currentOutputFile = previousData_output_KB / (HPCAPP_MAX_SIZE_PER_INPUT_FILE / KB);

                offsetRead = (previousData_input_KB % (HPCAPP_MAX_SIZE_PER_INPUT_FILE / KB)) * KB;
                offsetWrite = (previousData_output_KB % (HPCAPP_MAX_SIZE_PER_INPUT_FILE / KB)) * KB;
            }
            else if (isMaster(myRank)){
                if (workersSet == 0) {
                    previousData_input_KB = (myRank) * sliceToWorkers_KB * numIterations ;
                    previousData_output_KB = (myRank) * sliceToMaster_KB * numIterations ;
                } else{
                    previousData_input_KB = (myRank/workersSet) * sliceToWorkers_KB * numIterations * workersSet;
                    previousData_output_KB = (myRank/workersSet) * sliceToMaster_KB * numIterations * workersSet;
                }

                currentInputFile = previousData_input_KB / (HPCAPP_MAX_SIZE_PER_INPUT_FILE / KB);
                currentOutputFile = previousData_output_KB / (HPCAPP_MAX_SIZE_PER_INPUT_FILE / KB);

                offsetRead = (previousData_input_KB % (HPCAPP_MAX_SIZE_PER_INPUT_FILE / KB)) * KB;
                offsetWrite = (previousData_output_KB % (HPCAPP_MAX_SIZE_PER_INPUT_FILE / KB)) * KB;
            }

            // Set file names
            sprintf (inputFileName, "%s_%d.dat", APP_HPC_INPUT_FILENAME, currentInputFile);
            sprintf (outFileName, "%s_%d.dat", APP_HPC_RESULTS_FILENAME, currentOutputFile);


            if (isMaster(myRank))
                printf ("---> Rank:%d - inputFile:%d - readOffset:%d - outputFile:%d - writeOffset:%d\n",
                        myRank,
                        currentInputFile,
                        offsetRead,
                        currentOutputFile,
                        offsetWrite);

            // Assign names to the results
            jobResults->newJobResultSet("moduleIdName");
            if (isMaster(myRank)) jobResults->newJobResultSet("Master");
            else jobResults->newJobResultSet("Slave_rank");
            jobResults->newJobResultSet("totalIO");
            jobResults->newJobResultSet("totalCPU");
            jobResults->newJobResultSet("Real run-time");
            jobResults->newJobResultSet("Simulation time");

            // Init state
            currentState = HPCAPP_INIT;

            totalIO = ioStart = ioEnd = 0.0;
            totalNET = netStart = netEnd = 0.0;
            totalCPU = cpuStart = cpuEnd = 0.0;


            if ((CHECK_MEMORY_HPC_APP) && (isMaster(myRank))){
                getRAM(true);
            }

            // Show info?
            showStartedModule (" WorkerSet:%d -  %s ", workersSet, mpiCommunicationsToString().c_str());

            printf ("PROCESS [%d] -> read:%d - write:%d - sliceWorkers:%d - sliceMaster:%d - sliceCPU:%d - allToAll:%d\n", myRank, workersRead, workersWrite, sliceToWorkers_KB, sliceToMaster_KB, sliceCPU, allToAllConnections);

}

void ApplicationHPC::processSelfMessage (cMessage *msg){

    // Call to super class
    if (!strcmp (msg->getName(), SM_SUPER.c_str())){

        MPI_Base::processSelfMessage(msg);

    }
    // Execute!!!
    else if (!strcmp (msg->getName(), SM_WAIT_TO_EXECUTE.c_str())){

            // Delete msg!
            cancelAndDelete (msg);

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

    else if (!strcmp (msg->getName(), SM_FINISH_JOB.c_str())){

        // Delete message
        cancelAndDelete (msg);

        // Show results
        showResults();
    }

    else
            showErrorMessage ("Unknown self message [%s]", msg->getName());


}

void ApplicationHPC::processResponseMessage (Packet *pkt){

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
            pkt->insertAtFront(sm_net);
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

            // Try to cast to icancloud_App_MEM_Message
            else {

                // Try to cast to icancloud_App_CPU_Message
                auto sm_mem = dynamicPtrCast<icancloud_App_MEM_Message>(sm);

                // NET call response...
                if (sm_mem != nullptr) {
                    pkt->insertAtFront(sm_mem);
                    processMEMCallResponse (pkt);
                }

                // Unknown message type
                else {
                    showErrorMessage("Unknown message type as response :%s",
                            sm->contentsToString(true).c_str());
                }
            }
        }
    }
}

void ApplicationHPC::calculateNextState (){

	// Master process?
	if (isMaster(myRank)){
		
		// Init state... read data!
		if (currentState == HPCAPP_INIT){
			
			if ((!createdResultsFile) && (!workersWrite)){
				
				totalResponses = 1;
				
				if (DEBUG_ApplicationHPC)
					showDebugMessage ("[It:%d] Process %d - State:%s: Creating results file %s", currentIteration, myRank, stateToString(currentState).c_str(), outFileName);
				
				createdResultsFile = true;
				icancloud_request_create (outFileName);
			}
			
			else{
				currentState = HPCAPP_MASTER_READING;
				totalResponses = workersSet;
				
				if ((!workersRead) && (DEBUG_ApplicationHPC))
				    showDebugMessage ("[It:%d] Process %d - State:%s: Reading data...", currentIteration, myRank, stateToString(currentState).c_str());

				ioStart = simTime();

				readInputData();		
			}				
		}
		
		// Read finished! Start delivering data among workers..
		else if (currentState == HPCAPP_MASTER_READING){
			
			// There are pending requests...
			if (totalResponses > 0){
				readInputData();
			}
			
			// All requests have been performed
			else{

			    if (DEBUG_ApplicationHPC)
			        showDebugMessage ("[It:%d] Process %d - State:%s: Read data completed!", currentIteration, myRank, stateToString(currentState).c_str());

				ioEnd = simTime();
				totalIO = totalIO + (ioEnd - ioStart);
				printf("ApplicationHPC::calculateNextState - total [%f] - start [%f] - end [%f]\n", totalIO.dbl(), ioStart.dbl(), ioEnd.dbl());
				currentState = HPCAPP_MASTER_DELIVERING;	
				
				if (DEBUG_ApplicationHPC)
					showDebugMessage ("[It:%d] Process %d - State:%s: Delivering data...", currentIteration, myRank, stateToString(currentState).c_str());

				netStart = simTime();

				deliverData();
			}
		}
		
		// Wait for results...
		else if (currentState == HPCAPP_MASTER_DELIVERING){
			
			currentState = HPCAPP_MASTER_WAITING_RESULTS;
			
			// Init responses
			totalResponses = workersSet - 1;
			
			if (DEBUG_ApplicationHPC)
				showDebugMessage ("[It:%d] Process %d - State:%s - Waiting for %d responses", currentIteration, myRank, stateToString(currentState).c_str(), totalResponses);
			
			// Receiving results...
			receivingResults();			
		}	
		
		// Waiting for results...
		else if (currentState == HPCAPP_MASTER_WAITING_RESULTS){
									
			// Init responses
			totalResponses--;
			
			if (DEBUG_ApplicationHPC)
				showDebugMessage ("[It:%d] Process %d - State:%s - Waiting for results. %d responses left!", currentIteration, myRank, stateToString(currentState).c_str(), totalResponses);
			
			// Check the number of responses received...
			if (totalResponses==0){

			    if (DEBUG_ApplicationHPC)
			        showDebugMessage ("[It:%d] Process %d - State:%s: Receiving responses completed!", currentIteration, myRank, stateToString(currentState).c_str());
				
				netEnd = simTime ();
				totalNET = totalNET + (netEnd - netStart);

				currentState = HPCAPP_MASTER_WRITTING;
				totalResponses = workersSet;
				
				if ((!workersWrite) && (DEBUG_ApplicationHPC))
					showDebugMessage ("[It:%d] Process %d - State:%s: Writing data...", currentIteration, myRank, stateToString(currentState).c_str());
						
				ioStart = simTime();

				writtingData();			
			}
			else
				receivingResults();					
		}		
		
		// Writting results...
		else if (currentState == HPCAPP_MASTER_WRITTING){
			
			// There are pending requests...
			if (totalResponses > 0){
				writtingData();
			}
			
			// All requests have been performed
			else{	

				ioEnd = simTime();
				totalIO = totalIO + (ioEnd - ioStart);
				printf("ApplicationHPC::calculateNextState - total [%f] - start [%f] - end [%f]\n", totalIO.dbl(), ioStart.dbl(), ioEnd.dbl());
				currentState = HPCAPP_INIT;		
				nextIteration();
			}			
		}			
	}	
			
	// Worker processes
	else{
		
		// Init state... read data!
		if (currentState == HPCAPP_INIT){
			
			if ((!createdResultsFile) && (workersWrite)){
				
				totalResponses = 1;
				
				if (DEBUG_ApplicationHPC)
				    showDebugMessage ("[It:%d] Process %d - State:%s: Creating results file %s", currentIteration, myRank, stateToString(currentState).c_str(), outFileName);

				createdResultsFile = true;
				icancloud_request_create (outFileName);
			}
			
			else{
				currentState = HPCAPP_WORKER_RECEIVING;
				
				if (DEBUG_ApplicationHPC)
					showDebugMessage ("[It:%d] Process %d - State:%s: Waiting data from coordinator process...", currentIteration, myRank, stateToString(currentState).c_str());
					

				netStart = simTime();
				receiveInputData();			
			}
		}
		
		// Worker read data
		else if (currentState == HPCAPP_WORKER_RECEIVING){
			
			currentState = HPCAPP_WORKER_READING;

			if (DEBUG_ApplicationHPC)
			    showDebugMessage ("[It:%d] Process %d - State:%s: Preparing for reading data...", currentIteration, myRank, stateToString(currentState).c_str());

			netEnd = simTime();
			totalNET = totalNET + (netEnd - netStart);

			ioStart = simTime();

			readInputDataWorkers();
		}

		// Worker starts allocating memory...
		else if (currentState == HPCAPP_WORKER_READING){

			currentState = HPCAPP_WORKER_ALLOCATING;

			if (DEBUG_ApplicationHPC)
			    showDebugMessage ("[It:%d] Process %d - State:%s: Allocating memory...", currentIteration, myRank, stateToString(currentState).c_str());

			ioEnd = simTime();
			totalIO = totalIO + (ioEnd - ioStart);
			printf("ApplicationHPC::calculateNextState - total [%f] - start [%f] - end [%f]\n", totalIO.dbl(), ioStart.dbl(), ioEnd.dbl());
			allocating();
		}

				
		// Worker starts to processing CPU
		else if (currentState == HPCAPP_WORKER_ALLOCATING){
			
			currentState = HPCAPP_WORKER_PROCESSING;
			
			if (DEBUG_ApplicationHPC)
				showDebugMessage ("[It:%d] Process %d - State:%s: Processing CPU...", currentIteration, myRank, stateToString(currentState).c_str());

			cpuStart = simTime();

			processingCPU();
		}	
		
		// Workers send data to master process
		else if (currentState == HPCAPP_WORKER_PROCESSING){
			
			currentState = HPCAPP_WORKER_WRITTING;

			if (DEBUG_ApplicationHPC)
			    showDebugMessage ("[It:%d] Process %d - State:%s: Writing data...", currentIteration, myRank, stateToString(currentState).c_str());

			cpuEnd = simTime();
			totalCPU = totalCPU + (cpuEnd - cpuStart);

			ioStart = simTime();
			writtingDataWorkers();
		}
		
		// Workers send data to master process
		else if (currentState == HPCAPP_WORKER_WRITTING){
			
			currentState = HPCAPP_WORKER_SENDING_BACK;

			if (DEBUG_ApplicationHPC)
			    showDebugMessage ("[It:%d] Process %d - State:%s: Sending results to coordinator...", currentIteration, myRank, stateToString(currentState).c_str());

			ioEnd = simTime();
			totalIO = totalIO + (ioEnd - ioStart);
			netStart = simTime();
			printf("ApplicationHPC::calculateNextState - total [%f] - start [%f] - end [%f]\n", totalIO.dbl(), ioStart.dbl(), ioEnd.dbl());
			sendResults();
		}	
		
		
		// Workers send data to master process
		else if (currentState == HPCAPP_WORKER_SENDING_BACK){

			netEnd = simTime();
			totalNET = totalNET + (netEnd  - netStart);

			if (DEBUG_ApplicationHPC)
			    showDebugMessage ("[It:%d] Process %d - State:%s: Iteration completed!", currentIteration, myRank, stateToString(currentState).c_str());

			currentState = HPCAPP_INIT;
			nextIteration();
		}			
	}
}

void ApplicationHPC::nextIteration (){
	
	printf ("***** [%s] Process %d. It:%d:   IO:%s   NET:%s   CPU:%s\n", simTime().str().c_str(), myRank,
					currentIteration,
					totalIO.str().c_str(),
					totalNET.str().c_str(),
					totalCPU.str().c_str());

	// Next iteration
	currentIteration++;

	if (isMaster (myRank))
		printf ("-----> Process %d - Iteration: %d/%d\n", myRank, currentIteration, numIterations);

	if ((CHECK_MEMORY_HPC_APP) && (isMaster(myRank)) && (currentIteration==2)){
				getRAM(false);
	}
			
	// All iterations complete?

	if (currentIteration > numIterations){

	    cMessage* msg;

	    msg = new cMessage(SM_FINISH_JOB.c_str());

	    scheduleAt(simTime() + 0.5, msg);

	}
	else		
		calculateNextState ();	
}

void ApplicationHPC::continueExecution (){
		
	calculateNextState ();
}

void ApplicationHPC::readInputData(){

	// Master process performs READ!
	if (!workersRead){
										
		if (DEBUG_ApplicationHPC)
			showDebugMessage ("Process %d - Reading file:%s - Offset:%u - Size:%u", myRank, inputFileName, offsetRead, sliceToWorkers_KB*KB);
			
			// Reading data for each worker process
			icancloud_request_read (inputFileName, offsetRead, sliceToWorkers_KB*KB);
			
			// Calculating next offset
			if ((offsetRead+(sliceToWorkers_KB*KB)) > HPCAPP_MAX_SIZE_PER_INPUT_FILE){
				currentInputFile++;
				sprintf (inputFileName, "%s_%d.dat", APP_HPC_INPUT_FILENAME, currentInputFile);
				offsetRead = 0;
			}
			else{
				offsetRead += sliceToWorkers_KB*KB;
			}
	}
	
	// Master process does not READ data
	else{
		totalResponses = 0;
		calculateNextState ();
	}
}

void ApplicationHPC::readInputDataWorkers(){
		
	// Master process performs READ!
	if (workersRead){
		
		if (DEBUG_ApplicationHPC)
			showDebugMessage ("[It:%d] Process %d - State:%s", currentIteration, myRank, stateToString(currentState).c_str());
		
		// Init
		totalResponses = 1;
			
		if (DEBUG_ApplicationHPC)
			showDebugMessage ("Process %d - Reading file:%s - Offset:%u - Size:%u", myRank, inputFileName, offsetRead, sliceToWorkers_KB*KB);
		
		// Reading data for each worker process
		icancloud_request_read (inputFileName, offsetRead, sliceToWorkers_KB*KB);
		
		// Calculating next offset
		if ((offsetRead+(sliceToWorkers_KB*KB)) > HPCAPP_MAX_SIZE_PER_INPUT_FILE){
			currentInputFile++;
			sprintf (inputFileName, "%s_%d.dat", APP_HPC_INPUT_FILENAME, currentInputFile);
			offsetRead = 0;
		}
		else
			offsetRead += sliceToWorkers_KB*KB;
	}
	
	// Master process does not READ data
	else{
		totalResponses = 0;
		calculateNextState ();
	}
}

void ApplicationHPC::allocating(){

	icancloud_request_allocMemory (1, 0);
}

void ApplicationHPC::deliverData(){
	
	unsigned int i;
	unsigned int dataSize;
			
		// Master Process READ!
		if (!workersRead)
			dataSize = sliceToWorkers_KB*KB;
		else
			dataSize = METADATA_MSG_SIZE;
			
		if (DEBUG_ApplicationHPC)
			showDebugMessage ("Process %d - Delivering data to worker processes (%d bytes)", myRank, dataSize);
	
		// Send data to each process
		for (i=(myRank+1); i<(myRank+workersSet); i++)
			mpi_send (i, dataSize);		
		
		// Wai for responses...
		calculateNextState();						
}

void ApplicationHPC::receivingResults(){
	
	// Master Process READ!
	if (!workersWrite)		
		mpi_recv (MPI_ANY_SENDER, sliceToMaster_KB*KB);
	else
		mpi_recv (MPI_ANY_SENDER, METADATA_MSG_SIZE);			
}

void ApplicationHPC::receiveInputData(){
	
	// Master Process READ!
	if (!workersRead)		
		mpi_recv (getMyMaster(myRank), sliceToWorkers_KB*KB);
	else
		mpi_recv (getMyMaster(myRank), METADATA_MSG_SIZE);
}

void ApplicationHPC::processingCPU(){
	
	if (DEBUG_ApplicationHPC)
		showDebugMessage ("Process %d - Processing...", myRank);	
	
	icancloud_request_cpu (sliceCPU);
}

void ApplicationHPC::writtingData(){
		
		// Master process performs WRITE!
		if (!workersWrite){
				
			if (DEBUG_ApplicationHPC)
				showDebugMessage ("Process %d - Writing file:%s - Offset:%u - Size:%u", myRank, outFileName, offsetRead, sliceToMaster_KB*KB);
			
			// Reading data for each worker process
			icancloud_request_write (outFileName, offsetWrite, sliceToMaster_KB*KB);
			
			// Calculating next offset
			if ((offsetWrite+(sliceToMaster_KB*KB)) > HPCAPP_MAX_SIZE_PER_INPUT_FILE){
				currentOutputFile++;
				sprintf (outFileName, "%s_%d.dat", APP_HPC_RESULTS_FILENAME, currentOutputFile);
				offsetWrite = 0;
			}
			else
				offsetWrite += sliceToMaster_KB*KB;
		}
		
		// Master process does not WRITE data
		else{
			totalResponses = 0;
			calculateNextState ();
		}
}

void ApplicationHPC::writtingDataWorkers(){
	
	// Master process performs WRITE!
	if (workersWrite){
		
		if (DEBUG_ApplicationHPC)
			showDebugMessage ("[It:%d] Process %d - State:%s", currentIteration, myRank, stateToString(currentState).c_str());

		// Init
		totalResponses = 1;		
			
		if (DEBUG_ApplicationHPC)
			showDebugMessage ("Process %d - Writing file:%s - Offset:%u - Size:%u", myRank, outFileName, offsetRead, sliceToMaster_KB*KB);
		
		// Reading data for each worker process
		icancloud_request_write (outFileName, offsetWrite, sliceToMaster_KB*KB);
		
		// Calculating next offset
		if ((offsetWrite+(sliceToMaster_KB*KB)) > HPCAPP_MAX_SIZE_PER_INPUT_FILE){
			currentOutputFile++;
			sprintf (outFileName, "%s_%d.dat", APP_HPC_RESULTS_FILENAME, currentOutputFile);
			offsetWrite = 0;
		}
		else
			offsetWrite += sliceToMaster_KB*KB;
	}
	
	// Master process does not WRITE data
	else{
		totalResponses = 0;
		calculateNextState ();
	}
}

void ApplicationHPC::sendResults(){
	
	unsigned int dataSize;
	
		// Master Process WRITE!
		if (!workersWrite)
			dataSize = sliceToMaster_KB*KB;
		else
			dataSize = METADATA_MSG_SIZE;
			
		if (DEBUG_ApplicationHPC)
			showDebugMessage ("Process %d - Sending results to MASTER (%d bytes)", myRank, dataSize);

		// Send data to each process			
		mpi_send (getMyMaster(myRank), dataSize);
	
	calculateNextState ();	
}

void ApplicationHPC::processNetCallResponse (Packet * pkt){

    int operation;
    const auto &responseMsg = pkt->peekAtFront<icancloud_App_NET_Message>();

    operation = responseMsg->getOperation ();

	// Create connection response...
	if (operation == SM_CREATE_CONNECTION){

		// Set the established connection.
		setEstablishedConnection (pkt);

		// delete (responseMsg);
	}
	else {
	    pkt->trimFront();
	    auto responseMsg = pkt->removeAtFront<icancloud_App_NET_Message>();
		showErrorMessage ("Unknown NET call response :%s", responseMsg->contentsToString(true).c_str());
	}
}

void ApplicationHPC::processIOCallResponse(Packet *pkt) {

    bool correctResult;
    std::ostringstream osStream;
    int operation;

    const auto & responseMsg = pkt->peekAtFront<icancloud_App_IO_Message>();

    // Init...
    correctResult = false;
    operation = responseMsg->getOperation();

    // Check the results...
    if (operation == SM_READ_FILE) {

        // Check state!
        if ((currentState != HPCAPP_MASTER_READING)
                && (currentState != HPCAPP_WORKER_READING))
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
    else if (operation == SM_WRITE_FILE) {

        // Check state!
        if ((currentState != HPCAPP_MASTER_WRITTING)
                && (currentState != HPCAPP_WORKER_WRITTING))
            throw new cRuntimeError(
                    "State error! Must be HPCAPP_MASTER_WRITTING\n");

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
    else if (operation == SM_CREATE_FILE) {

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

        // Error in response message...
        pkt->trimFront();
        auto responseMsg = pkt->removeAtFront<icancloud_App_IO_Message>();
        showErrorMessage("Error! Unknown IO operation:%s. %s",
                osStream.str().c_str(),
                responseMsg->contentsToString(DEBUG_MSG_ApplicationHPC).c_str());
    }

    // Error in response message?
    if (!correctResult) {
        pkt->trimFront();
        auto responseMsg = pkt->removeAtFront<icancloud_App_IO_Message>();
        showErrorMessage("Error in response message:%s. %s",
                osStream.str().c_str(),
                responseMsg->contentsToString(DEBUG_MSG_ApplicationHPC).c_str());

        throw new cRuntimeError("Error in response message\n");
    }

    // All OK!
    else {
        totalResponses--;

        if (DEBUG_ApplicationHPC)
            showDebugMessage("Process %d - Arrives I/O response. Left:%d",
                    myRank, totalResponses);

        // Error in synchronization phase?
        if (totalResponses < 0)
            throw new cRuntimeError("Synchronization error!\n");

        calculateNextState();
    }

    delete (pkt);
}

void ApplicationHPC::processCPUCallResponse (Packet * pkt){
	
	
	//delete (responseMsg);
	calculateNextState ();	
}

void ApplicationHPC::processMEMCallResponse (Packet *pkt){


	// delete (responseMsg);
	calculateNextState ();
}

string ApplicationHPC::stateToString(int state){
	
	string stringState;	
	
		if (state == HPCAPP_INIT)
			stringState = "HPCAPP_INIT";
			
		else if (state == HPCAPP_MASTER_READING)
			stringState = "HPCAPP_MASTER_READING";
			
		else if (state == HPCAPP_MASTER_DELIVERING)
			stringState = "HPCAPP_MASTER_DELIVERING";
			
		else if (state == HPCAPP_MASTER_WAITING_RESULTS)
			stringState = "HPCAPP_MASTER_WAITING_RESULTS";				

		else if (state == HPCAPP_MASTER_WRITTING)
			stringState = "HPCAPP_MASTER_WRITTING";				
			
		else if (state == HPCAPP_WORKER_RECEIVING)
			stringState = "HPCAPP_WORKER_RECEIVING";
			
		else if (state == HPCAPP_WORKER_READING)
			stringState = "HPCAPP_WORKER_READING";
			
		else if (state == HPCAPP_WORKER_PROCESSING)
			stringState = "HPCAPP_WORKER_PROCESSING";
			
		else if (state == HPCAPP_WORKER_WRITTING)
			stringState = "HPCAPP_WORKER_WRITTING";
			
		else if (state == HPCAPP_WORKER_SENDING_BACK)
			stringState = "HPCAPP_WORKER_SENDING_BACK";

		else if (state == HPCAPP_WORKER_ALLOCATING)
					stringState = "HPCAPP_WORKER_ALLOCATING";
							
		else stringState = "Unknown state";		
	
			
	return stringState;
}

void ApplicationHPC::showResults (){

    std::ostringstream osStream;

    // End time
    simEndTime = simTime();
    runEndTime = time(nullptr);

    if (isMaster(myRank)){
        printf ("App [%s] - Rank MASTER:%d - Simulation time:%f - Real execution time:%f - IO:%f  NET:%f  CPU:%f\n",
                                                                                                   moduleIdName.c_str(),
                                                                                                   myRank,
                                                                                                   (simEndTime-simStartTime).dbl(),
                                                                                                   (difftime (runEndTime,runStartTime)),
                                                                                                   totalIO.dbl(),
                                                                                                   totalNET.dbl(),
                                                                                                   totalCPU.dbl());
    }else{
        printf ("App [%s] - Rank:%d - Simulation time:%f - Real execution time:%f - IO:%f  NET:%f  CPU:%f\n",
                                                                                                   moduleIdName.c_str(),
                                                                                                   myRank,
                                                                                                   (simEndTime-simStartTime).dbl(),
                                                                                                   (difftime (runEndTime,runStartTime)),
                                                                                                   totalIO.dbl(),
                                                                                                   totalNET.dbl(),
                                                                                                   totalCPU.dbl());
    }

        osStream << moduleIdName.c_str();
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

        userPtr->notify_UserJobHasFinished(this);

}



} // namespace icancloud
} // namespace inet

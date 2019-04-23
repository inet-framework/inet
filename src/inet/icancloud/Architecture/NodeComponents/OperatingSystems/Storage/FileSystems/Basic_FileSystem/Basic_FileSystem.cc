#include "Basic_FileSystem.h"

namespace inet {

namespace icancloud {


Define_Module (Basic_FileSystem);

const const_simtime_t Basic_FileSystem::OPEN_TIME = 0.00011;
const const_simtime_t Basic_FileSystem::CLOSE_TIME = 0.000013;
const const_simtime_t Basic_FileSystem::CREATE_TIME = 0.000016;


Basic_FileSystem::~Basic_FileSystem(){

    if (pendingMessage)
        delete(pendingMessage);

}


void Basic_FileSystem::initialize(int stage) {

    icancloud_Base::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        std::ostringstream osStream;

        // Set the moduleIdName
        osStream << "Basic_FileSystem." << getId();
        moduleIdName = osStream.str();

        // Init gate IDs
        fromVMGate = gate("fromVM");
        toVMGate = gate("toVM");
        fromIORGate = gate("fromIOR");
        toIORGate = gate("toIOR");

        // Show file info?
        if (DEBUG_FS_Basic_Files)
            showStartedModule(" %s ", FSFilesToString().c_str());
    }
}


void Basic_FileSystem::finish(){
    // Finish the super-class
    icancloud_Base::finish();
}


cGate* Basic_FileSystem::getOutGate (cMessage *msg){

		// If msg arrive from Output
		if (msg->getArrivalGate()==fromVMGate){
			if (toVMGate->getNextGate()->isConnected()){
				return (toVMGate);
			}
		}

		// If msg arrive from Inputs
		else if (msg->getArrivalGate()==fromIORGate){
			if (toIORGate->getNextGate()->isConnected()){
				return (toIORGate);
			}
		}

	// If gate not found!
	return nullptr;
}


void Basic_FileSystem::processSelfMessage (cMessage *msg){

     //icancloud_Message *sm;
     auto pkt = check_and_cast<Packet *>(msg);
     auto sm = pkt->peekAtFront<icancloud_Message>();
     if (sm == nullptr)
             throw cRuntimeError("Header type error,  not of the type icancloud_Message");
    // Cast!
     cancelEvent(pkt);
     // Send message back!
     sendResponseMessage (pkt);
}


void Basic_FileSystem::processRequestMessage (Packet *pkt){
	processIORequest (pkt);
}


void Basic_FileSystem::processResponseMessage (Packet *pktSm){

	//icancloud_App_IO_Message *sm_io;
	//icancloud_BlockList_Message *sm_bl;
    pktSm->trimFront();
    auto sm = pktSm->removeAtFront<icancloud_Message>();
	auto sm_bl = dynamicPtrCast<icancloud_BlockList_Message>(sm);

	if (sm_bl == nullptr)
	    throw cRuntimeError("Header type error,  not of the type icancloud_BlockList_Message");
	auto sm_io = Ptr<icancloud_App_IO_Message>(static_cast<icancloud_App_IO_Message *>(sm_bl->transformToApp_IO_Message()));
	// sm_io = sm_bl->transformToApp_IO_Message();

	auto pktBl = new Packet("icancloud_App_IO_Message");
	pktSm->insertAtFront(sm_bl);
	delete (pktSm);

	pktBl->insertAtFront(sm_io);
	sendResponseMessage (pktBl);
}


void Basic_FileSystem::processIORequest (Packet *pktIo){

	string fileName;				// File Name
	int i;
	int operation;
	//icancloud_App_IO_Message *sm_io;			// IO Request message
	//icancloud_BlockList_Message *sm_bl;		// Block List message
	vector <icancloud_File>::iterator list_it;	// File list iterator
	pktIo->trimFront();
	auto sm = pktIo->removeAtFront<icancloud_Message>();
	// Cast!
	auto sm_io = dynamicPtrCast<icancloud_App_IO_Message>(sm);

	operation = sm_io->getOperation();


    if (operation != SM_CHANGE_DISK_STATE){
    	// Get file name!
    	fileName = sm_io->getFileName();    	
    	
    	if (DEBUG_Basic_FS)
    		showDebugMessage ("Processing IO Request. %s", sm_io->contentsToString(DEBUG_MSG_Basic_FS).c_str());
    		
    	// Show file info?
	    if (DEBUG_PARANOIC_Basic_FS){
	    	showDebugMessage ("------ Start FS contents ------");
	    	showStartedModule (" %s ", FSFilesToString().c_str());
	    	showDebugMessage ("------ End FS contents ------");
	    }    		

		// Open File
		if (operation == SM_OPEN_FILE){

//			pendingMessage = sm_io;

			// Answer message is sent back...
			sm_io->setIsResponse (true);

			// Search for the file
			list_it = searchFile (fileName);

			// Add the return value...
			if (list_it == fileList.end()){
				sm_io->setResult (icancloud_FILE_NOT_FOUND);	
				throw cRuntimeError("Error! File does not exists!!!");							
			}
			else
				sm_io->setResult (icancloud_OK);

//			latencyMessage = new cMessage (SM_LATENCY_MESSAGE.c_str());
			pktIo->insertAtFront(sm_io);
			scheduleAt (OPEN_TIME+simTime(), pktIo);
		}
		// Close File
		else if (operation == SM_CLOSE_FILE){

//			pendingMessage = sm_io;

			// Answer message is sent back...
			sm_io->setResult (icancloud_OK);
			sm_io->setIsResponse (true);

//			latencyMessage = new cMessage (SM_LATENCY_MESSAGE.c_str());
			pktIo->insertAtFront(sm_io);
			scheduleAt (CLOSE_TIME+simTime(), pktIo);
		}

		// Create File
		else if (operation == SM_CREATE_FILE){
		
//			pendingMessage = sm_io;

			// Answer message is sent back...
			sm_io->setIsResponse(true);

			// If fileName already exists, trunc it!
			list_it = searchFile (fileName);

			if (list_it != fileList.end())
				deleteFile (fileName);

			// Creates the file!
			if ((insertNewFile (sm_io->getSize()*KB, fileName)) == icancloud_OK)
				sm_io->setResult (icancloud_OK);
			else
				sm_io->setResult (icancloud_DISK_FULL);

			 // Show file info?
			if ((DEBUG_FS_Basic_Files) && (DEBUG_Basic_FS))
				showDebugMessage (" %s ", FSFilesToString().c_str());

//			latencyMessage = new cMessage (SM_LATENCY_MESSAGE.c_str());
			pktIo->insertAtFront(sm_io);
			scheduleAt (CREATE_TIME+simTime(), pktIo);
		}

		// Delete File
		else if (operation == SM_DELETE_FILE){
			
//			pendingMessage = sm_io;

			// Answer message is sent back...
			sm_io->setResult (icancloud_OK);
			sm_io->setIsResponse(true);

			// Removes the file!
			deleteFile (fileName);

			 // Show file info?
			if ((DEBUG_FS_Basic_Files) && (DEBUG_Basic_FS))
				showDebugMessage (" %s ", FSFilesToString().c_str());

//			latencyMessage = new cMessage (SM_LATENCY_MESSAGE.c_str());
			pktIo->insertAtFront(sm_io);
			scheduleAt (CLOSE_TIME+simTime(), pktIo);
		}

		// Read/Write File
		else if ((operation == SM_READ_FILE) ||
				 (operation == SM_WRITE_FILE)){

			// If numBytes == 0, IO request ends here, send back the message!
			if (sm_io->getSize() == 0){
				
				if (DEBUG_Basic_FS)
		    		showDebugMessage ("Returning request (numBytes = 0). %s", sm_io->contentsToString(DEBUG_MSG_Basic_FS).c_str());
				
				throw cRuntimeError("Error! Requesting 0 bytes!!!");
								
				sm_io->setResult (icancloud_OK);
				sm_io->setIsResponse (true);
				pktIo->insertAtFront(sm_io);
				sendResponseMessage (pktIo);
			}

			// At least there is a byte to make the request! Calculate the involved blocks...
			else{

				// Creates a new block list message
			    auto sm_bl = Ptr<icancloud_BlockList_Message>(static_cast<icancloud_BlockList_Message *>(new icancloud_BlockList_Message (sm_io.get())));

			    auto pktSmBl = new Packet("icancloud_BlockList_Message");

			    pktSmBl->insertAtBack(sm_bl);
				// Creates a a message that contains one branch
				translateIORequets (pktSmBl);

				const auto &sm_blAux = pktSmBl->peekAtFront<icancloud_BlockList_Message>();

				// Error?
				if (sm_blAux->getIsResponse()){
					
					//showErrorMessage ("Error in File System [%d]", sm_bl->getResult ());
					//throw cRuntimeError("Error! Translating Error!!!");
									
					sm_io->setIsResponse (true);
					sm_io->setResult (sm_blAux->getResult());
					pktIo->insertAtFront(sm_io);

					delete (pktSmBl);

					sendResponseMessage (pktIo);
				}
				else{
					// Erase the request message!
				    pktIo->insertAtFront(sm_io);
					delete (pktIo);
					sendRequestMessage (pktSmBl, toVMGate);
				}
			}
		}
		// Unknown IO operation
		else
			showErrorMessage ("Unknown operation [%d]", sm_io->getOperation());			
    }
    else {
        auto sm_bl = Ptr<icancloud_BlockList_Message>(static_cast<icancloud_BlockList_Message *>(new icancloud_BlockList_Message (sm_io.get())));
    	// sm_bl = new icancloud_BlockList_Message (sm_io);
    	sm_bl->setOperation (SM_CHANGE_DISK_STATE);
    	// Set the corresponding parameters
    	sm_bl->setChangingState(sm_io->getChangingState().c_str());

    	int changeStateSize = sm_io->get_component_to_change_size();
    	for (i = 0; i < changeStateSize; i++){
			sm_bl->add_component_index_To_change_state(sm_io->get_component_to_change(i));
		}
        auto pktSmBl = new Packet("icancloud_BlockList_Message");

        pktSmBl->insertAtFront(sm_bl);
        pktIo->insertAtFront(sm_io);
    	delete (pktIo);
    	sendRequestMessage (pktSmBl, toVMGate);
    }
}


int Basic_FileSystem::insertNewFile (unsigned int fileSize, string fileName){

    icancloud_File newFile;
		
		// Calculate info for new file
		newFile.setFileName (fileName);
		newFile.setFileSize (fileSize);

		// Insert new file
		fileList.push_back (newFile);

	return icancloud_OK;
}


vector <icancloud_File>::iterator Basic_FileSystem::searchFile (string fileName){

	bool fileLocated;							// Is the file in the File System
	vector <icancloud_File>::iterator list_it;		// List iterator

		// Init...
		fileLocated = false;
		list_it=fileList.begin();

		// Walk through the list searching the requested block!
		while (list_it!=fileList.end() && (!fileLocated)){

			if (!list_it->getFileName().compare(fileName))
				fileLocated = true;
			else
				list_it++;
		}

		return list_it;
}


void Basic_FileSystem::deleteFile (string fileName){

	vector <icancloud_File>::iterator list_it;


		// Search the file!
		list_it = searchFile (fileName);

		// File found! remove it!
		if (list_it != fileList.end())
			fileList.erase (list_it);

}


void Basic_FileSystem::translateIORequets (Packet *pkt){

	unsigned int numSectors;
	string fileName;
	unsigned int numBytes;
	vector <icancloud_File>::iterator list_it;	// List iterator
	pkt->trimFront();
	auto sm_fsTranslated = pkt->removeAtFront<icancloud_BlockList_Message>();
	// Init
	fileName = sm_fsTranslated->getFileName ();
	numBytes = sm_fsTranslated->getSize ();
	numSectors = 0;
	// Search for the requested file
	list_it = searchFile (fileName);
	if (DEBUG_Basic_FS) printf("%s\n",FSFilesToString().c_str());
	// If file not found!!!
	if (list_it == fileList.end()){
	    sm_fsTranslated->setIsResponse (true);
	    sm_fsTranslated->setResult (icancloud_FILE_NOT_FOUND);
	    throw cRuntimeError("Error! File not found!!!");
	}
	else{
	    numSectors = (unsigned int) ceil (numBytes/ BYTES_PER_SECTOR);
	    // Add the branch!
	    sm_fsTranslated->getFileForUpdate().addBranch (0, numSectors);
	    // Set the request size!
	    sm_fsTranslated->getFileForUpdate().setFileSize (sm_fsTranslated->getFile().getTotalSectors() * BYTES_PER_SECTOR);
	}
	if (DEBUG_DETAILED_Basic_FS)
	    showDebugMessage ("Translating request -> %s",
	            sm_fsTranslated->getFile().contentsToString(DEBUG_BRANCHES_Basic_FS, DEBUG_MSG_Basic_FS).c_str());
	pkt->insertAtFront(sm_fsTranslated);
}


string Basic_FileSystem::FSFilesToString (){

	vector <icancloud_File>::iterator list_it;		// List iterator
	std::ostringstream osStream;				// Stream
	unsigned int fileNumber;					// File number

		// Init...
		fileNumber = 0;
		osStream << "File System contents..." << endl;

		// Walk through the list searching the requested block!
		for (list_it=fileList.begin(); list_it!=fileList.end(); ++list_it){
			osStream << "file[" << fileNumber << "]: "<< list_it->contentsToString(DEBUG_BRANCHES_Basic_FS, DEBUG_MSG_Basic_FS);
			fileNumber++;
		}

	return osStream.str();
}



} // namespace icancloud
} // namespace inet

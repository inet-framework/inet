#include "NodeVirtualFileSystem.h"

namespace inet {

namespace icancloud {


Define_Module(NodeVirtualFileSystem);


NodeVirtualFileSystem::~NodeVirtualFileSystem(){
	
	userPathTable.clear();
}


void NodeVirtualFileSystem::initialize(int stage) {

    icancloud_Base::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        int i;
        std::ostringstream osStream;
        string ior_data;

        // Set the moduleIdName
        osStream << "NodeVirtualFileSystem." << getId();
        moduleIdName = osStream.str();

        // Get module parameters
        numFS = par("numFS");
        storageClientIndex = 0;

        // Get gate Ids
        fromMemoryGate = gate("fromMemory");
        toMemoryGate = gate("toMemory");

        // Init the gate IDs to/from Input gates...
        fromFSGates = new cGate*[numFS];
        toFSGates = new cGate*[numFS];

        for (i = 0; i < numFS; i++) {
            fromFSGates[i] = gate("fromFS", i);
            toFSGates[i] = gate("toFS", i);
        }
    }

}


void NodeVirtualFileSystem::finish(){

	// Finish the super-class
	icancloud_Base::finish();
}


cGate* NodeVirtualFileSystem::getOutGate (cMessage *msg){

	int i;

		// If msg arrive from cache
		if (msg->getArrivalGate()==fromMemoryGate){
			if (toMemoryGate->getNextGate()->isConnected()){
				return (toMemoryGate);
			}
		}

		// If msg arrive from FS's
		else if (msg->arrivedOn("fromFS")){
			for (i=0; i<numFS; i++)
				if (msg->arrivedOn ("fromFS", i))
					return (toFSGates[i]);
		}

	// If gate not found!
	return nullptr;
}


void NodeVirtualFileSystem::processSelfMessage (cMessage *msg){
	showErrorMessage ("Unknown self message [%s]", msg->getName());
}


void NodeVirtualFileSystem::processRequestMessage (Packet *pktSm){

	string fileName;
	bool found;
	string::size_type searchResult;

//	icancloud_App_IO_Message *sm_io;
	vector <pathEntry>::iterator vIterator;
	vector <uPathEntry>::iterator userIt;

	int userID;
	int index;
	int operation;
	string moduleType;

	string path;
	pathEntry newEntry;
	uPathEntry newUPath;


	const auto &sm = pktSm->peekAtFront<icancloud_Message>();
	// Cast!
	const auto &sm_io = CHK(dynamicPtrCast<const icancloud_App_IO_Message>(sm));

	// Init
	found = false;
	userID = sm_io->getUid();
	operation = sm_io->getOperation();
    // The message is to change the state of the disk
    if (operation == SM_CHANGE_DISK_STATE) {
        for (index = 0; index < numFS; index++) {
            sendRequestMessage(pktSm, toFSGates[index]);
        }
        // The message is to configure the HBS manager to nfs mode
    }
    else if (operation == SM_SET_HBS_TO_REMOTE) {

        sendRequestMessage(pktSm, toFSGates[0]);

    }
    else if (operation == SM_SET_IOR) {

        path = sm_io->getFileName();

        for (userIt = userPathTable.begin();
                ((userIt != userPathTable.end()) && (!found)); userIt++) {

            if (userID == (*(userIt)).userID) {

                // Exists the path?
                for (vIterator = (*(userIt)).pathTable.begin();
                        ((vIterator != (*(userIt)).pathTable.end()) && (!found));
                        userIt++) {

                    searchResult = path.find((*vIterator).path, 0);

                    if (searchResult == 0) {
                        found = true;
                    }
                }
            }
        }

        if (!found) {

            // set the path entry
            newEntry.type = LOCAL_FS_TYPE;
            newEntry.path = path.c_str();
            newEntry.index = 0;

            // set the user;
            newUPath.userID = userID;
            newUPath.pathTable.push_back(newEntry);

            userPathTable.push_back(newUPath);
        }
        pktSm->trimFront();
        auto sm_io = pktSm->removeAtFront<icancloud_App_IO_Message>();
        sm_io->setIsResponse(true);
        pktSm->insertAtFront(sm_io);
        sendResponseMessage(pktSm);

    } else if (operation == SM_DELETE_USER_FS) {

        // Removes the user ior!
        deleteUser(userID);

        // Send to the fs to remove the user files
        sendRequestMessage(pktSm, toFSGates[0]);
    } else {

        // Request came from FS. Remote operation! Send to memory...
        if (sm_io->getRemoteOperation()) {
            sendRequestMessage(pktSm, toMemoryGate);
        }

        // Request came from memory!
        else {

            // Get the file name
            fileName = sm_io->getFileName();

            // Search the path!

            for (userIt = userPathTable.begin();
                    ((userIt != userPathTable.end()) && (!found)); userIt++) {

                if (userID == (*(userIt)).userID) {

                    for (vIterator = (*(userIt)).pathTable.begin();
                            ((vIterator != (*(userIt)).pathTable.end())
                                    && (!found)); vIterator++) {

                        searchResult = fileName.find((*vIterator).path, 0);

                        if (searchResult == 0) {
                            found = true;
                            index = (*vIterator).index;
                            moduleType = (*vIterator).type;
                        }
                    }
                }
            }
            // Set the Module index, if found!
            if (found) {

                // Is a FS destination or a remote Operation? Set Remote operation field and send message to corresponding module
                if (moduleType == LOCAL_FS_TYPE) {
                    pktSm->trimFront();
                    auto sm_io = pktSm->removeAtFront<icancloud_App_IO_Message>();
                    if (DEBUG_IO_Rediretor)
                        showDebugMessage("Redirecting request to FS[%u] %s",
                                index,
                                sm_io->contentsToString(DEBUG_MSG_IO_Rediretor).c_str());
                    sm_io->setNextModuleIndex(index);
                    sm_io->setRemoteOperation(false);
                    pktSm->insertAtFront(sm_io);
                    sendRequestMessage(pktSm, toFSGates[index]);
//					}
//
//					else if (moduleType == REMOTE_FS_TYPE){
//
//						if (DEBUG_IO_Rediretor)
//							showDebugMessage ("Redirecting request to APP[%u] - ServerID:%d %s", storageClientIndex, index, sm_io->contentsToString(DEBUG_MSG_IO_Rediretor).c_str());
//
//						sm_io->setNextModuleIndex (storageClientIndex);
//						sm_io->setConnectionId(index);
//						sm_io->setRemoteOperation (true);
//						sendRequestMessage (sm_io, toMemoryGate);
                } else {
                    showErrorMessage(
                            "moduleType unknown in virtual filesystem[%s]",
                            moduleType.c_str());
                }
            }

            // Path not found! :(
            else {
                showErrorMessage("Path not found for file [%s]",
                        fileName.c_str());
            }
        }
    }
}


void NodeVirtualFileSystem::processResponseMessage (Packet *pkt){

	// Send back the message
	sendResponseMessage (pkt);
}

void NodeVirtualFileSystem::deleteUser (int user){

	vector <uPathEntry>::iterator userIt;
	bool found = false;

	for (userIt=userPathTable.begin();((userIt!=userPathTable.end()) && (!found)); userIt++){

			if (user == (*(userIt)).userID){
				userPathTable.erase(userIt);
				found = true;
			}

	}
}


string NodeVirtualFileSystem::fsTableToString (){

	std::ostringstream osStream;	
	vector <uPathEntry>::iterator userIt;
	int i;
	int size;
		osStream << "Virtual File System table..." << endl;

		for (userIt=userPathTable.begin();(userIt!=userPathTable.end()); userIt++){

			osStream << "User: " << (*(userIt)).userID << endl;

			size = (*(userIt)).pathTable.size();
			for (i=0; i<size; i++){

				osStream << "  Entry[" << i << "] Prefix-Path:" << (*(userIt)).pathTable[i].path.c_str() <<
							"  Index:" << (*(userIt)).pathTable[i].index <<
							"  Type:" <<  (*(userIt)).pathTable[i].type.c_str() << endl;
			}
		}

	return osStream.str();
}



} // namespace icancloud
} // namespace inet

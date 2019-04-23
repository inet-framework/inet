#include "VirtualFileSystem.h"

namespace inet {

namespace icancloud {


Define_Module(VirtualFileSystem);


VirtualFileSystem::~VirtualFileSystem(){
	
	pathTable.clear();
}


void VirtualFileSystem::initialize(int stage) {

    icancloud_Base::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        int i;
        std::ostringstream osStream;
        string ior_data;

        // Set the moduleIdName
        osStream << "VirtualFileSystem." << getId();
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


void VirtualFileSystem::finish(){

	// Finish the super-class
	icancloud_Base::finish();
}


cGate* VirtualFileSystem::getOutGate (cMessage *msg){

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


void VirtualFileSystem::processSelfMessage (cMessage *msg){
	showErrorMessage ("Unknown self message [%s]", msg->getName());
}


void VirtualFileSystem::processRequestMessage(Packet *pktSm) {

    string fileName;
    bool found;
    string::size_type searchResult;

    // icancloud_App_IO_Message *sm_io;
    vector<pathEntry>::iterator vIterator;
    int operation;

    unsigned int index;
    string moduleType;

    string path;
    pathEntry newEntry;

    const auto &sm = pktSm->peekAtFront<icancloud_Message>();
    // Cast!
    const auto &sm_io = CHK(dynamicPtrCast<const icancloud_App_IO_Message>(sm));

    // Init
    found = false;
    operation = sm_io->getOperation();

    if (operation == SM_SET_IOR) {

        path = sm_io->getFileName();

        // Exists the path?
        for (vIterator = pathTable.begin();
                ((vIterator != pathTable.end()) && (!found)); vIterator++) {

            searchResult = path.find((*vIterator).path, 0);

            if (searchResult == 0) {
                found = true;
            }
        }

        if (!found) {

            newEntry.type = LOCAL_FS_TYPE;
            newEntry.path = path.c_str();
            newEntry.index = 0;
            pathTable.push_back(newEntry);
        }
        pktSm->trimFront();
        auto sm_io = pktSm->removeAtFront<icancloud_App_IO_Message>();
        sm_io->setIsResponse(true);
        pktSm->insertAtFront(sm_io);
        sendResponseMessage(pktSm);

    }
    else if (operation == SM_DELETE_USER_FS) {
        pktSm->trimFront();
        auto sm_io = pktSm->removeAtFront<icancloud_App_IO_Message>();
        sm_io->setIsResponse(true);
        pktSm->insertAtFront(sm_io);
        sendResponseMessage(pktSm);
    }
    else {
        // Request came from FS. Remote operation! Send to memory...
        if (sm_io->getRemoteOperation()) {
            sendRequestMessage(pktSm, toMemoryGate);
        }

        // Request came from memory!
        else {

            // Get the file name
            fileName = sm_io->getFileName();

            // Search the path!
            for (vIterator = pathTable.begin();
                    ((vIterator != pathTable.end()) && (!found)); vIterator++) {

                searchResult = fileName.find((*vIterator).path, 0);

                if (searchResult == 0) {
                    found = true;
                    index = (*vIterator).index;
                    moduleType = (*vIterator).type;
                }
            }

            // Set the Module index, if found!
            if (found) {

                // Is a FS destination
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
                } else {
                    showErrorMessage(
                            "moduleType unknown in virtual filesystem[%s]",
                            moduleType.c_str());
                }
            }
            // Is a remote Operation? Set Remote operation field and send message to corresponding module
            else {
                if (DEBUG_IO_Rediretor)
                    showDebugMessage(
                            "Redirecting request to APP[%u] - ServerID:%d %s",
                            storageClientIndex, index,
                            sm_io->contentsToString(DEBUG_MSG_IO_Rediretor).c_str());
                pktSm->trimFront();
                auto sm_io = pktSm->removeAtFront<icancloud_App_IO_Message>();
                sm_io->setNextModuleIndex(storageClientIndex);
                sm_io->setConnectionId(index);
                sm_io->setRemoteOperation(true);
                pktSm->insertAtFront(sm_io);
                sendRequestMessage(pktSm, toMemoryGate);
            }

        }
    }
}


void VirtualFileSystem::processResponseMessage (Packet *pktSm){

	// Send back the message
	sendResponseMessage (pktSm);
}


string VirtualFileSystem::IORTableToString (){

	std::ostringstream osStream;	
	unsigned int i;

		osStream << "Virtual File System table..." << endl;

		for (i=0; i<pathTable.size(); i++){		

			osStream << "  Entry[" << i << "] Prefix-Path:" << pathTable[i].path.c_str() <<
						"  Index:" << pathTable[i].index <<
						"  Type:" <<  pathTable[i].type.c_str() << endl;
		}

	return osStream.str();
}



} // namespace icancloud
} // namespace inet

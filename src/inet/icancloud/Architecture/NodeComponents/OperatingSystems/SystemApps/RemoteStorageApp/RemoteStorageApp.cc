#include "inet/icancloud/Architecture/NodeComponents/OperatingSystems/SystemApps/RemoteStorageApp/RemoteStorageApp.h"

namespace inet {

namespace icancloud {


Define_Module (RemoteStorageApp);

RemoteStorageApp::~RemoteStorageApp(){
	icancloud_Base::finish();
}


void RemoteStorageApp::initialize(int stage) {

    // Init the super-class
    icancloud_Base::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {

        fromOSGate = gate("fromOS");
        toOSGate = gate("toOS");

        localStoragePort = -1;
        storageConnectionType.clear();
        sm_ior_creation.clear();
        sm_files_creation.clear();
    }
}


void RemoteStorageApp::finish(){

	// Finish the super-class
	icancloud_Base::finish();
}


void RemoteStorageApp::processSelfMessage (cMessage *msg){

}


void RemoteStorageApp::processRequestMessage (Packet *pktSm){
    pktSm->trimFront();
    auto sm = pktSm->removeAtFront<icancloud_Message>();
	sm->setNextModuleIndex(-1);
    pktSm->insertAtFront(sm);

	sendRequestMessage (pktSm, toOSGate);
}


void RemoteStorageApp::processResponseMessage(Packet *pktSm) {

    //icancloud_App_NET_Message *sm_net;
    //icancloud_App_IO_Message *sm_io;

    const auto &sm = pktSm->peekAtFront<icancloud_Message>();
    const auto &sm_net = dynamicPtrCast<const icancloud_App_NET_Message>(sm);
    const auto &sm_io = dynamicPtrCast<const icancloud_App_IO_Message>(sm);

    // Try to cast to icancloud_App_NET_Message
    //sm_net = dynamic_cast<icancloud_App_NET_Message *>(sm);

    // Try to cast to icancloud_App_IO_Message
    //sm_io = dynamic_cast<icancloud_App_IO_Message *>(sm);

    if (sm_net != nullptr) {

        processNetCallResponse(pktSm);

    }
    // Not a NET call... try cast to I/O call.
    else if (sm_io != nullptr) {

        // Cast to icancloud_App_NET_Message
        // sm_io = dynamic_cast<icancloud_App_IO_Message *>(sm);

        // IO call response...
        //if (sm_io != nullptr) {
//
//            processIOCallResponse (sm_io);
//
//        }
        processIOCallResponse (pktSm);
    }
    else
        showErrorMessage("Unknown response message: %s",
                sm->contentsToString(true).c_str());

}

cGate* RemoteStorageApp::getOutGate(cMessage *msg) {
    // If msg arrive from IOR
    if (msg->getArrivalGate() == fromOSGate) {
        if (toOSGate->getNextGate()->isConnected()) {
            return (toOSGate);
        }
    }

    // If gate not found!
    return nullptr;
}

void RemoteStorageApp::processNetCallResponse(Packet *pktSm) {

    const auto &sm = pktSm->peekAtFront<icancloud_Message>();
    const auto &sm_net = dynamicPtrCast<const icancloud_App_NET_Message>(sm);

    if (sm_net->getOperation() == SM_CLOSE_VM_CONNECTIONS) {
        pktSm->trimFront();
        auto sm_net = pktSm->removeAtFront<icancloud_App_NET_Message>();
        sm_net->setOperation(SM_NOTIFY_USER_CONNECTIONS_CLOSED);
        sm_net->setIsResponse(false);
        pktSm->insertAtFront(sm_net);
        sendRequestMessage(pktSm, toOSGate);
    }
    // Set the established connection.
    else
        setDataStorage_EstablishedConnection(pktSm);

}


void RemoteStorageApp::processIOCallResponse (Packet *pktIo){
	Enter_Method_Silent();
	//vector<preloadCreation*>::iterator it;
	//vector<icancloud_App_IO_Message*>::iterator sm_it;
	int operation;
	// icancloud_App_IO_Message* notify_msg;
	bool found;

	const auto & sm_io = pktIo->peekAtFront<icancloud_App_IO_Message>();

	operation = sm_io->getOperation();
	found = false;

	if (operation == SM_SET_IOR){

		// Search into the structure the vm_id "cell"
		for (auto it = sm_ior_creation.begin(); (it < sm_ior_creation.end()) && (!found); it++){

			if (((*it)->uId == sm_io->getUid()) && ((*it)->pId == sm_io->getPid())){

				// Decrement the number of messages received
				(*it)->numberOfIor--;

				// If it is the last, send all the messages to create files
				if ((*it)->numberOfIor == 0){

					// there is no files to preload..
					if ((*it)->sm_files.size() == 0){

						auto notify_msg = makeShared<icancloud_App_IO_Message>();
						notify_msg ->setOperation(SM_NOTIFY_PRELOAD_FINALIZATION);
						notify_msg ->setUid((*it)->uId);
						notify_msg ->setPid((*it)->pId);
						notify_msg ->setCommId((*it)->spId);
						auto pktNotify = new Packet("icancloud_App_IO_Message");
						pktNotify->insertAtFront(notify_msg);

						sendRequestMessage (pktNotify, toOSGate);

                        sm_ior_creation.erase(it);
					}

					// There is at least one file to preload ..
					else{
					    int size = (*it)->sm_files.size();
						for (int i = 0; i < size; i++){
							sendRequestMessage ((*((*it)->sm_files.begin() + i)), toOSGate);
						}

                        sm_files_creation.push_back((*it));
                        sm_ior_creation.erase(it);


					}
				}

				found = true;
			}
		}

		if (!found){
		    throw cRuntimeError ("Error at RemoteStorageApp::processIOCallResponse. The FS that the user:%i is creating does not exists\n",sm_io->getUid());
		}

		delete(pktIo);

	} else if (operation == SM_CREATE_FILE){

		// This is the preloaded files creation
	    for (auto it = sm_files_creation.begin(); (it < sm_files_creation.end()) && !found; it++){

	        if (((*it)->uId == sm_io->getUid()) && ((*it)->pId == sm_io->getPid())){

					for (auto sm_it = (*it)->sm_files.begin(); (sm_it < (*it)->sm_files.end()) && (!found); ){
					    const auto &smAux = (*sm_it)->peekAtFront<icancloud_App_IO_Message>();
						if (strcmp(smAux->getFileName(), sm_io->getFileName()) == 0){

							(*it)->sm_files.erase(sm_it);
							found = true;
						} else {
						    sm_it++;
						}
					}
					// there are no more messages to wait for notify user the start
					if ((*it)->sm_files.size() == 0){

					    //notify_msg = new icancloud_App_IO_Message();
					    auto notify_msg = makeShared<icancloud_App_IO_Message>();
						notify_msg -> setOperation(SM_NOTIFY_PRELOAD_FINALIZATION);
						notify_msg -> setUid((*it)->uId);
                        notify_msg -> setPid((*it)->pId);
                        notify_msg -> setCommId((*it)->spId);
                        auto pktNotify = new Packet("icancloud_App_IO_Message");
                        pktNotify->insertAtFront(notify_msg);

						sendRequestMessage (pktNotify, toOSGate);

						sm_files_creation.erase(it);
					}

					found = true;
			}
		}

	    // If the file is not found, the file should be a remote creation call.. So, return to app caller
        if (!found){

            sendResponseMessage(pktIo);


        }
        // The preload file has been created. The message has to be deleted.
        else
            delete(pktIo);

	} else if (operation == SM_DELETE_USER_FS){
		//notify_msg = new icancloud_App_IO_Message();
		auto notify_msg = makeShared<icancloud_App_IO_Message>();
		notify_msg -> setOperation(SM_NOTIFY_USER_FS_DELETED);
		notify_msg -> setUid(sm_io->getUid());
        notify_msg -> setPid(sm_io->getPid());
        auto pktNotify = new Packet("icancloud_App_IO_Message");
        pktNotify->insertAtFront(notify_msg);
		sendRequestMessage (pktNotify, toOSGate);
		delete(pktIo);
	} else {
	    pktIo->trimFront();
	    auto sm_io = pktIo->removeAtFront<icancloud_App_IO_Message>();
 		sm_io->setIsResponse(true);
		// Update message length
		sm_io->updateLength ();
		pktIo->insertAtFront(sm_io);
		sendResponseMessage(pktIo);
	}
}

// ----------------- --------------------------- ----------------- //
// ----------------- Operations with connections ----------------- //
// ----------------- --------------------------- ----------------- //


void RemoteStorageApp::closeConnection (int uId, int pId){

	Enter_Method_Silent();

	//icancloud_App_NET_Message *sm_close_connection;

	// Build the message for closing connection (node host)
	auto sm_close_connection =  makeShared<icancloud_App_NET_Message>();
	sm_close_connection -> setUid(uId);
	sm_close_connection -> setPid(pId);
	sm_close_connection-> setOperation(SM_CLOSE_VM_CONNECTIONS);
	auto pkt = new Packet("icancloud_App_NET_Message");
	pkt->insertAtFront(sm_close_connection);

	sendRequestMessage (pkt, toOSGate);
}

// ----------------- -------------- ----------------- //
// ----------------- Remote Storage ----------------- //
// ----------------- -------------- ----------------- //

void RemoteStorageApp::initialize_storage_data(int localPort){
	localStoragePort = localPort;
    storageConnectionType = "INET";
}


void RemoteStorageApp::create_listen (int uId, int pId){

	// Call to a context switching
	Enter_Method_Silent();

	// Define ..
	//icancloud_App_NET_Message *sm_net;					// Request message!
	int index;
	bool found = false;

	for (int i =0; (i < (int)lrep.size()) && (!found); i++){
	    if (
	            ((*(lrep.begin() + i))->uId == uId) &&
	            ((*(lrep.begin() + i))->pId == pId)
            ){
	        found = true;
	    }
	}

	if (!found){

	    // Insert the port in the structure..
            listen_repetition* lrep_;
            lrep_ = new listen_repetition();
            lrep_->uId = uId;
            lrep_->pId = pId;

            lrep.push_back(lrep_);

        // Creates the message
            //auto sm_net = new icancloud_App_NET_Message ();
            auto sm_net = makeShared<icancloud_App_NET_Message>();
            sm_net->setOperation (SM_LISTEN_CONNECTION);

            // Set parameters
            sm_net->setLocalPort (localStoragePort);
            sm_net->setConnectionType (storageConnectionType.c_str());
            index = getParentModule()->getSubmodule("remoteStorage")->getIndex();
            sm_net->setNextModuleIndex (index);
            sm_net->setUid(uId);
            sm_net->setPid(pId);

            auto pktSmNet = new Packet("icancloud_App_NET_Message");
            pktSmNet->insertAtFront(sm_net);

            if (DEBUG_RemoteStorage)
                showDebugMessage ("[AppSystemRequests] Listen connection %s", sm_net->contentsToString(DEBUG_RemoteStorage).c_str());

        // Send the request message to Network Service module
            sendRequestMessage (pktSmNet, toOSGate);
	}
}


void RemoteStorageApp::connect_to_storage_node (vector<string> destinationIPs, string fsType, int destPort, int uid, int pid, string vIP, int jobId){

	// Call to a context switching
		Enter_Method_Silent();
	// Request message!
		//icancloud_App_NET_Message *sm_net;
	    //vector<preloadCreation*>::iterator it;
	    bool found = false;
		int i;

		i = destinationIPs.size();

		while(i != 0){

			// Creates the message
			//sm_net = new icancloud_App_NET_Message ();
		    auto sm_net = makeShared<icancloud_App_NET_Message> ();
			sm_net->setOperation (SM_VM_REQUEST_CONNECTION_TO_STORAGE);

			// Set parameters
			sm_net->setDestinationIP ((*(destinationIPs.end() - i)).c_str());
			sm_net->setDestinationPort (localStoragePort);
			sm_net->setLocalIP (vIP.c_str());
			sm_net->setLocalPort (0);
			sm_net->setConnectionType ("INET");

			sm_net->setCommId(jobId);
			sm_net->setUid(uid);
			sm_net->setPid(pid);
			sm_net->setFsType(fsType.c_str());
			sm_net->setTargetPosition(destinationIPs.size());
			// Add the connection to the connection vector
			auto pktSmNet = new Packet("icancloud_App_NET_Message");
			pktSmNet->insertAtFront(sm_net);

			if (DEBUG_AppSystem)
				showDebugMessage ("[AppSystemRequests] Creating connection %s", sm_net->contentsToString(DEBUG_RemoteStorage).c_str());


			// Send the request message to Network Service module
			sendRequestMessage (pktSmNet, toOSGate);
			i--;
		}

	    for (auto it = sm_files_creation.begin(); (it < sm_files_creation.end()) && !found; it++){

           if (((*it)->uId == uid) && ((*it)->pId == pid)){

               //icancloud_App_IO_Message* msg_io;
               auto msg_io = makeShared<icancloud_App_IO_Message>();
               msg_io->setUid(uid);
               msg_io->setPid(pid);
               msg_io->setCommId(jobId);
               msg_io->setFileName("connection");
               auto pktSmIo = new Packet("icancloud_App_IO_Message");
               pktSmIo->insertAtFront(msg_io);
               (*it)->sm_files.push_back(pktSmIo);
               found = true;

           }
	    }

	    if (!found){

	        preloadCreation* preloadCell;

	        preloadCell = new preloadCreation();
	        preloadCell->uId = uid;
	        preloadCell->pId = pid;
	        preloadCell->spId = jobId;
	        preloadCell->numberOfIor = 0;

            //icancloud_App_IO_Message* msg_io;
            //msg_io = new icancloud_App_IO_Message();
	        auto msg_io = makeShared<icancloud_App_IO_Message>();
            msg_io->setUid(uid);
            msg_io->setPid(pid);
            msg_io->setCommId(jobId);
            msg_io->setFileName("connection");
            auto pktSmIo = new Packet("icancloud_App_IO_Message");
            pktSmIo->insertAtFront(msg_io);

            preloadCell->sm_files.clear();
            preloadCell->sm_files.push_back(pktSmIo);
            sm_ior_creation.push_back(preloadCell);

        }

}


void RemoteStorageApp::createFilesToPreload(int uid, int pid, int spId, string vmIP, vector<preload_T*> filesToPreload,  vector<fsStructure_T*> fsPaths, bool remoteHost) {
    Enter_Method_Silent();
    // Define ..
    //icancloud_App_IO_Message *sm_io;
    string type;
    string path;
    string index;
    int numberOfIOR = fsPaths.size();
    preloadCreation* preloadCell;
    std::ostringstream info;

    while (fsPaths.size() != 0) {

        // Initialize ..
        // sm_io = new icancloud_App_IO_Message();
        auto sm_io = makeShared<icancloud_App_IO_Message>();

        // Set the operation and the user
        sm_io->setOperation(SM_SET_IOR);
        sm_io->setUid(uid);
        sm_io->setPid(pid);
        sm_io->setCommId(spId);

        // Set the corresponding parameters
        sm_io->setFileName((*(fsPaths.begin()))->fsRoute.c_str());
        sm_io->updateLength ();

        // Set the ip to initialize the vm into the local net manager (H_NET_MANAGER)
        sm_io->setNfs_destAddress(vmIP.c_str());

        auto pktSmIo = new Packet("icancloud_App_IO_Message");
        pktSmIo->insertAtFront(sm_io);


        // Send the message
        sendRequestMessage (pktSmIo, toOSGate);

        // delete the ior processed
        fsPaths.erase(fsPaths.begin());
    }

    preloadCell = new preloadCreation();
    preloadCell->uId = uid;
    preloadCell->pId = pid;
    preloadCell->spId = spId;
    preloadCell->numberOfIor = numberOfIOR;
    preloadCell->sm_files.clear();

    while (filesToPreload.size() != 0) {

        // Initialize ..

        //sm_io = new icancloud_App_IO_Message();

        auto sm_io = makeShared<icancloud_App_IO_Message>();
        sm_io -> setOperation(SM_CREATE_FILE);
        sm_io -> setUid(uid);
        sm_io -> setPid(pid);

        // Set the corresponding parameters
        sm_io->setFileName((*(filesToPreload.begin()))->fileName.c_str());
        sm_io->setOffset (0);
        sm_io->setSize ((*(filesToPreload.begin()))->fileSize);
        sm_io->updateLength();

        // To distinguish between if files will be loaded at remote or local host
        if (remoteHost) {
            sm_io->setNfs_connectionID(0);
        } else {
            sm_io->setNfs_connectionID(-1);
        }

        auto pktSmIo = new Packet("icancloud_App_IO_Message");
        pktSmIo->insertAtFront(sm_io);

        preloadCell->sm_files.push_back(pktSmIo);

        filesToPreload.erase(filesToPreload.begin());
    }

    sm_ior_creation.push_back(preloadCell);

}

void RemoteStorageApp::deleteUserFSFiles(int uid, int pid) {
    // Call to a context switching
    Enter_Method_Silent();

    // Request message!
    //icancloud_App_IO_Message *sm_io;

    auto sm_io = makeShared<icancloud_App_IO_Message>();

    sm_io->setOperation (SM_DELETE_USER_FS);
    sm_io->setNfs_connectionID(0);
    sm_io->setUid(uid);
    sm_io->setPid(pid);
    auto pktSmIo = new Packet("icancloud_App_IO_Message");
    pktSmIo->insertAtFront(sm_io);

    sendRequestMessage (pktSmIo, toOSGate);
}


void RemoteStorageApp::setDataStorage_EstablishedConnection(Packet *pktSmNet) {

    // Request message!
    //vector<preloadCreation*>::iterator it;
    bool found = false;
    //vector<icancloud_App_IO_Message*>::iterator sm_it;
    //icancloud_App_IO_Message* notify_msg;

    const auto &sm = pktSmNet->peekAtFront<icancloud_Message>();
    const auto &sm_net = CHK(dynamicPtrCast<const icancloud_App_NET_Message>(sm));

    // This is the preloaded files creation
    for (auto it = sm_ior_creation.begin(); (it < sm_ior_creation.end()) && !found; it++) {
        if (((*it)->uId == sm_net->getUid())
                && ((*it)->pId == sm_net->getPid())) {

            for (auto sm_it = (*it)->sm_files.begin();
                    (sm_it < (*it)->sm_files.end()) && (!found);) {
                const auto &smAux = (*sm_it)->peekAtFront<icancloud_App_IO_Message>();
                if (strcmp(smAux->getFileName(), "connection") == 0) {

                    (*it)->sm_files.erase(sm_it);
                    found = true;
                } else {
                    sm_it++;
                }
            }

            if (!found)
                throw cRuntimeError(
                        "RemoteStorageApp::setDataStorage_EstablishedConnection->error connection not found at sm_files..\n");

            // there are no more messages to wait for notify user the start
            if ((*it)->sm_files.size() == 0) {
                auto notify_msg = makeShared<icancloud_App_IO_Message>();
                notify_msg->setOperation(SM_NOTIFY_PRELOAD_FINALIZATION);
                notify_msg->setUid((*it)->uId);
                notify_msg->setPid((*it)->pId);
                notify_msg->setCommId((*it)->spId);
                auto pktNotify = new Packet("icancloud_App_IO_Message");
                pktNotify->insertAtFront(notify_msg);
                sendRequestMessage(pktNotify, toOSGate);
                sm_ior_creation.erase(it);
            }
        }
    }

    delete (pktSmNet);
}



} // namespace icancloud
} // namespace inet

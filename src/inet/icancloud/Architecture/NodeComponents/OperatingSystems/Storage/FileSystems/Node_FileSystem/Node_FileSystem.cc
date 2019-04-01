#include "Node_FileSystem.h"

namespace inet {

namespace icancloud {


Define_Module (Node_FileSystem);

const const_simtime_t Node_FileSystem::OPEN_TIME = 0.00011;
const const_simtime_t Node_FileSystem::CLOSE_TIME = 0.000013;
const const_simtime_t Node_FileSystem::CREATE_TIME = 0.000016;


Node_FileSystem::~Node_FileSystem(){

}


void Node_FileSystem::initialize(int stage) {
    icancloud_Base::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        std::ostringstream osStream;
        string preload_data;

        // Set the moduleIdName
        osStream << "Basic_FileSystem." << getId();
        moduleIdName = osStream.str();

        // Init gate IDs
        fromVMGate = gate("fromVM");
        toVMGate = gate("toVM");
        fromIORGate = gate("fromIOR");
        toIORGate = gate("toIOR");

        pendingMessages.clear();

        usersFileList.clear();
        msg_delete_user_id.clear();

        // Show file info?
        if (DEBUG_FS_Basic_Files)
            showStartedModule(" %s ", FSFilesToString().c_str());
    }
}


void Node_FileSystem::finish(){


    // Finish the super-class
    icancloud_Base::finish();
}


cGate* Node_FileSystem::getOutGate (cMessage *msg){

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


void Node_FileSystem::processSelfMessage (cMessage *msg){

    //icancloud_Message *sm;
    cMessage* extract_sm;
        // Operation performed in FS...
        if (!strcmp (msg->getName(), SM_LATENCY_MESSAGE.c_str())){

            // Init pending message...
            extract_sm = extractPendingMessage(msg);

            // Cast!
            auto pkt = check_and_cast<Packet *>(extract_sm);
            const auto &sm = CHK(pkt->peekAtFront<icancloud_Message>());
            if (sm == nullptr)
                throw cRuntimeError("Header not present");
            // Send message back!
            sendResponseMessage (pkt);

            cancelAndDelete(msg);

        }

        else
            showErrorMessage ("Unknown self message [%s]", msg->getName());
}


void Node_FileSystem::processRequestMessage (Packet *pkt){
    processIORequest (pkt);
}


void Node_FileSystem::processResponseMessage (Packet *pkt){

    //icancloud_App_IO_Message *sm_io;
    //icancloud_BlockList_Message *sm_bl;
    pkt->trimFront();
    auto sm = pkt->removeAtFront<icancloud_Message>();
    auto sm_bl = dynamicPtrCast<icancloud_BlockList_Message>(sm);

    CHK(sm_bl);

    bool ok = checkResponseMsg(pkt);
    if (!ok) {

        auto sm_io = Ptr<icancloud_App_IO_Message>(static_cast<icancloud_App_IO_Message *>(sm_bl->transformToApp_IO_Message()));
        //auto sm_io = sm_bl->transformToApp_IO_Message();
        auto pktIo = new Packet("icancloud_App_IO_Message");
        pktIo->insertAtFront(sm_io);
        sendResponseMessage (pktIo);
    }
    pkt->insertAtFront(sm_bl);
    delete (pkt);
}


void Node_FileSystem::processIORequest (Packet *pktIo){
    string fileName;                // File Name
    int uId;                        // User Id
    int i;
    userFileList* userFiles;
    bool existsUser = false;
    bool modifyStorage;
    int old_file_size = 0;
    int new_file_size = 0;
    //icancloud_App_IO_Message *sm_io;            // IO Request message
    //icancloud_BlockList_Message *sm_bl;         // Block List message
    vector <icancloud_File>::iterator list_it;  // File list iterator
    int operation;

        // Cast!
    const auto& sm = pktIo->peekAtFront<icancloud_Message>();
    const auto& sm_io = CHK(dynamicPtrCast<const icancloud_App_IO_Message>(sm));

    //Get user name!
    uId = sm_io->getUid();

    operation = sm_io->getOperation();

    if (operation != SM_CHANGE_DISK_STATE) {

        // Search the user
        // Get file name!
        fileName = sm_io->getFileName();

        // Get the user files

        vector<userFileList*>::iterator user_it;       // User list iterator

        // Init...
        user_it = usersFileList.begin();

        while (user_it != usersFileList.end() && !(existsUser)) {

            if ((*user_it)->userID == uId) {

                existsUser = true;
                userFiles = (*(user_it));

            } else
                user_it++;
        }

        if (!existsUser) {
            userFiles = new userFileList();
            userFiles->userID = uId;
            userFiles->fileList.clear();

            usersFileList.push_back(userFiles);
        }

        if (DEBUG_Basic_FS)
            showDebugMessage("Processing IO Request. %s",
                    sm_io->contentsToString(DEBUG_MSG_Basic_FS).c_str());

        // Show file info?
        if (DEBUG_PARANOIC_Basic_FS) {
            showDebugMessage("------ Start FS contents ------");
            showStartedModule(" %s ", FSFilesToString().c_str());
            showDebugMessage("------ End FS contents ------");
        }

        // Open File
        if (operation == SM_OPEN_FILE) {
            pktIo->trimFront();
            auto sm_io = pktIo->removeAtFront<icancloud_App_IO_Message>();

            // Answer message is sent back...
            sm_io->setIsResponse(true);

            // Search for the file
            list_it = searchFile(fileName, userFiles);

            // Add the return value...
            if (list_it == userFiles->fileList.end()) {
                sm_io->setResult(icancloud_FILE_NOT_FOUND);
                throw cRuntimeError("Error! File does not exists!!!");
            } else
                sm_io->setResult(icancloud_OK);

            pktIo->insertAtFront(sm_io);

            auto latencyMessage = new cMessage(SM_LATENCY_MESSAGE.c_str());
            scheduleAt(OPEN_TIME + simTime(), latencyMessage);

            pushPendingMessage(pktIo, latencyMessage);
        }

        // Close File
        else if (operation == SM_CLOSE_FILE) {

            // Answer message is sent back...
            pktIo->trimFront();
            auto sm_io = pktIo->removeAtFront<icancloud_App_IO_Message>();
            sm_io->setResult(icancloud_OK);
            sm_io->setIsResponse(true);
            pktIo->insertAtFront(sm_io);

            auto latencyMessage = new cMessage(SM_LATENCY_MESSAGE.c_str());
            scheduleAt(CLOSE_TIME + simTime(), latencyMessage);

            pushPendingMessage(pktIo, latencyMessage);
        }

        // Create File
        else if (operation == SM_CREATE_FILE) {

            // Initialize the blocklist message
            pktIo->trimFront();
            auto sm_io = pktIo->removeAtFront<icancloud_App_IO_Message>();

            //sm_bl = new icancloud_BlockList_Message(sm_io);
            auto sm_bl = Ptr<icancloud_BlockList_Message>(static_cast<icancloud_BlockList_Message *>(new icancloud_BlockList_Message (sm_io.get())));
            modifyStorage = true;

            // Answer message is sent back...
            sm_io->setIsResponse(true);

            // If fileName already exists, trunc it!
            list_it = searchFile(fileName, userFiles);

            if (list_it != userFiles->fileList.end()) {

                old_file_size = list_it->getFileSize();
                new_file_size = sm_io->getSize();

                // If new file is smaller than older it is needed to free space from the disk
                if (old_file_size > new_file_size) {

                    sm_bl->setOperation(SM_DELETE_FILE);
                    sm_bl->setSize(old_file_size - new_file_size);

                } else if (old_file_size > new_file_size) {

                    sm_bl->setSize(new_file_size - old_file_size);

                } else {
                    modifyStorage = false;
                }

                deleteFile(fileName, userFiles);
            }

            // Creates the file!
            if ((insertNewFile(sm_io->getSize(), fileName, userFiles)) == icancloud_OK)
                sm_io->setResult(icancloud_OK);
            else
                sm_io->setResult(icancloud_DISK_FULL);

            // Show file info?
            if ((DEBUG_FS_Basic_Files) && (DEBUG_Basic_FS))
                showDebugMessage(" %s ", FSFilesToString().c_str());

            auto pktSmBl = new Packet("icancloud_BlockList_Message");
            pktSmBl->insertAtFront(sm_bl);

            if ((sm_io->getNfs_connectionID() != -1) && (modifyStorage)) {

                // send the message to modify the capacity of the disk
//                    sendRequestMessage (sm_bl, toVMGate);
//            } else {
                //delete (sm_bl);
                delete(pktSmBl);
            }

            pktIo->insertAtFront(sm_io);

            latencyMessage = new cMessage(SM_LATENCY_MESSAGE.c_str());
            scheduleAt(CREATE_TIME + simTime(), latencyMessage);

            pushPendingMessage(pktIo, latencyMessage);

        }

        // Delete File
        else if (operation == SM_DELETE_FILE) {

            // Initialize the blocklist message
            //sm_bl = new icancloud_BlockList_Message(sm_io);
            pktIo->trimFront();
            auto sm_io = pktIo->removeAtFront<icancloud_App_IO_Message>();
            auto sm_bl = Ptr<icancloud_BlockList_Message>(static_cast<icancloud_BlockList_Message *>(new icancloud_BlockList_Message (sm_io.get())));

            // Answer message is sent back...
            sm_io->setResult(icancloud_OK);
            sm_io->setIsResponse(true);

            // Removes the file!
            deleteFile(fileName, userFiles);

            pktIo->insertAtFront(sm_io);

            auto pktSmBl = new Packet("icancloud_BlockList_Message");
            pktSmBl->insertAtFront(sm_bl);

            // send the message to modify the capacity of the disk
            if (sm_io->getNfs_connectionID() != -1) {
                sendRequestMessage(pktSmBl, toVMGate);
            }
            else
                delete pktSmBl;

            // Show file info?
            if ((DEBUG_FS_Basic_Files) && (DEBUG_Basic_FS))
                showDebugMessage(" %s ", FSFilesToString().c_str());

            latencyMessage = new cMessage(SM_LATENCY_MESSAGE.c_str());
            scheduleAt(CLOSE_TIME + simTime(), latencyMessage);

            pushPendingMessage(pktIo, latencyMessage);
        }

        // Delete User
        else if (operation == SM_DELETE_USER_FS) {

            // Removes the user's files!
            deleteUser(pktIo);
            // Answer message is sent back...
            pktIo->trimFront();
            auto sm_io = pktIo->removeAtFront<icancloud_App_IO_Message>();
            sm_io->setResult(icancloud_OK);
            sm_io->setIsResponse(true);
            pktIo->insertAtFront(sm_io);
            sendResponseMessage(pktIo);

        }

        // Read/Write File
        else if ((operation == SM_READ_FILE) || (operation == SM_WRITE_FILE)) {

            // If numBytes == 0, IO request ends here, send back the message!
            if (sm_io->getSize() == 0) {

                if (DEBUG_Basic_FS)
                    showDebugMessage("Returning request (numBytes = 0). %s",
                            sm_io->contentsToString(DEBUG_MSG_Basic_FS).c_str());

                throw cRuntimeError("Error! Requesting 0 bytes!!!");
                pktIo->trimFront();
                auto sm_io = pktIo->removeAtFront<icancloud_App_IO_Message>();
                sm_io->setResult(icancloud_OK);
                sm_io->setIsResponse(true);
                pktIo->insertAtFront(sm_io);
                sendResponseMessage(pktIo);
            }

            // At least there is a byte to make the request! Calculate the involved blocks...
            else {


                pktIo->trimFront();
                auto sm_io = pktIo->removeAtFront<icancloud_App_IO_Message>();
                // Creates a new block list message
                auto sm_bl = Ptr<icancloud_BlockList_Message>(static_cast<icancloud_BlockList_Message *>(new icancloud_BlockList_Message (sm_io.get())));
                // auto sm_bl = new icancloud_BlockList_Message(sm_io);

                auto pktSmBl = new Packet("icancloud_BlockList_Message");
                pktSmBl->insertAtFront(sm_bl);

                // Creates a a message that contains one branch
                translateIORequests(pktSmBl, userFiles);

                auto sm_blAux = pktSmBl->peekAtFront<icancloud_BlockList_Message>();

                // Error?
                if (sm_blAux->getIsResponse()) {

                    //showErrorMessage ("Error in File System [%d]", sm_bl->getResult ());
                    //throw cRuntimeError("Error! Translating Error!!!");

                    sm_io->setIsResponse(true);
                    sm_io->setResult(sm_bl->getResult());
                    pktIo->insertAtFront(sm_io);
                    delete (pktSmBl);
                    sendResponseMessage(pktIo);
                } else {
                    // Erase the request message!
                    pktIo->insertAtFront(sm_io);
                    delete (pktIo);
                    sendRequestMessage(pktSmBl, toVMGate);
                }
            }
        }
        // Unknown IO operation
        else
            showErrorMessage("Unknown operation [%d]", sm_io->getOperation());
    } else {
        pktIo->trimFront();
        auto sm_io = pktIo->removeAtFront<icancloud_App_IO_Message>();
        auto sm_bl = Ptr<icancloud_BlockList_Message>(static_cast<icancloud_BlockList_Message *>(new icancloud_BlockList_Message (sm_io.get())));

        // sm_bl = new icancloud_BlockList_Message(sm_io);
        sm_bl->setOperation(SM_CHANGE_DISK_STATE);

        // Set the corresponding parameters
        sm_bl->setChangingState(sm_io->getChangingState().c_str());

        int changeStateSize = sm_io->get_component_to_change_size();
        for (i = 0; i < changeStateSize; i++) {
            sm_bl->add_component_index_To_change_state(
                    sm_io->get_component_to_change(i));

        }
        pktIo->insertAtFront(sm_io);

        auto pktSmBl = new Packet("icancloud_BlockList_Message");
        pktSmBl->insertAtFront(sm_bl);
        delete (pktIo);
        sendRequestMessage(pktSmBl, toVMGate);
    }
}


int Node_FileSystem::load_UserPreloadFiles (string preload_data, int userID){

    int i, result;
    bool existsUser;

    CfgPreloadFS *preloadCfg;
    userFileList* user;
        // Init...
        result = icancloud_OK;
        preloadCfg = new CfgPreloadFS();

        if (!preload_data.empty() && (userID != -1)){

            existsUser = searchUser(userID);

            if (!existsUser){
                user = new userFileList;
                user->userID = userID;
                user->fileList.clear();
            }

            preloadCfg->parseFile(preload_data.c_str());

            // No servers file!
            if (preloadCfg->getNumFiles() == 0)
                result = icancloud_ERROR;

            else{

                // Store info in current class
                for (i=0; i<preloadCfg->getNumFiles(); i++){
                    insertNewFile (preloadCfg->getSizeKB(i)*KB, preloadCfg->getFileName(i).c_str(), user);
                }
            }
        }
        else
            result = icancloud_ERROR;


    return result;

}


int Node_FileSystem::insertNewFile (unsigned int fileSize, string fileName, userFileList* user){

    icancloud_File newFile;

        // Calculate info for new file
        newFile.setFileName (fileName);
        newFile.setFileSize (fileSize);

        // Insert new file
        user->fileList.push_back (newFile);

    return icancloud_OK;
}


bool Node_FileSystem::searchUser (int userName){

    bool userFind;                                  // Is in the users file list
    vector <userFileList*>::iterator user_it;       // User list iterator

        // Init...
        userFind = false;
        user_it = usersFileList.begin();

        while (user_it!=usersFileList.end() && !(userFind)){

            if ((*user_it)->userID == userName){

                userFind = true;

            }
            else
                user_it++;
        }

        return userFind;
}

vector <icancloud_File>::iterator Node_FileSystem::searchFile (string fileName, userFileList* user){

    bool fileLocated;                               // Is the file in the File System
    vector <icancloud_File>::iterator list_it;      // File list iterator

        // Init...
        fileLocated = false;

        if (user == nullptr){
            throw cRuntimeError("Error in method searchFile [Node_FileSystem]! User does not exists in node FS!!!");
        }
        list_it=user->fileList.begin();

        // Walk through the list searching the requested block!
        while (list_it!=user->fileList.end() && (!fileLocated)){

            if (!list_it->getFileName().compare(fileName))
                fileLocated = true;
            else
                list_it++;
        }

        return list_it;
}

void Node_FileSystem::deleteUser (Packet  *pkt){

    vector <userFileList*>::iterator list_it;
    int uId;
    bool userFind;
//    icancloud_BlockList_Message* sm_bl;
    userFind = false;
    pkt->trimFront();
    auto sm = pkt->removeAtFront<icancloud_Message>();
    auto sm_io = CHK(dynamicPtrCast<icancloud_App_IO_Message>(sm));

    // Init...
    uId = sm_io->getUid();
    list_it = usersFileList.begin();
    while (list_it!=usersFileList.end() && !(userFind)){
        if ((*list_it)->userID == uId){
            userFind = true;
            for (unsigned int i = 0; i < (*list_it)->fileList.size(); i++){
                // Initialize the blocklist message
                auto sm_bl = Ptr<icancloud_BlockList_Message>(static_cast<icancloud_BlockList_Message *>(new icancloud_BlockList_Message (sm_io.get())));
                //sm_bl = new icancloud_BlockList_Message (sm_io);
                sm_bl->setOperation(SM_DELETE_FILE);
                sm_bl->setNfs_id(1);
                sm_bl->setSize((*list_it)->fileList[i].getFileSize());
                auto pktSmBl = new Packet("icancloud_BlockList_Message");
                pktSmBl->insertAtFront(sm_bl);
                msg_delete_user_id.push_back(pktSmBl->getId());
                sendRequestMessage (pktSmBl, toVMGate);
            }
            usersFileList.erase(list_it);
        }
        else
            list_it++;
    }
    pkt->insertAtFront(sm_io);
}


bool Node_FileSystem::checkResponseMsg(Packet *pkt){

    bool found = false;
    int msgId;


    const auto &sm = pkt->peekAtFront<icancloud_Message>();
    const auto &sm_bl = CHK(dynamicPtrCast<const icancloud_BlockList_Message>(sm));
    if (sm_bl == nullptr)
        throw cRuntimeError("Header not found");

    for (int i = 0; ( (i < (int)msg_delete_user_id.size()) && (!found)); i++){
        msgId = (*(msg_delete_user_id.begin() + i));
        if (msgId == pkt->getId()){
            found = true;
            msg_delete_user_id.erase(msg_delete_user_id.begin() + i);
        }
    }

    return found;
}

void Node_FileSystem::deleteFile (string fileName, userFileList* user){

    vector <icancloud_File>::iterator list_it;


    if (user != nullptr){
        // Search the file!
        list_it = searchFile (fileName, user);

        // File found! remove it!
        if (list_it != user->fileList.end())
            user->fileList.erase (list_it);
    }
    else{
        printf("Error erasing the file %s. The user not exists.\n", fileName.c_str());
    }

}

void Node_FileSystem::translateIORequests (Packet *pkt, userFileList* user){

    unsigned int numSectors;
    string fileName;
    unsigned int numBytes;
    vector <icancloud_File>::iterator list_it;  // List iterator

    pkt->trimFront();
    auto sm = pkt->removeAtFront<icancloud_Message>();
    auto sm_fsTranslated = CHK(dynamicPtrCast<icancloud_BlockList_Message>(sm));

        // Init
    fileName = sm_fsTranslated->getFileName();
    numBytes = sm_fsTranslated->getSize();
    numSectors = 0;
    if (user == nullptr)
        throw cRuntimeError(
                "Error in method translateIORequests[Node_FileSystem]! User does not exists in node FS!!!");

    // Search for the requested file
    list_it = searchFile(fileName, user);
    if (DEBUG_Basic_FS)
        printf("%s\n", FSFilesToString().c_str());
    // If file not found!!!

    if (list_it == user->fileList.end()) {

        sm_fsTranslated->setIsResponse(true);
        sm_fsTranslated->setResult(icancloud_FILE_NOT_FOUND);
        throw cRuntimeError("Error! File not found!!!");
    }

    else {

        numSectors = (unsigned int) ceil(numBytes / BYTES_PER_SECTOR);

        // Add the branch!
        sm_fsTranslated->getFileForUpdate().addBranch(0, numSectors);

        // Set the request size!
        sm_fsTranslated->getFileForUpdate().setFileSize(
                sm_fsTranslated->getFile().getTotalSectors() * BYTES_PER_SECTOR);
    }

    if (DEBUG_DETAILED_Basic_FS)
        showDebugMessage("Translating request -> %s",
                sm_fsTranslated->getFile().contentsToString(
                        DEBUG_BRANCHES_Basic_FS, DEBUG_MSG_Basic_FS).c_str());

    pkt->insertAtFront(sm_fsTranslated);
}


string Node_FileSystem::FSFilesToString (){

    vector <userFileList*>::iterator user_it;       // User list iterator
    vector <icancloud_File>::iterator list_it;      // List iterator

    std::ostringstream osStream;                // Stream
    unsigned int fileNumber;                    // File number

        // Init...
        fileNumber = 0;
        osStream << "File System contents..." << endl;

        for (user_it = usersFileList.begin(); user_it != usersFileList.end(); user_it++){

            osStream << "userID: " << (*user_it)->userID << std::endl;

            // Walk through the list searching the requested block!
            for (list_it=(*user_it)->fileList.begin(); list_it!=(*user_it)->fileList.end(); ++list_it){
                osStream << "file[" << fileNumber << "]: "<< list_it->contentsToString(DEBUG_BRANCHES_Basic_FS, DEBUG_MSG_Basic_FS);
                fileNumber++;
            }
        }

    return osStream.str();
}

void Node_FileSystem::pushPendingMessage(cMessage *msg, cMessage* latencyMsg){
    pendingMsg* pending;

    pending = new pendingMsg();
    pending->msg = msg;
    pending->msgID = latencyMsg->getId();

    pendingMessages.push_back(pending);

}

cMessage* Node_FileSystem::extractPendingMessage(cMessage* msg){

    vector<pendingMsg*>::iterator it;
    bool found;
    pendingMsg* sm_pending;
    cMessage* sms;
    found = false;

    for (it = pendingMessages.begin(); it < pendingMessages.end();){
        sm_pending = (*it);
        if (sm_pending->msgID == msg->getId()){
            sms = sm_pending->msg;
            found = true;
            pendingMessages.erase(it);
        }else {
            it++;
        }
    }

    if (!found){
        showErrorMessage("Node_FileSystem::extractPendingMessage-> PendingMessages does not contains the message ..");
    }

    return sms;

}


} // namespace icancloud
} // namespace inet

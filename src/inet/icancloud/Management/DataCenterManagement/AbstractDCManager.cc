//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "inet/icancloud/Management/DataCenterManagement/AbstractDCManager.h"

namespace inet {

namespace icancloud {

#define TIME_COMPUTEBOOKMSG_SEC 180
#define TIME_STORAGEBOOKMSG_SEC 240
const string AbstractDCManager::OUTPUT_DIRECTORY="results";

AbstractDCManager::~AbstractDCManager() {
    requestsQueue.clear();
    pendingStorageRequests.clear();
    pendingRemoteStorageDeletion.clear();
    connectionsDeletion.clear();
    cpus.clear();
    memories.clear();
    storages.clear();
    networks.clear();
}

void AbstractDCManager::initialize(int stage) {
    DataCenterAPI::initialize(stage);
    UserManagement::initialize(stage);
    StorageManagement::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        // Define ..
        cMessage *msg;
        cModule* networkManagerMod;
        char log_name[NAME_SIZE];
        time_t rawtime;
        struct tm * timeinfo;
        std::ostringstream file;

        // Initialize the superclasses
        // Finish the super-class

        if (configDone != true)
            configDone = false;
        // Initialize the network manager
        networkManagerMod = getParentModule()->getSubmodule("networkManager");
        networkManager = check_and_cast<NetworkManager*>(networkManagerMod);

        // Users ..

        requestsQueue.clear();
        no_more_users = false;

        // Pending storage requests ..

        pendingStorageRequests.clear();
        schedulerQueueBlocked = false;

        // Get the topology of the data center!

        if (!configDone) {
            dataCenterConfig = new CfgDataCenter();

            cModule* topology = getParentModule()->getSubmodule("topology");
            if (topology == nullptr)
                throw cRuntimeError(
                        "AbstractDCManager::initialize() -> Error during initialization. There is no topology\n");

            int computeSize = topology->par("computeNodeQuantity").intValue();

            for (int i = 0; i < computeSize; i++) {
                cModule* computeNodeMod = topology->getSubmodule("computeNode",
                        i);
                dataCenterConfig->setNodeType(
                        computeNodeMod->par("id").stringValue(),
                        computeNodeMod->par("quantity").intValue());
            }

            int storageSize = topology->par("storageNodeQuantity").intValue();

            for (int i = 0; i < storageSize; i++) {
                cModule* storageNodeMod = topology->getSubmodule("storageNode",
                        i);
                dataCenterConfig->setStorageNodeType(
                        storageNodeMod->par("id").stringValue(),
                        storageNodeMod->par("quantity").intValue());
            }
        }

        // Initialize the pendingRemoteStorageDeletion vector
        pendingRemoteStorageDeletion.clear();

        // Initialize the connectionsDeletion vector
        connectionsDeletion.clear();

        // Get the time before start
        timeToStartManager = par("timeToStart").doubleValue();

        // Initialize the structures
        requestsQueue.clear();
        temporalRequestsQueue.clear();
        schedulerQueueBlocked = false;

        // Get the time to log and scheduling events
        timeBetweenScheduleEvents_s =
                par("timeBetweenScheduleEvents_s");
        timeBetweenLogResults_s = par("timeBetweenLogResults_s");

        if (timeBetweenScheduleEvents_s < 1.0)
            throw cRuntimeError(
                    "The time between schedule events has to be at least 1..");
        if (timeBetweenLogResults_s < 0.5)
            throw cRuntimeError(
                    "The time between schedule events has to be at least 1..");

        // Get the parameters for print the energy data to a file
        printEnergyTrace = par("printEnergyTrace").boolValue();
        printEnergyToFile = par("printEnergyToFile").boolValue();

        // Create the file
        if (printEnergyToFile) {
            // Create the folder (if it doesn't exists)
            DIR *logDir;

            // Check if log Directory exists
            logDir = opendir(OUTPUT_DIRECTORY.c_str());

            // If dir do not exists, then create it!
            if (logDir == nullptr) {
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)
                _mkdir(OUTPUT_DIRECTORY.c_str());
#else
                mkdir(OUTPUT_DIRECTORY.c_str(), 00777);
#endif
                //closedir (logDir);
            } else {
                closedir(logDir);
            }

            // Continue with the file creation.
            time(&rawtime);
            timeinfo = localtime(&rawtime);
            sprintf(log_name,
                    "%s/CloudEnergy_%04d-%02d-%02d_%02d:%02d:%02d.txt",
                    OUTPUT_DIRECTORY.c_str(), timeinfo->tm_year + 1900,
                    timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour,
                    timeinfo->tm_min, timeinfo->tm_sec);
            logName = log_name;

        } else {
            logName = "";
        }

        smAlarm = new cMessage(SM_APP_ALARM.c_str());
        logAlarm = new cMessage("logPrint");

        // Prepare the message waiting until all modules will be initialized-
        msg = new cMessage(SM_CALL_INITIALIZE.c_str());
        scheduleAt(simTime() + timeToStartManager, msg);

        memorization = par("memorization").boolValue();
    }
}

void AbstractDCManager::finish(){
    // Finish the super-class
    DataCenterAPI::finish();
    UserManagement::finish();
    StorageManagement::finish();
}

void AbstractDCManager::processSelfMessage (cMessage *msg){

    //Define ..
       // vector<AbstractUser*>::iterator userIt;
        string state;

    //FilePrinter
        std::ostringstream line;

    // Begin ..
        if(!strcmp (msg->getName(), SM_APP_ALARM.c_str())){

            schedule();

            if ((checkFinalization()) && (!simulationPerTime)){
                cancelAndDelete(smAlarm);
                finalizeManager();
            }
            else if (simulationPerTime){
                cancelAndDelete(smAlarm);
                finalizeManager();
            }
            else if
            (!smAlarm->isScheduled())
                scheduleAt(simTime()+timeBetweenScheduleEvents_s, smAlarm);

        }

        else if (!strcmp (msg->getName(), "logPrint")){
                    cancelEvent(logAlarm);
                    // Print energy values..
                    if ((printEnergyToFile) && (printEnergyTrace)){
                        printEnergyValues();
                        scheduleAt (simTime()+timeBetweenLogResults_s, logAlarm);
                    }
        }

        else if (!strcmp (msg->getName(), SM_CALL_INITIALIZE.c_str())){

            // Delete the incoming message
            cancelAndDelete(msg);

            // Initialize the manager
            initManager();

            // Create an alarm to log the energy data events
            scheduleAt (simTime()+timeBetweenLogResults_s, logAlarm);


            initScheduler();

        }

        else if (!strcmp (msg->getName(), "deleteUserGen")){

            // Delete the incoming message
            cancelAndDelete(msg);

            // Delete user generator
            delete(userGenerator);

        }

        else if (!strcmp (msg->getName(), "finalize")){

              //Shutdown all the compute nodes!
           delete(nodesMap);
           delete(storage_nodesMap);
           cancelAndDelete(msg);

           endSimulation();

        }
        /*
         *  If this alarm has been actived, the user does not book the node properly, so it will be free
         *  for other user booking
         */
        else if (!strcmp (msg->getName(), "computeBookAlarm")){

            freeComputingBookByTimeout(msg->getId());
        }

        else if (!strcmp (msg->getName(), "storageBookAlarm")){

            freeStorageBook (msg->getId());
        }

        else {
            cancelAndDelete(msg);
        }
}

void AbstractDCManager::processRequestMessage (Packet *pkt){
    throw cRuntimeError("AbstractDCManager::processRequestMessage->DataCenterManager does not process request messages\n");
}

void AbstractDCManager::processResponseMessage (Packet *pkt){
    throw cRuntimeError("AbstractDCManager::processResponseMessage->DataCenterManager does not process request messages\n");
}

cGate* AbstractDCManager::getOutGate (cMessage *msg){
    return nullptr;
}

ICCLog acs_f;

void AbstractDCManager::initManager (int totalNodes){

    // Define.
        std::ofstream line;
        string file;
        int i;
        int computeNodeSetSize = 0;
        int storageSetSize = 0;
        int totalNumberNodes = 0;

    // Definition for parse
        // Nodes
            vector<string> nodeNames;
            string nodeTypeName;
            HeterogeneousSet* hetNodeSet;

    // Define auxiliar variables to link the module to the object
         cModule* nodeMod;
         string nodeName;
         Node* nodeChecked;

    if (!isCfgLoaded()){

            // Initialize structures and parameters
                nodesMap = new MachinesMap();
                storage_nodesMap = new MachinesMap();

            // Get the Nodes
                computeNodeSetSize = dataCenterConfig->getNumberOfNodeTypes();

            // Memorization
               MemoSupport* cpu;
               MemoSupport* memory;
               MemoSupport* storage;
               MemoSupport* network;
               string component;

               bool componentsLoaded;

               // Memorization init
                   cpus.clear();
                   memories.clear();
                   storages.clear();
                   networks.clear();
                   cpu = nullptr;
                   memory = nullptr;
                   storage = nullptr;
                   network = nullptr;

                for (int j = 0; j < computeNodeSetSize; j++){

                    nodeNames = dataCenterConfig->generateStructureOfNodes(j);

                    hetNodeSet = new HeterogeneousSet();

                    componentsLoaded = false;

                    // link all the created nodes by omnet in the vector nodeSet.
                          for (i = 0 ; i < (int)nodeNames.size(); i++){

                              nodeName = (*(nodeNames.begin() + i));

                              nodeMod = getParentModule()->getParentModule()->getModuleByPath(nodeName.c_str());

                              nodeChecked = check_and_cast<Node*>(nodeMod);
                              nodeChecked->initNode();

                              if ((memorization) && (!componentsLoaded)){
                                  componentsLoaded = true;
                                  bool found = false;
                                  // CPU
                                      component = nodeChecked->getCPUName();

                                      for (int k = 0; k < (int)cpus.size() && (!found); k++){
                                          if (strcmp ((*(cpus.begin() + k))->getComponentName().c_str(), component.c_str()) == 0){
                                              found = true;
                                              cpu = (*(cpus.begin() + k));
                                          }
                                      }

                                      if (!found){
                                          cpu = new MemoSupport( component );
                                          printf ("AbstractMeterUnit::loadMemo()-> %s\n", component.c_str());
                                          cpus.push_back(cpu);
                                      }

                                  // MEMORY
                                      component = nodeChecked->getMemoryName();
                                      found = false;

                                      for (int k = 0; k < (int)memories.size() && (!found); k++){
                                          if (strcmp ((*(memories.begin() + k))->getComponentName().c_str(), component.c_str()) == 0){
                                              found = true;
                                              memory = (*(memories.begin() + k));
                                          }
                                      }

                                      if (!found){
                                          memory = new MemoSupport( component );
                                          printf ("AbstractMeterUnit::loadMemo()-> %s\n", component.c_str());
                                          memories.push_back(memory);
                                      }

                                  // STORAGE
                                      component = nodeChecked->getStorageName();
                                      found = false;

                                      for (int k = 0; k < (int)storages.size() && (!found); k++){
                                          if (strcmp ((*(storages.begin() + k))->getComponentName().c_str(), component.c_str()) == 0){
                                              found = true;
                                              storage = (*(storages.begin() + k));
                                          }
                                      }

                                      if (!found){
                                          storage = new MemoSupport( component );
                                          printf ("AbstractMeterUnit::loadMemo()-> %s\n", component.c_str());
                                          storages.push_back(storage);
                                      }

                                  // NETWORK
                                      component = nodeChecked->getNetworkName();
                                      found = false;

                                      for (int k = 0; k < (int)networks.size() && (!found); k++){
                                          if (strcmp ((*(networks.begin() + k))->getComponentName().c_str(), component.c_str()) == 0){
                                              found = true;
                                              network = (*(networks.begin() + k));
                                          }
                                      }

                                      if (!found){
                                          network = new MemoSupport( component );
                                          printf ("AbstractMeterUnit::loadMemo()-> %s\n", component.c_str());
                                          networks.push_back(network);
                                      }
                              }
                              nodeChecked->loadMemo(cpu,memory,storage,network);

                              string state = (nodeMod->par("initialState").stringValue());
                              if (state == MACHINE_STATE_OFF) nodeChecked->turnOff();
                              else nodeChecked->turnOn();

                              if (i == 0){
                                  hetNodeSet->setElementType(nodeChecked->getElementType());
                              }
                              totalNumberNodes++;
                          }
                          componentsLoaded = false;
                          cpu = nullptr;
                          memory = nullptr;
                          storage = nullptr;
                          network = nullptr;

                    nodesMap->setInstances(hetNodeSet);

                }

            // Get the Storage Nodes
                storageSetSize = dataCenterConfig->getNumberOfStorageNodeTypes();

                for (int j = 0; j < storageSetSize; j++){

                    nodeNames = dataCenterConfig->generateStructureOfStorageNodes(j);

                    hetNodeSet = new HeterogeneousSet();

                    componentsLoaded = false;

                // link all the created nodes by omnet in the vector nodeSet.
                      for (i = 0 ; i < (int)nodeNames.size(); i++){

                          nodeName = (*(nodeNames.begin() + i));

                          nodeMod = getParentModule()->getParentModule()->getModuleByPath(nodeName.c_str());

                          nodeChecked = check_and_cast<Node*>(nodeMod);
                          nodeChecked->initNode();

                          if ((memorization) && (!componentsLoaded)){
                                componentsLoaded = true;
                                bool found = false;
                                // CPU
                                    component = nodeChecked->getCPUName();

                                    for (int k = 0; k < (int)cpus.size() && (!found); k++){
                                        if (strcmp ((*(cpus.begin() + k))->getComponentName().c_str(), component.c_str()) == 0){
                                            found = true;
                                            cpu = (*(cpus.begin() + k));
                                        }
                                    }

                                    if (!found){
                                        cpu = new MemoSupport( component );
                                        printf ("AbstractMeterUnit::loadMemo()-> %s\n", component.c_str());
                                        cpus.push_back(cpu);
                                    }

                                // MEMORY
                                    component = nodeChecked->getMemoryName();
                                    found = false;

                                    for (int k = 0; k < (int)memories.size() && (!found); k++){
                                        if (strcmp ((*(memories.begin() + k))->getComponentName().c_str(), component.c_str()) == 0){
                                            found = true;
                                            memory = (*(memories.begin() + k));
                                        }
                                    }

                                    if (!found){
                                        memory = new MemoSupport( component );
                                        printf ("AbstractMeterUnit::loadMemo()-> %s\n", component.c_str());
                                        memories.push_back(memory);
                                    }

                                // STORAGE
                                    component = nodeChecked->getStorageName();
                                    found = false;

                                    for (int k = 0; k < (int)storages.size() && (!found); k++){
                                        if (strcmp ((*(storages.begin() + k))->getComponentName().c_str(), component.c_str()) == 0){
                                            found = true;
                                            storage = (*(storages.begin() + k));
                                        }
                                        }

                                    if (!found){
                                        storage = new MemoSupport( component );
                                        printf ("AbstractMeterUnit::loadMemo()-> %s\n", component.c_str());
                                        storages.push_back(storage);
                                    }

                                    // NETWORK
                                        component = nodeChecked->getNetworkName();
                                        found = false;

                                        for (int k = 0; k < (int)networks.size() && (!found); k++){
                                            if (strcmp ((*(networks.begin() + k))->getComponentName().c_str(), component.c_str()) == 0){
                                                found = true;
                                                network = (*(networks.begin() + k));
                                            }
                                        }

                                        if (!found){
                                            network = new MemoSupport( component );
                                            printf ("AbstractMeterUnit::loadMemo()-> %s\n", component.c_str());
                                            networks.push_back(network);
                                        }
                                }
                          nodeChecked->loadMemo(cpu,memory,storage,network);

                          string state = (nodeMod->par("initialState").stringValue());
                          if (state == MACHINE_STATE_OFF) nodeChecked->turnOff();
                          else nodeChecked->turnOn();

                          if (i == 0){
                              hetNodeSet->setElementType(nodeChecked->getElementType());
                          }
                          totalNumberNodes++;
                      }

                    storage_nodesMap->setInstances(hetNodeSet);
                }
        }

    //Configure the compute nodes

       configureMap (nodesMap);

   // Configure the storage nodes

       configureMap (storage_nodesMap);

   // Print the trace
   if (printEnergyTrace) {

       if (totalNodes != -1) totalNumberNodes = totalNodes;

         // print the mode and the number of nodes
             acs_f.Open(logName, par("outputCompression").boolValue());
             acs_f.Append("@Logger-mode;%d\n", totalNumberNodes) ;
             acs_f.Close();

   }
}

void AbstractDCManager::finalizeDCManager(){

    Enter_Method_Silent();

    int numberOfNodeTypes;
    int i, j;
    int nodeQuantity;
    AbstractNode* node;

       //Shutdown all the compute nodes!

       numberOfNodeTypes = nodesMap->size();

       for (i = 0; i < numberOfNodeTypes; i++){

           nodeQuantity = nodesMap->getSetQuantity(i);

           for (j = 0; j < nodeQuantity; j++){
               node = getNodeByIndex(i,j);
               if (node->isON()) node->turnOff();
           }
       }

       //Shutdown all the storage nodes!

       numberOfNodeTypes = storage_nodesMap->size();

       for (i = 0; i < numberOfNodeTypes; i++){

           nodeQuantity = storage_nodesMap->getSetQuantity(i);

           for (j = 0; j < nodeQuantity; j++){
               node = getNodeByIndex(i,j);
               if (node->isON()) node->turnOff();
           }
       }

       cMessage* waitToExecuteMsg = new cMessage ("finalize");
       scheduleAt (simTime()+10, waitToExecuteMsg);

}

void AbstractDCManager::initScheduler(){
    setupScheduler();
    schedule();
    if (!smAlarm->isScheduled()) scheduleAt(simTime()+timeBetweenScheduleEvents_s, smAlarm);

}


/***********************************************************************
 * Booking methods
 ***********************************************************************/
int AbstractDCManager::bookComputeResources( int uid, int jobID, int nodeSet, int nodeID, int cores){

    // Define ..
        AbstractNode* node;
        int messageID;
        int numCores;
        cMessage *bookAlarm;
        pendingComputingBookMessage* bookStructure;

    // Initialize ..
        messageID = -1;

    // Begin ..
        // If the book is for a set of cores..
        if (cores != -1){
            node = getNodeByIndex(nodeSet, nodeID);
            numCores = node->getNumCores();
            // If it has enough cores
            if (numCores >= cores){

                bookStructure = new pendingComputingBookMessage();

                bookAlarm = new cMessage ("computeBookAlarm");
                messageID = bookAlarm->getId();

                bookStructure->uid = uid;
                bookStructure->nodeSet = nodeSet;
                bookStructure->nodeId = nodeID;
                bookStructure->cores = cores;
                bookStructure->message = bookAlarm;
                bookStructure->jobID = jobID;

                pendingComputeBook.push_back(bookStructure);

                scheduleAt (simTime()+TIME_COMPUTEBOOKMSG_SEC, bookAlarm);

            // If it has not enough cores
            } else {
                messageID = -1;
            }
        }

        // If the book is for the entire node..
        else{
//            node->setNumFreeCPU(0);

            bookStructure = new pendingComputingBookMessage();

            bookAlarm = new cMessage ("computeBookAlarm");
            messageID = bookAlarm->getId();

            bookStructure->uid = uid;
            bookStructure->nodeSet = nodeSet;
            bookStructure->nodeId = nodeID;
            bookStructure->cores = cores;
            bookStructure->message = bookAlarm;

            pendingComputeBook.push_back(bookStructure);

            scheduleAt (simTime()+TIME_COMPUTEBOOKMSG_SEC, bookAlarm);
        }

    return messageID;
}

void AbstractDCManager::eraseComputingBook(int messageID){

   cMessage* msg;
   bool found = false;

     for (int i = 0; (i < (int)pendingComputeBook.size()) && (!found); i++){

         msg = (*(pendingComputeBook.begin() + i))->message;

         if (msg->getId() == messageID){
             cancelAndDelete(msg);

             createUserComputeEntry((*(pendingComputeBook.begin() + i))->uid, (*(pendingComputeBook.begin() + i))->nodeSet, (*(pendingComputeBook.begin() + i))->nodeId, (*(pendingComputeBook.begin() + i))->cores, (*(pendingComputeBook.begin() + i))->jobID);

             // Erase the user book
             pendingComputeBook.erase((pendingComputeBook.begin() + i));
             found = true;
         }
     }
}

void AbstractDCManager::adminBookComputingResources (string nodeSetName, int nodeId, int cores, int uid, int jobId){

    createUserComputeEntry(uid, nodeSetName, nodeId, cores, jobId);
}

void AbstractDCManager::freeComputingBookByTimeout(int messageID){

    pendingComputingBookMessage* msg;
    bool found = false;
    int numCores = 0;
    AbstractNode* node;

      for (int i = 0; (i < (int)pendingComputeBook.size()) && (!found); i++){

          msg = (*(pendingComputeBook.begin() + i));

          if (msg->message->getId() == messageID){
              cancelAndDelete(msg->message);
              node = getNodeByIndex(msg->nodeSet, msg->nodeId);

              numCores = node->getNumCores();
              numCores += msg->cores;
//              node->setNumFreeCPU(numCores);

              pendingComputeBook.erase((pendingComputeBook.begin() + i));
              found = true;
          }

      }
}

connection_T* AbstractDCManager::bookStorageResources(int uid, int jobId, int nodeSet, int nodeID, double storage){
    // Define ..
          AbstractNode* node;
          double remainingStorage;
          connection_T* connection;
          cMessage* bookAlarm;
          pendingStorageBookMessage* bookStructure;

      // Initialize ..
          node = getNodeByIndex(nodeSet, nodeID);
          remainingStorage = node->getFreeStorage() - storage;

          if (remainingStorage > 0){

              // Set temporally the disk size until te operations will begin
//                  node->setFreeStorage(remainingStorage);

              // Create the structure for wait until the operations begin
                  bookStructure = new pendingStorageBookMessage();
                  bookAlarm = new cMessage ("storageBookAlarm");
                  bookStructure->message = bookAlarm;
                  bookStructure->nodeSet = node->getTypeName();
                  bookStructure->nodeId = node->getId();
                  bookStructure->storage = storage;
                  bookStructure->uid  = uid;
                  bookStructure->jobID = jobId;
                  bookStructure->ip = node->getIP();

              // Get the connection to the node
                  connection = new connection_T();
                  connection->uId = uid;
                  connection->pId = jobId;
                  connection->ip = node->getIP();
                  connection->port = node->getStoragePort();

              // Create the alarm
                  pendingStorageBook.push_back(bookStructure);

                  scheduleAt (simTime()+TIME_STORAGEBOOKMSG_SEC, bookAlarm);

          } else {
              connection = nullptr;
          }

      return connection;
}

void AbstractDCManager::freeStorageBook(int messageID, int uid, int jobId, string ip, double storage){

    pendingStorageBookMessage* pending;
    bool found = false;
    double storageSize = 0.0;
    AbstractNode* node;

      for (int i = 0; (i < (int)pendingStorageBook.size()) && (!found); i++){

          pending = (*(pendingStorageBook.begin() + i));

          if (
                  ((messageID != -1) && (pending->message->getId() == messageID)) ||
                  ((pending->uid = uid) && (pending->jobID = jobId) && (strcmp(ip.c_str(), pending->ip.c_str()) == 0) && (pending->storage == storage) )
             ){
              cancelAndDelete(pending->message);
              node = getNodeByIndex(pending->nodeSet, pending->nodeId);

              storageSize = node->getFreeStorage();
              storageSize += pending->storage;
//              node->setFreeStorage(storageSize);

              pendingStorageBook.erase((pendingStorageBook.begin() + i));
              found = true;
          }

      }
}

void AbstractDCManager::processRollBack(int uid, int jobId){
    int i;
    int numCores = 0;
    double storageSize;
    AbstractNode* node;

    for (i = 0; i < (int)pendingComputeBook.size();){
        if ( ((*(pendingComputeBook.begin() + i))->uid == uid) && ((*(pendingComputeBook.begin() + i))->jobID == jobId)){
            cancelAndDelete((*(pendingComputeBook.begin() + i))->message);
            node = getNodeByIndex((*(pendingComputeBook.begin() + i))->nodeSet, (*(pendingComputeBook.begin() + i))->nodeId);

//            numCores = node->getFreeCores();
            numCores += (*(pendingComputeBook.begin() + i))->cores;
//            node->setNumFreeCPU(numCores);

            pendingComputeBook.erase((pendingComputeBook.begin() + i));
        }

        else{
            i++;
        }
    }

    for (i = 0; i < (int)pendingStorageBook.size();){
        if ( ((*(pendingStorageBook.begin() + i))->uid == uid) && ((*(pendingStorageBook.begin() + i))->jobID == jobId)){
            cancelAndDelete((*(pendingStorageBook.begin() + i))->message);
            node = getNodeByIndex((*(pendingStorageBook.begin() + i))->nodeSet, (*(pendingStorageBook.begin() + i))->nodeId);

            storageSize = node->getFreeStorage();
            storageSize += (*(pendingStorageBook.begin() + i))->storage;
//            node->setFreeStorage(storageSize);

            pendingStorageBook.erase((pendingStorageBook.begin() + i));
        }
        else
            i++;
    }

    freeUserComputeEntry(uid,jobId);
}

bool AbstractDCManager::newUser (AbstractUser *user){
    bool admitted;

    admitted = UserManagement::newUser(user);

    if (admitted) networkManager->createNewUser(user->getUserId());

    return admitted;

}

void AbstractDCManager::notifyManager(Packet *pkt){
    throw cRuntimeError ("AbstractDCManager->to be implemented");
}

void AbstractDCManager::userStorageRequest (StorageRequest* st_req, AbstractNode* nodeHost){

    int requestOperation;
    int i;
    bool noSpace;

    AbstractNode* node;
    vector<preload_T*> files;
    vector<fsStructure_T*> fsStructure;
    vector<AbstractNode*> nodes;

    // Init..
        files.clear();
        fsStructure.clear();
        noSpace = false;

    // Extract the information from the storage request for the storage management
        requestOperation = st_req->getOperation();
        files = st_req->getPreloadFilesSet();
        fsStructure = st_req->getFSSet();
        if (nodeHost == nullptr) throw cRuntimeError ("AbstractDCManager::userStorageRequest->req->getOperation() == %i, error. The storage request has an unknown operation\n", st_req->getConnection(0)->ip.c_str());

      if (requestOperation == REQUEST_LOCAL_STORAGE){
          // Get the io manager of the OS from the host node to communicate the new operation
              if (nodeHost == nullptr) throw cRuntimeError ("AbstractDCManager::userStorageRequest->req->getOperation() == %s, error. The storage request has an unknown operation\n", st_req->getConnection(0)->ip.c_str());
          // Erase the first position that is the node host

      }
      else if (requestOperation == REQUEST_REMOTE_STORAGE){

           // get the storage nodes
          nodes = selectStorageNodes(st_req);

          if (nodes.size() != 0){

              // for each node, create the connection
              for (i = 0; i < (int)nodes.size();  i++){
                  node =(*(nodes.begin()+i));

                  // If the nodes are off, turn on
                      if ((node->isOFF())){
                          node->turnOn();
                      }
              }
          }
          else{
              noSpace = true;
          }

      }

      if (noSpace){
          //TODO: Return the st_req and analyzes the error at sched
          showDebugMessage("AbstractDCManager::userStorageRequest->No space at storage remote hosts\n");
      }else{
          // Invoke the method with all after extract all the info from the storage request
          StorageManagement::manageStorageRequest(st_req, nodeHost, nodes);
      }
}


void AbstractDCManager::freeComputeResources(string nodeSet, int nodeIndex, int cores){

}

void AbstractDCManager::checkManagerFinalization(){

    no_more_users = true;

    if ((no_more_users) && (userList.size() == 0)) finalizeManager();
}

double AbstractDCManager::getEnergyConsumed(bool computeNodes, bool storageNodes){

    // Definition ..
        std::ostringstream line;
        Node* node;
        Machine* ab_node;
        string nodeState;
        double totalConsumption;
        int i,j, nodesMapSize, nodeSetQuantity;

    // Initialize ..
        totalConsumption = 0.0;

    // Compute nodes ...
        if (computeNodes){
            // Initialize ..
                nodesMapSize = nodesMap->size();

            // Calculate the power consumption of all the nodes
            for (i = 0; i < nodesMapSize; i++){

                nodeSetQuantity = getSetStorageSize(i);

                for (j = 0; j < nodeSetQuantity;j++){
                    ab_node = getNodeByIndex(i,j);
                    node = dynamic_cast<Node*> (ab_node);
                    if (node == nullptr) throw cRuntimeError ("DataCenterAPI::getEnergyConsumed->node is nullptr after dynamic casting \n");
                    totalConsumption += node->getEnergyConsumed();
                }
            }
        }

    // Storage nodes
        if (storageNodes){

            // Initialize ..
                nodesMapSize = storage_nodesMap->size();

            // Calculate the power consumption of all the nodes
            for (i = 0; i < nodesMapSize; i++){

                nodeSetQuantity = storage_nodesMap->getSetQuantity(i);

                for (j = 0; j < nodeSetQuantity;j++){
                    ab_node = storage_nodesMap->getMachineByIndex(i,j);
                    node = dynamic_cast<Node*> (ab_node);
                    if (node == nullptr) throw cRuntimeError ("DataCenterAPI::getEnergyConsumed->node is nullptr after dynamic casting \n");

                    totalConsumption += node->getEnergyConsumed();
                }
            }
        }

    return totalConsumption;
}


/****************************************************************************************
 *                                  Private methods
 ****************************************************************************************/
void AbstractDCManager::deleteUser (int userId){

// Delete user from networkManager
    networkManager->deleteUser (userId);

    UserManagement::deleteUser(userId);
}

void AbstractDCManager::configureMap (MachinesMap* map){

    // Define ...
        int size;
        int i,j;
        int nodeSetQuantity;
        cModule* nodeMod;
        string state;
        string nodeip;
        Machine* anode;
        Node* node;

    // Init ..
        size = map->size();

    // To check the network
    if (networkManager != nullptr)
       for (i = 0; i < size; i++){

            nodeSetQuantity = map->getSetQuantity(i);

            for (j = 0; j < nodeSetQuantity;j++){

                anode = map->getMachineByIndex(i,j);

                node = dynamic_cast<Node*>(anode);

                //Activate network nodes

                nodeMod = getParentModule()->getSubmodule(node->getName(), j);

                if (nodeMod != nullptr){

                    // Obtain the ip parameter
                    nodeip = nodeMod->par("ip").stringValue();

                    // register the node at network manager
                    networkManager->registerNode(nodeip);

                    //set the manager at OS of the node for returning finish operations

                    node->setManager(this);

                }
                else{
                    throw cRuntimeError("Error when ip has been collected from node at AbstractDCManager::configureMap\n");
                }
            } // end for by node
        } // end for by map set
    else{
        throw cRuntimeError("Error. Network manager is nullptr! [AbstractDCManager::configureMap]\n");

    }
}


} // namespace icancloud
} // namespace inet

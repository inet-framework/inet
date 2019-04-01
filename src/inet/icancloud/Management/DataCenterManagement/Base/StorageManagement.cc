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

#include "inet/icancloud/Management/DataCenterManagement/Base/StorageManagement.h"

namespace inet {

namespace icancloud {



StorageManagement::~StorageManagement() {
    // TODO Auto-generated destructor stub
}

void StorageManagement::initialize(int stage) {
    icancloud_Base::initialize (stage);

    if (stage == INITSTAGE_LOCAL) {
        // The number of Parallel file system remote servers (from .ned parameter)
        numberOfPFSRemoteServers = par("numberOfPFSRemoteServers");
    }
}

void StorageManagement::finish(){
    icancloud_Base::finish();
}

void StorageManagement::manageStorageRequest(StorageRequest* st_req, AbstractNode* nodeHost, vector<AbstractNode*> nodesTarget){

    // Extract the information from the storage request for the storage management
        int uid;
        int pId;
        int spId;

    // Init..
        uid = st_req->getUid();
        pId = st_req->getPid();
        spId = st_req->getSPid();

    // Define ..
       PendingStorageRequest* pendingRequest;
       processOperations* processOperation;
       subprocessOperations* subprocessOperation;
       connection_T* connection;
       vector<connection_T*> connections;
       bool userFound;
       bool processFound;
       bool subprocessFound;

   // Init..
       connections.clear();
       userFound = false;
       processFound = false;
       subprocessFound = false;

   // Search if the connection exists..
       if (nodeHost == nullptr) throw cRuntimeError ("StorageManagement::userStorageRequest->Host to manage operations is nullptr!");
   // Build the structure pending storage
       for (int i = 0; (i < (int)pendingStorageRequests.size()) && (!userFound); i++){

           pendingRequest = (*(pendingStorageRequests.begin() + i));

           // the user exists
           if (pendingRequest->uId == uid){
               userFound = true;

               // Search for the first level (processId)
               for(int j = 0; j < (int)(pendingRequest->processOperation.size()) && (!processFound); j++){

                   processOperation = (*(pendingRequest->processOperation.begin() + j));

                   // the job exists
                   if (processOperation->pId == pId){
                       processFound = true;

                       for (int k = 0; (k < (int)processOperation->pendingOperation.size()) && (!subprocessFound); k++){
                           subprocessOperation = (*(processOperation->pendingOperation.begin() + k));

                           // if the subprocessExists
                           if (subprocessOperation->spId == processOperation->pId){
                               // Not virtualized environment
                               subprocessFound = true;
                           } else if(subprocessOperation->spId == spId){
                               // Virtualized environment
                               subprocessFound = true;
                           }
                       }
                   }
               }
           }
       }

       // If not exists subprocess requests, create a new entry
       if (!subprocessFound){
           subprocessOperation = new subprocessOperations();
           subprocessOperation->spId = spId;
           subprocessOperation->operation = st_req->getOperation();
           subprocessOperation->numberOfConnections = nodesTarget.size() + 1;
           subprocessOperation->pendingOperation.clear();

       }

           // Its time to distinguish between local or remote storage.
           if (st_req->getOperation() == REQUEST_LOCAL_STORAGE){

              string opIp = st_req->getConnection(0)->ip;

           // Create the connection
              connection = new connection_t();
              connection->ip = st_req->getConnection(0)->ip;
              connection->pId =st_req->getConnection(0)->pId;
              connection->port =st_req->getConnection(0)->port;
              connection->uId =st_req->getConnection(0)->uId;
             subprocessOperation->pendingOperation.push_back(connection);

             nodeHost->setLocalFiles(uid, pId, spId, opIp, st_req->getPreloadFilesSet(), st_req->getFSSet());

           } else if(st_req->getOperation() == REQUEST_REMOTE_STORAGE){
               // Connections have the created data for connect node host with nodes target
               connections = createConnectionToStorage (st_req, nodeHost, nodesTarget);

               for (int i = 0; i < (int)connections.size(); i++){
                   // save each connection as pending operation
                   connection = new connection_t();
                   connection->ip = (*(connections.begin() +i))->ip;
                   connection->pId =(*(connections.begin() +i))->pId;
                   connection->port =(*(connections.begin() +i))->port;
                   connection->uId =(*(connections.begin() +i))->uId;
                   subprocessOperation->pendingOperation.push_back(connection);

               }

           }else
               throw cRuntimeError  ("StorageManagement::manageStorageRequest->error. Operation unknown\n");

       // Continue with the process for storaging a pending request
       if (!processFound){
           processOperation = new processOperations();
           processOperation->pId = pId;
           processOperation->pendingOperation.clear();
       }

       processOperation->pendingOperation.push_back(subprocessOperation);

       // If the user does not exists, create an entry for him.
       if (!userFound){
           pendingRequest = new PendingStorageRequest();
           pendingRequest->uId = uid;
           pendingRequest->processOperation.clear();
       }

       pendingRequest->processOperation.push_back(processOperation);

       pendingStorageRequests.push_back(pendingRequest);

}

vector<connection_T*> StorageManagement::createConnectionToStorage(StorageRequest* st_req, AbstractNode* nodeHost,vector<AbstractNode*> nodesTarget){

    AbstractNode* nodeTarget;
    connection_T* connection;
    vector<connection_T*> connections;
    vector<string> ips;
    string files_per_preload;
    unsigned int i;


    // Init..
        ips.clear();
        connections.clear();
        connection = nullptr;

    // Get the ip's of nodes target
        for (i = 0; i < nodesTarget.size(); i++){

            ips.push_back((*(nodesTarget.begin()+i))->getIP().c_str());

            // Set the connection into the pending request
            connection = new connection_T();

            connection->ip = ((*(nodesTarget.begin()+i))->getIP().c_str());
            connection->port = ((*(nodesTarget.begin()+i))->getStoragePort());
            connection->uId = st_req->getUid();
            connection->pId = st_req->getConnection(i)->pId;

            connections.push_back(connection);
        }

    // Create the connection ..
        while (!nodesTarget.empty()){

            // Get the first node target
                nodeTarget = (*(nodesTarget.begin()));

            //  Set the files into de FS of the node.
                nodeTarget->setRemoteFiles (st_req->getUid(), st_req->getPid(),st_req->getSPid(), nodeHost->getIP(), st_req->getPreloadFilesSet(), st_req->getFSSet());

            // Create the listen connection into the target node
                nodeTarget->createDataStorageListen(st_req->getUid(), st_req->getPid());

            // Delete the node target from the list
            nodesTarget.erase(nodesTarget.begin());
        }

        // Connect to the host
            nodeHost->connectToRemoteStorage(ips, st_req->getFsType(), st_req->getUid(), st_req->getPid(), nodeHost->getIP().c_str(), st_req->getConnection(0)->pId);

        return connections;
}

void StorageManagement::formatFSFromNodes (vector<AbstractNode*> nodes, int uId, int pId, bool turnOffNode){

    // Define ..
    std::ostringstream user;
    PendingRemoteStorageDeletion* pendingRemoteUnit;
    unsigned int i;

    // Init ..

    // Create the pending remote storage deletion element
    pendingRemoteUnit = new PendingRemoteStorageDeletion();
    pendingRemoteUnit->uId = uId;
    pendingRemoteUnit->pId = pId;
    pendingRemoteUnit->remoteStorageQuantity = 0;

    // The vm has remote storage
    if (nodes.size() != 0){

        for (i = 0; i < nodes.size(); i++){
            (*(nodes.begin()+i))->deleteUserFSFiles(uId, pId);
            pendingRemoteUnit->remoteStorageQuantity++;

        }

        // Insert into the pendingRemoteStorageDeletion vector
            pendingRemoteStorageDeletion.push_back(pendingRemoteUnit);
    }
    // The vm has only local storage
    else{
        // Insert into the pendingRemoteStorageDeletion vector
            pendingRemoteUnit->remoteStorageQuantity++;
            pendingRemoteStorageDeletion.push_back(pendingRemoteUnit);
            //icancloud_Message* msg;
            auto pkt = new Packet(SM_NOTIFICATION.c_str());
            auto sm = makeShared<icancloud_Message>();
            sm->setPid(pId);
            sm->setUid(uId);
            sm->setOperation(SM_NOTIFY_USER_FS_DELETED);//notifyFSFormatted;
            pkt->insertAtFront(sm);
            notifyManager(pkt);
    }

}

void StorageManagement::notifyManager(Packet *msg){
    throw cRuntimeError ("StorageManagement->to be implemented");
}

void StorageManagement::connection_realized (StorageRequest* attendeed_req){

    // TODO
    throw cRuntimeError (" StorageManagement::connection_realized ->to be implemented\n");

}


} // namespace icancloud
} // namespace inet

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

#include "inet/icancloud/Users/Base/userStorage.h"

namespace inet {

namespace icancloud {



userStorage::~userStorage() {
    dir_remote_storage.clear();
}


void userStorage::initialize(int stage) {
    userBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        waiting_for_remote_storage_Queue = new JobQueue();
        dir_remote_storage.clear();
    }
}

void userStorage::finish(){
    userBase::finish();

    dir_remote_storage.clear();
}

vector<StorageRequest*> userStorage::createFSforJob(jobBase* job, string opIp, string nodeSetName, int nodeId, int optionalID){

    // Define ..
        vector<StorageRequest*>::iterator reqIt;
        vector<StorageRequest*> requests_vector_storage;
        AbstractRequest* reqA;

    // Init ..
        requests_vector_storage.clear();

        // Set the FS parameters
        if ((job->getFSSize() == 0) && (job->getPreloadSize() == 0)){

            throw cRuntimeError ("The user %s[%i] has not defined a folder structure for FS neither preload to its vms..", this->getUserName().c_str(), this->getUserId());

        } else if (job->getFSSize() == 0){

            throw cRuntimeError("The user %s[%i] has not defined an file system structure to its vms..", this->getUserName().c_str(), this->getUserId());

        } else {
                int commId = allocateJob(job);
                job->setCommId(commId);
            // Create the requests for allocate preload or connect to remote storage..
                requests_vector_storage = generateStorageRequests (job, opIp, optionalID);
            // Send all requests for remote storage before change the state of vms to pending remote storage
                for (reqIt = requests_vector_storage.begin();reqIt != requests_vector_storage.end(); reqIt++){
                    (*reqIt)->setNodeSetId(nodeSetName);
                    (*reqIt)->setNodeId(nodeId);

                    reqA = check_and_cast<AbstractRequest*>((*reqIt));
                    send_request_to_manager(reqA);
                }

            // Move the job until the request will be atendeed..

                waitingQueue->move_to_qDst(waitingQueue->get_index_of_job(job->getId()),
                                           waiting_for_remote_storage_Queue,
                                           waiting_for_remote_storage_Queue->get_queue_size());
        }

        return requests_vector_storage;
}



/*************************************************************************************************************
 *                                      PRIVATE METHODS
 **************************************************************************************************************/

vector<StorageRequest*> userStorage::generateStorageRequests (jobBase* job, string opIp, int pId ){

    // Define ..
    string remoteFS;
    string remotePreloadData;
    vector<StorageRequest*> req_vector;
    StorageRequest* req;
    vector<fsStructure_T*> auxPaths;
    connection_T* connection;
    vector<fsStructure_T*> fsConfig;
    vector<preload_T*> preloadConfig;

    // Initialize ..
        fsConfig = job->getFSComplete();
        preloadConfig = job->getPreloadComplete();
        remoteFS = getFSType();
        auxPaths.clear();

    // The fs has remote routes
        if (hasRemoteFS(fsConfig)){

            req = new StorageRequest();

            // Initialize the values of the request
                req->setOperation(REQUEST_REMOTE_STORAGE);
                req->setState(REQUEST_PENDING);
                setUserToRequest(req);
                if (pId == -1)
                    req->setPid(job->getJobId());
                else{
                    req->setPid(pId);
                    req->setSPid(job->getJobId());
                }

                // Get the remote preload data
                auxPaths = getRemoteFSPaths(fsConfig);

                if (preloadConfig.size() != 0)
                    req->setPreloadSet(filterPreloadFilesByFS(auxPaths,preloadConfig));

                req->setFSPathSet(auxPaths);

            // Begin ..
                if (strcmp (remoteFS.c_str(), "PFS") == 0){

                    req->setFsType(FS_PFS);

                } else if (strcmp (remoteFS.c_str(), "NFS") == 0){

                    req->setFsType(FS_NFS);

                } else if (strcmp (remoteFS.c_str(), "LOCAL") == 0){

                    throw cRuntimeError("userStorage-> generateStorageRequests detected a remote storage and the remotefs is local.");

                } else {

                    throw cRuntimeError("userStorage-> select VMs to generateStorageRequests storage. Error, remotefs not recognized.%s", remoteFS.c_str());
                }

//                // Create the connection
                   connection = new connection_T();
                   connection->ip = opIp;
                   connection->port = -1;
                   connection->pId = job->getJobId();
                   connection->uId = this->getId();
                   req->setConnection(connection);

                req_vector.insert(req_vector.begin(), req);
        }
        // The fs has local routes
        if (hasLocalFS(fsConfig)) {

            req = new StorageRequest();

            // Initialize the values of the request
                req->setOperation(REQUEST_LOCAL_STORAGE);
                setUserToRequest(req);
                if (pId != -1){
                    req->setPid(pId);
                    req->setSPid(job->getJobId());
                } else {
                    req->setPid(job->getJobId());
                }

                req->setFsType(FS_LOCAL);
                req->setState(REQUEST_PENDING);

            // Get the remote preload data
                auxPaths = getLocalFSPaths(fsConfig);

                if ((preloadConfig.size() != 0) && (auxPaths.size() != 0))
                    req->setPreloadSet(filterPreloadFilesByFS(auxPaths,preloadConfig));

            // Begin ..
                req->setFSPathSet(auxPaths);

            // Create the connection for the manager identification
               connection = new connection_T();
               connection->ip = opIp;
               connection->port = -1;
               connection->pId = job->getJobId();
               connection->uId = this->getId();
               req->setConnection(connection);

                req_vector.insert(req_vector.begin(), req);
        }

    return req_vector;
}

vector<fsStructure_T*> userStorage::getRemoteFSPaths (vector<fsStructure_T*> fsConfig){


    vector<fsStructure_T*> paths;
    int fsSize, i;
    fsStructure_T* fs;
    fsSize = fsConfig.size();


    paths.clear();

    // Preload numEntries files!!!
    for (i=0; i<fsSize; i++){

       // Load the current path entry
       fs = (*(fsConfig.begin() + i));

       if (fs->fsType != FS_LOCAL)
             paths.push_back(fs);

    }

    return paths;

}

vector<fsStructure_T*> userStorage::getLocalFSPaths (vector<fsStructure_T*> fsConfig){

    vector<fsStructure_T*> paths;
    int fsSize, i;
    fsStructure_T* fs;
    fsSize = fsConfig.size();

    paths.clear();

    // Preload numEntries files!!!
    for (i=0; i<fsSize; i++){

       // Load the current path entry
       fs = (*(fsConfig.begin() + i));

       if (fs->fsType == FS_LOCAL)
             paths.push_back(fs);

    }

    return paths;


}


vector <preload_T*> userStorage::filterPreloadFilesByFS (vector<fsStructure_T*> fsPaths, vector<preload_T*> preloadFiles){

    // Define ..
        vector<preload_T*> preloadResult;
        vector<string> preloadFiltered;
        vector<string> fsFiltered;
        vector<fsStructure_T*>::iterator fsIt;
        string fs;

        string folderfs;
        string folderPreload;
        string files_per_preload;

        unsigned int j;
        bool differentMatch;
        bool ok;

    // Init..
        differentMatch = false;
        ok = false;
        preloadResult.clear();
        preloadFiltered.clear();
        fsFiltered.clear();

    for (fsIt = fsPaths.begin(); fsIt != fsPaths.end(); fsIt++){

        fs = (*fsIt)->fsRoute.c_str();

        // Divide into a vector the routes of each folder
        fsFiltered = divide (fs.c_str(),'/');

        if (fsFiltered.size() == 0) {
            fsFiltered.push_back(fs.c_str());

        }else if ((fsFiltered.size() == 1) && ((*fsFiltered.begin()) == "")) {
            fsFiltered.erase(fsFiltered.begin());
            fsFiltered.push_back(fs.c_str());
        }else{
            fsFiltered.erase(fsFiltered.begin());
        }

        while (preloadFiles.size() != 0){

            // Analyze the folders of each preload file


            // Divide the name into different folders of each file
                    preloadFiltered = divide((*(preloadFiles.begin()))->fileName.c_str(), '/');
                    if ((*preloadFiltered.begin()) == "") {
                        preloadFiltered.erase(preloadFiltered.begin());
                    }

                for (unsigned int i = 0; i < preloadFiltered.size(); i++){
                // Compare each folder
                    j = 0;

                // Get the name of the first folder
                    folderfs = (*(fsFiltered.begin()+j)).c_str();
                    folderPreload = (*(preloadFiltered.begin()+j)).c_str();

                // If the first name of the folder is '/' and the preloadFilteredName == 1 means that the preload is in the root ("/")
                    if ( ((fsFiltered.size() == 1) && ((strcmp ( folderfs.c_str(), "/")) == 0)) &&
                         (preloadFiltered.size() == 1) ){
                        ok = true;
                    }

                // Compare if the folder names are are different!
                    else if (strcmp (folderfs.c_str(), folderPreload.c_str()) != 0){
                        differentMatch = true;
                    }

                    j++;

                // the preload has the name of the file,
                    if (!ok){
                        if (j == preloadFiltered.size() -1){
                            if (j != fsFiltered.size()){
                                differentMatch = true;
                            }
                        }

                        if (j == fsFiltered.size()){
                            if (j != preloadFiltered.size() -1){
                                differentMatch = true;
                            }
                        }
                    }
                }

                if (!differentMatch){
                    preloadResult.push_back((*(preloadFiles.begin())));
                }

                ok = false;
                differentMatch = false;
                preloadFiles.erase(preloadFiles.begin());
        }
    }

    return preloadResult;
}

bool userStorage::hasRemoteFS (vector<fsStructure_T*> fsConfig){


        int fsSize, i;
        fsStructure_T* fs;
        fsSize = fsConfig.size();
        bool remoteFS = false;


        // Preload numEntries files!!!
        for (i=0; (i<fsSize) && (!remoteFS); i++){

            // Load the current path entry
            fs = (*(fsConfig.begin() + i));

            if (fs->fsType.c_str() != FS_LOCAL)
                    remoteFS = true;

        }

        return remoteFS;
}

bool userStorage::hasLocalFS (vector<fsStructure_T*> fsConfig){

    int fsSize, i;
   fsStructure_T* fs;
   fsSize = fsConfig.size();
   bool localFS = false;


   // Preload numEntries files!!!
   for (i=0; (i<fsSize) && (!localFS); i++){

       // Load the current path entry
       fs = (*(fsConfig.begin() + i));

       if (fs->fsType == FS_LOCAL)
               localFS = true;

   }

   return localFS;

}

//preload_T* userStorage::getPreload (string name){
//
//    unsigned int i;
//    vector<preload_T*>::iterator it;
//    bool found = false;
//    preload_T* preload;
//    string subname;
//
//    for (i = 0; ((i < configPreload.size()) && (!found)); i++){
//        it = configPreload.begin() + i;
//
//        //compare size is different..
//        if (name.size() > (*it)->fileName.size()){
//            subname = name.substr(0,(*it)->fileName.size());
//        } else {
//            subname = name;
//        }
//
//        if ((strcmp(subname.c_str(), (*it)->fileName.c_str())) == 0){
//            found = true;
//            preload = (*(it));
//        }
//    }
//
//    if (!found) preload = nullptr;
//
//    return preload;
//
//}
//
//fsStructure_T* userStorage::getFSStructure (string name){
//
//    unsigned int i;
//    vector<fsStructure_T*>::iterator it;
//    bool found = false;
//    fsStructure_T* fs;
//    string subname;
//
//    for (i = 0; ((i < configFS.size()) && (!found)); i++){
//        it = configFS.begin() + i;
//
//        //compare size is different..
//        if (name.size() > (*it)->fsType.size()){
//            subname = name.substr(0,(*it)->fsType.size());
//        } else {
//            subname = name;
//        }
//
//        if ((strcmp(subname.c_str(), (*it)->fsType.c_str())) == 0){
//            found = true;
//            fs = (*(it));
//        }
//    }
//
//    if (!found) fs = nullptr;
//
//    return fs;
//
//}
//
//



} // namespace icancloud
} // namespace inet

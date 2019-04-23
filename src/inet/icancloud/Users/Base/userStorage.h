/**
 * @class userStorage userStorage.cc userStorage.h
 *
 *  This class has control for tenants storage requests.
 *  It has two main parts. A vector with the connections (Address, port) to remote storage servers,
 *  and  the jobs waiting for performing operations by system manager related to set file system parameters
 *  and preload files needed to execute these jobs.
 *
 *  This is an abstract class that AbstractUser inherits from. The abstract methods are:
 *    - virtual void send_request_to_manager(AbstractRequest* req);
 *    - virtual int allocateJob(jobBase* job);
 *
 * @authors Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
 * @date 2012-06-10
 */

#ifndef USERSTORAGE_H_
#define USERSTORAGE_H_

#include "inet/icancloud/Users/Base/userBase.h"

namespace inet {

namespace icancloud {


class userStorage : virtual public userBase {

protected:

    // IPs remote connections to storage
        vector<connection_T*> dir_remote_storage;

        /* Aux queue for management */
        JobQueue* waiting_for_remote_storage_Queue;

protected:


    /*
     * Destructor
     */
    virtual ~userStorage();

    /**
    * Module ending.
    */
    void finish() override;

    /**
    * Module initialization.
    */
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    

    /*
     * This method allocates the job given as parameter into the machine associated to it
     */
    virtual int allocateJob(jobBase* job) = 0;

    /*
     * This method send the request to the manager.
     */
    virtual void send_request_to_manager(AbstractRequest* req) = 0;
    /*
     * This method crates the fs for a job. Creates the entry and the folder structures.
     * In addition is possible to preload files defined inside the job structure.
     * The spId parameter is subprocessId, that is the id of a virtual machine if the job will be executed inside.
    */
    virtual vector<StorageRequest*> createFSforJob (jobBase* job, string opIp, string nodeSetName, int nodeId, int optionalID = -1);

    /*
     * This method returns true if there are any pending request in waiting_for_remote_storage_Queue and
     */
    bool hasPendingStorageRequests(){return !(waiting_for_system_response->get_queue_size() == 0);};

   /*
    * This method generate all the storage requests by an fs configuration and the files that the job needs to execute
    * as precondition (preload).
    */
   vector<StorageRequest*> generateStorageRequests (jobBase* job, string opIp, int pId = -1);

   /*
    * This method returns a vector with the remote paths of the filesystem
    */
   vector<fsStructure_T*> getRemoteFSPaths (vector<fsStructure_T*> fsConfig);

   /*
    * This method returns a vector with the local paths of the filesystem
    */
   vector<fsStructure_T*> getLocalFSPaths (vector<fsStructure_T*> fsConfig);

   /*
    * This method returns the files from preload that matches with the folders defined in iorPaths
    */
   vector <preload_T*> filterPreloadFilesByFS (vector<fsStructure_T*> fsPaths, vector<preload_T*> preloadFiles);

   /*
    * This method analyses the ior_T and return true if it has a remote filesystem defined
    */
   bool hasRemoteFS (vector<fsStructure_T*> fsConfig);

   /*
    * This method analyses the ior_T and return true if it has a local filesystem defined
    */
   bool hasLocalFS (vector<fsStructure_T*> fsConfig);

   /*
   * The method is invoked by user behavior.
   */
   int getPreloadConfiguration_size (){return configPreload.size();};

   /*
   * The method is invoked by user behavior.
   */
   int getFSConfiguration_size (){return configFS.size();};

   /*
    * This method set the user identification (unique) given by omnet when a module is created.
    */
   virtual void setUserToRequest(AbstractRequest* req){req->setUid(this->getId());};

};

} // namespace icancloud
} // namespace inet

#endif /* USERSTORAGE_H_ */

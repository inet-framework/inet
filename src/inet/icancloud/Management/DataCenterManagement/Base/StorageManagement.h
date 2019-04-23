/**
 * @class RequestsManagement RequestsManagement.cc RequestsManagement.h
 *
 * This api is responsible for controlling the data servers and the remote storage. It controls who, what and where
 * are using storage resources.
 *
 * @authors: Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
 * @date 2014-06-06
 */

#ifndef STORAGEMANAGEMENT_H_
#define STORAGEMANAGEMENT_H_

#include "inet/icancloud/Architecture/Node/AbstractNode.h"
#include "inet/icancloud/Base/Request/Request.h"
#include "inet/icancloud/Base/icancloud_Base.h"

namespace inet {

namespace icancloud {


class StorageManagement : virtual public icancloud_Base {

protected:

    /*
     *  This second level is to manage virtualization. If the operation came from a not virtualized
     *  environment spId will be equal to pId.
     */

        struct subprocessOperations_t{
            int spId;
            int operation;
           vector<connection_T*> pendingOperation;
           int numberOfConnections;
        };
        typedef struct subprocessOperations_t subprocessOperations;

        struct processOperations_t{
            int pId;
            vector<subprocessOperations*> pendingOperation;
        };
        typedef struct processOperations_t processOperations;

        struct pendingStorageRequest_t{
            int uId;
            vector <processOperations*> processOperation;
        };
        typedef struct pendingStorageRequest_t PendingStorageRequest;

      // Struct for delete fs from a user
          struct PendingRemoteStorageDeletion{
              int uId;
              int pId;
              int remoteStorageQuantity;
          };

      // Struct for waiting to close network connections
          struct PendingConnectionDeletion{
              int uId;
              int pId;
              int connectionsQuantity;
          };

protected:
      /** This vector allocates the storage requests for create FS or files until they will be realized*/
      vector <PendingStorageRequest*> pendingStorageRequests;

      /** This vector allocates the storage requests until it will be performed*/
      vector <PendingRemoteStorageDeletion*> pendingRemoteStorageDeletion;

      /* this vector allocates the connections until it will be performed */
      vector<PendingConnectionDeletion*> connectionsDeletion;

      // The number of Parallel file system remote servers (from .ned parameter)
      int numberOfPFSRemoteServers;

    virtual ~StorageManagement();

    /*
    * Module initialization
    */
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    

    /**
    * Module ending.
    */
    virtual void finish() override;


    /********************************************************************************************
     *              Operations for managing the storage of the system
     *********************************************************************************************/

    /*
   *  Create the filesystem and load a set of files (if there exists any) included into the the request.
   *  All the parameters are included into the request. The destination node, the files and the structure of the file system
   */
    void manageStorageRequest(StorageRequest* st_req, AbstractNode* nodeHost, vector<AbstractNode*> nodesTarget);

    /*
     * This method creates the connections between node host and remote data storage, and also creates the
     * structure of the fs and preload the files given as parameters at remote nodes fs.
     */
    vector<connection_T*> createConnectionToStorage(StorageRequest* st_req, AbstractNode* nodeHost,vector<AbstractNode*> nodesTarget);

    /*
     * The parameter turnOffNode=true switch off the node if it has not vms allocated on it
     * turnOffNode = false, changes the state to idle otherwise
     */
    void formatFSFromNodes (vector<AbstractNode*> nodes, int uId, int pId, bool turnOffNode);

    /*
     * This method is invoked by manager base when remote storage connections were done
     */
    void connection_realized(StorageRequest* attendeed_req);


public:

    /*
     * It notify to the manager any event
     */
    virtual void notifyManager(Packet *);

    /*
     * This method returns the quantity of remote servers that will be used when a virtual machine request resources.
     */
    int getNumberOfPFSRemoteServers(){return numberOfPFSRemoteServers;};

};

} // namespace icancloud
} // namespace inet

#endif /* STORAGEMANAGEMENT_H */

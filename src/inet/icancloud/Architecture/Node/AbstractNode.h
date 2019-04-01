//
// Abstract class that inherits from Machine.
// It implements the base to Nodes from a Data center
//
// This class models Computing node.
//
// @author Gabriel González Castañé
// @date 2014-12-12

#ifndef _ABSTRACTNODE_H_
#define _ABSTRACTNODE_H_

#include "inet/icancloud/Architecture/Machine/Machine.h"
#include "inet/icancloud/Architecture/NodeComponents/OperatingSystems/SyscallManager/NodeSyscallManager/SyscallManager.h"

#define INITIAL_STATE "off"

namespace inet {

namespace icancloud {


class AbstractNode : virtual public Machine{

protected:

    int storageLocalPort;                              // the port to accept connections to access to remote data


    /*
     * Destructor
     */
    virtual ~AbstractNode();

    /*
     * Method that initialize the module
     */
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    

    /*
     * Method that finalize the module
     */
    virtual void finish() override;

    /*
     * This method initialize a pointer to the syscall manager to receive the results of some operations.
     */
    virtual void initialize_syscallManager(int localPort);


public:

    // Inith the node main parameters
    virtual void initNode() =0;

    // -----------------------------------------------------------------------/
    // ----------------- Operations with states ------------------------------/
    // -----------------------------------------------------------------------/

    bool isON(){return (getState() != MACHINE_STATE_OFF);};   // This method returns the state attribute
    bool isOFF(){return (getState() == MACHINE_STATE_OFF);};   // This method returns the state attribute

    // -----------------------------------------------------------------------/
    // ----------------- Operations with applications ------------------------/
    // -----------------------------------------------------------------------/
    /*
     * Setter for manager
     */
    virtual void setManager(icancloud_Base* manager) override {managerPtr = manager;};

    void freeResources(){ if (getNumProcessesRunning() == 0) changeState (MACHINE_STATE_IDLE);}

    virtual void notifyManager(Packet * msg) = 0;

    /*******************************************************************************
     *                          Operations with storage
     *******************************************************************************/

    /*
     * This method returns the port used by the cloud to create the connections between compute nodes and data storage nodes.
     */
    int getStoragePort (){return storageLocalPort;};

    /*
     * This method allow the connection from a compute node to a storage node. It is invoked by the cloud manager and the responsible
     * of the data storage connection is the remote storage app allocated at target (data storage) node.
     */
    void connectToRemoteStorage (vector<string> destinationIPs, string fsType, int uId, int pId, string ipFrom, int jobId){os->connectToRemoteStorage (destinationIPs, fsType, storageLocalPort, uId, pId, ipFrom.c_str(), jobId);};

    /*
     * This method creates a listen connection in the data storage node, in order to wait until the compute node (which hosts
     * the vm) completes the connection. Host node requests a 'remote storage connection' against the storageLocalPort of the storage node.
     */
    void createDataStorageListen (int uId, int pId){os->createDataStorageListen (uId, pId);};

    /*
     * This method load remote files and configure the filesystems for the account of the user into the remote storage nodes selected by the scheduler of the cloud manager
     */
    void setRemoteFiles ( int uId, int pId, int spId, string ipFrom, vector<preload_T*> filesToPreload, vector<fsStructure_T*> fsPaths){os->setRemoteFiles(uId,pId,spId, ipFrom, filesToPreload, fsPaths);};

    /*
     * This method delete the fs and the files from the remote storage nodes that have allocated files from him.
     */
    void deleteUserFSFiles( int uId, int pId){os->deleteUserFSFiles(uId, pId);};

    /*
     * This method create files into the local filesystem of the node.
     */
    void setLocalFiles (int uId, int pId,int spId, string ipFrom, vector<preload_T*> filesToPreload, vector<fsStructure_T*> fsPaths){os->createLocalFiles(uId,pId,spId, ipFrom, filesToPreload, fsPaths);};

    /*
     * This method close a connection from the user id = uId of the process pId
     */
    void closeConnections (int uId, int pId);

};


} // namespace icancloud
} // namespace inet

#endif /* _ABSTRACTNODE_H_ */

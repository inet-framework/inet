/*
 * @class AbstractRequest AbstractRequest.h "AbstractRequest.h"
 *
 * This abstract class define and manages the main methods and attributes for a tenant resources request
 * it offers methods for:
 *      managing single vm request
 *      analyzing the resources requested
 * @author Gabriel Gonzalez Castane
 * @date 2013-05-04
 */

#ifndef REQUEST_H_
#define REQUEST_H_

#include <omnetpp.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <string>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <stdexcept>
#include <math.h>
#include "stdio.h"
#include "stdlib.h"
#include "inet/icancloud/Base/include/Constants.h"
#include "inet/icancloud/Base/include/icancloud_types.h"

namespace inet {

namespace icancloud {


using std::vector;
using std::string;

class AbstractRequest {

protected:

    int operation;
    int timesEnqueue;
    int state;

    int userId;
    int vmId;
    int jobId;

    string ip;

public:
    AbstractRequest();
    virtual ~AbstractRequest();

    void setOperation(int op){operation = op;};
    int getOperation(){return operation;};

    void setState (int new_state){state = new_state;};
    int getState (){return state;};

    void incrementTimesEnqueue(){timesEnqueue++;};
    int getTimesEnqueue(){return timesEnqueue;};
    void setTimesEnqueue(int times){timesEnqueue=times;}

    void setUid(int uid){userId = uid;};
    int getUid(){return userId;};

    void setPid(int jId){vmId = jId;};
    int getPid(){return vmId;};

    void setSPid(int jId){jobId = jId;};
    int getSPid(){return jobId;};


    string getIp() {return ip;};
    void setIp(string ip_) {ip = ip_;};

    virtual AbstractRequest* dup () = 0;
    virtual bool compareReq(AbstractRequest* req) = 0;

};


class RequestBase: public AbstractRequest {

public:

    virtual ~RequestBase();

    AbstractRequest* dup ();
    bool compareReq(AbstractRequest* req);

};

class StorageRequest: public AbstractRequest{

protected:

    /** fs is used by user to define the type of FileSystem for a remote storage request */
    string fs;

    /** Preload files for the target nodes */
    vector<preload_T*> preloadFiles_v;

    /** File system paths for the target nodes */
    vector<fsStructure_T*> fsPaths;

    /** Connections for remote storage*/
    vector<connection_T*> connections;

    /** These parameters are to ease the task of localize the resources where will be operate */
    string nodeSetId;
    int nodeId;

public:

    /*
     * Constructor
     */
    StorageRequest();

    /*
     *Destructor
     */
    virtual ~StorageRequest();

    /*
     *  Duplicate the storage response request
     */
    virtual AbstractRequest* dup ();

    /*
     * Comparation between two requests
     */
    virtual bool compareReq(AbstractRequest* req);

    /**
     *
     *------------------------- Storage requests methods---------------------------
     *
     **/

    string getNodeSetId(){return nodeSetId;};
    int getNodeId(){return nodeId;};
    void setNodeSetId(const string & newId){ nodeSetId = newId;};
    void setNodeId(int newId){ nodeId = newId;};

    // Set the fs type
        void setFsType(const string &fsType){fs = fsType;};

    // Returns the fs type
        string getFsType(){return fs;};

        int getPreloadSize (){return preloadFiles_v.size();};
        preload_T* getPreload (int index){return (*(preloadFiles_v.begin()+index));};
        void setPreloadSet (vector<preload_T*> data);
        void erasePreload(int index){preloadFiles_v.erase(preloadFiles_v.begin()+index);};
        vector<preload_T*> getPreloadFilesSet (){return preloadFiles_v;};
        double getTotalFilesSize();

        int getFSPathsSize (){return fsPaths.size();};
        fsStructure_T* getFSPath(int index){return (*(fsPaths.begin()+index));};
        void setFSPathSet (vector<fsStructure_T*> data){for (int i = 0; i < ((int)data.size()); i++){fsPaths.push_back(*(data.begin()+i));}};
        void eraseFSPath(int index){fsPaths.erase(fsPaths.begin()+index);};
        vector<fsStructure_T*> getFSSet (){return fsPaths;};

        //  set the storage connections
            void setConnectionVector (vector<connection_T*> con){for (int i = 0; i < ((int)connections.size()); i++){con.push_back(*(con.begin()+i));}};;

        // Set the connection at position given by index
            void setConnection(connection_T* connection, int index = -1);

        // returns the connection vector
            vector<connection_T*> getConnections(){return connections;};

        // returns the connection vector at index position
            connection_T* getConnection(int index){return (*(connections.begin()+index));};

        // Returns the quantity of storageResponses at the request
            int getConnectionSize(){return connections.size();};

};

class PhysicalResourcesRequest : public AbstractRequest {

protected:

    // If the node is defined by nodeType
    int nodeSet;
    int nodeIndex;

    // If the node has to be selected by scheduler due to parameters
    int cores;
    double memory;
    double storage;

    // This message is for book resources
    int messageID;

    // This vector is used for allocate the storage requests of a job
    vector<StorageRequest*> storageRequests;

public:
    /*
     * Methods
     */
    PhysicalResourcesRequest();
    PhysicalResourcesRequest(PhysicalResourcesRequest* request);
    PhysicalResourcesRequest(StorageRequest* request);
    PhysicalResourcesRequest(RequestBase* request);
    virtual ~PhysicalResourcesRequest();

    void setSet(int set){nodeSet = set;};
    int getSet(){return nodeSet;};

    void setIndex(int index){nodeIndex = index;};
    int getIndex(){return nodeIndex;};

    void setCores(int quantity){cores = quantity;};
    int getCores(){return cores;};

    void setMemory(double quantity){memory = quantity;};
    double getMemory(){return memory;};

    void setStorage(double quantity){storage = quantity;};
    double getStorage(){return storage;};

    void setMessageID(int msgID){messageID = msgID;};
    int getMessageID(){return messageID;};

    int getStorageRequestsSize(){return storageRequests.size();};
    StorageRequest* getStorageRequest(int index){return (*(storageRequests.begin() + index));};
    void setStorageRequest(StorageRequest* stReq){storageRequests.push_back(stReq);};

    virtual AbstractRequest* dup ();
    virtual bool compareReq(AbstractRequest* req);

};

} // namespace icancloud
} // namespace inet

#endif /* REQUEST_H_ */

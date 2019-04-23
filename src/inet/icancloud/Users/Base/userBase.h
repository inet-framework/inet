
/**
 * @class userBase userBase.cc userBase.h
 *
 *  This module is the interface between the user generator and the user profile.
 *  It contains the main methods and attributes that defines a user.
 *
 * @authors Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
 * @date 2012-06-10
 */

#ifndef USERBASE_H_
#define USERBASE_H_


#include "queuesManager.h"
#include "inet/icancloud/Base/Parser/cfgMPI.h"
#include "inet/icancloud/Management/DataCenterManagement/Base/RequestsManagement.h"
#include "inet/icancloud/Applications/Base/jobBase.h"

namespace inet {

namespace icancloud {


class userBase :virtual public queuesManager {

    friend class jobBase;

protected:
    /** The user ID. Is the name given by the user in the configuration **/
        string userName;
        int userID;

    /** File system parameters */
        vector <preload_T*> configPreload;
        vector <fsStructure_T*> configFS;
        string fsType;

    /** MPI configuration parsed parameters */
        CfgMPI* configMPI;

    /** A pointer to the system manager */
        RequestsManagement* managerPtr;

    /** Structure for control the pending requests */
        vector<AbstractRequest*> pending_requests;

    /** Aux queue for management */
        JobQueue* waiting_for_system_response;

        void abandonSystem();

public:

    virtual ~userBase();

    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    
    void finish() override;

    // ---------------------------------------------------------------------------
    // ---------------- METHODS BE USED BY USER GENERATOR -------------
    // ---------------------------------------------------------------------------

    /*
     * This method add a job to jobList when it is parsed from the parameters defined in omnetpp.ini
     */
    virtual void addParsedJob (jobBase *job){};

    /*
     * The method is invoked by userGeneratorCell.
     */
    void setPreloadConfiguration (preload_T *element){configPreload.push_back(element);};

    /*
     * The method is invoked by userGeneratorCell.
     */
    void setFSConfiguration (fsStructure_T *element){configFS.push_back(element);};

    /*
     * The method is invoked by userGeneratorCell.
     */
    void setFSType (string value){fsType = value;};

    /*
     * The method is invoked by userGeneratorCell.
     */
    string getFSType (){return fsType;};

    /*
     * This method set the userName
     */
    void setUserName (string newName){userName = newName;};

    /*
     * The method returns the parameter userName.
     */
    string getUserName (){return userName;};

    /*
     * The method returns the parameter userName.
     */
    int getUserId (){return userID;};


    /*
     * This method is used by MPI_Base.cc to get the values of the processes mpi to simulate an app in mpi environments
     */
    CfgMPI* getMPIEnv(){return configMPI;};

    /*
     * This method send the request to the system manager
     */
    virtual void sendRequestToSystemManager (AbstractRequest* request);

    /*
     * This method is invoked to erase the request from pending requests vector
     */
    void requestArrival(AbstractRequest* request);

    /*
     * This method is invoked to erase the request and move the job to the waiting queue from pending requests vector
     */
    void requestArrivalError(AbstractRequest* request);

    /*
     * This method return if a user has pending requests to be send to the system manager
     */
    virtual bool hasPendingRequests(){return (int)(pending_requests.size())>0;};
};

} // namespace icancloud
} // namespace inet

#endif /* USERGENERATORINTERFACE_H_ */

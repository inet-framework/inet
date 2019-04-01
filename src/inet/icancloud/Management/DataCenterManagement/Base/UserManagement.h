/**
 * @class UserManagement UserManagement.cc UserManagement.h
 *
 * This class is responsible for managing the tenants that appear at system.
 *
 * @authors: Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
 * @date 2014-06-06
 */

#ifndef USERMANAGEMENT_H_
#define USERMANAGEMENT_H_

#include "inet/icancloud/Base/icancloud_Base.h"
#include "inet/icancloud/Users/UserGenerator/core/AbstractUserGenerator.h"

namespace inet {

namespace icancloud {


class AbstractUser;
class AbstractUserGenerator;

class UserManagement : virtual public icancloud_Base {

protected:

    // Attributes to manage the booked resources of each user
    struct computeResources{
        string nodeSet;
        int node;
        int cores;
        int jobID;
    };

    /** type definition for managing tenants*/
    struct tenant{
        AbstractUser* userPtr;
        vector<computeResources*> jobResources;
    };
    typedef struct tenant tenant_T;

    /** Structure to manage the users */
        vector <tenant_T*> userList;


    /** If the system does not accept more users */
        AbstractUserGenerator* userGenerator;
        bool no_more_users;
        bool simulationPerTime;

protected:

    /*
     * Destructor
     */
    virtual ~UserManagement();


    /*
     * This method returns the user allocated at users list with the module id number
     * equal thant the parameter (if it is)
     *
     */
    AbstractUser* getUserByModuleID(int modID);



    // ---------------------- Interact with users ------------------------------------
public:
    /*
     * This method is called by the userGenerator for adding the created users to the  environment
     */
    virtual bool newUser (AbstractUser *user);

protected:
    /*
     * This method returns a user in the vector usersList with the ID == name;
     * @Param: The userID.
     */
    AbstractUser* getUserById (int uid);

    /*
     * This method returns a user in the vector usersList at the "userIndex" position.
     * @Param: The index of the user in the vector.
     */
    AbstractUser* getUserByIndex (int userIndex);

    /*********************************************************************************
     * System Manger management of node resources used by tenant
     *********************************************************************************/

    /*
     * this method free the compute resources that the user is using for the jobID.
     */
    void freeUserComputeEntry(int uid, int jobID);

    /*
     * This method will be defined at dc manager to access to the node resources and unbook them
     */
    virtual void freeComputeResources(string nodeSet, int nodeIndex, int cores) = 0;

    /*
     * This method creates an entry (but it does not decrement the node resources) with the
     * information of a job computing reservation.
     */
    void createUserComputeEntry(int uid, string nodeSet, int node, int cores, int jobID);

    /*********************************************************************************
     * Inherited methods from hpc_base
     *********************************************************************************/

    /*
     * Module initialization
     */
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
     

     /**
     * Module ending.
     */
     virtual void finish() override;

    /*****************************************************************
    *                    Management of the users
    *****************************************************************/

    /*
     * This method returns the user position in the vector usersList with the module unique identifier.
     * @Param: The int that identifies the object.
     */
    int getUserPositionByUserID (int uid);

    /*
     * This method returns a user by a given user id.
     */
    AbstractUser* getUserByID(int uId);

    /*
     * Virtual method to be implemented at manager that has finish and clean memory and the structures.
     */
    virtual void finalizeManager() = 0;

    /*
     * This method checks if there is no users executing and it the manager does not wait for more users (no_more_users = true)
     */
    bool checkFinalization();

public:

    /*
     * Removes the user form the queueof users
     */
    virtual void deleteUser (int userId);

    /*
     * If allowToExecute = true means that the execution will be performed until the last user finish
     * allowToExecute = false means that the execution has finished the time and it is forced to finish
     */
    void finalizeUserGenerator(AbstractUserGenerator *userGen,bool allowToExecute);

    /*
     * This method returns if simulation is by time or by a quantity of tenants.
     */
    bool isSimulationPerTime(){return simulationPerTime;};
};

} // namespace icancloud
} // namespace inet

#endif /* USERMANAGEMENT_H_ */

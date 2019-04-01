
/**
 * @class interface AllocationManagement AllocationManagement.h AllocationManagement.cc
 *
 * This class is responsible for allocating the resources requested by users into storage nodes while
 * it controls where are the remote storage from each VM
 *
 * @authors: Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
 */

#ifndef ALLOCATION_MANAGEMENT_H_
#define ALLOCATION_MANAGEMENT_H_

#include "inet/icancloud/Management/DataCenterManagement/AbstractDCManager.h"
#include "inet/icancloud/Base/VMID/VMID.h"

namespace inet {

namespace icancloud {


class AllocationManagement : virtual public AbstractDCManager{

	protected:


    // Storage  nodes structure. To control the resources
        // Structure to control the resources of the remote storage.
        struct nodeAllocation{
           AbstractNode* storageNode;
           vector<VMID*> vms;
        };

    //  Nodes occupation is the vector of all the remote storage.-
        vector <nodeAllocation*> storageNodesOccupation;

    // Users
        // This structure is to find the vm data easily
             struct vmAllocation{
                vector<int> storageNodePosition;
                VMID* vm;
             };

         // this structure is to find the user resources easilly
             struct userVMAllocation_t{
                vector<vmAllocation*> vmResourcesInfo;
                int user;
             };

             typedef userVMAllocation_t userVMAllocation;

         // Users vector
             vector <userVMAllocation*> userVmData;


     /*
      * Destructor
      */
     virtual ~AllocationManagement();

     /*
      * Module Initialization
      */
     virtual void initialize(int stage) override;
     virtual int numInitStages() const override { return NUM_INIT_STAGES; }
      

     /*
      * Module finalization
      */
     void finish() override;

     /*
      * Initializes the control structure for storage nodes.
      */
     void setupStorageNodes();

     /*
      * This method is invoked by setup storage nodes to set a storage node into the structures.
      */
     void setStorageNode(AbstractNode* stNode);

     /*
      * This method returns the quantity of storage nodes at system controlled by system manager
      */
     int getStorageNodesSize();

     /*
      * This method returns an storage node allocated at control structure at position idx given as parameter
      */
     AbstractNode* getStorageNode(int idx);

      /*
       * This method update allocates where a process or vm with id = vmPid of a user with id userID
       * has assigned its remote storage resources.
       */
     void updateVmManagementStructures (vector<int> selected_nodes, int vmPid, int userID);

     /*
      * This method returns the position at structures of a vm own to user with id uid
      */
     int searchUserVMAllocation(int uid);

     /*
      * This method returns a set of nodes that allocates the remote storage of process or VM (id = pId) from a
      *  user (uidIdx)
      */

     vector<AbstractNode*> getStorageNodesOfVM(int userIdx, int pId);

     /*
      * This method delete the process (VM) from the control structures.
      */
     vector<AbstractNode*> deleteVMfromStorageNodes(int userIdx, int pId);

     /*
      * This method returns where a process is allocated.
      */
     AbstractNode* getVmAllocation (int uId,int pId);

};

} // namespace icancloud
} // namespace inet

#endif

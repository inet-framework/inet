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

#include "inet/icancloud/Management/CloudManagement/Base/AllocationManagement.h"

namespace inet {

namespace icancloud {



AllocationManagement::~AllocationManagement(){

}

void AllocationManagement::initialize(int stage) {
    AbstractDCManager::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        storageNodesOccupation.clear();
        userVmData.clear();
    }
}

void AllocationManagement::finish(){

    storageNodesOccupation.clear();
    userVmData.clear();

 AbstractDCManager::finish();

}

void AllocationManagement::setupStorageNodes(){
    int storageNodeMapSize;
    int storageNodeSetSize;
    int i,j;
    AbstractNode* node;

    //Load the storage nodes to the structure of the scheduler
        storageNodeMapSize = getStorageMapSize();

    // build the set of storage nodes to know where allocate the vms
    for (i = 0; i < storageNodeMapSize; i++){
        storageNodeSetSize = getSetSize(i, true);
        for (j = 0; j < storageNodeSetSize; j++){
            node = getNodeByIndex(i,j, true);
            setStorageNode(node);
        }
    }
}

void AllocationManagement::setStorageNode(AbstractNode* stNode){
    nodeAllocation* al = new nodeAllocation();
    al->storageNode = stNode;
    al->vms.clear();
    storageNodesOccupation.push_back(al);
}

int AllocationManagement::getStorageNodesSize(){
    return (storageNodesOccupation.size());
}

AbstractNode* AllocationManagement::getStorageNode(int idx){
    return (*(storageNodesOccupation.begin() + idx))->storageNode;
}

void AllocationManagement::updateVmManagementStructures (vector<int> selected_nodes, int vmPid, int userID){
    // Define ..
        vector<AbstractNode*> nodes;
        nodeAllocation* initialNode;
        VMID* vmID;
        unsigned int i;
        int position;
        vmAllocation* vmAl;
        userVMAllocation* userAl;

    // Initialize ..
        nodes.clear();

// Update User structure
    // Insert into the vm structure
    if (selected_nodes.size() != 0){

        position = searchUserVMAllocation(userID);

        // If not found create it
        if (position == -1){
            userAl = new userVMAllocation();
            userAl->user = userID;
            userAl->vmResourcesInfo.clear();
            // Insert the user into the structure
            userVmData.push_back(userAl);
        } else {
            userAl = (*(userVmData.begin() + position));
        }

        // Create the entry ..
        vmAl = new vmAllocation();
        vmID = new VMID();
        vmID->initialize(userID, vmPid);

        vmAl->vm = vmID;

        // Load the nodes ..
        for (i = 0; i < selected_nodes.size();i++){
            vmAl->storageNodePosition.push_back(*(selected_nodes.begin() +i));
        }

        userAl->vmResourcesInfo.push_back(vmAl);

    }
// Update nodes structure
    // Insert into the nodes structure
    while (selected_nodes.size() > 0){
        // Get the node structure
            initialNode = *(storageNodesOccupation.begin() + (*(selected_nodes.begin())));

        // insert the vms name into the node
            vmID = new VMID();

            vmID->initialize(userID, vmPid);

            initialNode->vms.push_back(vmID);

            selected_nodes.erase(selected_nodes.begin());
    }

}

int AllocationManagement::searchUserVMAllocation(int uid){
    int position = -1;
    bool found = false;


    // Search the user entry if exists
      for (int i = 0; (i < (int)userVmData.size()) && (!found); i++){

          if (uid == (*(userVmData.begin() + i))->user){
              position = i;
              found = true;
          }
      }

      return position;

}


vector<AbstractNode*> AllocationManagement::getStorageNodesOfVM(int userIdx, int pId){

   userVMAllocation* userAl;
   vmAllocation* vmAl;

   bool vmFound;

   AbstractNode* node;
   vector<AbstractNode*> nodes;
   unsigned int i,k,j;


       // Init ..
           vmFound = false;
           nodes.clear();
           userAl = (*(userVmData.begin() + userIdx));

    // Search into the entries for the vms..
       for (i = 0; (i < userAl->vmResourcesInfo.size()) && (!vmFound);){
           vmAl = (*(userAl->vmResourcesInfo.begin()));

           // I found the vm, now get the storage nodes indexes to free the resources!
           if (vmAl->vm->getVMID() == pId){

               vmFound = true;

               // Get the positions of each node
               for (j = 0; j < vmAl->storageNodePosition.size(); j++){

                   // Get the position of the node
                   k = (*(vmAl->storageNodePosition.begin()+j));

                   // Obtain the node to delete the filesystems
                   node = (*(storageNodesOccupation.begin()+k))->storageNode;

                   // Insert into the vector of nodes to delete all the fs
                   nodes.push_back(node);

               }
           }
      }
      return nodes;
}

vector<AbstractNode*> AllocationManagement::deleteVMfromStorageNodes(int userIdx, int pId){

   userVMAllocation* userAl;
   vmAllocation* vmAl;

   bool vmFound;

   AbstractNode* node;
   vector<AbstractNode*> nodes;
   unsigned int i,k,j;


       // Init ..
           vmFound = false;
           nodes.clear();
           userAl = (*(userVmData.begin() + userIdx));

    // Search into the entries for the vms..
       for (i = 0; (i < userAl->vmResourcesInfo.size()) && (!vmFound);){
           vmAl = (*(userAl->vmResourcesInfo.begin()));

           // I found the vm, now get the storage nodes indexes to free the resources!
           if (vmAl->vm->getVMID() == pId){

               vmFound = true;

               // Get the positions of each node
               for (j = 0; j < vmAl->storageNodePosition.size(); j++){

                   // Get the position of the node
                   k = (*(vmAl->storageNodePosition.begin()+j));

                   // Obtain the node to delete the filesystems
                   node = (*(storageNodesOccupation.begin()+k))->storageNode;

                   // Insert into the vector of nodes to delete all the fs
                   nodes.push_back(node);

               }

           }

           //Delete the vm pointers to the nodes from the user ..
               vmAl->storageNodePosition.erase( vmAl->storageNodePosition.begin());
       }

       // If there are no more vms with remote storage from the user, delete the user!
        userAl->vmResourcesInfo.clear();
        userVmData.erase(userVmData.begin() + userIdx);


       return nodes;
}


AbstractNode* AllocationManagement::getVmAllocation (int uId, int pId){
    // Define ..
        vmAllocation* vmAl;
        userVMAllocation* userAl;
        bool vmFound;
        unsigned int i,j;
        AbstractNode* node;

    // Init ..
        vmFound = false;

    // Search into the entries for the vms..
        for (i = 0; (i < userVmData.size()) && (!vmFound);i++){
            userAl = (*(userVmData.begin()+i));

            if (userAl->user == uId){
                for (j = 0; (j < userAl->vmResourcesInfo.size()) && (!vmFound);j++){
                    vmAl = (*(userAl->vmResourcesInfo.begin()+j));

                    // I found the vm, now get the storage nodes indexes to free the resources!
                        if (vmAl->vm->getVMID() == pId){

                            vmFound = true;
                            // Get the allocation
                           node = getNodeByIndex(vmAl->vm->getVM()->getNodeSetName(), vmAl->vm->getVM()->getNodeName());

                        }

                }
            }


        }

        return node;
}

} // namespace icancloud
} // namespace inet

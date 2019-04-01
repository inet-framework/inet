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

#include "inet/icancloud/Management/DataCenterManagement/Base/UserManagement.h"

namespace inet {

namespace icancloud {


UserManagement::~UserManagement() {
    // TODO Auto-generated destructor stub
}

void UserManagement::initialize(int stage) {
    icancloud_Base::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        userList.clear();
        no_more_users = false;
        simulationPerTime = false;

    }
}


void UserManagement::finish(){

    userList.clear();
    icancloud_Base::finish();
}



AbstractUser* UserManagement::getUserByModuleID(int modID){
    vector <tenant_T*>::iterator userIt;
    AbstractUser* userFound;
    bool found;
    int modIDAux;

    found = false;
    userFound = nullptr;

    for (userIt = userList.begin(); (userIt < userList.end()) && (!found); userIt++){

        modIDAux = (*userIt)->userPtr->getId();
            if (modID == modIDAux){
                found = true;
                userFound = (*userIt)->userPtr;
            }
    }

    if (!found) throw cRuntimeError ("UserManagement::getUserByModuleID does not found the modID: %i\n", modID);

    return userFound;
}




// ----------------- Interact with users ----------------- //

bool UserManagement::newUser (AbstractUser *user){

    tenant_T* newUser;

    bool admitted = false;

    // Configure users..
      if (!no_more_users){
          newUser = new tenant_T();
          newUser->userPtr = user;
          newUser->jobResources.clear();

        userList.push_back(newUser);
        admitted = true;
        user->startUser();
      }

    return admitted;

}



AbstractUser* UserManagement::getUserById (int uid){

    vector <tenant_T*>::iterator userIt;
    AbstractUser* user;
    bool found = false;
    user = nullptr;

    for (userIt = userList.begin(); (userIt < userList.end()) && (!found);userIt++ ){
        if ((*userIt)->userPtr->getUserId() == uid){
            user = (*userIt)->userPtr;
            found = true;
        }
    }

    return user;

}

AbstractUser* UserManagement::getUserByIndex (int userIndex){

    return userList[userIndex]->userPtr;

}

void UserManagement::deleteUser (int userId){

    // Define ..
        vector <tenant_T*>::iterator userIt;
 //       vector <AbstractRequest*>::iterator reqIt;
        AbstractUser *user;
        int userFound;
        bool found;

    // Init ..
        found = false;

    // Delete from the userList;
        for (userIt = userList.begin(); (userIt < userList.end()) && (!found); userIt++){
            user = (*userIt)->userPtr;
            userFound = user->getUserId();
            if (userFound ==  userId){
                userList.erase(userIt);
                delete(user);
                found = true;
            }
        }
}

void UserManagement::finalizeUserGenerator(AbstractUserGenerator *userGen, bool allowToExecute){

    userGenerator = userGen;
    userGenerator->callFinish();

    no_more_users = true;
    simulationPerTime = allowToExecute;

    Enter_Method_Silent();
    cMessage* msg = new cMessage("deleteUserGen");
    scheduleAt (simTime(), msg);

}

bool UserManagement::checkFinalization(){

    return ((no_more_users) && (userList.size() == 0));
}

void UserManagement::freeUserComputeEntry(int uid, int jobID){
    // Restore the cores value..

    // Begin ..
        tenant* user;
        int i ;
        bool found = false;

    // Initialize. ..
        for (i = 0; (i < (int)userList.size()) && (!found); i++){
            user = (*(userList.begin() + i));
            if (user->userPtr->getId() == uid){
                found = true;
            }
        }



    if (!found){
       throw cRuntimeError ("An inexistent user is creating an userComputeEntry (UserManagement). userId:%i\n", uid);
    }
    // Creates only the new entry
    else{
        found = false;
        for (i = 0; i < (int)(user->jobResources.size()) && (!found); i++){
            if ((*(user->jobResources.begin()+i))->jobID == jobID ){

                found = true;

                freeComputeResources((*(user->jobResources.begin()+i))->nodeSet, (*(user->jobResources.begin()+i))->node, (*(user->jobResources.begin()+i))->cores );

                user->jobResources.erase(user->jobResources.begin()+i);
            }
        }
    }
}

void UserManagement::createUserComputeEntry(int uid, string nodeSet, int node, int cores, int jobID){
    tenant* user;
    computeResources* resources;
    int i;
    bool found = false;

// Initialize. ..
    for (i = 0; (i < (int)userList.size()) && (!found); i++){
        user = (*(userList.begin() + i));
        if (user->userPtr->getId() == uid){
            found = true;
        }
    }

    if (!found){
        throw cRuntimeError ("An inexistent user is creating an userComputeEntry (UserManagement). userId:%i\n", uid);
    }
    // Creates only the new entry
    else{
        resources = new computeResources;
        resources->nodeSet = nodeSet;
        resources->node = node;
        resources->cores = cores;
        resources->jobID = jobID;
        user->jobResources.push_back(resources);
    }

}

int UserManagement::getUserPositionByUserID (int uid){

        vector <tenant_T*>::iterator userIt;
        int userPosition = -1;
        bool found = false;
        int counter = 0;

        for (userIt = userList.begin(); (userIt < userList.end()) && (!found);userIt++ ){
            if ((*userIt)->userPtr->getId() == uid)
                found = true;
             else
                counter++;

        }

        return userPosition;
}

AbstractUser* UserManagement::getUserByID (int uId){

        vector <tenant_T*>::iterator userIt;
        bool found = false;
        int counter = 0;
        AbstractUser* user;

        for (userIt = userList.begin(); (userIt < userList.end()) && (!found);userIt++ ){
            if ((*userIt)->userPtr->getId() == uId){
                found = true;
                user = (*userIt)->userPtr;
            }
             else
                counter++;

        }

        return user;
}


} // namespace icancloud
} // namespace inet

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

#include "inet/icancloud/Applications/Base/UserJob.h"

namespace inet {

namespace icancloud {


UserJob::~UserJob() {
    appType = "";
    mPtr =nullptr;
    fsStructures.clear();
}


 void UserJob::initialize(int stage) {
    jobBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        appType = "";
        mPtr = nullptr;
        fsStructures.clear();
        preloadFiles.clear();
        jobResults = new JobResultsSet();
    }
}

 void UserJob::startExecution(){
     API_OS::startExecution();
 }

 void UserJob::finish(){

     fsStructures.clear();
     preloadFiles.clear();

     jobBase::finish();

 }

 UserJob* UserJob::cloneJob (cModule* userMod){

     // Define ...
         cModule *cloneApp;
         AbstractUser* user;
         UserJob* newJob;
         cModuleType *modType;
         std::ostringstream appPath;
         int i, numParameters, size;

     // Init ..
         appPath << this->getNedTypeName();

     // Create the app module
         modType = cModuleType::get (appPath.str().c_str());

     // Create the app into the user module
         cloneApp = modType->create(appPath.str().c_str(), userMod);

     // Configure the main parameters
         numParameters = this->getNumParams();
         for (i = 0; i < numParameters ; i++){
             cloneApp->par(i) = this->par(i);
         }

         cloneApp->setName("app");

     // Finalize and build the module
         cloneApp->finalizeParameters();
         cloneApp->buildInside();

         // Call initiialize
         cloneApp->callInitialize();

         newJob = check_and_cast<UserJob*> (cloneApp);
         newJob->setOriginalName(this->getOriginalName().c_str());
         newJob->setAppType(this->getAppType());

         user =  check_and_cast <AbstractUser*>  (userMod);
         newJob->setUpUser(user);

         size = this->getPreloadSize();
         for (i = 0; i < size; i++){
             newJob->setPreloadFile(this->getPreloadFile(i));
         }

         size = this->getFSSize();
         for (i = 0; i < size; i++){
             newJob->setFSElement(this->getFSElement(i));
         }

         return newJob;
 }


void UserJob::setMachine (Machine* m){
    mPtr = m;
    jobResults->setMachineType(mPtr->getTypeName());
}

void UserJob::setUpUser (AbstractUser *user){

    userPtr = user;
}

} // namespace icancloud
} // namespace inet

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

#include "inet/icancloud/Users/Base/userBase.h"

namespace inet {

namespace icancloud {


void userBase::abandonSystem(){

    RequestBase* request;
    request = new RequestBase();
    request->setOperation(REQUEST_ABANDON_SYSTEM);
    request->setUid(this->getId());

    managerPtr->userSendRequest(request);
}

userBase::~userBase() {
    configPreload.clear();
    configFS.clear();
}

void userBase::initialize(int stage) {

    queuesManager::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {

        pending_requests.clear();
        fsType = "LOCAL";
        configMPI = nullptr;
        configPreload.clear();
        configFS.clear();
        waiting_for_system_response = new JobQueue();
        userID = this->getId();

        //Set up the manager
        cModule* managerMod;

        managerMod = this->getParentModule()->getSubmodule("manager");
        managerPtr = dynamic_cast<RequestsManagement*>(managerMod);

        if (managerPtr == nullptr)
            throw cRuntimeError(
                    "userBase::initialize()->can not cast the manager to RequestManagement class\n");
    }
}

void userBase::finish(){

    queuesManager::finish();

    fsType = "LOCAL";
    configMPI = nullptr;
    configPreload.clear();
    configFS.clear();
    waiting_for_system_response->clear();
}

void userBase::sendRequestToSystemManager (AbstractRequest* request){

         managerPtr->userSendRequest(request);

     // insert to wait until a response will be given
         pending_requests.push_back(request);

     // Move the job until the request will be atendeed..
         if (request->getOperation() == REQUEST_RESOURCES)
             waitingQueue->move_to_qDst(waitingQueue->get_index_of_job(request->getUid()), waiting_for_system_response, waiting_for_system_response->get_queue_size());
}

void userBase::requestArrival(AbstractRequest* request){

    int i;
    AbstractRequest* req;
    bool found;

    found = false;

    for (i = 0; (i < (int)pending_requests.size()) && (!found); i++){

        req = (*(pending_requests.begin() + i));

        if (request->compareReq(req)){
            pending_requests.erase((pending_requests.begin() + i));
            found = true;
        }
    }

}

void userBase::requestArrivalError(AbstractRequest* request){

    int i;
    AbstractRequest* req;
    bool found;

    found = false;

    waiting_for_system_response->move_to_qDst(waiting_for_system_response->get_index_of_job(request->getUid()), waitingQueue, waitingQueue->get_queue_size());

    for (i = 0; (i < (int)pending_requests.size()) && (!found); i++){

        req = (*(pending_requests.begin() + i));

        if (request->compareReq(req)){
            pending_requests.erase((pending_requests.begin() + i));
            found = true;
        }
    }



}

} // namespace icancloud
} // namespace inet

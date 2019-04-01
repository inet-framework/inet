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

#include "inet/icancloud/Management/DataCenterManagement/Base/RequestsManagement.h"

namespace inet {

namespace icancloud {



RequestsManagement::~RequestsManagement() {
    requestsQueue.clear();
    temporalRequestsQueue.clear();
    executingRequests.clear();
}

void RequestsManagement::initialize(int stage) {
    icancloud_Base::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        schedulerQueueBlocked = false;
        requestsQueue.clear();
        temporalRequestsQueue.clear();
        executingRequests.clear();
    }
}

void RequestsManagement::finish(){
    requestsQueue.clear();
    temporalRequestsQueue.clear();
    executingRequests.clear();
    icancloud_Base::finish();
}

bool RequestsManagement::schedulerBlock(){

    bool result;

    result = true;

    if (schedulerQueueBlocked){

        result = false;

    } else {
        blockArrivalRequests(true);
        schedulerQueueBlocked = true;
    }


    return result;

}

bool RequestsManagement::schedulerUnblock(){

    Enter_Method_Silent();

    bool result;

    result = true;

    if (schedulerQueueBlocked){

        result = true;
        schedulerQueueBlocked = false;

    }

    blockArrivalRequests(false);

    return result;

}

// ---------------------- Interact with requests and requestsQueue ------------------------------------
void RequestsManagement::userSendRequest(AbstractRequest* request){

    // Insert into the requestsQueue
    if (!schedulerQueueBlocked){
        requestsQueue.push_back(request);
    } else {
        temporalRequestsQueue.push_back(request);
    }

}

void RequestsManagement::reinsertRequest (AbstractRequest* req, unsigned int index){

    // Define ...
//        vector<AbstractRequest*>::iterator it;

    // Insert ..
        if ((index >= 0) && (index < requestsQueue.size())){
            if (requestsQueue.size()>0) requestsQueue.insert(requestsQueue.begin()+ index, req);
        } else {
            req->incrementTimesEnqueue();
            requestsQueue.push_back(req);
        }
}


vector<AbstractRequest*> RequestsManagement::getRequestByUserID (int userModID){

    //Define...
        vector<AbstractRequest*>::iterator reqIt;
        vector<AbstractRequest*> reqs;
        bool found;

    //Initialize...
        found = false;
        reqs.clear();

    //Search the job into jobsList
        for (reqIt = requestsQueue.begin(); reqIt < requestsQueue.end(); reqIt++){

            if ( (*reqIt)->getUid() == userModID ){
                reqs.push_back((*reqIt));
            }
        }

        if (!found)
            reqs.clear();

        return reqs;
}

AbstractRequest* RequestsManagement::getRequestByIndex (unsigned int index){

    // Define ...
        AbstractRequest* req;

    // Initialize...
        req = nullptr;

        if ((index >= 0) && (index < requestsQueue.size())){

            // Obtain the request at position = index
            if (requestsQueue.size()>0) req = (*(requestsQueue.begin()+index));

        }

        return req;

}

int RequestsManagement::numPendingRequests(){

    return requestsQueue.size();

}

void RequestsManagement::eraseRequest (unsigned int index){

    vector<AbstractRequest*>::iterator reqIt;

    if ((index >= 0) && (index < requestsQueue.size()) ){

        if (requestsQueue.size()>0){
            reqIt = requestsQueue.begin() + index;
            requestsQueue.erase(reqIt);
        }
    } else {
        throw cRuntimeError("Error in [RequestsManagement]: -------> erase_request: requestsQueue size:%i index:%i\n ", requestsQueue.size(), index);
    }

}

void RequestsManagement::eraseRequest (AbstractRequest* req){

    vector<AbstractRequest*>::iterator reqIt;
    unsigned int index;
    bool found = false;

    for ((index = 0); (index < requestsQueue.size()) && (!found); index++ ){

        reqIt = requestsQueue.begin() + index;

        if (req->compareReq(*(reqIt))){
            found = true;

            if (req->getOperation() == REQUEST_STORAGE){
                insertExecutingRequest(req);
            }

            requestsQueue.erase(reqIt);

        }
    }

}

void RequestsManagement::user_request(AbstractRequest* request){

    // Insert into the requestsQueue
    if (!schedulerQueueBlocked){
        requestsQueue.push_back(request);
    } else {
        temporalRequestsQueue.push_back(request);
    }

}

void RequestsManagement::blockArrivalRequests(bool blocked){

    if (!blocked){
        while (temporalRequestsQueue.size() != 0){
            requestsQueue.push_back((*temporalRequestsQueue.begin()));
            temporalRequestsQueue.erase(temporalRequestsQueue.begin());
        }
    }

    schedulerQueueBlocked = blocked;
}

} // namespace icancloud
} // namespace inet

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

#include "inet/icancloud/Base/Request/Request.h"

namespace inet {

namespace icancloud {


PhysicalResourcesRequest::PhysicalResourcesRequest() {
    nodeSet = -1;
    nodeIndex = -1;
    cores = -1;
    memory = 0.0;
    storage = 0.0;
    messageID = -1;

    // Storage Request
    storageRequests.clear();

    // Request base
    operation = REQUEST_NOP;
    timesEnqueue = 0;
    state = NOT_REQUEST;
    jobId = -1;
    vmId = -1;
    userId = -1;
}


PhysicalResourcesRequest::PhysicalResourcesRequest(PhysicalResourcesRequest* request){
    nodeSet = request->getSet();
    nodeIndex = request->getIndex();
    cores = request->getCores();
    memory = request->getMemory();
    storage = request->getStorage();
    messageID = request->getMessageID();

    // Storage Request
    AbstractRequest* req;
    StorageRequest* stReq;
    for (int i = 0; i < (int)storageRequests.size();i++){
        req = (*(storageRequests.begin() + i))->dup();
        stReq = dynamic_cast<StorageRequest*>(req);
        storageRequests.push_back(stReq);
    }


    // Request base
    operation = request->getOperation();
    timesEnqueue= request->getTimesEnqueue();
    state= request->getState ();
    jobId = request->getPid();
    userId = request->getUid();

}

PhysicalResourcesRequest::PhysicalResourcesRequest(StorageRequest* request){
    nodeSet = -1;
    nodeIndex = -1;
    cores = -1;
    memory = 0.0;
    storage = 0.0;
    messageID = -1;
    vmId = -1;
    storageRequests.clear();

    // Storage Request
    StorageRequest* stReq;

    stReq = new StorageRequest();
    stReq->setOperation(request->getOperation());
    stReq->setTimesEnqueue(request->getTimesEnqueue());
    stReq->setState(request->getState());
    stReq->setFsType(request->getFsType());

    if (request->getPreloadSize() != 0){
        stReq->setPreloadSet(request->getPreloadFilesSet());
    }

    if (request->getFSPathsSize() != 0){
        stReq->setFSPathSet(request->getFSSet());
    }

    if (request->getConnectionSize() != 0){
        stReq->setConnectionVector(request->getConnections());
    }

    storageRequests.push_back(stReq);

    // Request base
    operation = request->getOperation();
    timesEnqueue = request->getTimesEnqueue();
    state = request->getState ();
    jobId = request->getPid();
    userId = request->getUid();
}

PhysicalResourcesRequest::PhysicalResourcesRequest(RequestBase* request){
    nodeSet = -1;
    nodeIndex = -1;
    cores = -1;
    memory = 0.0;
    storage = 0.0;
    messageID = -1;

    // Storage Request
    storageRequests.clear();

    // Request base
    operation = request->getOperation();
    timesEnqueue= request->getTimesEnqueue();
    state= request->getState ();
    jobId = request->getUid();
    userId = request->getPid();
}

PhysicalResourcesRequest::~PhysicalResourcesRequest() {
    nodeSet = -1;
    nodeIndex = -1;
    cores = -1;
    memory = 0.0;
    storage = 0.0;
    messageID = -1;

    operation = -1;
    timesEnqueue = 0;
    state = 0;

    storageRequests.clear();
    jobId = -1;
    userId = -1;
}

AbstractRequest* PhysicalResourcesRequest::dup (){

    PhysicalResourcesRequest* newRequest;
    AbstractRequest* req;
    StorageRequest* stReq;

    int i;

    newRequest = new PhysicalResourcesRequest();

    newRequest->operation = getOperation();
    newRequest->timesEnqueue = timesEnqueue;
    newRequest->state = state;

    newRequest->nodeSet = getSet();
    newRequest->nodeIndex = getIndex();
    newRequest->cores = getCores();
    newRequest->memory = getMemory();
    newRequest->storage = getStorage();
    newRequest->messageID = getMessageID();
    newRequest->jobId =getSPid();
    newRequest->userId = getUid();
    newRequest->ip = ip;
    newRequest->vmId = getPid();
    for (i = 0; i < (int)storageRequests.size();i++){
        req = (*(storageRequests.begin() + i))->dup();
        stReq = dynamic_cast<StorageRequest*>(req);
        storageRequests.push_back(stReq);
    }

    req = dynamic_cast<AbstractRequest*>(newRequest);

    return req;

}

bool PhysicalResourcesRequest::compareReq(AbstractRequest* req){

    PhysicalResourcesRequest* hreq;
    vector<connection_T*> connections;
    bool equal = true;

        hreq = dynamic_cast<PhysicalResourcesRequest*>(req);

        if (hreq != nullptr){
            if ((hreq->getOperation() == getOperation()) &&
            (getTimesEnqueue() == hreq->getTimesEnqueue()) &&
            (getState() == hreq->getState()) &&
            (getSet() == hreq->getSet()) &&
            (getSPid() == req->getSPid()) &&
            (getIndex() == hreq->getIndex()) &&
            (getCores() == hreq->getCores()) &&
            (getMemory() == hreq->getMemory()) &&
            (getStorage() == hreq->getStorage()) &&
            (getMessageID() == hreq->getMessageID()) &&
            (getPid() == hreq->getPid())&&
            (getUid() == hreq->getUid())&&
            (strcmp(getIp().c_str(), req->getIp().c_str()) == 0) &&
            (getStorageRequestsSize() == hreq->getStorageRequestsSize())
            ){
                for (int i = 0; (i < getStorageRequestsSize()) && (equal); i++){
                    equal = getStorageRequest(i)->compareReq(hreq->getStorageRequest(i));
                }

            }
            else
                equal = false;
        }
        else {
            equal = false;
        }
        return equal;
}



} // namespace icancloud
} // namespace inet

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


StorageRequest::StorageRequest (){

	operation = -1;
    timesEnqueue = 0;
    state = 0;
    preloadFiles_v.clear();
    fsPaths.clear();
    connections.clear();
    nodeSetId = "";
    nodeId = -1;
    vmId = -1;
}

StorageRequest::~StorageRequest() {
    operation = -1;
    timesEnqueue = 0;
    state = 0;
    preloadFiles_v.clear();
    fsPaths.clear();
    connections.clear();
    vmId = -1;
}

AbstractRequest* StorageRequest::dup (){

	StorageRequest* newRequest;
	AbstractRequest* req;

	newRequest = new StorageRequest();
	newRequest->operation = getOperation();
	newRequest->timesEnqueue = timesEnqueue;
	newRequest->state = state;
	newRequest->fs = getFsType();
	newRequest->jobId = getSPid();
	newRequest->userId = getUid();
    newRequest->ip = ip;
    newRequest->nodeSetId = nodeSetId;
    newRequest->nodeId = nodeId;
    newRequest->vmId = getPid();

	if (preloadFiles_v.size() != 0){
		newRequest->setPreloadSet(preloadFiles_v);
	}

	if (fsPaths.size() != 0){
		newRequest->setFSPathSet(fsPaths);
	}

	if (connections.size() != 0){
	    newRequest->setConnectionVector(getConnections());
	}

	req = dynamic_cast<AbstractRequest*>(newRequest);
	return req;

}

bool StorageRequest::compareReq(AbstractRequest* req){

        vector<connection_T*> connections;
        StorageRequest* stReq;

        stReq = dynamic_cast<StorageRequest*>(req);

        bool equal = false;

        if (stReq != nullptr){
            if ((stReq->getOperation() == getOperation()) &&
            (getTimesEnqueue() == stReq->getTimesEnqueue()) &&
            (getState() == stReq->getState()) &&
            (getPid() == stReq->getPid()) &&
            (getUid() == stReq->getUid()) &&
            (getSPid() == req->getSPid()) &&
            (stReq->getConnectionSize() == getConnectionSize()) &&
            (getFSPathsSize() == stReq->getFSPathsSize()) &&
            (strcmp(getIp().c_str(), stReq->getIp().c_str()) == 0) &&
            (strcmp(getNodeSetId().c_str(), stReq->getNodeSetId().c_str()) == 0) &&
            (getNodeId() == stReq->getNodeId())&&
            (getPreloadSize() == stReq->getPreloadSize())
            ){
                equal = true;
            }
        }
        return equal;
}


void StorageRequest::setPreloadSet (vector<preload_T*> data){
    for (int i = 0; i < (int)data.size(); i++) {
        preloadFiles_v.push_back((*(data.begin() + i)));
    };
};

double StorageRequest::getTotalFilesSize(){

    double size = 0.0;

    for (int i = 0; i < (int)preloadFiles_v.size(); i++) {
        size += (*(preloadFiles_v.begin() + i))->fileSize;
    };

    return size;
}


void StorageRequest::setConnection(connection_T* connection, int index){

    if (index == -1) connections.push_back(connection);
    else{
        connections.insert((connections.begin() + index), connection);
    }
}

} // namespace icancloud
} // namespace inet

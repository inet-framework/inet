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



RequestBase::~RequestBase() {
    operation = REQUEST_NOP;
    timesEnqueue = 0;
    state = NOT_REQUEST;

}


AbstractRequest* RequestBase::dup (){

	RequestBase* newRequest;
	AbstractRequest* req;

	newRequest = new RequestBase();

	newRequest->operation = getOperation();
	newRequest->timesEnqueue = timesEnqueue;
	newRequest->state = state;
	newRequest->jobId = jobId;
	newRequest->vmId = vmId;
	newRequest->userId = userId;
	newRequest->ip = ip;

    req = dynamic_cast<AbstractRequest*>(newRequest);

	return req;

}

bool RequestBase::compareReq(AbstractRequest* req){

        vector<connection_T*> connections;
        bool equal = false;

        if ((req->getOperation() == getOperation()) &&
        (getTimesEnqueue() == req->getTimesEnqueue()) &&
        (getPid() == req->getPid()) &&
        (getUid() == req->getUid()) &&
        (getSPid() == req->getSPid()) &&
        (strcmp(getIp().c_str(), req->getIp().c_str()) == 0) &&
        (getState() == req->getState())
        ){
            equal = true;
        }

        return equal;
}


} // namespace icancloud
} // namespace inet

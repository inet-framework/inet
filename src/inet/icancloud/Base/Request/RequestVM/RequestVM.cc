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

#include "inet/icancloud/Base/Request/RequestVM/RequestVM.h"

namespace inet {

namespace icancloud {


RequestVM::RequestVM (){

	operation = -1;
	userId = -1;
	vmSet.clear();
    timesEnqueue = 0;
    state = 0;
    jobId = -1;
    vmId = -1;
    connections.clear();

}

RequestVM::RequestVM (int userID, int op, vector<VM*> newVMSet){

	operation = op;
	userId = userID;
	vmSet.clear();
    vmSet = newVMSet;
    timesEnqueue = 0;
    state = 0;
    jobId = -1;
    vmId = -1;
    connections.clear();
}

RequestVM::~RequestVM() {

    vmSet.clear();
    connections.clear();
}

VM* RequestVM::getVM(int position){

	// Define

		// vector<VM*>::iterator it;
		VM* vm;
		int vmSetSize;
	// Init ..

		vm = nullptr;
		vmSetSize = -1;

	// Begin
		vmSetSize = vmSet.size();
		if ((position <= vmSetSize) && (position > -1)) {

			vm = (*(vmSet.begin()+position));

		}

		return vm;
}

void RequestVM::eraseVM(int position){

	// Define

		vector<VM*>::iterator it;
		int vmSetSize;

	// Init ..
		vmSetSize = -1;

	// Begin

		vmSetSize = vmSet.size();
		if ((position < vmSetSize) && (position > -1)) {

			it = vmSet.begin() + position;
			vmSet.erase(it);

		}

}

RequestVM* RequestVM::dup (){

	RequestVM* newRequest;
	vector <VM*> vms;
	vector<connection_T*> connections;
	int i;

	newRequest = new RequestVM();
	vms.clear();
	connections.clear();

	newRequest->timesEnqueue = timesEnqueue;
	newRequest->state = state;
    newRequest->operation = getOperation();
    newRequest->jobId =getSPid();
    newRequest->userId = getUid();
    newRequest->vmId = getPid();
    newRequest->ip = ip;
	// vmSet
	if (vmSet.size() != 0){
		newRequest->setVectorVM(vmSet);
	}

	if (connections.size() != 0){
	    for (i =0; i < (int)connections.size(); i++){
	        newRequest->setConnection(getConnection(i));
	    }
	}

	// Duplicate the requested resources

    int differentTypes;

    differentTypes = vmsToBeSelected.size();

    for (int i = 0; i < differentTypes; i++){
        newRequest->setNewSelection(getSelectionType(i), getSelectionQuantity(i));
    }

	return newRequest;

}

bool RequestVM::compareReq(AbstractRequest* req){

        RequestVM* reqVM = dynamic_cast<RequestVM*>(req);
        vector <VM*> vms;
        vector<connection_T*> connections;
        bool equal = false;
        if (reqVM != nullptr){

            if ((reqVM->getOperation() == getOperation()) &&
            (reqVM->getUid() == getUid()) &&
            (reqVM->getPid() == getPid()) &&
            (getSPid() == req->getSPid()) &&
            (getTimesEnqueue() == reqVM->getTimesEnqueue()) &&
            (getState() == reqVM->getState()) &&
            (reqVM->getConnectionSize() == getConnectionSize()) &&
            (strcmp(getIp().c_str(), req->getIp().c_str()) == 0) &&
            (getVMQuantity() == reqVM->getVMQuantity()) &&
            ((int)vmsToBeSelected.size() == reqVM->getDifferentTypesQuantity())
            ){
                equal = true;
            }
        }

        return equal;
}

void RequestVM::setNewSelection(string typeName, int quantity){

    elementType* el;
    userVmType* type;

    el = new elementType();
    el->setType(typeName);

    type = new userVmType();
    type->type = el;
    type->quantity = quantity;

    vmsToBeSelected.push_back(type);

}

string RequestVM::getSelectionType(int indexAtSelection){
    return ((*(vmsToBeSelected.begin() + indexAtSelection))->type->getType());
}

int RequestVM::getSelectionQuantity(int indexAtSelection){
    return ((*(vmsToBeSelected.begin() + indexAtSelection))->quantity);

}

void RequestVM::decreaseSelectionQuantity(int indexAtSelection){
   ( (*(vmsToBeSelected.begin() + indexAtSelection))->quantity--);

}

void RequestVM::eraseSelectionType(int indexAtSelection){
    vmsToBeSelected.erase(vmsToBeSelected.begin() + indexAtSelection);

}


void RequestVM::setForSingleRequest(elementType* el){

    userVmType* type;

    type = new userVmType();
    type->type = el;
    type->quantity = 1;

    vmsToBeSelected.push_back(type);
}

} // namespace icancloud
} // namespace inet

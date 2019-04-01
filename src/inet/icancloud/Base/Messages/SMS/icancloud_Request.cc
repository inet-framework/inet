#include "inet/icancloud/Base/Messages/SMS/icancloud_Request.h"

namespace inet {

namespace icancloud {



icancloud_Request::icancloud_Request (){
	arrivedSubRequests = 0;
	parentRequest = nullptr;
}


icancloud_Request::icancloud_Request (Packet *newParent){
	arrivedSubRequests = 0;
	parentRequest = newParent;
}


icancloud_Request::icancloud_Request (inet::Packet *newParent, unsigned int numSubReq){
	
	unsigned int i;
	
		arrivedSubRequests = 0;
		parentRequest = newParent;
		subRequests.reserve (numSubReq);
		
		for (i=0; i<numSubReq; i++)
			subRequests.push_back (nullptr);
}


icancloud_Request::~icancloud_Request (){
	
	int i;		

		// Removes messages...
		for (i=0; i<((int)subRequests.size()); i++){
			
			if (subRequests[i] != nullptr)				
				delete (subRequests[i]);
				subRequests[i] = nullptr;
		}
		
	subRequests.clear();		
}


Packet* icancloud_Request::getParentRequest () {
	return parentRequest;	
}


void icancloud_Request::setParentRequest (inet::Packet* newParent){
	parentRequest = newParent;
}


unsigned int icancloud_Request::getNumSubRequest (){
	return subRequests.size();
}


unsigned int icancloud_Request::getNumArrivedSubRequest () const {
	return arrivedSubRequests;
}


void icancloud_Request::setSubRequest (Packet* subRequest, unsigned int index){
	
	if (index < subRequests.size())
		subRequests[index] = subRequest;
	else		
		throw cRuntimeError("[icancloud_Request.setSubRequest] Index out of bounds!");	
	
}


void icancloud_Request::addSubRequest (Packet* subRequest){
	subRequests.push_back (subRequest);
}


inet::Packet* icancloud_Request::getSubRequest (unsigned int index){
	
	Packet * pkt = nullptr;
	
	if (index < subRequests.size()){
	    if (subRequests[index] != nullptr) {
	        const auto & sm = subRequests[index]->peekAtFront<icancloud_Message>();
	        if (sm == nullptr)
	            throw cRuntimeError("Packet doesn't include icancloud_Message header");
	        pkt = subRequests[index];
	    }
	}
	else
	    throw cRuntimeError("[icancloud_Request.getSubRequest] Index out of bounds!");
		
	return pkt;
}


inet::Packet* icancloud_Request::popSubRequest(unsigned int index) {

    Packet* subRequest = nullptr;

    if (index < subRequests.size()) {
        if (subRequests[index] != nullptr) {
            const auto & sm = subRequests[index]->peekAtFront<icancloud_Message>();
            if (sm == nullptr)
                throw cRuntimeError(
                        "Packet doesn't include icancloud_Message header");
            subRequest = (subRequests[index]);
            subRequests[index] = nullptr;
        }
    }
    else
        throw cRuntimeError(
                "[icancloud_Request.getSubRequest] Index out of bounds!");

    return subRequest;
}

inet::Packet* icancloud_Request::popNextSubRequest() {

    Packet* subRequest = nullptr;
    int i;
    bool found;

    // Init...
    subRequest = nullptr;
    found = false;
    i = 0;

    // Search for the next subRequest
    while ((!found) && (i < ((int) subRequests.size()))) {

        if (subRequests[i] != nullptr) {
            // Casting
            subRequest = (subRequests[i]);

            const auto &sm = subRequests[i]->peekAtFront<icancloud_Message>();
            if (sm == nullptr)
                throw cRuntimeError("Packet doesn't include icancloud_Message header");

            // Found!
            if (!sm->getIsResponse()) {
                //subRequests.erase(subRequests.begin() + i);
                //subRequests[i] = nullptr;
                found = true;
            }

        }

        // Try with next subRequest...
        if (!found)
            i++;
    }

    return subRequest;
}


bool icancloud_Request::arrivesAllSubRequest (){
	
	return (arrivedSubRequests == subRequests.size());
}


void icancloud_Request::arrivesSubRequest (inet::Packet* subRequest, unsigned int index){

    const auto & sm = subRequest->peekAtFront<icancloud_Message>();
    if (sm == nullptr)
        throw cRuntimeError("Packet doesn't include icancloud_Message header");

	if (index < subRequests.size()){
		
		if (subRequests[index] == nullptr){
			subRequests[index] = subRequest;
		}
		else{
		    subRequests[index] = nullptr;
		}
        arrivedSubRequests++;
	}
	else{
		throw cRuntimeError("[icancloud_Request.arrivesSubRequest] Index out of bounds!");
	}
}

void icancloud_Request::clearSubRequests() {
    // Removes messages...
    for (int i = 0; i < ((int) subRequests.size()); i++) {

        if (subRequests[i] != nullptr)
            delete (subRequests[i]);
        subRequests[i] = nullptr;
    }
}

void icancloud_Request::clear() {

    int i;

    if (parentRequest != nullptr) {
        delete (parentRequest);
        parentRequest = nullptr;
    }

    // Removes messages...
    for (i = 0; i < ((int) subRequests.size()); i++) {

        if (subRequests[i] != nullptr)
            delete (subRequests[i]);
        subRequests[i] = nullptr;
    }
}


} // namespace icancloud
} // namespace inet

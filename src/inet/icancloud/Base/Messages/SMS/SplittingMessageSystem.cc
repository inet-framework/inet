#include "inet/icancloud/Base/Messages/SMS/SplittingMessageSystem.h"

namespace inet {

namespace icancloud {


const bool SplittingMessageSystem::verboseMode = false;



int SplittingMessageSystem::searchRequest (inet::Packet* request){

	bool found;
	int index;

	// Init...
	found = false;
	index = 0;

	// Search the request!
	for (auto reqIterator = requestVector.begin();
	        (reqIterator != requestVector.end() && (!found));
	        ++reqIterator){
	    // Found?
	    if ((*reqIterator)->getParentRequest() == request)
	        found = true;
	    else
	        index++;
	}
	// not found...
	if (!found)
	    index = NOT_FOUND;

	return index;
}


int SplittingMessageSystem::getNumberOfSubRequest (inet::Packet* request){

	int index;

		// Search...
	index = searchRequest (request);
	// Request found...
	if (index != NOT_FOUND)
	    return ((requestVector[index])->getNumSubRequest());
	else
	    throw cRuntimeError("[SplittingMessageSystem::removeRequest] Request not found!!!");
}


bool SplittingMessageSystem::arrivesAllSubRequest (inet::Packet* request){

	int index;
	// Search...
	index = searchRequest (request);
	// Request found...
	if (index != NOT_FOUND){
	    return ((requestVector[index])->arrivesAllSubRequest());
	}
	else
	    throw cRuntimeError("[SplittingMessageSystem::arrivesAllSubRequest] Request not found!!!");
}


void SplittingMessageSystem::addRequest (inet::Packet* request, unsigned int numSubRequests){

	icancloud_Request *newRequest;
	newRequest = new icancloud_Request (request, numSubRequests);
	requestVector.push_back (newRequest);
}


void SplittingMessageSystem::removeRequest (inet::Packet* request){

	int index;
	vector <icancloud_Request*>::iterator reqIterator;

		// Search for the corresponding request
		index = searchRequest (request);

		// Request found... remove it!
		if (index != NOT_FOUND){
			reqIterator = requestVector.begin();
			reqIterator = reqIterator + index;
			(*reqIterator)->clearSubRequests();
			delete (*reqIterator);
			requestVector.erase (reqIterator);
		}
		else
			throw cRuntimeError("[SplittingMessageSystem::removeRequest] Request not found!!!");
}


void SplittingMessageSystem::setSubRequest (inet::Packet* request, Packet* subRequest, unsigned int subReqIndex){

	int index;

		// Search...
		index = searchRequest (request);

		// Request found...
		if (index != NOT_FOUND){

			// Check if subReqIndex is correct!
			(requestVector[index])->setSubRequest (subRequest, subReqIndex);
		}
		else
			throw cRuntimeError("[SplittingMessageSystem::setSubRequest] Request not found!!!");
}


inet::Packet* SplittingMessageSystem::popSubRequest (inet::Packet* parentRequest, int subRequestIndex){

	//icancloud_Message* subRequest;
    inet::Packet *subRequest = nullptr;
	int index;
	// Init...
	// Search for the parent request
	index = searchRequest (parentRequest);
	// found!
	if (index!=NOT_FOUND){
	    subRequest = (requestVector[index])->popSubRequest (subRequestIndex);
	}
	return subRequest;
}


inet::Packet* SplittingMessageSystem::popNextSubRequest (inet::Packet* parentRequest){
	
	//icancloud_Message* subRequest;
    inet::Packet *subRequest = nullptr;
	int index;
	// Search for the parent request
	index = searchRequest (parentRequest);
	// found!
	if (index!=NOT_FOUND){
	    subRequest = (requestVector[index])->popNextSubRequest ();
	}
	return subRequest;
}


void SplittingMessageSystem::arrivesSubRequest (inet::Packet* subRequest, inet::Packet* parentRequest){

	//icancloud_Message *subReqMsg;
	std::ostringstream info;
	int indexParent;
	int indexSubRequest;


	// Search for the parent request
	indexParent = searchRequest (parentRequest);
		
		// Cast to icancloud_Message

	auto subReqMsg = subRequest->peekAtFront<icancloud_Message>();
    //subReqMsg = check_and_cast<icancloud_Message *>(subRequest);

    // found!
    if (indexParent != NOT_FOUND) {

        // Get the subRequest index!
        indexSubRequest = subReqMsg->getCurrentRequest();

        // Link!
        (requestVector[indexParent])->arrivesSubRequest(subRequest, indexSubRequest);
        delete (subRequest);
    }
    else {
        info << "Parent request not found! subRequest info:"
                << subReqMsg->contentsToString(verboseMode);
        throw cRuntimeError(info.str().c_str());
    }
}

int SplittingMessageSystem::getNumberOfRequests() {
    return requestVector.size();
}

string SplittingMessageSystem::requestToString(inet::Packet* request, bool printContents) {

    std::ostringstream info;
    int index;

    // Search for the request...
    index = searchRequest(request);

    // Request not found...
    if (index == NOT_FOUND)
        info << "Request Not Found!" << endl;
    else {
        if (printContents)
            info << requestToStringByIndex(index);
    }

    return info.str();
}

SplittingMessageSystem::~SplittingMessageSystem() {

}

} // namespace icancloud
} // namespace inet

#include "inet/icancloud/Base/Messages/SMS/SMS_PFS.h"

namespace inet {

namespace icancloud {


using namespace omnetpp;
const unsigned int SMS_PFS::meta_dataKB = 1;


SMS_PFS::SMS_PFS (unsigned int newStrideSizeKB, unsigned int newNumServers){
	strideSize = newStrideSizeKB * KB;
	numServers = newNumServers;
	currentCommId = 0;
}
SMS_PFS::SMS_PFS(){
   strideSize = 0;
   numServers = 0;
   currentCommId = 0;

}
SMS_PFS::~SMS_PFS(){

}

void SMS_PFS::splitRequest (cMessage *msg){

	unsigned int i;
	//icancloud_App_IO_Message *sm_io;			// IO Request message
	//icancloud_App_IO_Message *subRequestMsg;	// SubRequests message!

	unsigned int numberOfsubRequest;		// Number of stripped request
	unsigned int requestSize;				// Request Size
	unsigned int subRequestOffset;			// SubRequest offset
	unsigned int subRequestSize;			// Subrequest Size

	int operation;

	auto pkt = check_and_cast<Packet *>(msg);
	// Cast!
	//sm_io = check_and_cast<icancloud_App_IO_Message *>(msg);
	const auto &sm_io = pkt->peekAtFront<icancloud_App_IO_Message>();
	operation = sm_io->getOperation();

		// Open File
		if (operation == SM_OPEN_FILE){

			// Add the new Request!
			addRequest (pkt, numServers);

			// One OPEN request per server...
			for (i=0; i<numServers; i++){

		        auto pktSub = pkt->dup();
#ifdef COPYCONTROLINFO
		        // I am not sure if it is necessary to copy the control info, the original code copy it, I cannot find sense
		        if (pkt->getControlInfo()) {
		            auto controlOld = check_and_cast<TcpCommand *>(pkt->getControlInfo());
		            pktSub->setControlInfo (controlOld->dup());
		        }
#endif
		        // Copy the message and set new values!
		        pktSub->trimFront();
		        auto subRequestMsg = pktSub->removeAtFront<icancloud_App_IO_Message>();
				// Copy the message and set new values!
				//subRequestMsg = (icancloud_App_IO_Message *) sm_io->dup();
				subRequestMsg->setParentRequest (pkt);

				// Read the metadata in each node!
				subRequestMsg->setOffset (0);
	    		subRequestMsg->setSize(meta_dataKB*KB);
	    		subRequestMsg->setOperation(SM_READ_FILE);

	    		// Set the corresponding server
	    		subRequestMsg->setConnectionId(i);
	    		subRequestMsg->setNfs_connectionID(currentCommId);
	    		// Link current subRequest with its parent request
	    		//setSubRequest (msg, subRequestMsg, i);

	    		// Set message length
	    		subRequestMsg->updateLength();


	    		// Update the current subRequest Message ID...
	    		subRequestMsg->addRequestToTrace (i);
	    		pktSub->insertAtFront(subRequestMsg);
	    		
	    		// Insert current subRequest!
	    		subRequests.push_back (pktSub);
			}
		}

		// Create File (2 messages per server: create+write_metaData)
		else if (operation == SM_CREATE_FILE){

			// Add the new Request!
			addRequest (pkt, 2*numServers);

			// Two requests per server...
			for (i=0; i<numServers; i++){

				// Copy the message and set new values! (Create message)
				//subRequestMsg = (icancloud_App_IO_Message *) sm_io->dup();
                auto pktSub = pkt->dup();
#ifdef COPYCONTROLINFO
                // I am not sure if it is necessary to copy the control info, the original code copy it, I cannot find sense
                if (pkt->getControlInfo()) {
                    auto controlOld = check_and_cast<TcpCommand *>(pkt->getControlInfo());
                    pktSub->setControlInfo (controlOld->dup());
                }
#endif
                // Copy the message and set new values!
                pktSub->trimFront();
                auto subRequestMsg = pktSub->removeAtFront<icancloud_App_IO_Message>();

				subRequestMsg->setParentRequest (pkt);

	    		// Set the corresponding server
	    		subRequestMsg->setConnectionId (i);
	    		subRequestMsg->setNfs_connectionID(currentCommId);
	    		// Set message length
	    		subRequestMsg->updateLength();

	    		// Update the current subRequest Message ID...
	    		subRequestMsg->addRequestToTrace (i);

	    		pktSub->insertAtFront(subRequestMsg);
	    		// Link current subRequest with its parent request
	    		//setSubRequest (msg, subRequestMsg, i);
	    		
	    		// Insert current subRequest!
	    		subRequests.push_back (pktSub);

	    		// Write message...
                pktSub = pkt->dup();
#ifdef COPYCONTROLINFO
                // I am not sure if it is necessary to copy the control info, the original code copy it, I cannot find sense
                if (pkt->getControlInfo()) {
                    auto controlOld = check_and_cast<TcpCommand *>(pkt->getControlInfo());
                    pktSub->setControlInfo (controlOld->dup());
                }
#endif
                // Copy the message and set new values!
                subRequestMsg = pktSub->removeAtFront<icancloud_App_IO_Message>();


	    		//subRequestMsg = (icancloud_App_IO_Message *) sm_io->dup();
				subRequestMsg->setParentRequest (pkt);

				// Read the metadata in each node!
				subRequestMsg->setOffset (0);
	    		subRequestMsg->setSize(meta_dataKB*KB);
	    		subRequestMsg->setOperation(SM_WRITE_FILE);

	    		// Set the corresponding server
	    		subRequestMsg->setConnectionId (i);
	    		subRequestMsg->setNfs_connectionID(currentCommId);
	    		// Set message length
	    		subRequestMsg->updateLength();

	    		// Update the current subRequest Message ID...
	    		subRequestMsg->addRequestToTrace (i+numServers);

	    		// Link current subRequest with its parent request
	    		//setSubRequest (msg, subRequestMsg, i+numServers);
	    		
	    		// Insert current subRequest!
	    		pktSub->insertAtFront(subRequestMsg);
	    		subRequests.push_back (pktSub);
			}
		}

		// Delete File
		else if (operation == SM_DELETE_FILE){

			// Add the new Request!
			addRequest (pkt, numServers);

			// One OPEN request per server...
			for (i=0; i<numServers; i++){

				// Copy the message and set new values!
				//subRequestMsg = (icancloud_App_IO_Message *) sm_io->dup();

                auto pktSub = pkt->dup();
#ifdef COPYCONTROLINFO
                // I am not sure if it is necessary to copy the control info, the original code copy it, I cannot find sense
                if (pkt->getControlInfo()) {
                    auto controlOld = check_and_cast<TcpCommand *>(pkt->getControlInfo());
                    pktSub->setControlInfo (controlOld->dup());
                }
#endif
                // Copy the message and set new values!
                pktSub->trimFront();
                auto subRequestMsg = pktSub->removeAtFront<icancloud_App_IO_Message>();

				subRequestMsg->setParentRequest (pkt);

	    		// Set the corresponding server
	    		subRequestMsg->setConnectionId (i);
	    		subRequestMsg->setNfs_connectionID(currentCommId);
	    		pktSub->insertAtFront(subRequestMsg);
	    		// Link current subRequest with its parent request
	    		setSubRequest (pkt, pktSub, i);

	    		subRequestMsg = pktSub->removeAtFront<icancloud_App_IO_Message>();
	    		// Set message length
	    		subRequestMsg->updateLength();

	    		// Update the current subRequest Message ID...
	    		subRequestMsg->addRequestToTrace (i);
	    		
	    		pktSub->insertAtFront(subRequestMsg);

	    		// Insert current subRequest!
	    		subRequests.push_back (pktSub);
			}
		}

		// Read/Write File
		else if ((operation == SM_READ_FILE) ||
				 (operation == SM_WRITE_FILE)){

			// Get request size...
			requestSize = B(sm_io->getChunkLength()).get();
			//	    	requestSize = sm_io->getSize();

			// Calculate the number of subRequests
			numberOfsubRequest = ((requestSize%strideSize)==0)?
								  (requestSize/strideSize):
								  (requestSize/strideSize)+1;
			numberOfsubRequest = 4;
			// Add the new Request!
		    addRequest (pkt, numberOfsubRequest);

			// First offset
			subRequestOffset = sm_io->getOffset();

			// create all subRequests
			for (i=0; i<numberOfsubRequest; i++){

				// Copy the message and set new values!
				//subRequestMsg = (icancloud_App_IO_Message *) sm_io->dup();
                auto pktSub = pkt->dup();
#ifdef COPYCONTROLINFO
                // I am not sure if it is necessary to copy the control info, the original code copy it, I cannot find sense
                if (pkt->getControlInfo()) {
                    auto controlOld = check_and_cast<TcpCommand *>(pkt->getControlInfo());
                    pktSub->setControlInfo (controlOld->dup());
                }
#endif
                // Copy the message and set new values!
                pktSub->trimFront();
                auto subRequestMsg = pktSub->removeAtFront<icancloud_App_IO_Message>();

				subRequestMsg->setParentRequest (pkt);

				//  Calculates the current subRequest size
				subRequestSize = (requestSize>=strideSize)?
									strideSize:
									requestSize;

				// Read the metadata in each node!
				subRequestMsg->setOffset (subRequestOffset);
	    		subRequestMsg->setSize (subRequestSize);

	    		// Set the corresponding server
                  subRequestMsg->setConnectionId (i%numServers);
                  subRequestMsg->setCommId(i%numServers);
                  subRequestMsg->setNfs_connectionID(i%numServers);

	    		//printf ("Adding new request. Offset:%d - Size:%d - to server:%d\n", subRequestOffset, subRequestSize, i%numServers);

	    		// Link current subRequest with its parent request
                pktSub->insertAtFront(subRequestMsg);
	    		setSubRequest (pkt, pktSub, i);
	    		subRequestMsg = pktSub->removeAtFront<icancloud_App_IO_Message>();

	    		// Set message length
	    		subRequestMsg->updateLength();

	    		// Update the current subRequest Message ID...
	    		subRequestMsg->addRequestToTrace (i);

	    		// update offset!
	    		subRequestOffset+=subRequestSize;
	    		requestSize-=subRequestSize;
	    		
	    		pktSub->insertAtFront(subRequestMsg);

	    		// Insert current subRequest!
	    		subRequests.push_back (pktSub);
			}
		}
}


Packet* SMS_PFS::getFirstSubRequest(){

	if (subRequests.empty())
		return nullptr;
	else
		return (subRequests.front());
}


Packet * SMS_PFS::popSubRequest(){

	//icancloud_App_IO_Message *msg;

    Packet *msg;
    if (subRequests.empty())
        msg = nullptr;
    else{
        msg = subRequests.front();
        subRequests.pop_front();
    }

	return msg;
}


string SMS_PFS::requestToStringByIndex (unsigned int index){

	std::ostringstream info;
	int i;
	int numSubRequest;
	//icancloud_App_IO_Message *sm_subReq;
	//icancloud_App_IO_Message *sm_io;


		// Request not found...
		if ((index>=requestVector.size()) || (index<0)){
			info << "Request" << index << " Not Found!" << endl;
		}

		// Request found!
		else{

		    //sm_io = check_and_cast<icancloud_App_IO_Message *>((requestVector[index])->getParentRequest());
		    auto pkt = requestVector[index]->getParentRequest();
		    const auto &sm_io = pkt->peekAtFront<icancloud_App_IO_Message>();

			// Get the number of subRequests
			numSubRequest = getNumberOfSubRequest (pkt);

			// Original request info...
			info << " Op:" << sm_io->operationToString()
				 << " File:" << sm_io->getFileName()				 << " Offset:" << sm_io->getOffset()
				 <<	" Size:" << B(sm_io->getChunkLength()).get()
				 <<	" subRequests:" << numSubRequest
				 << endl;

			// Get info of all subRequests...
			for (i=0; i<numSubRequest; i++){

				// Is nullptr?
				if ((requestVector[index])->getSubRequest(i) == nullptr)
					info << "  subRequest[" << i << "]: Not arrived yet!" << endl;
				else{

					// Cast!
				    //sm_subReq = check_and_cast<icancloud_App_IO_Message *>((requestVector[index])->getSubRequest(i));
				    auto pktSub = requestVector[index]->getSubRequest(i);
				    const auto &sm_subReq = pktSub->peekAtFront<icancloud_App_IO_Message>();

					info << "  subRequest[" << i << "]:"
						 << " Op:" << sm_io->operationToString()
						 << " Offset:" << sm_subReq->getOffset()
						 << " Size:" << sm_subReq->getSize()
						 << " CommunicationId:" << sm_subReq->getConnectionId()
						 << " appIndex:" << sm_subReq->getNextModuleIndex()
						 << endl;
				}
			}
		}

	return info.str();
}

void SMS_PFS::clear (){
	
	unsigned int i;
	
		for (i=0; i<requestVector.size(); i++){
			delete (requestVector[i]);
			requestVector[i] = nullptr;
		}	
	
	requestVector.clear();
}


} // namespace icancloud
} // namespace inet

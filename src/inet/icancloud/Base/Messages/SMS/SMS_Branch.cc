#include "inet/icancloud/Base/Messages/SMS/SMS_Branch.h"

namespace inet {

namespace icancloud {



SMS_Branch::SMS_Branch (){
}


void SMS_Branch::splitRequest(cMessage *msg) {

    int currentSubRequest;
    int numberOfBranches;

    off_blockList_t branchOffset;
    size_blockList_t branchSize;

    //icancloud_BlockList_Message *sm_bl;	// Message (request) to make the casting!
    //icancloud_BlockList_Message *subRequestMsg;	// SubRequest message

    auto pkt = check_and_cast<Packet *>(msg);
    const auto &sm = pkt->peekAtFront<icancloud_Message>();
    const auto &sm_bl = CHK(dynamicPtrCast<const icancloud_BlockList_Message>(sm));
    // Cast the original message

    // Add the new Request!
    numberOfBranches = sm_bl->getFile().getNumberOfBranches();
    addRequest(pkt, numberOfBranches);

    // Create all corresponding subRequests...
    for (currentSubRequest = 0; currentSubRequest < numberOfBranches;
            currentSubRequest++) {

        // Creates a new subRequest message with the corresponding parameters
        // subRequestMsg = (icancloud_BlockList_Message *) sm_bl->dup();
        auto pktBl = pkt->dup();
#ifdef COPYCONTROLINFO
        // I am not sure if it is necessary to copy the control info, the original code copy it, I cannot find sense
        if (pkt->getControlInfo()) {
            auto controlOld = check_and_cast<TcpCommand *>(pkt->getControlInfo());
            pktBl->setControlInfo (controlOld->dup());
        }
#endif
        pktBl->trimFront();
        auto subRequestMsg = pktBl->removeAtFront<icancloud_BlockList_Message>();

        subRequestMsg->setParentRequest(pkt);

        // Set the corresponding attributes
        branchOffset = sm_bl->getFile().getBranchOffset(currentSubRequest);
        branchSize = sm_bl->getFile().getBranchSize(currentSubRequest);

        subRequestMsg->getFileForUpdate().addBranch(branchOffset, branchSize);
        subRequestMsg->getFileForUpdate().setFileName(sm_bl->getFile().getFileName());
        subRequestMsg->getFileForUpdate().setFileSize(branchSize);

        // Update message length
        subRequestMsg->updateLength();
        pktBl->insertAtFront(subRequestMsg);

        // Insert current subRequest!
        subRequests.push_back(pktBl);
    }
}


inet::Packet* SMS_Branch::getFirstSubRequest(){

	if (subRequests.empty())
		return nullptr;
	else
		return (subRequests.front());
}


inet::Packet* SMS_Branch::popSubRequest() {

    inet::Packet *msg;

    if (subRequests.empty())
        msg = nullptr;
    else
    {
        msg = subRequests.front();
        subRequests.pop_front();
    }

    const auto &sm = msg->peekAtFront<icancloud_Message>();
    const auto &sm_bl = dynamicPtrCast<const icancloud_BlockList_Message>(sm);
    if (sm_bl == nullptr)
        throw cRuntimeError("Header is incorrect");
    return msg;
}


string SMS_Branch::requestToStringByIndex (unsigned int index){
    string str;

    str = "";

    return str;
}


void SMS_Branch::clear (){
	
	unsigned int i;
	
		for (i=0; i<requestVector.size(); i++){
			delete (requestVector[i]);
			requestVector[i] = nullptr;
		}	
		
	requestVector.clear();	
}

} // namespace icancloud
} // namespace inet

#include "inet/icancloud/Base/Messages/SMS/SMS_RAID_0.h"

namespace inet {

namespace icancloud {


const bool SMS_RAID_0::SHOW_BRANCHES = true;

using namespace omnetpp;

SMS_RAID_0::SMS_RAID_0 (unsigned int newStrideSize, unsigned int numBlockServers){
	strideSize = newStrideSize;
	blocksPerRequest = strideSize/BYTES_PER_SECTOR;
	numDisks = numBlockServers;
}


void SMS_RAID_0::splitRequest (cMessage *msg){

	off_blockList_t currentOffset;				// Current subRequest offset
	size_blockList_t currentBranchSize;			// Current branch size
	unsigned int currentBranch;					// Current branch

	size_blockList_t currentSubRequestSize;		// Current subRequest size
	size_blockList_t remainingBlocks;			// Remaining blocks to be assigned!
	size_blockList_t subReqRemainingBlocks;		// Remaining blocks current subRequest!

	unsigned int disk;							// Disk ID
	unsigned int i;

	//icancloud_BlockList_Message *sm_bl;				// Message (request) to make the casting!
	//icancloud_BlockList_Message *subRequestMsg;		// SubRequests message!

	//vector <icancloud_BlockList_Message *> msgVector;	// Vector that contains the stripped requests!

	vector <inet::Packet *> msgVector;   // Vector that contains the stripped requests!

	// Cast the original message
    auto pkt = check_and_cast<inet::Packet *>(msg);
    const auto &sm = pkt->peekAtFront<icancloud_Message>();
    const auto &sm_bl = CHK(dynamicPtrCast<const icancloud_BlockList_Message>(sm));

    // Init...
    remainingBlocks = sm_bl->getFile().getTotalSectors();
    currentBranch = 0;
    currentOffset = sm_bl->getFile().getBranchOffset(currentBranch);
    currentBranchSize = sm_bl->getFile().getBranchSize(currentBranch);

    // While exists at least 1 unassigned block...
    while (remainingBlocks > 0) {

        // Init...
        subReqRemainingBlocks = blocksPerRequest;

        // Locate the disk of current request!
        disk = (currentOffset / blocksPerRequest) % numDisks;

        // Copy the message base parameters
        auto subRequestPkt = pkt->dup();
#ifdef COPYCONTROLINFO
        // I am not sure if it is necessary to copy the control info, the original code copy it, I cannot find sense
        if (pkt->getControlInfo()) {
            auto controlOld = check_and_cast<TcpCommand *>(pkt->getControlInfo());
            subRequestPkt->setControlInfo (controlOld->dup());
        }
#endif
        subRequestPkt->trimFront();
        auto subRequestMsg = subRequestPkt->removeAtFront<icancloud_BlockList_Message>();

        // Link current subRequest with parent request!
        subRequestMsg->setParentRequest(pkt);

        // Set diskId
        subRequestMsg->setNextModuleIndex(disk);

        // Calculates the blocks of current subRequest...
        while ((subReqRemainingBlocks > 0) && (remainingBlocks > 0)) {

            // A complete stride!
            if (currentBranchSize >= subReqRemainingBlocks)
                currentSubRequestSize = subReqRemainingBlocks;
            else
                currentSubRequestSize = currentBranchSize;

            // Add the new stride request
            subRequestMsg->getFileForUpdate().addBranch(currentOffset,
                    currentSubRequestSize);

            // Update control variables
            currentOffset += currentSubRequestSize;
            currentBranchSize -= currentSubRequestSize;
            remainingBlocks -= currentSubRequestSize;
            subReqRemainingBlocks -= currentSubRequestSize;

            // Next Branch? Update control variables...
            if ((currentBranchSize <= 0) && (remainingBlocks > 0)) {

                if (sm_bl->getFile().getNumberOfBranches()
                        > (currentBranch + 1)) {
                    currentBranch++;
                    currentOffset = sm_bl->getFile().getBranchOffset(
                            currentBranch);
                    currentBranchSize = sm_bl->getFile().getBranchSize(
                            currentBranch);
                }
            }

        }

        // SubRequest Size
        subRequestMsg->getFileForUpdate().setFileSize(currentSubRequestSize * BYTES_PER_SECTOR);

        // Update message length
        subRequestMsg->updateLength();
        subRequestPkt->insertAtFront(subRequestMsg);

        // Adds the new subRequest to vector
        msgVector.push_back(subRequestPkt);
    }

    // Add a new request to List!
    addRequest(pkt, msgVector.size());

    for (i = 0; i < msgVector.size(); i++) {
        msgVector[i]->trimFront();
        auto sm = msgVector[i]->removeAtFront<icancloud_Message>();
        sm->addRequestToTrace(i);
        msgVector[i]->insertAtFront(sm);
        setSubRequest(pkt, msgVector[i], i);
    }

    // Free memory
    msgVector.clear();
}


string SMS_RAID_0::requestToStringByIndex(unsigned int index) {

    std::ostringstream info;
    int i;
    int numSubRequest;
    //icancloud_BlockList_Message *sm_subReq;
    //icancloud_BlockList_Message *sm_bl;

    // Request not found...
    if ((index >= requestVector.size()) || (index < 0)) {
        info << "Request" << index << " Not Found!" << endl;
    }

    else {

        // Cast!
        auto pktBl = requestVector[index]->getParentRequest();
        const auto &sm = pktBl->peekAtFront<icancloud_Message>();
        const auto &sm_bl = CHK(dynamicPtrCast<const icancloud_BlockList_Message>(sm));

        // Get the number of subRequests
        numSubRequest = getNumberOfSubRequest(pktBl);

        // Original request info...
        info << "subRequests:" << numSubRequest
                << sm_bl->getFile().contentsToString(SHOW_BRANCHES, true)
                << endl;

        // Get info of all subRequests...
        for (i = 0; i < numSubRequest; i++) {

            // Is nullptr?
            if ((requestVector[index])->getSubRequest(i) == nullptr)
                info << "  subRequest[" << i << "]: Not arrived yet!" << endl;
            else {

                // Cast!
                auto pktBlAux = requestVector[index]->getSubRequest(i);
                const auto &sm = pktBlAux->peekAtFront<icancloud_Message>();
                const auto &sm_subReq = CHK(dynamicPtrCast<const icancloud_BlockList_Message>(sm));

                info << "  subRequest[" << i << "]:"
                        << sm_subReq->getFile().contentsToString(SHOW_BRANCHES,
                                true) << endl;
            }
        }
    }

    return info.str();
}


void SMS_RAID_0::clear (){
	
	unsigned int i;
	
		for (i=0; i<requestVector.size(); i++){
			delete (requestVector[i]);
			requestVector[i] = nullptr;
		}	
	
	requestVector.clear();
}


} // namespace icancloud
} // namespace inet

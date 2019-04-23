#include "inet/icancloud/Base/Messages/SMS/SMS_MainMemory.h"

namespace inet {

namespace icancloud {


using namespace omnetpp;

SMS_MainMemory::SMS_MainMemory(unsigned int newMemorySize,
							  unsigned int newReadAheadBlocks,
							  unsigned int newMemoryBlockSize){

	memorySize = newMemorySize;
	readAheadBlocks = newReadAheadBlocks;
	memoryBlockSize = newMemoryBlockSize;
}


void SMS_MainMemory::splitRequest(cMessage*msg){

	int offset;									// Request offset
	unsigned int requestSize;							// Request size

	int currentSubRequest;						// Current subRequest (number)
	int currentOffset;							// Current subRequest offset
	int currentSize;							// Current subRequest size;

	int bytesInFirstBlock;						// Bytes of first blocks (write only)
	int firstRequestBlock;						// First requested block
	int numRequestedBlocks;						// Total number of requested blocks

	int operation;

	auto pkt = check_and_cast<inet::Packet*>(msg);

	const auto &sm = pkt->peekAtFront<icancloud_Message>();
	const auto &sm_io = CHK(dynamicPtrCast<const icancloud_App_IO_Message>(sm));


    // Init...
    currentSubRequest = 0;

    // Cast the original message
    // NET call response...
    if (sm_io != nullptr) {

        // Get the request message parameters...
        offset = sm_io->getOffset();
        requestSize = sm_io->getSize();

        // Bytes in first block!
        bytesInFirstBlock =
                (requestSize < (memoryBlockSize - (offset % memoryBlockSize))) ?
                        requestSize :
                        memoryBlockSize - (offset % memoryBlockSize);

        numRequestedBlocks =
                ((std::abs((int) (requestSize - bytesInFirstBlock)) % memoryBlockSize) == 0) ?
                        (std::abs((int)(requestSize - bytesInFirstBlock)) / memoryBlockSize)
                                + 1 :
                        (std::abs((int)(requestSize - bytesInFirstBlock)) / memoryBlockSize)
                                + 2;

        // Calculate the involved data blocks
        firstRequestBlock = offset / memoryBlockSize;

        operation = sm_io->getOperation();
        // READ?
        pkt->insertAtFront(sm_io);

        if (operation == SM_READ_FILE) {

            currentOffset = firstRequestBlock * memoryBlockSize;

            // read the read_ahead blocks!
            numRequestedBlocks += readAheadBlocks;

            // Add the new Request!
            addRequest(pkt, numRequestedBlocks);

            // Create all corresponding subRequests...
            for (currentSubRequest = 0; currentSubRequest < numRequestedBlocks;
                    currentSubRequest++) {

                auto pktSub = pkt->dup();
#ifdef COPYCONTROLINFO
                // I am not sure if it is necessary to copy the control info, the original code copy it, I cannot find sense
                if (pkt->getControlInfo()) {
                    auto controlOld = check_and_cast<TcpCommand *>(pkt->getControlInfo());
                    pktSub->setControlInfo (controlOld->dup());
                }
#endif
                pktSub->trimFront();
                auto subRequestMsg = pktSub->removeAtFront<icancloud_App_IO_Message>();
                // Creates a new subRequest message with the corresponding parameters
                subRequestMsg->addRequestToTrace(currentSubRequest);
                subRequestMsg->setParentRequest(pkt);
                subRequestMsg->setOffset(currentOffset);
                subRequestMsg->setSize(memoryBlockSize);
                subRequestMsg->updateLength();
                pktSub->insertAtFront(subRequestMsg);

                // Insert current subRequest!
                subRequests.push_back(pktSub);

                // Update offset
                currentOffset += memoryBlockSize;
            }
        }

        // WRITE?
        else {

            // Add the new Request!
            addRequest(pkt, numRequestedBlocks);

            auto pktSub = pkt->dup();

            // Set the first requested block!
            pktSub->trimFront();
            auto subRequestMsg = pktSub->removeAtFront<icancloud_App_IO_Message>();

            subRequestMsg->addRequestToTrace(currentSubRequest);
            subRequestMsg->setParentRequest(pkt);
            subRequestMsg->setOffset(offset);
            subRequestMsg->setSize(bytesInFirstBlock);
            subRequestMsg->updateLength();
            pktSub->insertAtFront(subRequestMsg);
            subRequests.push_back(pktSub);

            // Update offset!
            currentOffset = offset + bytesInFirstBlock;

            // Create all corresponding subRequests...
            for (currentSubRequest = 1; currentSubRequest < numRequestedBlocks;
                    currentSubRequest++) {

                // Set the first requested block!
                auto pktSub = pkt->dup();
                pktSub->trimFront();
                auto subRequestMsg = pktSub->removeAtFront<icancloud_App_IO_Message>();

                subRequestMsg->addRequestToTrace(currentSubRequest);
                subRequestMsg->setParentRequest(pkt);
                subRequestMsg->setOffset(currentOffset);

                // Current subRequest size
                if ((offset + requestSize - currentOffset) >= memoryBlockSize)
                    currentSize = memoryBlockSize;
                else
                    currentSize = offset + requestSize - currentOffset;

                // Set parameters...
                subRequestMsg->setSize(currentSize);
                subRequestMsg->updateLength();

                pktSub->insertAtFront(subRequestMsg);

                // Add current subRequest!
                subRequests.push_back(pktSub);

                // Update offset!
                currentOffset += currentSize;
            }
        }
    }

    // Mem message?
    else {

        const auto &sm_mem = dynamicPtrCast<const icancloud_App_MEM_Message>(sm);


        if (sm_mem != nullptr) {
            auto sm = sm_mem->dupSharedGeneric();

            pkt->insertAtFront(sm_mem);
            addRequest(pkt, 1);
            auto subRequestMsg = dynamicPtrCast<icancloud_App_IO_Message> (sm);
            subRequestMsg->addRequestToTrace(currentSubRequest);
            subRequestMsg->setParentRequest(pkt);

            auto pktSub = new inet::Packet("icancloud_Message");
            pktSub->insertAtFront(subRequestMsg);

            subRequests.push_back(pktSub);
        }

        else {
            printf("Wrong message type!\n");
            exit(0);
        }
    }

    //printf ("abcde\n%s\n", requestToStringByIndex (requestVector.size()-1).c_str());
}


bool SMS_MainMemory::arrivesRequiredBlocks(inet::Packet * request, unsigned int extraBlocks){
	
	int parentIndex, subIndex;
	bool allArrived;

		// Search...
		parentIndex = searchRequest (request);
		allArrived = true;
		subIndex = 0;

		// Request found...
		if (parentIndex != NOT_FOUND){
			while ((subIndex < ((int)((requestVector[parentIndex])->getNumSubRequest())- ((int)extraBlocks))) && (allArrived)){
				
				if ((requestVector[parentIndex])->getSubRequest(subIndex) == nullptr)
					allArrived = false;
				else
					subIndex++;
			}
		}
		else
			allArrived = false;
		
		
	return allArrived;
}


inet::Packet* SMS_MainMemory::getFirstSubRequest(){

	if (subRequests.empty())
		return nullptr;
	else {
	    const auto &sm = subRequests.front()->peekAtFront<icancloud_App_IO_Message>();
	    if (sm == nullptr)
	        throw cRuntimeError("Incorrect header");
		return (subRequests.front());
	}
}


inet::Packet* SMS_MainMemory::popSubRequest(){

	Packet *msg;
	if (subRequests.empty())
	    msg = nullptr;
	else{
	    msg = subRequests.front();
	    subRequests.pop_front();
	}
	const auto &sm = subRequests.front()->peekAtFront<icancloud_Message>();
	const auto &sm_bl = CHK(dynamicPtrCast<const icancloud_App_IO_Message>(sm));
	if (sm_bl == nullptr)
	    throw cRuntimeError("Incorrect header");

	return msg;
}


string SMS_MainMemory::requestToStringByIndex(unsigned int index) {

    std::ostringstream info;
    int i;
    int numSubRequest;
    // Request not found...
    if ((index >= requestVector.size()) || (index < 0)) {
        info << "Request" << index << " Not Found!" << endl;
    }
    // Request found!
    else {
        auto pkt = (requestVector[index])->getParentRequest();
        if (pkt != nullptr) {

            const auto &sm_io = pkt->peekAtFront<icancloud_App_IO_Message>();
            // Get the number of subRequests
            numSubRequest = getNumberOfSubRequest(pkt);
            // Original request info...
            info << " Op:" << sm_io->operationToString() << " File:"
                    << sm_io->getFileName() << " Offset:" << sm_io->getOffset()
                    << " Size:" << sm_io->getSize() << " subRequests:"
                    << numSubRequest << endl;

            // Get info of all subRequests...
            for (i = 0; i < numSubRequest; i++) {

                // Is nullptr?
                if ((requestVector[index])->getSubRequest(i) == nullptr)
                    info << "  subRequest[" << i << "]: Not arrived yet!"
                            << endl;
                else {

                    // Cast!
                    auto pktSubReq = requestVector[index]->getSubRequest(i);

                    const auto &sm_subReq = pktSubReq->peekAtFront<
                            icancloud_App_IO_Message>();
                    info << "  subRequest[" << i << "]:" << " Offset:"
                            << sm_subReq->getOffset() << " Size:"
                            << sm_subReq->getSize() << endl;
                }
            }
        }
    }

    return info.str();
}


void SMS_MainMemory::clear() {

//    std::list<icancloud_App_IO_Message*>::iterator iter;
    unsigned int i;

    for (i = 0; i < requestVector.size(); i++) {
        (requestVector[i])->clear();
        delete (requestVector[i]);
    }

    requestVector.clear();

    // Remove each subRequest message from list!
    for (auto iter = subRequests.begin(); iter != subRequests.end(); iter++) {

        if ((*iter) != nullptr) {
            delete (*iter);
            *iter = nullptr;
        }
    }

    // Remove the list!
    subRequests.clear();
}



} // namespace icancloud
} // namespace inet

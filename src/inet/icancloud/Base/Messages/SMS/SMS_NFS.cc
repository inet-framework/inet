#include "inet/icancloud/Base/Messages/SMS/SMS_NFS.h"

namespace inet {

namespace icancloud {


using namespace inet;
using namespace omnetpp;

SMS_NFS::SMS_NFS (int requestSize_KB){
	requestSizeNFS = requestSize_KB*KB;
}


void SMS_NFS::splitRequest(cMessage *msg) {

    int currentSubRequest;					// Current subRequest
    int numberOfsubRequest;					// Total number of subrequest

    int requestSize;						// Request size!
    int currentOffset;						// Current subRequest offset
    int currentSubRequestSize;				// Current subRequest size
    int currentSize;			// Current accumulated size, 0 to requestSize

    //icancloud_App_IO_Message *sm_io;			// Message (request) to make the casting!
    //icancloud_App_IO_Message *subRequestMsg;	// SubRequests message!

    int operation;

    // Init...
    numberOfsubRequest = currentSubRequest = currentSubRequestSize = 0;
    currentOffset = currentSize = requestSize = 0;

    auto pkt = check_and_cast<inet::Packet*>(msg);

    const auto &sm = pkt->peekAtFront<icancloud_Message>();
    const auto &sm_io = CHK(dynamicPtrCast<const icancloud_App_IO_Message>(sm));

    // Cast the original message
    //sm_io = check_and_cast<icancloud_App_IO_Message *>(msg);
    operation = sm_io->getOperation();

    // Read or write operation! Calculate number of subRequests!
    if ((operation == SM_READ_FILE) || (operation == SM_WRITE_FILE)) {

        // Get the offset and size!
//			requestSize = sm_io->getByteLength();
        requestSize = sm_io->getSize();
        currentOffset = sm_io->getOffset();

        // Calculate the number of subRequests
        numberOfsubRequest = ((requestSize % requestSizeNFS) == 0) ?  (requestSize / requestSizeNFS) : (requestSize / requestSizeNFS) + 1;
    }

    // only 1 message, do not split!
    else
        numberOfsubRequest = 1;

    // Add the new Request!

    addRequest(pkt, numberOfsubRequest);

    // Read or write operation!
    if ((operation == SM_READ_FILE) || (operation == SM_WRITE_FILE)) {

        // Generate the subRequest!
        for (currentSubRequest = 0; currentSubRequest < numberOfsubRequest;
                currentSubRequest++) {

            // Calculates current subRequest size
            currentSubRequestSize =
                    ((requestSize - currentSize) >= requestSizeNFS) ?
                            requestSizeNFS : (requestSize - currentSize);

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
            subRequestMsg->setParentRequest(pkt);
            subRequestMsg->setOffset(currentOffset);
            subRequestMsg->setChunkLength(B(currentSubRequestSize));
//	    		subRequestMsg->setSize(currentSubRequestSize);

            // Link current subRequest with its parent request
            pktSub->insertAtFront(subRequestMsg);
            setSubRequest(pkt, pktSub, currentSubRequest);

            // Update current request size part!
            currentSize += currentSubRequestSize;
            currentOffset += currentSubRequestSize;

            // Set subRequest message length
            auto subRequestMsgAux = pktSub->removeAtFront<icancloud_App_IO_Message>();
            if (operation == SM_READ_FILE)
                subRequestMsgAux->setChunkLength(B(SM_NFS2_READ_REQUEST));
            else if (operation == SM_WRITE_FILE)
                subRequestMsgAux->setChunkLength(B(SM_NFS2_WRITE_REQUEST + currentSubRequestSize));
            // Update the current subRequest Message ID...
            subRequestMsgAux->addRequestToTrace(currentSubRequest);
            pktSub->insertAtFront(subRequestMsgAux);
        }
    }

    // Do not split the message!
    else if ((operation == SM_CREATE_FILE) || (operation == SM_DELETE_FILE)
            || (operation == SM_OPEN_FILE) || (operation == SM_CLOSE_FILE)) {

        // Copy the message!
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
        subRequestMsg->setParentRequest(pkt);
        pktSub->insertAtFront(subRequestMsg);


        // Link current subRequest with its parent request
        setSubRequest(pkt, pktSub, 0);

        auto subRequestMsgAux = pktSub->removeAtFront<icancloud_App_IO_Message>();
        // Set subRequest message length
        if (operation == SM_CREATE_FILE)
            subRequestMsgAux->setChunkLength(B(SM_NFS2_CREATE_REQUEST));
        else if (operation == SM_DELETE_FILE)
            subRequestMsgAux->setChunkLength(B(SM_NFS2_DELETE_REQUEST));
        else if (operation == SM_OPEN_FILE)
            subRequestMsgAux->setChunkLength(B(SM_NFS2_OPEN_REQUEST));
        else if (operation == SM_CLOSE_FILE)
            subRequestMsgAux->setChunkLength(B(SM_NFS2_CLOSE_REQUEST));
        // Update the current subRequest Message ID...
        subRequestMsgAux->addRequestToTrace(currentSubRequest);
        pktSub->insertAtFront(subRequestMsgAux);
    }
}


string SMS_NFS::requestToStringByIndex(unsigned int index) {
    std::ostringstream info;
    int i;
    int numSubRequest;
    //icancloud_App_IO_Message *sm_subReq;
    //icancloud_App_IO_Message *sm_io;
    // Request not found...
    if ((index >= requestVector.size()) || (index < 0)) {
        info << "Request" << index << " Not Found!" << endl;
    }
    // Request found!
    else {

        auto pkt = (requestVector[index])->getParentRequest();
        const auto &sm_io = pkt->peekAtFront<icancloud_App_IO_Message>();
        //sm_io = check_and_cast<icancloud_App_IO_Message *>((requestVector[index])->getParentRequest());
        // Get the number of subRequests
        numSubRequest = getNumberOfSubRequest(pkt);
        // Original request info...
        info << " Op:" << sm_io->operationToString() << " File:"
                << sm_io->getFileName() << " Offset:"
                << sm_io->getOffset() << " Size:" << sm_io->getSize()
                << " subRequests:" << numSubRequest << endl;
        // Get info of all subRequests...
        for (i = 0; i < numSubRequest; i++) {

            // Is nullptr?
            if ((requestVector[index])->getSubRequest(i) == nullptr)
                info << "  subRequest[" << i << "]: Not arrived yet!" << endl;
            // SubRequest is here!!!
            else {
                // Cast!
                auto pktSubReq = requestVector[index]->getSubRequest(i);
                const auto &sm_subReq = pktSubReq->peekAtFront<icancloud_App_IO_Message>();
                // Has already arrived?
                if (!sm_subReq->getIsResponse())
                    info << "  subRequest[" << i << "]: Not sent yet!" << endl;
                else
                    info << "  subRequest[" << i << "]:" << " Op:"
                    << sm_subReq->operationToString()
                    << " Offset:" << sm_subReq->getOffset()
                    << " Size:" << sm_subReq->getSize() << endl;
            }
        }
    }
    return info.str();
}


void SMS_NFS::clear (){
	unsigned int i;
	for (i=0; i<requestVector.size(); i++){
	    (requestVector[i])->clear();
	    delete (requestVector[i]);
	}
	requestVector.clear();
}


} // namespace icancloud
} // namespace inet

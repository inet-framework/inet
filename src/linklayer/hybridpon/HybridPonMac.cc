///
/// @file   HybridPonMac.cc
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Jun/30/2009
///
/// @brief  Implements 'HybridPonMac' class for a Hybrid TDM/WDM-PON ONU.
///
/// @remarks Copyright (C) 2009-2010 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


// #define DEBUG_SLOT_MGR


#include "HybridPonMac.h"


// Register modules.
Define_Module(HybridPonMac);


///
/// Handle a user frame from UNIs. Put the frame into a FIFO, if there
/// is enough space. Otherwise, drop it.
/// @param[in] frame a cPacket pointer
///
void HybridPonMac::handleFrameFromUni(cPacket *frame)
{
#ifdef DEBUG_HYBRIDPONMAC
    ev << getFullPath() << ": A frame from UNI received" << endl;
#endif

//     if ( busyQueue + frame->getBitLength() + ETH_OVERHEAD_SIZE <= queueSize) {
//         EthFrame *ethFrame = new EthFrame();
//         ethFrame->setBitLength(ETH_OVERHEAD_SIZE);
//         ethFrame->encapsulate(pkt);
//         busyQueue += ethFrame->getBitLength();
//         queue.insert( ethFrame );

#ifdef DEBUG_HYBRIDPONMAC
        ev << getFullPath() << ": busyQueue = " << busyQueue << endl;
#endif
//     }
//     else {
//         monitor->recordLossStats(frame->getSrcAddress(), frame->getDstnAddress(), frame->getBitLength() );
//         delete frame;

#ifdef DEBUG_HYBRIDPONMAC
        ev << getFullPath() << ": A frame from UNI dropped!" << endl;
#endif
//    }
}


///
/// Handle a PON frame from PON. Extract user frames from it and send
/// them to UNIs.
///
/// @param[in] frame a HybridPonFrame pointer
/// @todo Implement local switching for multiple UNIs.
///
void HybridPonMac::handleDataFromPon(HybridPonFrame *frame)
{
#ifdef DEBUG_HYBRIDPONMAC
    ev << getFullPath() << ": PON frame received with downstream data" << endl;
#endif

    cQueue &frameQueue = frame->getEncapsulatedFrames();
    while(!frameQueue.empty()) {
        cPacket *frame = (cPacket *)frameQueue.pop();
 //       IpPacket *pkt = (IpPacket *)ethFrame->decapsulate();
        send(frame, "toUni", 0);  /**< @todo Support multiple UNIs with local switching. */
        delete (cPacket *) frame;
    }
    delete frame;
}


///
/// Handle a PON frame from PON including a grant for ONU upstream
/// data. Creates a new PON frame, encapsulate user frames from the
/// FIFO queue in it, and send it to OLT.
///
/// @param[in] frame a HybridPonFrame pointer
///
void HybridPonMac::handleGrantFromPon(HybridPonFrame *ponFrameDs)
{
#ifdef DEBUG_HYBRIDPONMAC
    ev << getFullPath() << ": PON frame with grant received =" << PonFrameDs->getGrant() << endl;
#endif

    HybridPonFrame *ponFrameUs = new HybridPonFrame("", HYBRID_PON_FRAME);
    ponFrameUs->setLambda(lambda);
    cPacketQueue &frameQueue = ponFrameUs->getEncapsulatedFrames();
    int encapsulatedFramesSize = 0;

    // First, check the grant size for upstream data to determine
    // if it's for polling (request) only or polling and upstream data.
	int grant = ponFrameDs->getGrant();
    if (grant > 0) {
        // It's for both polling (request) and upstream data.

        if (queue.empty() != true) {
            cPacket *frame = (cPacket *)queue.front();
            while ( grant >= (frame->getBitLength() + encapsulatedFramesSize) ) {
                // The length of the 1st frame in the FIFO queue is less than or
                // equal to the remaining data grant.

                frame = (cPacket *)queue.pop();
                encapsulatedFramesSize += frame->getBitLength();
                busyQueue -= frame->getBitLength();
                frameQueue.insert(frame);

                if (queue.empty() == true) {
                    break;
                }
                else {
                    frame = (cPacket *)queue.front();
                }
            }   // end of while loop
        }
    }

    // Set request and other fields of the new upstream PON frame.
    ponFrameUs->setRequest(busyQueue);
    ponFrameUs->setLambda(lambda);
    ponFrameUs->setBitLength(PREAMBLE_SIZE + DELIMITER_SIZE + REQUEST_SIZE + encapsulatedFramesSize);

#ifdef DEBUG_HYBRIDPONMAC
    ev << getFullPath() << ": PON frame sent upstream with length = " <<
        ponFrameUs->getBitLength() << " and request = " << ponFrameUs->getRequest() << endl;
#endif

    // Here we don't include transmission delay because in PON,
    // the ONU only modulates incoming CW burst, not receives it as usual.
    send(ponFrameUs, "toOlt");

    delete ponFrameDs;
}


///
/// Initialize member variables and allocate memory for them, if needed.
///
void HybridPonMac::initialize()
{
	lambda = (int) getParentModule()->par("lambda");
	queueSize = par("queueSize");
    busyQueue = 0;

// 	monitor = (Monitor *) ( gate("toMonitor")->getPathEndGate()->getOwnerModule() );
// #ifdef DEBUG_HYBRIDPONMAC
// 	ev << getFullPath() << ": monitor pointing to module with id = " << monitor->getId() << endl;
// #endif
}


///
/// Handle messages by calling appropriate functions for their
/// processing. Start simulation and run until it will be terminated by
/// kernel.
///
/// @param[in] msg
///
void HybridPonMac::handleMessage(cMessage *msg)
{
#ifdef TRACE_MSG
	ev.printf();
	PrintMsg(*msg);
#endif

	if (msg->getArrivalGateId() == findGate("ponGate$i")) {
		// It is a PON frame from the OLT.

		// Check if the message type is correct.
		assert ( msg->getKind() == HYBRID_PON_FRAME );

		//HybridPonFrame *ponFrameFromOlt = (HybridPonFrame *)msg;
		if ( ((HybridPonFrame *)msg)->getFrameType() == 0 ) {
			// It's a PON frame with downstream data.
			handleDataFromPon((HybridPonFrame *)msg);
		}
		else {
			// It's a PON frame with grant.
			handleGrantFromPon((HybridPonFrame *)msg);
		}
	}
	else if (1) {	// TODO: Insert a condition to check the input gate of the message.
		// It is a frame from the UNIs.

		handleFrameFromUni((cPacket *)msg);
	}
	else {
		ev << getFullPath() << ": ERROR: unexpected message kind " << msg->getKind() << "received." << endl;
		delete (cMessage *) msg;
		exit(1);
	}
}


///
/// Do post-processing.
///
void HybridPonMac::finish()
{
}

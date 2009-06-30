///
/// @file   HybridPonMac.cc
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Tue Jun 30 12:26:54 2009
/// 
/// @brief  Implements 'HybridPonMac' class for a Hybrid TDM/WDM-PON ONU.
///
/// @remarks Copyright (C) 2009 Kyeong Soo (Joseph) Kim. All rights reserved.
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

    cQueue &ethFrameQueue = frame->getEncapsulatedEthFrames();
    while(!ethFrameQueue.empty()) {
        EthFrame *ethFrame = (EthFrame *)ethFrame4Queue.pop();
        IpPacket *pkt = (IpPacket *)ethFrame->decapsulate();
        send(pkt, "toUni", 0);  /**< @todo Support multiple UNIs with local switching. */
        delete (EthFrame *) ethFrame;
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
void HybridPonMac::handleGrantFromPon(HybridPonFrame *frame)
{
#ifdef DEBUG_HYBRIDPONMAC
    ev << getFullPath() << ": PON frame with grant received =" << frame->getGrant() << endl;
#endif

    HybridPonFrame *ponFrameToOlt = new HybridPonFrame("", HYBRID_PON_FRAME);
    ponFrameToOlt->setLambda(lambda);
    cQueue &ethFrameQueue = ponFrameToOlt->getEncapsulatedEthFrames();
    int encapsulatedEthFramesSize = 0;

    // First, check the grant size for upstream data to determine
    // if it's for polling (request) only or polling and upstream data.
	int grant = frame->getGrant();
    if (grant > 0) {
        // It's for both polling (request) and upstream data.

        if (queue.empty() != true) {
            EthFrame *ethFrame = (EthFrame *)queue.front();
            while ( grant >= (ethFrame->getBitLength() + encapsulatedEthFramesSize) ) {
                // The length of the 1st Ethernet frame in the FIFO queue is less than or
                // equal to the remaining data grant.

                ethFrame = (EthFrame *)queue.pop();
                encapsulatedEthFramesSize += ethFrame->getBitLength();
                busyQueue -= ethFrame->getBitLength();
                ethFrameQueue.insert(ethFrame);

                if (queue.empty() == true) {
                    break;
                }
                else {
                    ethFrame = (EthFrame *)queue.front();
                }
            }   // end of while loop
        }
    }

    // Set request and other fields of the new upstream PON frame.
    ponFrameToOlt->setRequest(busyQueue);
    ponFrameToOlt->setLambda(lambda);
    ponFrameToOlt->setBitLength(PREAMBLE_SIZE + DELIMITER_SIZE + REQUEST_SIZE + encapsulatedEthFramesSize);

#ifdef DEBUG_HYBRIDPONMAC
    ev << getFullPath() << ": PON frame sent upstream with length = " <<
        ponFrameToOlt->getBitLength() << " and request = " << ponFrameToOlt->getRequest() << endl;
#endif

    // Here we don't include transmission delay because in PON,
    // the ONU only modulates incoming CW burst, not receives it as usual.
    send(ponFrameToOlt, "toOlt");

    delete frame;
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

    switch (msg->getKind()) {

    case IP_PACKET:
        receiveFrameFromUni((cPacket *)msg);
        break;

    case HYBRID_PON_FRAME:
		// Check if the PON frame received is from OLT.
		assert( msg->getArrivalGateId() == findGate("fromOlt") );

        //if (msg->getArrivalGateId() == findGate("fromOlt")) {
        //    // It's a downstream PON frame from the OLT.

		//HybridPonFrame *ponFrameFromOlt = (HybridPonFrame *)msg;
		if ( ((HybridPonFrame *)msg)->getId() == true ) {
			// It's a PON frame with downstream data.
			receiveDataFromPon((HybridPonFrame *)msg);
		}
		else {
			// It's a PON frame with grant.
			receiveGrantFromPon((HybridPonFrame *)msg);
		}

		//}
        break;

    default:
        ev << getFullPath() << ": ERROR: unexpected message kind " << msg->getKind() << "received." << endl;
        delete (cMessage *) msg;
        exit(1);
    }   // end of switch()
}


/// 
/// Do post-processing.
///
void HybridPonMac::finish()
{
}

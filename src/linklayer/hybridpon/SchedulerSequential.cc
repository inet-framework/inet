// $Id$
//------------------------------------------------------------------------------
//	scheduler_sequential.cc --
//
//	This file implements 'Sequential' class, derived from the 'Scheduler'
//  class, for PON OLT.
//
//	Copyright (C) 2009 Kyeong Soo (Joseph) Kim
//------------------------------------------------------------------------------


// For debugging
// #define DEBUG_SCHEDULER


#include "Scheduler.h"


// Register module
Define_Module(Sequential);


//------------------------------------------------------------------------------
// Sequential::receiveIpPacket --
//
//		receives an IP packet from the upper layer.
//
// Arguments:
// 		IpPacket *pkt;
//
// Results:
//		A PON frame is generated, and unless a buffer overflow occurs,
//      its transmission is scheduled.
//------------------------------------------------------------------------------

void Sequential::receiveIpPacket(IpPacket *pkt)
{
#ifdef DEBUG_SCHEDULER
    ev << getFullPath() << ": IP packet received" << endl;
#endif

    // Check if there is room in the queue for a PON frame
    // corresponding to the incoming IP packet
    if (busyQueue + pkt->getBitLength() + ETH_OVERHEAD_SIZE + DS_DATA_OVERHEAD_SIZE <= queueSize) {

        // **** for now, this is how we determine to which ONU it is addressed
        // **** assumes equal number of users per onu, as we have been doing
        int lambda = pkt->getDstnAddress()/numUsersPerOnu;

#ifdef DEBUG_SCHEDULER
        ev << getFullPath() << ": dstnAddress = " << pkt->getDstnAddress() << ", destination ONU = " << lambda << endl;
#endif

        // Encapsulate IP packet in Ethernet frame
        EthFrame *ethFrame = new EthFrame();
        ethFrame->setBitLength(ETH_OVERHEAD_SIZE); // including preamble, DA, SA, FT, & CRC fields
        ethFrame->encapsulate(pkt);
        int frameLength = ethFrame->getBitLength();

        // Create ponFrame
        HybridPonFrame *ponFrameToOnu = new HybridPonFrame("", HYBRID_PON_FRAME);
        ponFrameToOnu->setLambda(lambda);
        ponFrameToOnu->setId(true);
        // detected by Joseph Kim, 12/14/2003
        // ponFrameToOnu->setBitLength(PREAMBLE_SIZE + DELIMITER_SIZE + ID_SIZE + pkt->getBitLength());
        ponFrameToOnu->setBitLength(frameLength + DS_DATA_OVERHEAD_SIZE);
        (ponFrameToOnu->getEncapsulatedEthFrames()).insert(ethFrame);

        // We emulate a queueing operation using a counter (busyQueue) and
        // an additional event scheduled for a messsage deletion from a queue
        // when it is actually transmitted later.
        busyQueue += ponFrameToOnu->getBitLength();   // increase queue size by the length of the PON frame
        simtime_t txTime = seqSchedule(lambda, ponFrameToOnu);
        DummyPacket *msg = new DummyPacket("", ACTUAL_TX);
        msg->setBitLength(frameLength);
        scheduleAt(txTime, msg);
    }
    else {
        monitor->recordLossStats(pkt->getSrcAddress(), pkt->getDstnAddress(), pkt->getBitLength());
#ifdef DEBUG_SCHEDULER
        ev << getFullPath() << ": IP packet dropped due to buffer overflow!" << endl;
#endif
        delete (IpPacket *) pkt;
    }
}


//------------------------------------------------------------------------------
// Sequential::handleGrant --
//
//		handles a grant PON frame to an ONU.
//
// Arguments:
//      int             lambda;
// 		HybridPonFrame    *grant;
//
// Results:
//      The grant PON frame is scheduled for TX through seqSchedule().
//------------------------------------------------------------------------------

void Sequential::handleGrant(int lambda, HybridPonFrame *grant)
{
    if (seqSchedule(lambda, grant) < 0.0) {
        // Scheduling cancelled!
#ifdef DEBUG_SCHEDULER
        ev << getFullPath() << ": grant PON frame lost due to excessive scheduling delay!" << endl;
#endif
    }
    // Reschedule new ONU pollEvent
    if (pollEvent[lambda]->isScheduled()) {
        cancelEvent(pollEvent[lambda]);
    }
    scheduleAt(simTime()+onuTimeout, pollEvent[lambda]);
}


//------------------------------------------------------------------------------
// Sequential::transmitDataFrame --
//
//		emulates actual transmission of downstream data frame by decreasing
//      a queue counter implementing a virtual FIFO for downstream data.
//
// Arguments:
//      DummyPacket *msg;
//
// Results:
//		The queue counter has been decreased by the frame length transmitted.
//		The message has been deleted.
//------------------------------------------------------------------------------

void Sequential::transmitDataFrame(DummyPacket *msg)
{
#ifdef DEBUG_SCHEDULER
    ev << getFullPath() << ": Actual transmission of scheduled message" << endl;
#endif
    busyQueue -= msg->getBitLength();
    if (busyQueue < 0) {
        ev << getFullPath() << ": ERROR: something wrong with busyQueue counter!" << endl;
        exit(1);
    }
    delete (DummyPacket *)msg;
}


//------------------------------------------------------------------------------
// Sequential::initializeSpecific --
//
//		does initialization specific to a sequential mode.
//
// Arguments:
//
// Results:
//		All member variables are allocated memory and initialized.
//------------------------------------------------------------------------------

void Sequential::initializeSpecific(void)
{
    // Initialize NED parameters.
	queueSize = par("queueSize");

    busyQueue = 0;
}


//------------------------------------------------------------------------------
// Sequential::handleMessage --
//
//		handles messages by calling appropriate functions for their processing.
//
// Arguments:
// 		cMessage *msg
//
// Results:
//		Simulation starts and runs until it will be terminated by kernel.
//------------------------------------------------------------------------------

void Sequential::handleMessage(cMessage *msg)
{
#ifdef TRACE_MSG
	ev.printf();
	PrintMsg(*msg);
#endif

	switch (msg->getKind()) {
    case ONU_POLL:
        sendOnuPoll((HybridPonMessage *)msg);
        break;
    case IP_PACKET:
        // Check if the IP packet received is from an IP Packet generator.
        if ( (msg->getArrivalGateId() == gate("fromPacketGenerator")->getId()) ) {
            receiveIpPacket((IpPacket *)msg);
        }
        break;
    case HYBRID_PON_FRAME:
        // Check if the PON frame received is from an ONU.
        if ( msg->getArrivalGateId() == gate("in")->getId() ) {
            receiveHybridPonFrame((HybridPonFrame *)msg);
        }
        break;
    case ACTUAL_TX:
        transmitDataFrame((DummyPacket *)msg);
        break;
    case ACTUAL_TX_POLL:
        transmitPollFrame((HybridPonMessage *)msg);
        break;
    default:
        ev << getFullPath() << ": Unexpected message type: " << msg->getKind() << endl;
        exit(1);
	}	// end of switch()
}


//------------------------------------------------------------------------------
// Sequential::finishSpecific
//
//		does finalization specific to a sequential mode.
//
// Arguments:
//
// Results:
//		Any manually allocated memories for member variables have been deallocated.
//------------------------------------------------------------------------------

void Sequential::finishSpecific(void)
{
}

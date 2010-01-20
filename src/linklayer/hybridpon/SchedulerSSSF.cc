// $Id$
//-------------------------------------------------------------------------------
//	SchedulerSSSF.cc --
//
//	This file implements an 'SSSF' class for the "Sequential Scheduling with
//	Schedule-time Framing (S^3F)" algorithm reported in [1] for SUCCESS WDM-PON.
//
//	This scheduler has the following features:
//	- Based on VOQing (of Ethernet frames for downstream), one FIFO queue per
//		stream (either up- or downstream).
//		*** For the sake of implementation simplicity, now we have two separate
//		*** physical queues, VOQ and Tx Queue, but they are handled as one
//		*** logical queue, esp. in processing bit and frame length counters;
//	- Instant framing of downstream data as in ONU;
//	- Granting based on queue statuses for both up- and downstream traffic
//      with independent max. size limitation.
//
//  References:
//  [1] Kyeong Soo Kim, David Gutierrez, Fu-Tai An, and Leonid G. Kazovsky,
//      "Design and performance analysis of scheduling algorithms for WDM-PON
//      under SUCCESS-HPON architecture," IEEE/OSA Journal of Lightwave Technology,
//      vol. 23, no. 11, pp. 3716-3731, Nov. 2005.
//
//	Copyright (C) 2009 Kyeong Soo (Joseph) Kim
//-------------------------------------------------------------------------------


// For debugging
// #define DEBUG_SCHEDULER


#include "Scheduler.h"


// Register module.
Define_Module(SSSF);


//------------------------------------------------------------------------------
//	Misc. functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// SSSF::assignGrants --
//
//		Assigns grants for both up- and downstream traffic.
//
// Arguments:
//		int		ch;			// Channel index
//		int		usRequest;	// Request from an ONU for upstream traffic
//
// Results:
//      Grants are assigned to both up- and downstream traffic proportional
//		to their requests (ONU upstream queue status for upstream and
//		OLT VOQ status for downstream, respectively).
//		Note that we limit each size of grant to 'cwMax' independently.
//------------------------------------------------------------------------------

int SSSF::assignGrants(int ch, int usRequest)
{
	////////////////////////////////////////////////////////////////////////
	// Assign grants to upstream (via 'return') and downstream traffic (via
	// 'dsTxCtr[ch]') based on the upstream queue status at ONU and the
	// downstream VOQ status at OLT.
	// *** Note that the 'voqBitCtr[ch]' counts the # of bits in txQueue[ch]
	// *** as well, which are for Ethernet frames already scheduled for
	// *** transmission and therefore should be excluded in granting.
	////////////////////////////////////////////////////////////////////////
	int dsRequest = voqBitCtr[ch];
	if (txQueue[ch].empty() == false) {
		HybridPonFrame *frame = (HybridPonFrame *)txQueue[ch].front();
		dsRequest -= frame->getBitLength() - DS_DATA_OVERHEAD_SIZE;
	}

    dsTxCtr[ch] = min(dsRequest, cwMax);
    return min(usRequest, cwMax);
}


//------------------------------------------------------------------------------
// SSSF::debugOnuPollListStatus --
//
//		Generates a snapshot of 'onuPollList' status.
//
// Arguments:
//
// Results:
//      Prints out all the elements of 'onuPollList' to the event window.
//------------------------------------------------------------------------------

void SSSF::debugOnuPollListStatus(void)
{
	ev << "onuPollList status at " << simTime() << " sec.:" << endl;
	ev << "Number of elements = " << onuPollList.size() << endl;
	OnuPollList::const_iterator iter;
	int i;
	for (iter = onuPollList.begin(), i = 0; iter != onuPollList.end(); ++iter, ++i) {
		ev << "onuPollList[" << i << "]:" << endl;
		ev << "\t- channel =\t" << (*iter).channel << endl;
		ev << "\t- time =\t" << (*iter).time << endl;
	}
}


//------------------------------------------------------------------------------
// SSSF::cancelOnuPoll --
//
//		Cancels an ONU poll message and removes a corresponding element
//		from the 'onuPollList'.
//
// Arguments:
//      cMessage    msg;    ONU_POLL event
//
// Results:
//      The given message is cancelled in the main event list and the corresponding
//		element (i.e., the one with the same channel) is removed from 'onuPollList'.
//      It returns the pointer to the cancelled message.
//		Otherwise, 'NULL' is returned.
//------------------------------------------------------------------------------

cMessage *SSSF::cancelOnuPoll(HybridPonMessage *msg)
{
	if (onuPollList.empty()) {
		// Something wrong!
		error("%s::cancelOnuPoll: Tried to remove an element from empty 'onuPollList'", getFullPath().c_str());
	}
	else {
		// Find the element with the same 'channel' as the given message.
		int ch = msg->getOnuIdx();				// get ONU/channel index.
		OnuPollList::iterator iter;
		for (iter = onuPollList.begin(); iter != onuPollList.end(); ++iter) {
			if ((*iter).channel == ch) {
				onuPollList.erase(iter);	// Remove it from the list.
				return cancelEvent(msg);	// Do normal event cancellation.
			}
		}
	}
	return NULL;
}


//------------------------------------------------------------------------------
// SSSF::scheduleOnuPoll --
//
//		Schedules an ONU poll message and maintains a sorted list of pairs of
//		the channel and scheduling time of the message.
//
// Arguments:
//      simtime_t   t;      scheduling time
//      cMessage    msg;    ONU_POLL event
//
// Results:
//      The given message is scheduled in the main event list and its channel
//      and scheduling time is stored in the sorted list (ascending order
//      based on the scheduling time).
//      It returns 0 for compatibility with the 'scheduleAt()' function.
//------------------------------------------------------------------------------

int SSSF::scheduleOnuPoll(simtime_t t, HybridPonMessage *msg)
{
    // Maintain a sorted list of the scheduled ONU poll events.
	OnuPoll poll;
    poll.channel = msg->getOnuIdx();
    poll.time = t;
    OnuPollList::iterator iter;
    bool inserted = false;
    for (iter = onuPollList.begin(); iter != onuPollList.end(); ++iter) {
        if (t < (*iter).time) {
            onuPollList.insert(iter, poll);
            inserted = true;
            break;
        }
    }
	if (!inserted) {
		// The current message has the latest (largest) scheduling time.
		// So, append it at the end of the list.
        onuPollList.push_back(poll);
	}

    return scheduleAt(t, msg);		// Do normal message schedule.
}


////------------------------------------------------------------------------------
//// SSSF::rescheduleOnuPolls --
////
////		Reschedules scheduled ONU poll messages based on the scheduled time concept.
////
//// Arguments:
////
//// Results:
////      If there is any 'ONU_POLL' message whose scheduled time is less than
////		the scheduled TX time of the corresponding polling frame as of now,
////		the message is rescheduled for now immediately.
////      Otherwise, nothing happens.
////------------------------------------------------------------------------------
//
//void SSSF::rescheduleOnuPolls(void)
//{
//	int ctr = 0;        // Counter for the # of checking for rescheduling
//	OnuPollList::iterator iter;
//
//	for (iter = onuPollList.begin(); iter != onuPollList.end(); iter++) {
//		// Check if the # of checking exceeds the rescheduling depth.
//		ctr++;
//		if (ctr > rsDepth) {
//			break;
//		}
//
//		int ch = (*iter).channel;
//		simtime_t t = (*iter).time;
//
//		// Get a scheduled TX time for this channel now.
//		int txIdx, rxIdx;
//		simtime_t txTime = getTxTime(ch, true, txIdx, rxIdx);
//
//		// Check if the scheduled TX time is equal to or greater than the scheduled ONU timeout.
//		if (txTime >= t) {
//
//			// Cancel the 'ONU_POLL' message and reschedule it right now.
//			if (pollEvent[ch]->isScheduled()) {
//				cancelOnuPoll(pollEvent[ch]);
//				scheduleOnuPoll(simTime(), pollEvent[ch]);
//			}
//			else {
//				error("%s::rescheduleOnuPolls: Error in ONU poll rescheduling", getFullPath().c_str());
//			}
//
//			// Should break here because the iterator is invalidated!
//			break;
//		}
//	}   // end of for()
////      const int		ch;			// channel index
//}


//------------------------------------------------------------------------------
//	Scheduling functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// SSSF::getTxTime --
//
//		frame based on the sequential scheduling algorithm.
//
// Arguments:
//      const bool		isGrant;	// flag for grant
//		int				&txIdx;		// TX index
//		int				&rxIdx;		// RX index
//
// Results:
//      Calculates the earliest possible transmission time for given channel and
//		The earliest possible transmission time is returned.
//		Also, 'txIdx' and 'rxIdx' (only for a grant) are set to indices
//		of the earliest available TX and RX, respectively.
//------------------------------------------------------------------------------

simtime_t SSSF::getTxTime(
	const int ch,
	const bool isGrant,
	int &txIdx,
	int &rxIdx
)
{
	// Initialize variables.
//	int d = ch;
	simtime_t t;
	simtime_t now = simTime();

	// Update all CH[], TX[], RX[] that are less than the current simulation
    // time to the current simulation time.
	for (int i = 0; i < numOnus; i++) {
		if (CH[i] < now) {
			CH[i] = now;
		}
	}
	for (int i = 0; i < numTransmitters; i++) {
		if (TX[i] < now) {
			TX[i] = now;
		}
	}
	for (int i = 0; i < numReceivers; i++) {
		if (RX[i] < now) {
			RX[i] = now;
		}
	}

#ifdef DEBUG_SCHEDULER
	ev << getFullPath() << "::getTxTime() is called for a "
		<< (isGrant ? "grant" : "data")
		<< " PON frame for ONU[" << ch << "]" << endl;
#endif

	// Pick the earliest available transmitter.
	txIdx = 0;
	for (int i = 1; i < numTransmitters; i++) {
		if (TX[i] <= TX[txIdx]) {
			txIdx = i;
		}
	}

	if (isGrant) {	// This is a grant frame.

		// Pick the earliest available receiver.
		rxIdx = 0;
		for (int i = 1; i < numReceivers; i++) {
			if (RX[i] <= RX[rxIdx]) {
				rxIdx = i;
			}
		}

		// Schedule transmission time 't' as follows:
		// t = max(receiver is free, transmitter is free, channel is free)
		t = max(
				max(
					RX[rxIdx] + GUARD_TIME - RTT[ch] - DS_GRANT_OVERHEAD_SIZE/BITRATE,
					TX[txIdx] + GUARD_TIME
				),
				CH[ch] + GUARD_TIME
			);
    }
    else {	// This is a data frame.

        // Schedule transmission time 't' as follows:
        // t = max(transmitter is free, channel is free)
        t = max( TX[txIdx] + GUARD_TIME, CH[ch] + GUARD_TIME );
    }

	return t;	// Return the calculated transmission time.
}


//------------------------------------------------------------------------------
// SSSF::scheduleFrame --
//
//      Schedules the transmission (and the upstream reception as well for a grant)
//		of a PON frame based on the sequential scheduling algorithm.
//
// Arguments:
//		const simtime_t	txTime;
//      const int		ch;
//		const int		txIdx;
//		const int		rxIdx;
//      HybridPonFrame	*ponFrameToOnu;
//
// Results:
//		Global status variables are updated and the frame transmission is scheduled.
//------------------------------------------------------------------------------

void SSSF::scheduleFrame(
	const simtime_t txTime,
	const int		ch,
	const int		txIdx,
	const int		rxIdx,
	HybridPonFrame	*ponFrameToOnu)
{
	// DEBUG: Check if the scheduled TX time is already passed.
	assert( txTime >= simTime() );

	// Get frame attributes and initialize variables used frequently.
	bool id = ponFrameToOnu->getId();
	simtime_t txDelay = ponFrameToOnu->getBitLength() / BITRATE;

#ifdef DEBUG_SCHEDULER
	ev << getFullPath() << "::scheduleFrame() called for a " << (id ? "data" : "grant")
		<< "PON frame for ONU[" << ch << "]" << endl;
#endif

    // Update transmitter and channel available times.
	CH[ch] = txTime + txDelay;
    TX[txIdx] = txTime + txDelay;

	// For a grant, update the receiver available time as well.
	if (!id) {
        RX[rxIdx] = txTime + txDelay + RTT[ch];
		// Note that the RX[rxIdx] is scheduled for reception via CH[ch] at 'txTime+RTT[ch]'
		// and will be available after the frame reception ('txDelay').

#ifdef TRACE_TXRX
		// Schedule a frame reception to record RX usage.
		DummyPacket *msg = new DummyPacket("Frame RX", RECEIVE_RX);
		msg->setLambda(ch);
		msg->setIdx(rxIdx);
		msg->setBitLength(ponFrameToOnu->getBitLength());
		scheduleAt(txTime + RTT[ch], msg);
#endif
	}

#ifdef DEBUG_SCHEDULER
    if (id) {
        ev << getFullPath() << ": A data frame scheduled at " << txTime << " sec." << endl;
        ev << getFullPath() << ": using TX[" << txIdx << "] and CH[" << ch << "]" << endl;
    }
    else {
        ev << getFullPath() << ": A grant frame scheduled at " << txTime << " sec." << endl;
        ev << getFullPath() << ": using TX[" << txIdx << "], CH[" << ch << "] and reception at time = "
           << txTime + RTT[ch] << " using RX [" << rxIdx << "]" << endl;
    }
#endif

	// Finally, schedule a frame transmission at 'txTime'.
	DummyPacket *msg = new DummyPacket("Actual Frame TX", ACTUAL_TX);
	msg->setLambda(ch);
	msg->setId(id);
#ifdef TRACE_TXRX
	msg->setIdx(txIdx);
#endif
	scheduleAt(txTime, msg);
}


//------------------------------------------------------------------------------
//	Event handling functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// SSSF::sendOnuPoll --
//
//		Generates a polling PON frame for transmission.
//
// Arguments:
//      cMessage	*msg;
//
// Results:
//		A polling PON frame is generated for a given ONU and
//		delivered to 'handleGrant()' for granting and transmission.
//------------------------------------------------------------------------------

void SSSF::sendOnuPoll(HybridPonMessage *msg)
{
#ifdef DEBUG_SCHEDULER
    ev << getFullPath() << ": pollEvent[" << msg->getOnuIdx() << "] received" << endl;
#endif

	// Get the channel.ONU index.
    int ch = msg->getOnuIdx();

	////////////////////////////////////////////////////////////////////////////
	// Note that handling 'onuPollList' is the job of calling function!!!
	////////////////////////////////////////////////////////////////////////////
	//// Remove the 1st element from the 'onuPollList'.
	//assert(!onuPollList.empty());
	//assert((onuPollList.front()).channel == ch);
	//onuPollList.pop_front();

    // Record ONU Timeout.
    monitor->recordOnuTimeout(ch);

	// Check the VOQ if there is room for a polling frame.
	if (voqBitCtr[ch+numOnus] + POLL_FRAME_SIZE <= voqSize) {	// ch+numOnus -> VOQ index!

		// Create a polling (i.e., grant with size 0) frame.
		HybridPonFrame *ponFrameToOnu = new HybridPonFrame("", HYBRID_PON_FRAME);
		ponFrameToOnu->setLambda(ch);
		ponFrameToOnu->setId(false);
		ponFrameToOnu->setGrant(0);	// Only for payload portion (for Ethernet frames),
									// not including CW for overhead & report fields
		ponFrameToOnu->setBitLength(POLL_FRAME_SIZE);

#ifdef DEBUG_SCHEDULER
		ev << getFullPath() << "::sendOnuPoll: Destination ONU = " << ch << endl;
#endif

		// Handle the generated polling frame.
		handleGrant(ch, ponFrameToOnu);
	}
	else {
		// Check any duplicated poll message.
		assert(!pollEvent[ch]->isScheduled());
		//if (pollEvent[ch]->isScheduled())
		//	cancelOnuPoll(pollEvent[ch]);

		// Reset ONU Timeout.
		scheduleOnuPoll(simTime()+onuTimeout, pollEvent[ch]);	// From 'now'
	}
}


//------------------------------------------------------------------------------
// SSSF::receiveEthernetFrame --
//
//		receives an Ethernet frame from the switch.
//
// Arguments:
// 		EtherFrame	*frame;
//
// Results:
//		If there is a room in the VOQ, the frame is appended at the end of it.
//		If this is an HOL frame (i.e., no other Ethernet frames in the VOQ),
//		it is encapsulated in a PON frame, scheduled for transmission, and
//		put into a TX queue.
//		If there is no room in the VOQ, the frame is dropped.
//------------------------------------------------------------------------------

//void SSSF::receiveIpPacket(IpPacket *pkt)
void SSSF::receiveEthernetFrame(EtherFrame *frame)
{
	// Get packet attributes.
    int srcAddress = pkt->getSrcAddress();
    int dstnAddress = pkt->getDstnAddress();
    long length = pkt->getBitLength();
    int lambda = dstnAddress / numUsersPerOnu;
		// For now, this is how we determine to which ONU it is addressed.
	    // Here we assume equal number of users per onu, as we have been doing.

#ifdef DEBUG_SCHEDULER
	ev << getFullPath() << ": IP packet received" << endl;
	ev << getFullPath() << ": dstnAddress = " << dstnAddress
		<< ", destination ONU = " << lambda << endl;
#endif

    // Check if there is a room in the VOQ for an Ethernet frame encapsulating
	// the IP packet.
    if ( voqBitCtr[lambda] + length + ETH_OVERHEAD_SIZE <= voqSize ) {

		// Encapsulate the IP packet in an Ethernet frame.
	    EthFrame *ethFrame = new EthFrame();
		ethFrame->setBitLength(ETH_OVERHEAD_SIZE);	// including preamble, DA, SA, FT, & CRC fields
		ethFrame->encapsulate(pkt);
		int frameLength = ethFrame->getBitLength();

		// Update the arrival counter for OLT incoming rate estimation.
		dsArrvCtr[lambda] += frameLength;	// Bit

		// Schedule a frame transmission if both the VOQ and the TX queue are empty.
		// *** Note that we allow the schedule of frame transmission when VOQ (& TXQ)
		// *** is empty irrespective of the size of the remaining grant.
		if ( voqBitCtr[lambda] == 0 ) {

			// DEBUG: Check VOQ and TX queue.
			assert( (voq[lambda].empty() == true) && (txQueue[lambda].empty() == true) );

			// Encapsulate the Ethernet frame in a PON frame.
			HybridPonFrame *ponFrameToOnu = new HybridPonFrame("", HYBRID_PON_FRAME);
			ponFrameToOnu->setLambda(lambda);
			ponFrameToOnu->setId(true);
			ponFrameToOnu->setBitLength(frameLength + DS_DATA_OVERHEAD_SIZE);
			(ponFrameToOnu->getEncapsulatedEthFrames()).insert(ethFrame);

			// Schedule transmission.
            int txIdx, rxIdx;
            simtime_t txTime = getTxTime(lambda, false, txIdx, rxIdx);	// 'false' -> DS data frame.
            scheduleFrame(txTime, lambda, txIdx, rxIdx, ponFrameToOnu);

			// Store the PON frame in a TX queue.
			txQueue[lambda].insert(ponFrameToOnu);

			// Update the DS TX counter.
			dsTxCtr[lambda] -= frameLength;
        }
		else {
			voq[lambda].insert(ethFrame);	// Append the frame at the end of the VOQ.
		}

		////////////////////////////////////////////////////////////////////////
		// Here, we treat the VOQ and the TX queue as one logical queue
		// in handling VOQ bit and frame length counters.
		////////////////////////////////////////////////////////////////////////

		// Update the VOQ bit counter.
		voqBitCtr[lambda] += frameLength;

		// Record updated VOQ statistics.
		vQueueOctet[lambda].record(voqBitCtr[lambda]/8);		// Octet
		vQueueLength[lambda].record(voq[lambda].length()+txQueue[lambda].length());	// # of frames
	}
	else {
#ifdef DEBUG_SCHEDULER
		ev << getFullPath() << ": IP packet dropped due to VOQ buffer overflow!" << endl;
#endif

		// Record packet loss statistics.
		monitor->recordLossStats(srcAddress, dstnAddress, length);

		delete (IpPacket *) pkt;
	}
}


//------------------------------------------------------------------------------
// SSSF::receiveHybridPonFrame --
//
//		Receives a PON frame from an ONU.
//
// Arguments:
// 		HybridPonFrame	*frame;
//
// Results:
//      Upstream IP packets, if any in the received frame, are sent to the upper
//      layer (i.e., IP packet sink) and if VOQ is empty, a grant based on the
//		request from the ONU is generated and scheduled for transmission.
//		Otherwise, no grant is generated.
//------------------------------------------------------------------------------

void SSSF::receiveHybridPonFrame(HybridPonFrame *ponFrameFromOnu)
{
#ifdef DEBUG_SCHEDULER
    ev << getFullPath() << ": PON frame received from ONU [" << ponFrameFromOnu->getLambda() << "]" << endl;
#endif

    // Extract Ethernet frames from the PON frame, then IP packets from
	// the Ethernet frames.
    cQueue &ethFrameQueue = ponFrameFromOnu->getEncapsulatedEthFrames();
    while( ethFrameQueue.empty() == false ) {
        EthFrame *ethFrame = (EthFrame *)ethFrameQueue.pop();

#ifdef DEBUG_SCHEDULER
		ev << getFullPath() << ": Ethernet frame removed from PON frame" << endl;
#endif

        IpPacket *pkt = (IpPacket *)ethFrame->decapsulate();
        send(pkt, "toPacketSink");
        delete (EthFrame *) ethFrame;

#ifdef DEBUG_SCHEDULER
        ev << getFullPath() << ": Packet removed from Ethernet Frame" << endl;
#endif
    }

	int lambda = ponFrameFromOnu->getLambda();

	// Assign grants for both up- and downstream traffic based on the request.
	int usRequest = ponFrameFromOnu->getRequest();
	int grant = assignGrants(lambda, usRequest);

	// Update status variables and reset counters.
	vRequest[lambda] = usRequest;
	vRxTime[lambda] = simTime();
	dsArrvCtr[lambda] = 0;	// Reset DS arrival counter.

	// Generate and schedule a new grant frame if the assigned grant is nonzero
	// and the VOQ is empty (i.e., there has been no rescheduling of ONU timeout
	// event since last grant TX).
	int voqIdx = lambda + numOnus;
	if ( (grant > 0) && (voq[voqIdx].empty() == true) ) {

		// Check if the VOQ has a room enough for a grant frame.
		long length = POLL_FRAME_SIZE + grant;	// PON frame length for the given grant
		if ( voqBitCtr[voqIdx] + length <= voqSize ) {
			HybridPonFrame *ponFrameToOnu = new HybridPonFrame("", HYBRID_PON_FRAME);
			ponFrameToOnu->setLambda(lambda);
			ponFrameToOnu->setId(false);
			ponFrameToOnu->setGrant(grant);
				// Note that the grant here is only for payload, i.e., Ethernet frames,
				// not including overhead & report field.
			ponFrameToOnu->setBitLength(length);

#ifdef DEBUG_SCHEDULER
			ev << getFullPath() << ": Scheduling a grant for " << grant
				<< " bits for ONU [" << lambda << "]" << endl;
#endif

			// Handle the generated grant.
			handleGrant(lambda, ponFrameToOnu);
		}
		else {	// No grant is generated.
			// Reset ONU timeout.
			if (pollEvent[lambda]->isScheduled()) {
				cancelOnuPoll(pollEvent[lambda]);
			}
			scheduleOnuPoll(simTime()+onuTimeout, pollEvent[lambda]);	// From 'now'
		}

    }		// end of if () for request and VOQ emptiness check

    delete (HybridPonFrame *) ponFrameFromOnu;
}


//------------------------------------------------------------------------------
// SSSF::handleGrant --
//
//		Handles a grant PON frame to an ONU.
//
// Arguments:
//      int             lambda;
// 		HybridPonFrame    *grant;
//
// Results:
//      The grant is regranted, if it is a poll, and scheduled for transmission
//		through a sequnetial scheduler. Also, the ONU Timeout is reset.
//------------------------------------------------------------------------------

void SSSF::handleGrant(int lambda, HybridPonFrame *grant)
{
	int length = grant->getBitLength();
	int voqIdx = lambda + numOnus;

	////////////////////////////////////////////////////////////////////////////
	// Note that checking the VOQ for room for a given grant is the job
	// of the function calling 'handleGrant()'.
	// This way, we can avoid unnecessary creation and deletion of messages
	// that are so wasteful and greatly slow down the whole simulation.
	////////////////////////////////////////////////////////////////////////////

	// First, get a TX time based on sequential scheduling algorithm.
	int rxIdx, txIdx;
	simtime_t txTime = getTxTime(lambda, true, txIdx, rxIdx);	// 'true' means this frame is a grant.

	////////////////////////////////////////////////////////////////////////
	// Adjust the grant size based on the scheduled TX time, cwMax, and VOQ status.
	// Now only for polls (i.e., those resulting from ONU timeout).
	////////////////////////////////////////////////////////////////////////
	if (grant->getGrant() == 0) {
		// Estimated (predicted) grant has the following three components:
		// Previous request;
		// + Traffic newly arrived at ONU since last request;
		// - Any pending grants (in the VOQ)
		int estGrant = int(
			vRequest[lambda]
			+ vRate[lambda]*(txTime.dbl() - vRxTime[lambda].dbl() + RTT[lambda].dbl())
			- grantCtr[lambda] + 0.5
			);			// 0.5 added to round it off.

		// Limit the grant based on 'cwMax' and available room in the VOQ.
		// Note that the current frame length hasn't been added to voqBitCtr!
		estGrant = min(min(cwMax, voqSize-voqBitCtr[voqIdx]-length), estGrant);

		if (estGrant >= MIN_ETH_FRAME_SIZE) {
			// Update the grant only when the new one is equal to or greater than
			// the min. Eth. frame size.

			grant->setGrant(estGrant);
			length += estGrant;
			grant->setBitLength(length);	// Adjust the frame length accordingly.
		}
	}

	// Update the queued grants counter.
	grantCtr[lambda] += grant->getGrant();

	// Finally, schedule the transmission of the grant.
	scheduleFrame(txTime, lambda, txIdx, rxIdx, grant);

	// Append the grant at the end of the VOQ and update the VOQ bit counter.
	voqBitCtr[voqIdx] += length;
	voq[voqIdx].insert(grant);

	// Check VOQ bit counter.
	assert(voqBitCtr[voqIdx] <= voqSize);
	assert(voqBitCtr[voqIdx] >= 0);

	// Record updated VOQ statistics.
	vQueueOctet[voqIdx].record(voqBitCtr[voqIdx]/8);
	vQueueLength[voqIdx].record(voq[voqIdx].length());  // cQueue::length() returns the # of objects

	// Reset ONU pollEvent.
	////////////////////////////////////////////////////////////////////////////
	// When do we reschedule the poll event?
	// We reschedule it at 'onuTimeout' from
	//  - 'now' if grant is lost;
	//  - 'transmission end time (=scheduled TX time + TX delay)'
	//			if the grant is successfully scheduled.
	////////////////////////////////////////////////////////////////////////////
	if (pollEvent[lambda]->isScheduled()) {
		cancelOnuPoll(pollEvent[lambda]);
	}
	scheduleOnuPoll(txTime+length/BITRATE+onuTimeout, pollEvent[lambda]);	// From transmission end (including tx. delay) time

	// Reschedule any scheduled ONU polls, if needed.
	rescheduleOnuPolls();
}


//------------------------------------------------------------------------------
// SSSF::transmitDataFrame --
//
//		Transmits an HOL frame from a TX queue (for DS) or a VOQ (for US).
//
// Arguments:
//      DummyPacket	*msg;
//
// Results:
//		The PON frame is popped from a TX queue (or VOQ in case of upstream)
//		and sent to ONU.
//		VOQ bit and pending grant counters are updated accordingly.
//		The message is deleted.
//------------------------------------------------------------------------------

void SSSF::transmitDataFrame(DummyPacket *msg)
{
#ifdef DEBUG_SCHEDULER
	ev << getFullPath() << ": Actual transmission of a scheduled PON frame" << endl;
#endif

	// Get frame attributes and initialize variables.
	bool id = msg->getId();
	int ch = msg->getLambda();
	int voqIdx = (id ? ch : ch + numOnus);
	int numFrames = 0;		// Keep the # of frames encapsulated in DS PON frame.
	HybridPonFrame *frame;

	if ( id == false ) {	// Grant frame.
		assert(voq[voqIdx].empty() == false);			// DEBUG: Check VOQ.
		frame = (HybridPonFrame *) (voq[voqIdx].pop());	// Pop the HOL frame from the VOQ.

		// Update previous grant and queued grant counters.
		int grant = frame->getGrant();
		vGrant[ch] = grant;
		grantCtr[ch] -= grant;
		assert(grantCtr[ch] >= 0);	// DEBUG: Check queued grant counter.

		// Record grant PON frame statistics.
		simtime_t now = simTime();
		monitor->recordGrantStats(ch, frame->getBitLength(), frame->getGrant(), now-vTxTime[ch]);
		vTxTime[ch] = now;
	}
	else {	// Downstream data frame.
		assert(txQueue[ch].length() == 1);				// DEBUG: Check TX Queue.
		frame = (HybridPonFrame *) (txQueue[ch].pop());	// Pop the frame from the TX queue.

		// Schedule the transmission of a PON frame if the VOQ is not empty.
		// *** Note that we allow the schedule of at least one frame transmission
		// *** irrespective of the size of the remaining grant (i.e., the value of
		// *** 'dsTxCtr[ch]').
		if ( voq[voqIdx].empty() == false ) {

			// Initialize a PON frame and a pointer to encapsulated Ethernet frames.
			HybridPonFrame *ponFrameToOnu = new HybridPonFrame("", HYBRID_PON_FRAME);
			cQueue &ethFrameQueue = ponFrameToOnu->getEncapsulatedEthFrames();

			EthFrame *ethFrame;
			long numBits = 0L;
			numFrames = 0;
			do {
				ethFrame = (EthFrame *)voq[voqIdx].pop();
				long frameLength = ethFrame->getBitLength();
				dsTxCtr[ch] -= frameLength;
				numBits += frameLength;
				numFrames++;
				ethFrameQueue.insert(ethFrame);
				if (voq[voqIdx].empty() == true) {
					break;
				}
				else {
					ethFrame = (EthFrame *)voq[voqIdx].front();
				}
			} while ( dsTxCtr[ch] >= ethFrame->getBitLength() );

			// Set PON frame fields.
			ponFrameToOnu->setLambda(ch);
			ponFrameToOnu->setId(true);
			ponFrameToOnu->setBitLength(numBits + DS_DATA_OVERHEAD_SIZE);

			// Schedule the created PON frame.
			int txIdx, rxIdx;
			simtime_t txTime = getTxTime(ch, false, txIdx, rxIdx);	// 'false' -> DS data frame
			scheduleFrame(txTime, ch, txIdx, rxIdx, ponFrameToOnu);

			// Store the scheduled PON frame in the TX queue.
			txQueue[ch].insert(ponFrameToOnu);
		}	// end of 'if (voq ... )
	}	// end of 'else'

	assert(voqBitCtr[voqIdx] >= 0);	// DEBUG: Check VOQ counter before updating.
	voqBitCtr[voqIdx] -= frame->getBitLength() - (id ? DS_DATA_OVERHEAD_SIZE : 0);
		// Update VOQ counter.
		// For DS data, the PON frame OH should be excluded.
	assert(voqBitCtr[voqIdx] >= 0);	// DEBUG: Check VOQ counter after updating.

	// Record updated VOQ statistics.
	vQueueOctet[ch].record(voqBitCtr[voqIdx]/8);	// Octet
	vQueueLength[ch].record(voq[voqIdx].length() + (id ? numFrames : 0));	// # of frames
		// For DS data, count the # of frames in both VOQ and TX queue.

	// Send the frame to the ONU.
	// Here we include a transmission delay because it has not been taken
	// into account elsewhere.
	simtime_t txDelay = frame->getBitLength()/BITRATE;
	sendDelayed(frame, txDelay, "out");

#ifdef TRACE_TXRX
	// Record TX usage.
	int txIdx = msg->getIdx();
	vTxUsage[txIdx].record(0)	;	// '0' denotes 'release'.
	vTxUsage[txIdx].record(ch+1);	// A channel index starts from 1.

	// Schedule the release of TX to record TX usage.
	// *** Note that we reuse the incoming message 'msg'.
	msg->setKind(RELEASE_TX);
	scheduleAt(simTime() + txDelay, msg);
#else
    delete (DummyPacket *)msg;
#endif
}


#ifdef TRACE_TXRX
//------------------------------------------------------------------------------
// SSSF::releaseTx --
//
//		Records release of TX.
//
// Arguments:
//      DummyPacket		*msg;
//
// Results:
//		Release of TX is recorded and the 'msg' is deleted.
//------------------------------------------------------------------------------

void SSSF::releaseTx(DummyPacket *msg)
{
	int ch = msg->getLambda();
	int txIdx = msg->getIdx();

	vTxUsage[txIdx].record(ch+1);	// A channel index starts from 1.
	vTxUsage[txIdx].record(0);		// '0' denotes 'release'.

	delete (DummyPacket *)msg;
}


//------------------------------------------------------------------------------
// SSSF::receiveRx --
//
//		Records use of RX.
//
// Arguments:
//      DummyPacket		*msg;
//
// Results:
//		Use of RX is recorded and 'RELEASE_RX' event is scheduled.
//------------------------------------------------------------------------------

void SSSF::receiveRx(DummyPacket *msg)
{
	int ch = msg->getLambda();
	int rxIdx = msg->getIdx();
	long length = msg->getBitLength();

	vRxUsage[rxIdx].record(0)	;	// '0' denotes 'release'.
	vRxUsage[rxIdx].record(ch+1);	// A channel index starts from 1.

	// Schedule the release of RX to record RX usage.
	// *** Note that we reuse the incoming message 'msg'.
	msg->setKind(RELEASE_RX);
	scheduleAt(simTime() + length / BITRATE, msg);
}


//------------------------------------------------------------------------------
// SSSF::releaseRx --
//
//		Records release of RX.
//
// Arguments:
//      DummyPacket		*msg;
//
// Results:
//		Release of RX is recorded and the 'msg' is deleted.
//------------------------------------------------------------------------------

void SSSF::releaseRx(DummyPacket *msg)
{
	int ch = msg->getLambda();
	int rxIdx = msg->getIdx();

	vRxUsage[rxIdx].record(ch+1);	// A channel index starts from 1.
	vRxUsage[rxIdx].record(0);		// '0' denotes 'release'.

	delete (DummyPacket *)msg;
}
#endif


//------------------------------------------------------------------------------
// SSSF::initializeSpecific --
//
//		Does initialization specific to an improved sequential mode.
//
// Arguments:
//
// Results:
//		All member variables are allocated memories and initialized.
//------------------------------------------------------------------------------

void SSSF::initializeSpecific(void)
{
    // Initialize NED parameters.
	voqSize = par("voqSize");
//	rsDepth = par("rsDepth");

	// Initialize VOQ counters.
	// Note that the indexing is as follows:
	// - Downstream data:       [0...numOnus-1]
	// - Upstream grants/polls: [numOnus...2*numOnus-1]
	voq = new Voq[2*numOnus];
	voqBitCtr.assign(2*numOnus, 0);

	// Initialize TX queues for Seq. Scheduler Ver. 4 & 5.
	txQueue = new Voq[numOnus];

    // Initialize cOutVector objects for VOQ trace.
    vQueueLength = new cOutVector[2*numOnus];
    vQueueOctet = new cOutVector[2*numOnus];
    ostringstream vectorName;
    for (int voqIdx = 0; voqIdx < 2*numOnus; voqIdx++) {
        bool isDownstream = (voqIdx < numOnus ? true : false);
        int chIdx = (isDownstream ? voqIdx : voqIdx - numOnus);
        vectorName.str("");
        vectorName << (isDownstream ? "downstream" : "upstream") << " VOQ length [frame] for channel " << chIdx;
        vQueueLength[voqIdx].setName((vectorName.str()).c_str());
        vectorName.str("");
        vectorName << (isDownstream ? "downstream" : "upstream") << " VOQ size [octet] for channel " << chIdx;
        vQueueOctet[voqIdx].setName((vectorName.str()).c_str());
    }

#ifdef TRACE_TXRX
    // Initialize cOutVector objects for TX & RX usage trace.
    vTxUsage = new cOutVector[numTransmitters];
    for (int txIdx = 0; txIdx < numTransmitters; txIdx++) {
        vectorName.str("");
        vectorName << "TX[" << txIdx << "] usage";
        vTxUsage[txIdx].setName((vectorName.str()).c_str());
    }
	vRxUsage = new cOutVector[numReceivers];
    for (int rxIdx = 0; rxIdx < numReceivers; rxIdx++) {
        vectorName.str("");
        vectorName << "RX[" << rxIdx << "] usage";
        vRxUsage[rxIdx].setName((vectorName.str()).c_str());
    }
#endif

	// Initialize vectors for ONU incoming rate estimation.
	grantCtr.assign(numOnus, 0);
	vRate.assign(numOnus, 0.0);
	vGrant.assign(numOnus, 0);
	vRequest.assign(numOnus, 0);
	vRxTime.assign(numOnus, simtime_t(0.0));

	// Initialize vectors for OLT incoming rate estimation.
	dsArrvCtr.assign(numOnus, 0);
	dsTxCtr.assign(numOnus, 0);

	// Initialize vectors for grant PON frame Trace.
	vTxTime.assign(numOnus, simtime_t(0.0));
}


//------------------------------------------------------------------------------
// SSSF::handleMessage --
//
//		Handles a message by calling a function for its processing.
//
// Arguments:
// 		cMessage	*msg;
//
// Results:
//		A function is called based on the message type.
//------------------------------------------------------------------------------

void SSSF::handleMessage(cMessage *msg)
{
	// Check the # of elements in 'onuPollList'.
	assert(onuPollList.size() <= unsigned(numOnus));

	switch (msg->getKind()) {

	case ONU_POLL:
#ifdef TRACE_ONU_POLL_LIST
		debugOnuPollListStatus();
#endif

		// Check 'onuPollList' for possible errors.
		assert(!onuPollList.empty());
		assert((onuPollList.front()).channel == ((HybridPonMessage *) msg)->getOnuIdx());

		// Remove the 1st element from 'onuPollList'.
		onuPollList.pop_front();

		sendOnuPoll((HybridPonMessage *)msg);
        break;

	case IP_PACKET:
        // Check if the IP packet received is from IP Packet generator.
		assert( msg->getArrivalGateId() == gate("fromPacketGenerator")->getId() );

		receiveIpPacket((IpPacket *)msg);
        break;

	case HYBRID_PON_FRAME:
        // Check if the PON frame received is from ONU.
        assert( msg->getArrivalGateId() == gate("in")->getId() );

		receiveHybridPonFrame((HybridPonFrame *)msg);
        break;

	case ACTUAL_TX:
        transmitDataFrame((DummyPacket *)msg);
        break;

#ifdef TRACE_TXRX
	case RELEASE_TX:
		releaseTx((DummyPacket *)msg);
		break;

	case RECEIVE_RX:
		receiveRx((DummyPacket *)msg);
		break;

	case RELEASE_RX:
		releaseRx((DummyPacket *)msg);
		break;
#endif

	default:
		error("%s: Unexpected message type: %d", getFullPath().c_str(), msg->getKind());
	}	// end of switch()
}


//------------------------------------------------------------------------------
// SSSF::finishSpecific
//
//		Does finalization specific to improved sequential mode.
//
// Arguments:
//
// Results:
//		Manually allocated memories for member variables are deallocated.
//------------------------------------------------------------------------------

void SSSF::finishSpecific(void)
{
	delete [] voq;
	delete [] txQueue;
    delete [] vQueueLength;
    delete [] vQueueOctet;
#ifdef TRACE_TXRX
	delete [] vTxUsage;
	delete [] vRxUsage;
#endif
}

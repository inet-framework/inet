///
/// @file   OltSchedulerSSSF.cc
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   2010-02-22
///
/// @brief  Implements OltSchedulerSSSF class for a hybrid TDM/WDM-PON OLT.
///
/// @note
/// This file implements an OltSchedulerSSSF class for the &quot;Sequential
/// Scheduling with	Schedule-time Framing (S<sup>3</sup>F)&quot; algorithm reported in
/// [1] for SUCCESS-HPON.
///
/// @par
///	This scheduler has the following features:
/// <ul>
///	<li> Based on VOQing (of Ethernet frames for downstream),
///		one FIFO queue per stream (either up- or downstream).
///		<ul>
///		<li> For the sake of implementation simplicity, now we have two
///			separate physical queues, VOQ and Tx Queue, but they are
///			handled as one logical queue, esp. in processing bit and
///			frame length counters;
///		</ul>
/// <li> Instant framing of downstream data as in ONU;
///	<li> Granting based on queue statuses for both up- and downstream
///		traffic with independent max. size limitation.
/// </ul>
///
/// @par References:
/// <ol>
///	<li>
/// Kyeong Soo Kim, David Gutierrez, Fu-Tai An, and Leonid G. Kazovsky,
///	&quot;Design and performance analysis of scheduling algorithms for WDM-PON
///	under SUCCESS-HPON architecture,&quot; IEEE/OSA Journal of Lightwave Technology,
///	vol. 23, no. 11, pp. 3716-3731, Nov. 2005.
/// </li>
/// </ol>
///
/// @remarks Copyright (C) 2010 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


// For debugging
// #define DEBUG_OLT_SCHEDULER_SSF


#include "OltScheduler.h"

// Register module.
Define_Module(OltSchedulerSSSF)

//------------------------------------------------------------------------------
//	Misc. functions
//------------------------------------------------------------------------------

///
/// Assigns grants for both up- and downstream traffic.
/// Grants are assigned to both up- and downstream traffic proportional
/// to their reports (ONU upstream queue status for upstream and OLT VOQ
/// status for downstream, respectively).
/// Note that we limit each size of grant to 'cwMax' independently.
///
/// @param[in] ch		channel index
/// @param[in] usReport	report from ONU for upstream traffic
/// @return				the amount of grants assigned for upstream traffic
///
int OltSchedulerSSSF::assignGrants(int ch, int usReport)
{
	////////////////////////////////////////////////////////////////////////
	// Assign grants to upstream (via 'return') and downstream traffic (via
	// 'dsTxCtr[ch]') based on the upstream queue status at ONU and the
	// downstream VOQ status at OLT.
	// *** Note that the 'voqBitCtr[ch]' counts the # of bits in txQueue[ch]
	// *** as well, which are for Ethernet frames already scheduled for
	// *** transmission and therefore should be excluded in granting.
	////////////////////////////////////////////////////////////////////////
	int dsReport = voqBitCtr[ch];
	if (txQueue[ch].empty() == false)
	{
		HybridPonDsDataFrame *frame = (HybridPonDsDataFrame *) txQueue[ch].front();
		dsReport -= frame->getBitLength() - DS_DATA_OVERHEAD_SIZE;
	}

	dsTxCtr[ch] = min(dsReport, cwMax);
	return min(usReport, cwMax);
}

///
/// Generates a snapshot of 'onuPollList' status.
/// Prints out all the elements of 'onuPollList' to the event window.
///
void OltSchedulerSSSF::debugOnuPollListStatus(void)
{
	EV << "onuPollList status at " << simTime() << " sec.:" << endl;
	EV << "Number of elements = " << onuPollList.size() << endl;
	OnuPollList::const_iterator iter;
	int i;
	for (iter = onuPollList.begin(), i = 0; iter != onuPollList.end(); ++iter, ++i)
	{
		EV << "onuPollList[" << i << "]:" << endl;
		EV << "\t- channel =\t" << (*iter).channel << endl;
		EV << "\t- time =\t" << (*iter).time << endl;
	}
}

///
/// Handles a grant PON frame to the PON interface (i.e., ONU).
/// Regrants the grant, if it is for a poll, and schedules it for transmission
/// through a sequential scheduler.
/// Resets the ONU Timeout.
///
/// @param[in] ch		a WDM channel index
/// @param[in] grant	a HybridPonDsGrantFrame pointer
///
void OltSchedulerSSSF::handleGrant(int ch, HybridPonDsGrantFrame *grant)
{
	int length = grant->getBitLength();
	int voqIdx = ch + numOnus;	///< refer to the voq definition for its indexing

	////////////////////////////////////////////////////////////////////////////
	// Note that checking the VOQ for room for a given grant is the job
	// of the function calling 'handleGrant()'.
	// This way, we can avoid unnecessary creation and deletion of messages
	// that are so wasteful and greatly slow down the whole simulation.
	////////////////////////////////////////////////////////////////////////////

	// First, get a TX time based on sequential scheduling algorithm.
	int rxIdx, txIdx;
	simtime_t txTime = getTxTime(ch, true, txIdx, rxIdx); // 'true' means this frame is a grant.


	if (grant->getGrant() == 0)
	{
		// do special processing for a polling frame

		if (onuRegistered[ch] == false)
		{
			// start the ranging process if the ONU hasn't been registered.
			// TODO: Implement full discovery procedure later.

			rangingTimer[ch] = txTime;
		}
		else
		{
			////////////////////////////////////////////////////////////////////////
			// Adjust the grant size based on the scheduled TX time, cwMax, and VOQ status.
			// Now only for polls (i.e., those resulting from ONU timeout).
			////////////////////////////////////////////////////////////////////////

			// Estimated (predicted) grant has the following three components:
			// Previous report;
			// + Traffic newly arrived at ONU since last report;
			// - Any pending grants (in the VOQ)
			int estGrant = int(vReport[ch] + vRate[ch] * (txTime.dbl()
					- vRxTime[ch].dbl() + RTT[ch].dbl()) - grantCtr[ch] + 0.5); // 0.5 added to round it off.

			// Limit the grant based on 'cwMax' and available room in the VOQ.
			// Note that the current frame length hasn't been added to voqBitCtr!
			estGrant = min(min(cwMax, voqSize - voqBitCtr[voqIdx] - length),
					estGrant);

			if (estGrant >= MIN_ETH_FRAME_SIZE)
			{
				// Update the grant only when the new one is equal to or greater than
				// the min. Eth. frame size.

				grant->setGrant(estGrant);
				length += estGrant;
				grant->setBitLength(length); // Adjust the frame length accordingly.
			}
		}
	}	// end of polling frame processing

	// Update the queued grants counter.
	grantCtr[ch] += grant->getGrant();

	// Finally, schedule the transmission of the grant.
	scheduleFrame(txTime, ch, txIdx, rxIdx, grant);

	// Append the grant at the end of the VOQ and update the VOQ bit counter.
	voqBitCtr[voqIdx] += length;
	voq[voqIdx].insert(grant);

	// Check VOQ bit counter.
	ASSERT(voqBitCtr[voqIdx] <= voqSize);
	ASSERT(voqBitCtr[voqIdx] >= 0);

	// Record updated VOQ statistics.
	vQueueOctet[voqIdx].record(voqBitCtr[voqIdx] / 8);
	vQueueLength[voqIdx].record(voq[voqIdx].length()); // cQueue::length() returns the # of objects

	// Reset ONU pollEvent.
	////////////////////////////////////////////////////////////////////////////
	// When do we reschedule the poll event?
	// We reschedule it at 'onuTimeout' from
	//  - 'now' if grant is lost;
	//  - 'transmission end time (=scheduled TX time + TX delay)'
	//			if the grant is successfully scheduled.
	////////////////////////////////////////////////////////////////////////////
	if (pollEvent[ch]->isScheduled())
	{
		cancelOnuPoll(pollEvent[ch]);
	}
	scheduleOnuPoll(txTime + length / lineRate + onuTimeout, pollEvent[ch]); // From transmission end (including tx. delay) time

	// Reschedule any scheduled ONU polls, if needed.
	rescheduleOnuPolls();
}

//------------------------------------------------------------------------------
// OltSchedulerSSSF::cancelOnuPoll --
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

cMessage *OltSchedulerSSSF::cancelOnuPoll(HybridPonMessage *msg)
{
	if (onuPollList.empty())
	{
		// Something wrong!
		error(
				"%s::cancelOnuPoll: Tried to remove an element from empty 'onuPollList'",
				getFullPath().c_str());
	}
	else
	{
		// Find the element with the same 'channel' as the given message.
		int ch = msg->getOnuIdx(); // get ONU/channel index.
		OnuPollList::iterator iter;
		for (iter = onuPollList.begin(); iter != onuPollList.end(); ++iter)
		{
			if ((*iter).channel == ch)
			{
				onuPollList.erase(iter); // Remove it from the list.
				return cancelEvent(msg); // Do normal event cancellation.
			}
		}
	}
	return NULL;
}

//------------------------------------------------------------------------------
// OltSchedulerSSSF::scheduleOnuPoll --
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

int OltSchedulerSSSF::scheduleOnuPoll(simtime_t t, HybridPonMessage *msg)
{
	// Maintain a sorted list of the scheduled ONU poll events.
	OnuPoll poll;
	poll.channel = msg->getOnuIdx();
	poll.time = t;
	OnuPollList::iterator iter;
	bool inserted = false;
	for (iter = onuPollList.begin(); iter != onuPollList.end(); ++iter)
	{
		if (t < (*iter).time)
		{
			onuPollList.insert(iter, poll);
			inserted = true;
			break;
		}
	}
	if (!inserted)
	{
		// The current message has the latest (largest) scheduling time.
		// So, append it at the end of the list.
		onuPollList.push_back(poll);
	}

	return scheduleAt(t, msg); // Do normal message schedule.
}

////------------------------------------------------------------------------------
//// OltSchedulerSSSF::rescheduleOnuPolls --
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
// OltSchedulerSSSF::getTxTime --
//
//      Calculates the earliest possible transmission time of a PON
//      frame for a given channel.
//
// Arguments:
//      const int       ch;         // CH index
//      const bool		isGrant;	// flag for grant
//		int				&txIdx;		// TX index
//		int				&rxIdx;		// RX index
//
// Results:
//      The earliest possible transmission time is returned and
//		'txIdx' and 'rxIdx' (only for a grant) are set to indices of
//		the earliest available TX and RX, respectively.
//------------------------------------------------------------------------------

simtime_t OltSchedulerSSSF::getTxTime(const int ch, const bool isGrant,
		int &txIdx, int &rxIdx)
{
	// Initialize variables.
	//	int d = ch;
	simtime_t t;
	simtime_t now = simTime();

	// Update CH[], TX[], RX[] that are less than the current simulation
	// time to the current simulation time.
    if (CH[ch] < now)
        CH[ch] = now;
	// for (int i = 0; i < numOnus; i++)
	// {
	// 	if (CH[i] < now)
	// 		CH[i] = now;
	// }
	for (int i = 0; i < numTransmitters; i++)
	{
		if (TX[i] < now)
			TX[i] = now;
	}
	for (int i = 0; i < numReceivers; i++)
	{
		if (RX[i] < now)
			RX[i] = now;
	}

#ifdef DEBUG_OLT_SCHEDULER_SSF
	ev << getFullPath() << "::getTxTime() is called for a "
	<< (isGrant ? "grant" : "data")
	<< " PON frame for ONU[" << ch << "]" << endl;
#endif

	// Pick the earliest available transmitter.
	txIdx = 0;
	for (int i = 1; i < numTransmitters; i++)
	{
		if (TX[i] < TX[txIdx])
			txIdx = i;
	}

	if (isGrant == true)
	{
		// pick the earliest available receiver
		rxIdx = 0;
		for (int i = 1; i < numReceivers; i++)
		{
			if (RX[i] < RX[rxIdx])
				rxIdx = i;
		}

		// Schedule transmission time 't' as follows:
		// t = max(receiver is free, transmitter is free, channel is free)
		// t = max(max(RX[rxIdx] + GUARD_TIME - RTT[ch] - DS_GRANT_OVERHEAD_SIZE
		// 		/ lineRate, TX[txIdx] + GUARD_TIME), CH[ch] + GUARD_TIME);
		t = max(
            RX[rxIdx]-RTT[ch]-DS_GRANT_OVERHEAD_SIZE/lineRate,
            max(TX[txIdx], CH[ch])
            ) + GUARD_TIME;
	}
	else
	{
		// Schedule transmission time 't' as follows:
		// t = max(transmitter is free, channel is free)
		t = max(TX[txIdx], CH[ch]) + GUARD_TIME;
	}

	return t; // Return the earliest transmission time.
}

//------------------------------------------------------------------------------
// OltSchedulerSSSF::scheduleFrame --
//
//      Schedules the transmission (and the upstream reception as well for a grant)
//		of a PON frame based on the sequential scheduling algorithm.
//
// Arguments:
//		const simtime_t		txTime;
//      const int			ch;
//		const int			txIdx;
//		const int			rxIdx;
//      HybridPonDsFrame	*ponFrameToOnu;
//
// Results:
//		Global status variables are updated and the frame transmission is scheduled.
//------------------------------------------------------------------------------

void OltSchedulerSSSF::scheduleFrame(const simtime_t txTime, const int ch,
		const int txIdx, const int rxIdx, HybridPonDsFrame *ponFrameToOnu)
{
	// DEBUG: Check if the scheduled TX time is already passed.
	ASSERT( txTime >= simTime() );

	// Get frame attributes and initialize variables used frequently.
	short frameType = ponFrameToOnu->getFrameType();
	simtime_t txDelay = ponFrameToOnu->getBitLength() / lineRate;

#ifdef DEBUG_OLT_SCHEDULER_SSF
	ev << getFullPath() << "::scheduleFrame() called for a " << (id ? "data" : "grant")
	<< "PON frame for ONU[" << ch << "]" << endl;
#endif

	// Update transmitter and channel available times.
	CH[ch] = txTime + txDelay;
	TX[txIdx] = txTime + txDelay;

	// For a grant, update the receiver available time as well.
	if (frameType == 1)
	{
		RX[rxIdx] = txTime + txDelay + RTT[ch];
		// Note that the RX[rxIdx] is scheduled for reception via CH[ch] at 'txTime+RTT[ch]'
		// and will be available after the frame reception ('txDelay').

#ifdef TRACE_TXRX
		// Schedule a frame reception to record RX usage.
		DummyPacket *msg = new DummyPacket("Frame RX", RECEIVE_RX);
		msg->setChannel(ch);
		msg->setIdx(rxIdx);
		msg->setBitLength(ponFrameToOnu->getBitLength());
		scheduleAt(txTime + RTT[ch], msg);
#endif
	}

#ifdef DEBUG_OLT_SCHEDULER_SSF
	if (id)
	{
		ev << getFullPath() << ": A data frame scheduled at " << txTime << " sec." << endl;
		ev << getFullPath() << ": using TX[" << txIdx << "] and CH[" << ch << "]" << endl;
	}
	else
	{
		ev << getFullPath() << ": A grant frame scheduled at " << txTime << " sec." << endl;
		ev << getFullPath() << ": using TX[" << txIdx << "], CH[" << ch << "] and reception at time = "
		<< txTime + RTT[ch] << " using RX [" << rxIdx << "]" << endl;
	}
#endif

	// Finally, schedule a frame transmission at 'txTime'.
	DummyPacket *msg = new DummyPacket("Actual Frame TX", ACTUAL_TX);
	msg->setChannel(ch);
	msg->setFrameType(frameType);
#ifdef TRACE_TXRX
	msg->setIdx(txIdx);
#endif
    scheduleAt(txTime, msg);
//	scheduleAt(simtime_t(0.0), msg);
}

//------------------------------------------------------------------------------
//	Event handling functions
//------------------------------------------------------------------------------

///
/// Handles an Ethernet frame from SNI (i.e., Ethernet switch).
/// Puts the Ethernet frame into a VOQ, if there is enough space; in case
/// there is no other frame in the VOQ, encapsulates the Ethernet frame
/// in a data PON frame, and schedules the data PON frame for transmission
/// and puts it into a TX queue.
/// Otherwise, drops it.
///
/// @param[in] frame	an EtherFrame pointer
///
void OltSchedulerSSSF::handleEthernetFrameFromSni(EtherFrame *frame)
{
	//	// Get packet attributes.
	//    int srcAddress = pkt->getSrcAddress();
	//    int dstnAddress = pkt->getDstnAddress();
	//    long length = pkt->getBitLength();
	//    int lambda = dstnAddress / numUsersPerOnu;
	//		// For now, this is how we determine to which ONU it is addressed.
	//	    // Here we assume equal number of users per onu, as we have been doing.

	int ch = frame->getArrivalGate()->getIndex();
	int frameLength = frame->getBitLength();

#ifdef DEBUG_OLT_SCHEDULER_SSF
	ev << getFullPath() << ": Ethernet frame from the SNI with a channel = " << ch << endl;
	//	ev << getFullPath() << ": dstnAddress = " << dstnAddress
	//	<< ", destination ONU = " << ch << endl;
#endif

	// check if there is a room in the VOQ.
	if (voqBitCtr[ch] + frameLength <= voqSize)
	{
		//		// Encapsulate the IP packet in an Ethernet frame.
		//		EthFrame *ethFrame = new EthFrame();
		//		ethFrame->setBitLength(ETH_OVERHEAD_SIZE); // including preamble, DA, SA, FT, & CRC fields
		//		ethFrame->encapsulate(pkt);
		//		int frameLength = ethFrame->getBitLength();

		// update the arrival counter for OLT incoming rate estimation.
		dsArrvCtr[ch] += frameLength; // in bits

		// schedule a frame transmission if both the VOQ and the TX queue are empty.
		// *** Note that we allow the schedule of frame transmission when VOQ (& TXQ)
		// *** is empty irrespective of the size of the remaining grant.
		if (voqBitCtr[ch] == 0)
		{
			// DEBUG: Check VOQ and TX queue.
			ASSERT( (voq[ch].empty() == true) && (txQueue[ch].empty() == true) );

			// encapsulate the Ethernet frame in a downstream data PON frame.
			HybridPonDsDataFrame *ponFrameToOnu = new HybridPonDsDataFrame("",
					HYBRID_PON_FRAME);
			ponFrameToOnu->setChannel(ch);
			ponFrameToOnu->setFrameType(0);
			ponFrameToOnu->setBitLength(frameLength + DS_DATA_OVERHEAD_SIZE);
			(ponFrameToOnu->getEncapsulatedFrames()).insert(frame);

			// schedule transmission
			int txIdx, rxIdx;
			simtime_t txTime = getTxTime(ch, false, txIdx, rxIdx); ///< 'false' -> DS data frame.
			scheduleFrame(txTime, ch, txIdx, rxIdx, ponFrameToOnu);

			// put the PON frame into a TX queue
			txQueue[ch].insert(ponFrameToOnu);

			// update the DS TX counter
			dsTxCtr[ch] -= frameLength;
		}
		else
		{
			voq[ch].insert(frame); ///< put the Ethernet frame into the VOQ
		}

		////////////////////////////////////////////////////////////////////////
		// Here, we treat the VOQ and the TX queue as one logical queue
		// in handling VOQ bit and frame length counters.
		////////////////////////////////////////////////////////////////////////

		// update the VOQ bit counter.
		voqBitCtr[ch] += frameLength;

		// record updated VOQ statistics.
		vQueueOctet[ch].record(voqBitCtr[ch] / 8); ///< in octet
		vQueueLength[ch].record(voq[ch].length() + txQueue[ch].length()); ///< the number of frames
	}
	else
	{
#ifdef DEBUG_OLT_SCHEDULER_SSF
		ev << getFullPath() << ": Ethernet frame dropped due to VOQ buffer overflow!" << endl;
#endif

        // TODO: Implement statistics recording.
		//		// Record packet loss statistics.
		//		monitor->recordLossStats(srcAddress, dstnAddress, length);
		delete frame;
	}
}

///
/// Handles a data PON frame from the PON interface (i.e., ONU).
/// Extracts Ethernet frames from it, if any, and sends them to
/// the upper layer (i.e., Ethernet switch).
/// Generates a grant based on the report from the ONU and schedules it
/// for transmission, only when the grant VOQ is empty.
///
/// @param[in] ponFrameFromOnu	a HybridPonUsFrame pointer
///
void OltSchedulerSSSF::handleDataPonFrameFromPon(HybridPonUsFrame *ponFrameFromOnu)
{
	int ch = ponFrameFromOnu->getChannel();

#ifdef DEBUG_OLT_SCHEDULER_SSF
	ev << getFullPath() << ": data PON frame received with a WDM channel = " ch << endl;
#endif

	if (onuRegistered[ch] == false)
	{
		// finish the ranging process if the ONU hasn't been registered.
		// TODO: Implement full discovery procedure later.

		RTT[ch] = simTime() - rangingTimer[ch] - simtime_t(POLL_FRAME_SIZE / lineRate);
		onuRegistered[ch] = true;

		EV << "ONU[" << ch << "] ranged with RTT = " << RTT[ch] << endl;
	}

	// extract Ethernet frames
	cQueue &etherFrameQueue = ponFrameFromOnu->getEncapsulatedFrames();
	while (etherFrameQueue.empty() == false)
	{
		EtherFrame *etherFrame = (EtherFrame *) etherFrameQueue.pop();

#ifdef DEBUG_OLT_SCHEDULER_SSF
		EV << getFullPath() << ": Ethernet frame removed from PON frame" << endl;
#endif

		send(etherFrame, "ethg$o", ch);
	}

	// assign grants for both up- and downstream traffic based on the report
	int usReport = ponFrameFromOnu->getReport();
	int grant = assignGrants(ch, usReport);

	// Update status variables and reset counters.
	vReport[ch] = usReport;
	vRxTime[ch] = simTime();
	dsArrvCtr[ch] = 0; // Reset DS arrival counter.

	// generate and schedule a new grant frame if the assigned grant is nonzero
	// and the VOQ is empty (i.e., there has been no rescheduling of ONU timeout
	// event since last grant TX).
	int voqIdx = ch + numOnus;
	if ((grant > 0) && (voq[voqIdx].empty() == true))
	{
		// check if the VOQ has a room enough for a grant frame
		long length = POLL_FRAME_SIZE + grant; // PON frame length for the given grant
		if (voqBitCtr[voqIdx] + length <= voqSize)
		{
			HybridPonDsGrantFrame *ponFrameToOnu = new HybridPonDsGrantFrame("",
					HYBRID_PON_FRAME);
			ponFrameToOnu->setChannel(ch);
			ponFrameToOnu->setFrameType(1);
			ponFrameToOnu->setGrant(grant);
			// note that the grant here is only for payload, i.e., Ethernet frames,
			// not including overhead & report field.
			ponFrameToOnu->setBitLength(length);

#ifdef DEBUG_OLT_SCHEDULER_SSF
			ev << getFullPath() << ": Scheduling a grant for " << grant
			<< " bits with a WDM channel = " << ch << endl;
#endif

			// handle the generated grant.
			handleGrant(ch, ponFrameToOnu);
		}
		else
		{
			// no grant is generated.
			// reset ONU timeout.
			if (pollEvent[ch]->isScheduled())
			{
				cancelOnuPoll(pollEvent[ch]);
			}
			scheduleOnuPoll(simTime() + onuTimeout, pollEvent[ch]); // From 'now'
		}

	} // end of if () for report and VOQ emptiness check

	delete ponFrameFromOnu;
}

///
/// Generates a polling PON frame and delivers to 'handleGrant()'
/// for granting and transmission.
///
/// @param[in] msg	a HybridPonMessage pointer
///
void OltSchedulerSSSF::sendOnuPoll(HybridPonMessage *msg)
{
#ifdef DEBUG_OLT_SCHEDULER_SSF
	EV << getFullPath() << ": pollEvent[" << msg->getOnuIdx() << "] received" << endl;
#endif

	// get the channel from the ONU index
	int ch = msg->getOnuIdx();

	////////////////////////////////////////////////////////////////////////////
	// Note that handling 'onuPollList' is the job of calling function!!!
	////////////////////////////////////////////////////////////////////////////
	//// Remove the 1st element from the 'onuPollList'.
	//ASSERT(!onuPollList.empty());
	//ASSERT((onuPollList.front()).channel == ch);
	//onuPollList.pop_front();

    // TODO: Implement statistics recording.
	//    // Record ONU Timeout.
	//    monitor->recordOnuTimeout(ch);

	// check the VOQ if there is room for a polling frame
	if (voqBitCtr[ch + numOnus] + POLL_FRAME_SIZE <= voqSize)	///< 'ch+numOnus' is VOQ index
	{
		// create a polling (i.e., grant with size 0) frame
		HybridPonDsGrantFrame *ponFrameToOnu =
				new HybridPonDsGrantFrame("", HYBRID_PON_FRAME);
		ponFrameToOnu->setChannel(ch);
		//		ponFrameToOnu->setId(false);
		ponFrameToOnu->setFrameType(1);	///< type = 1 -> grant
		ponFrameToOnu->setGrant(0);	///< only for payload (for Ethernet frames), not including CW for overhead & report fields
		ponFrameToOnu->setBitLength(POLL_FRAME_SIZE);

#ifdef DEBUG_OLT_SCHEDULER_SSF
		EV << getFullPath() << "::sendOnuPoll: Destination ONU = " << ch << endl;
#endif

		// handle the generated polling frame
		handleGrant(ch, ponFrameToOnu);
	}
	else
	{
		// check any duplicated poll message
		ASSERT(!pollEvent[ch]->isScheduled());
		//if (pollEvent[ch]->isScheduled())
		//	cancelOnuPoll(pollEvent[ch]);

		// reset ONU Timeout
		scheduleOnuPoll(simTime() + onuTimeout, pollEvent[ch]);
	}
}

///
/// Transmits an HOL frame from a TX queue (for DS) or a VOQ (for US).
/// The PON frame is popped from a TX queue (or VOQ in case of upstream)
/// and sent to ONU.
/// VOQ bit and pending grant counters are updated accordingly.
/// The message is deleted.
///
/// @param[in] msg a DummyPacket pointer
///
void OltSchedulerSSSF::transmitPonFrame(DummyPacket *msg)
{
#ifdef DEBUG_OLT_SCHEDULER_SSF
	ev << getFullPath() << ": Actual transmission of a scheduled PON frame" << endl;
#endif

	// Get frame attributes and initialize variables.
	bool isData = (msg->getFrameType() == 0);
	int ch = msg->getChannel();
	int voqIdx = (isData ? ch : ch + numOnus);
	int numFrames = 0; // Keep the # of frames encapsulated in DS PON frame.
	long frameLength = 0;
//	HybridPonDsDataFrame *frame;

	if (isData == true)
	{
		// downstream data PON frame

		ASSERT(txQueue[ch].length() == 1); // DEBUG: Check TX Queue.
		HybridPonDsDataFrame *frame = check_and_cast<HybridPonDsDataFrame *>(txQueue[ch].pop());	///< pop the frame from the TX queue.

		// Schedule the transmission of a PON frame if the VOQ is not empty.
		// *** Note that we allow the schedule of at least one frame transmission
		// *** irrespective of the size of the remaining grant (i.e., the value of
		// *** 'dsTxCtr[ch]').
		if (voq[voqIdx].empty() == false)
		{
			// initialize a PON frame and a pointer to encapsulated Ethernet frames
			HybridPonDsDataFrame *ponFrameToOnu = new HybridPonDsDataFrame("",
					HYBRID_PON_FRAME);
			cQueue &etherFrameQueue = ponFrameToOnu->getEncapsulatedFrames();

			EtherFrame *etherFrame;
			long numBits = 0L;
			numFrames = 0;
			do
			{
				etherFrame = (EtherFrame *) voq[voqIdx].pop();
				long etherFrameLength = etherFrame->getBitLength();
				dsTxCtr[ch] -= etherFrameLength;
				numBits += etherFrameLength;
				numFrames++;
				etherFrameQueue.insert(etherFrame);
				if (voq[voqIdx].empty() == true)
				{
					break;
				}
				else
				{
					etherFrame = (EtherFrame *) voq[voqIdx].front();
				}
			} while (dsTxCtr[ch] >= etherFrame->getBitLength());

			// Set PON frame fields.
			ponFrameToOnu->setChannel(ch);
			ponFrameToOnu->setFrameType(0);
			ponFrameToOnu->setBitLength(numBits + DS_DATA_OVERHEAD_SIZE);

			// Schedule the created PON frame.
			int txIdx, rxIdx;
			simtime_t txTime = getTxTime(ch, false, txIdx, rxIdx); // 'false' -> DS data frame
			scheduleFrame(txTime, ch, txIdx, rxIdx, ponFrameToOnu);

			// store the scheduled PON frame in the TX queue
			txQueue[ch].insert(ponFrameToOnu);
		} // end of 'if (voq ... )'

		frameLength = frame->getBitLength();
	    send(frame, "wdmg$o");  ///< send the frame to the WDM layer
	} // end of 'if'
	else
	{
		// grant PON frame

		ASSERT(voq[voqIdx].empty() == false); // DEBUG: Check VOQ.
		HybridPonDsGrantFrame *frame = check_and_cast<HybridPonDsGrantFrame *>(voq[voqIdx].pop()); ///< pop the HOL frame from the VOQ.

		// Update previous grant and queued grant counters.
		int grant = frame->getGrant();
		vGrant[ch] = grant;
		grantCtr[ch] -= grant;
		ASSERT(grantCtr[ch] >= 0); // DEBUG: Check queued grant counter.

		// Record grant PON frame statistics.
		simtime_t now = simTime();
        // TODO: Implement statistics recording.
		//		monitor->recordGrantStats(ch, frame->getBitLength(), frame->getGrant(),
		//				now - vTxTime[ch]);
		vTxTime[ch] = now;

		frameLength = frame->getBitLength();
	    send(frame, "wdmg$o");  ///< send the frame to the WDM layer
	} // end of 'else'

	ASSERT(voqBitCtr[voqIdx] >= 0); // DEBUG: Check VOQ counter before updating.
	voqBitCtr[voqIdx] -= frameLength - (isData ? DS_DATA_OVERHEAD_SIZE : 0);
	// Update VOQ counter.
	// For DS data, the PON frame OH should be excluded.
	ASSERT(voqBitCtr[voqIdx] >= 0); // DEBUG: Check VOQ counter after updating.

	// record updated VOQ statistics
	vQueueOctet[ch].record(voqBitCtr[voqIdx] / 8); // Octet
	vQueueLength[ch].record(voq[voqIdx].length() + (isData ? numFrames : 0)); // # of frames
	// For DS data, count the # of frames in both VOQ and TX queue.

// 	// Here we include a transmission delay because it has not been taken
// 	// into account elsewhere.
// 	simtime_t txDelay(frame->getBitLength() / lineRate);
// 	sendDelayed(frame, txDelay, "wdmg$o");

#ifdef TRACE_TXRX
	// record TX usage
	int txIdx = msg->getIdx();
	vTxUsage[txIdx].record(0); // '0' denotes 'release'.
	vTxUsage[txIdx].record(ch+1); // A channel index starts from 1.

	// schedule the release of TX to record TX usage
	// *** Note that we reuse the incoming message 'msg'.
	msg->setKind(RELEASE_TX);
	scheduleAt(simTime() + txDelay, msg);
#else
	delete (DummyPacket *) msg;
#endif
}

#ifdef TRACE_TXRX
//------------------------------------------------------------------------------
// OltSchedulerSSSF::releaseTx --
//
//		Records release of TX.
//
// Arguments:
//      DummyPacket		*msg;
//
// Results:
//		Release of TX is recorded and the 'msg' is deleted.
//------------------------------------------------------------------------------

void OltSchedulerSSSF::releaseTx(DummyPacket *msg)
{
	int ch = msg->getLambda();
	int txIdx = msg->getIdx();

	vTxUsage[txIdx].record(ch+1); // A channel index starts from 1.
	vTxUsage[txIdx].record(0); // '0' denotes 'release'.

	delete (DummyPacket *)msg;
}

//------------------------------------------------------------------------------
// OltSchedulerSSSF::receiveRx --
//
//		Records use of RX.
//
// Arguments:
//      DummyPacket		*msg;
//
// Results:
//		Use of RX is recorded and 'RELEASE_RX' event is scheduled.
//------------------------------------------------------------------------------

void OltSchedulerSSSF::receiveRx(DummyPacket *msg)
{
	int ch = msg->getLambda();
	int rxIdx = msg->getIdx();
	long length = msg->getBitLength();

	vRxUsage[rxIdx].record(0); // '0' denotes 'release'.
	vRxUsage[rxIdx].record(ch+1); // A channel index starts from 1.

	// Schedule the release of RX to record RX usage.
	// *** Note that we reuse the incoming message 'msg'.
	msg->setKind(RELEASE_RX);
	scheduleAt(simTime() + length / lineRate, msg);
}

//------------------------------------------------------------------------------
// OltSchedulerSSSF::releaseRx --
//
//		Records release of RX.
//
// Arguments:
//      DummyPacket		*msg;
//
// Results:
//		Release of RX is recorded and the 'msg' is deleted.
//------------------------------------------------------------------------------

void OltSchedulerSSSF::releaseRx(DummyPacket *msg)
{
	int ch = msg->getLambda();
	int rxIdx = msg->getIdx();

	vRxUsage[rxIdx].record(ch+1); // A channel index starts from 1.
	vRxUsage[rxIdx].record(0); // '0' denotes 'release'.

	delete (DummyPacket *)msg;
}
#endif

///
/// Initializes member variables & activities and allocates memories
/// for them, if needed.
///
void OltSchedulerSSSF::initialize(void)
{
	OltScheduler::initialize();

	// initialize OltSchedulerSSSF NED parameters
	voqSize = par("voqSize").longValue();
	//	rsDepth = par("rsDepth");

	// initialize VOQ counters
	// note that the indexing is as follows:
	// - Downstream data:       [0...numOnus-1]
	// - Upstream grants/polls: [numOnus...2*numOnus-1]
	voq = new Voq[2 * numOnus];
	voqBitCtr.assign(2 * numOnus, 0);

	// initialize TX queues for Seq. Scheduler Ver. 4 & 5
	txQueue = new Voq[numOnus];

	// initialize cOutVector objects for VOQ trace
	vQueueLength = new cOutVector[2 * numOnus];
	vQueueOctet = new cOutVector[2 * numOnus];
	ostringstream vectorName;
	for (int voqIdx = 0; voqIdx < 2 * numOnus; voqIdx++)
	{
		bool isDownstream = (voqIdx < numOnus ? true : false);
		int chIdx = (isDownstream ? voqIdx : voqIdx - numOnus);
		vectorName.str("");
		vectorName << (isDownstream ? "downstream" : "upstream")
				<< " VOQ length [frame] for channel " << chIdx;
		vQueueLength[voqIdx].setName((vectorName.str()).c_str());
		vectorName.str("");
		vectorName << (isDownstream ? "downstream" : "upstream")
				<< " VOQ size [octet] for channel " << chIdx;
		vQueueOctet[voqIdx].setName((vectorName.str()).c_str());
	}

#ifdef TRACE_TXRX
	// initialize cOutVector objects for TX & RX usage trace
	vTxUsage = new cOutVector[numTransmitters];
	for (int txIdx = 0; txIdx < numTransmitters; txIdx++)
	{
		vectorName.str("");
		vectorName << "TX[" << txIdx << "] usage";
		vTxUsage[txIdx].setName((vectorName.str()).c_str());
	}
	vRxUsage = new cOutVector[numReceivers];
	for (int rxIdx = 0; rxIdx < numReceivers; rxIdx++)
	{
		vectorName.str("");
		vectorName << "RX[" << rxIdx << "] usage";
		vRxUsage[rxIdx].setName((vectorName.str()).c_str());
	}
#endif

	// initialize vectors for ONU incoming rate estimation
	grantCtr.assign(numOnus, 0);
	vRate.assign(numOnus, 0.0);
	vGrant.assign(numOnus, 0);
	vReport.assign(numOnus, 0);
	vRxTime.assign(numOnus, simtime_t(0.0));

	// initialize vectors for OLT incoming rate estimation
	dsArrvCtr.assign(numOnus, 0);
	dsTxCtr.assign(numOnus, 0);

	// initialize vectors for grant PON frame Trace
	vTxTime.assign(numOnus, simtime_t(0.0));
}

///
/// Handles a message by calling a function for its processing.
/// A function is called based on the message type and the port it is received from.
///
/// @param[in] msg a cMessage pointer
///
void OltSchedulerSSSF::handleMessage(cMessage *msg)
{
	// Check the # of elements in 'onuPollList'.
	ASSERT(onuPollList.size() <= unsigned(numOnus));

	if (msg->isSelfMessage() == false)
	{
		// get the full name of arrival gate.
		std::string inGate = msg->getArrivalGate()->getFullName();

#ifdef DEBUG_OLT_SCHEDULER_SSF
		ev << getFullPath() << ": A frame from " << inGate << " received" << endl;
#endif

		if (inGate.compare(0, 6, "ethg$i") == 0)
		{
			// Ethernet frame from the upper layer (i.e., Ethernet switch)
			handleEthernetFrameFromSni(check_and_cast<EtherFrame *> (msg));
		}
		else if (inGate.compare(0, 6, "wdmg$i") == 0)
		{
			// PON frame from the lower layer (i.e., WDM layer)
			handleDataPonFrameFromPon(check_and_cast<HybridPonUsFrame *> (msg));
		}
		else
		{
			// unknown message
			EV << getFullPath() << ": An unknown message received from " <<
					inGate << endl;
			error("%s: Unknown message received", getFullPath().c_str());
		}
	} // end of if()
	else
	{
		// this is a self message to indicate an event.

		switch (msg->getKind())
		{
		case ONU_POLL:
#ifdef TRACE_ONU_POLL_LIST
			debugOnuPollListStatus();
#endif

			// Check 'onuPollList' for possible errors.
			ASSERT(!onuPollList.empty());
			ASSERT((onuPollList.front()).channel == (check_and_cast<HybridPonMessage *>(msg))->getOnuIdx());

			// Remove the 1st element from 'onuPollList'.
			onuPollList.pop_front();

			sendOnuPoll((HybridPonMessage *) msg);
			break;

		case ACTUAL_TX:
			transmitPonFrame((DummyPacket *) msg);
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
			error("%s: Unexpected message type: %d", getFullPath().c_str(),
					msg->getKind());
		} // end of switch()
	} // end of else
}

///
/// Does post processing and deallocates memories manually allocated
/// for member variables.
///
void OltSchedulerSSSF::finish(void)
{
	OltScheduler::finish();

	delete[] voq;
	delete[] txQueue;
	delete[] vQueueLength;
	delete[] vQueueOctet;
#ifdef TRACE_TXRX
	delete [] vTxUsage;
	delete [] vRxUsage;
#endif
}

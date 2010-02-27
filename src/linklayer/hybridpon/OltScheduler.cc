///
/// @file   OltScheduler.cc
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Jun/30/2009
///
/// @brief  Implements 'OltScheduler' class for hybrid TDM/WDM-PON OLT.
///
/// @remarks Copyright (C) 2009-2010 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.


// For debugging
// #define DEBUG_OLT_SCHEDULER


#include "OltScheduler.h"

//// Register module.
//Define_Module(Scheduler);


//------------------------------------------------------------------------------
//	Scheduling functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Scheduler::seqSchedule --
//
//      schedules downstream TX of PON frames (either data or grants for
//      upstream traffic at ONUs) based on the sequential scheduling algorithm.
//
// Arguments:
//      int onu;
//      HybridPonDsFrame *ponFrameToOnu;
//
// Results:
//      If the frame is for downstream data, its transmission is scheduled
//      and the scheduled TX time is returned.
//      If the frame is for grant for either upstream data in ONU or polling ONU,
//      and the scheduled TX time is less than a given maximum bound, its
//      transmission is scheduled and the scheduled TX time is returned;
//      otherwise, the transmission is canceled, the frame is deleted, and
//      '-1' is returned, indicating TX failure.
//------------------------------------------------------------------------------

simtime_t OltScheduler::seqSchedule(int onu, HybridPonDsFrame *ponFrameToOnu)
{
	int d = onu;
	int l = ponFrameToOnu->getBitLength();
	int i = 0;
	int j = 0;
	simtime_t t;
	simtime_t delay;
	simtime_t now;

	// Update all CH[], TX[], RX[] that are less than the current simulation
	// time to the current simulation time.
	now = simTime();
	for (i = 0; i < numOnus; i++)
	{
		if (CH[i] < now)
		{
			CH[i] = now;
		}
	}
	for (i = 0; i < numTransmitters; i++)
	{
		if (TX[i] < now)
		{
			TX[i] = now;
		}
	}
	for (i = 0; i < numReceivers; i++)
	{
		if (RX[i] < now)
		{
			RX[i] = now;
		}
	}

#ifdef DEBUG_OLT_SCHEDULER
	if (ponFrameToOnu->getFrameType() == 0)
	{
		ev << getFullPath() << ": seqSchedule called for downstream data PON frame for ONU["
		<< onu << "]" << endl;
	}
	else
	{
		ev << getFullPath() << ": seqSchedule called for downstream grant PON frame for ONU["
		<< onu << "]" << endl;
	}
#endif

	// pick the earliest available transmitter
	i = 0;
	for (int m = 0; m < numTransmitters; m++)
	{
		if (TX[m] <= TX[i])
		{
			i = m;
		}
	}

	if (ponFrameToOnu->getFrameType() == 1)
	{
		// This is a grant frame providing optical CW burst for upstream traffic from ONU.

		// pick the earliest available receiver
		j = 0;
		for (int n = 0; n < numReceivers; n++)
		{
			if (RX[n] <= RX[j])
			{
				j = n;
			}
		}
		// schedule reception time t + RTT[d]
		// t = max (receiver is free, transmitter is free, channel is free)
		t = RX[j] + GUARD_TIME - RTT[d];
		if ((TX[i] + GUARD_TIME) > t)
		{
			t = TX[i] + GUARD_TIME;
		}
		if (CH[d] > t)
		{
			t = CH[d];
		}

		// if scheduled transmission time for grant (either CW or polling msg.) is larger
		// than a given max. limit, delete the frame and return -1.
		if (t - simTime() > maxTxDelay)
		{
#ifdef DEBUG_OLT_SCHEDULER
			ev << getFullPath() << ": TX of grant PON frame cancelled due to excessive scheduling delay!"
			<< endl;
#endif
			delete (HybridPonDsFrame *) ponFrameToOnu;
			return simtime_t(-1.0);
		}
#ifdef DEBUG_OLT_SCHEDULER
		ev << getFullPath() << ": grant PON frame scheduled at time  = " << t << endl;
		ev << getFullPath() << ": using TX[" << i << "], CH[" << d << "] and reception at time = "
		<< t + RTT[d] + GUARD_TIME << " using RX[" << j << "]" << endl;
#endif

		// update chosen receiver available time
		RX[j] = t + l / BITRATE + RTT[d];
		// schedule reception at time t+RTT[d] using RX[j] and CH[d]
	}
	else
	{
		// This is a normal data frame.

		// schedule transmission time
		// t = max (transmitter is free, channel is free)
		t = TX[i] + GUARD_TIME;
		if (CH[d] > t)
		{
			t = CH[d];
		}
	}

	// update transmitter and channel available times
	CH[d] = t + l / BITRATE;
	TX[i] = t + l / BITRATE;

	// calculate delay from now for sending time
	delay = t - simTime();
	if (delay >= 0)
	{
		// schedule transmission at time t using TX[i] and CH[d]
		// Here we included a transmission delay in sending
		// because the impact of tranmission rate in delay
		// has not been taken into account elsewhere.
		sendDelayed(ponFrameToOnu, delay + l / BITRATE, "out");
	}
	else
	{
		ev << getFullPath()
				<< ": seqSchedule error: trying to schedule a delayed send with negative delay"
				<< endl;
	}
#ifdef DEBUG_OLT_SCHEDULER
	if (ponFrameToOnu->getFrameType() == 0)
	{
		ev << getFullPath() << ": seqSchedule downstream ponFrame scheduled at time = " << t << endl;
		ev << getFullPath() << ": using TX[" << i << "] and CH[" << d << "]" << endl;
	}
	else
	{
		ev << getFullPath() << ": seqSchedule downstream-upstream ponFrame scheduled at time = " << t << endl;
		ev << getFullPath() << ": using TX[" << i << "], CH[" << d << "] and reception at time = "
		<< t + RTT[d] << " using RX [" << j << "]" << endl;
	}
#endif

	return t; // return scheduled transmission time
}

//------------------------------------------------------------------------------
//	Misc. functions
//------------------------------------------------------------------------------

// // QUICK DEBUG
// simtime_t Scheduler::debugRX(void)
// {
//     return RX[0];
// }
// // QUICK DEBUG


//------------------------------------------------------------------------------
// Scheduler::debugSchedulerStatus --
//
//		generates a snapshot of scheduler status variables (CH, RX, Tx).
//
// Arguments:
//
// Results:
//      The values of scheduler status variables have been displayed to stderr.
//------------------------------------------------------------------------------

void OltScheduler::debugSchedulerStatus(void)
{
	cerr << getFullPath() << "Snapshot of scheduler status variables at "
			<< simTime() << " sec" << endl;
	for (int i = 0; i < numOnus; i++)
		cerr << "CH[" << i << "] = " << CH[i] << endl;
	for (int i = 0; i < numReceivers; i++)
		cerr << "RX[" << i << "] = " << RX[i] << endl;
	for (int i = 0; i < numTransmitters; i++)
		cerr << "TX[" << i << "] = " << TX[i] << endl;
}

//------------------------------------------------------------------------------
// Scheduler::debugSnapshot --
//
//		generates a snapshot of all class variables.
//
// Arguments:
//
// Results:
//      The values of all class member variables have been displayed to stderr.
//------------------------------------------------------------------------------

void OltScheduler::debugSnapshot(void)
{
	cerr << "Snapshot of " << getFullPath() << " at " << simTime() << " sec:"
			<< endl;
	cerr << "* NED Parameters *" << endl;
	//    cerr << "- numChannels=" << numChannels << endl;
	cerr << "- numOnus=" << numOnus << endl;
	cerr << "- numReceivers=" << numReceivers << endl;
	cerr << "- numTransmitters=" << numTransmitters << endl;
	//    cerr << "- numUsersPerOnu=" << numUsersPerOnu << endl;
	cerr << "- queueSizePoll=" << queueSizePoll << endl;
	cerr << "- maxTxDelay=" << maxTxDelay << endl;
	cerr << "- cwMax=" << cwMax << endl;
	cerr << "- onuTimeout=" << onuTimeout << endl;
//	cerr << "- distances=" << distances << endl;

	cerr << "* Status Variables *" << endl;
	cerr << "- busyQueuePoll=" << busyQueuePoll << endl;
	for (int i = 0; i < numOnus; i++)
		cerr << "-RTT[" << i << "]=" << RTT[i] << endl;
	for (int i = 0; i < numOnus; i++)
		cerr << "-CH[" << i << "]=" << CH[i] << endl;
	for (int i = 0; i < numReceivers; i++)
		cerr << "-RX[" << i << "]=" << RX[i] << endl;
	for (int i = 0; i < numTransmitters; i++)
		cerr << "-TX[" << i << "]=" << TX[i] << endl;
	//     for (int i=0; i<2*numOnus; i++)
	//         cerr << "-voq[" << i << "].length()=" << voq[i].length() << endl;
	//     for (int i=0; i<2*numOnus; i++)
	//         cerr << "-voqBitCtr[" << i << "]=" << voqBitCtr[i] << endl;
	//     cerr << "-voqStartIdx=" << voqStartIdx << endl;
}

//------------------------------------------------------------------------------
//	Event handling functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Scheduler::sendOnuPoll --
//
//		generates a polling PON frame and schedules its transmission.
//
// Arguments:
//      HybridPonMessage *msg;
//
// Results:
//		A polling PON frame has been generated for a given ONU.
//      If there's room for the frame in the polling FIFO queue and its
//      transmission scheduling based on the sequential scheduling algorithm is
//      successful, its transmission is scheduled and the frame is put into
//      the queue.
//      Otherwise, the frame is lost (due to either buffer overflow or excessive
//      transmission scheduling delay).
//      The next polling event has been scheduled.
//------------------------------------------------------------------------------

void OltScheduler::sendOnuPoll(HybridPonMessage *msg)
{
#ifdef DEBUG_OLT_SCHEDULER
	ev << getFullPath() << ": pollEvent[" << msg->getOnuIdx() << "] received" << endl;
#endif

	int lambda = (short) msg->getOnuIdx(); // note that the message length is used as ONU number.

	//    // Record ONU Timeout.
	//    monitor->recordOnuTimeout(lambda);

	if (busyQueuePoll + POLL_FRAME_SIZE <= queueSizePoll)
	{
		// There's room for the frame in the queue.

		// Create a polling ponFrame.
		HybridPonDsGrantFrame *ponFrameToOnu = new HybridPonDsGrantFrame("",
				HYBRID_PON_FRAME);
		//		ponFrameToOnu->setLambda(lambda);
		ponFrameToOnu->setFrameType(1); // for "Grant"
		ponFrameToOnu->setGrant(0); // no CW burst for data (user frames)
		// - "grant" field doesn't count CW for overhead & report fields
		// - which are included in setting bit length by default.
		ponFrameToOnu->setBitLength(POLL_FRAME_SIZE);

#ifdef DEBUG_OLT_SCHEDULER
		ev << getFullPath() << "destination ONU = " << lambda << endl;
#endif
		// We emulate a queueing operation using a counter (busyQueuePoll) and
		// an additional event scheduled for a messsage deletion from a queue
		// when it is actually transmitted later.
		simtime_t txTime = seqSchedule(lambda, ponFrameToOnu);

		if (txTime >= 0)
		{
			// Transmission scheduling is ok.
			HybridPonMessage *msg = new HybridPonMessage("", ACTUAL_TX_POLL);
			msg->setBitLength(POLL_FRAME_SIZE);
			scheduleAt(txTime, msg);
			busyQueuePoll += POLL_FRAME_SIZE;
		}
		else
		{
			//             debugSchedulerStatus();
			//             debugSnapshot();
			//             snapshot(this);
			//             cerr << "At " << simTime() << " sec" << endl;
			//             error("Something wrong!");
			//             exit(1);
#ifdef DEBUG_OLT_SCHEDULER
			ev << getFullPath() << ": PON polling frame dropped due to excessive scheduling delay!" << endl;
#endif
		}
	}
	else
	{
#ifdef DEBUG_OLT_SCHEDULER
		ev << getFullPath() << ": PON polling frame dropped due to buffer overflow!" << endl;
#endif
	}

	// Schedule next ONU poll
	if (msg->isScheduled())
	{
		cancelEvent(msg);
	}
	if (pollEvent[lambda]->isScheduled())
	{
		cancelEvent(pollEvent[lambda]);
	}
	scheduleAt(simTime() + onuTimeout, pollEvent[lambda]);
}

//------------------------------------------------------------------------------
// Scheduler::receiveHybridPonFrame --
//
//		receives a PON frame from the WDM layer (i.e., from ONU).
//
// Arguments:
// 		HybridPonUsFrame *frame;
//
// Results:
//      Upstream Ethernet frames, if any in the received frame, is sent to a
//      switch (Ethernet bridge in the current implementation) and a downstream
//      grant PON frame based on the report received from the ONU, is made and
//      scheduled for transmission through seqSchedule().
//------------------------------------------------------------------------------

void OltScheduler::handleDataPonFrameFromPon(HybridPonUsFrame *ponFrameFromOnu)
{
	// get the channel (wavelength) information from the gate index
	int channel = ponFrameFromOnu->getArrivalGate()->getIndex();

#ifdef DEBUG_OLT_SCHEDULER
	ev << getFullPath() << ": PON frame received from ONU [" << channel << "]" << endl;
#endif

	// deliver Ethernet frames to a switch
	cQueue &etherFrameQueue = ponFrameFromOnu->getEncapsulatedFrames();
	while (!etherFrameQueue.empty())
	{
		EtherFrame *etherFrame = (EtherFrame *) etherFrameQueue.pop();

#ifdef DEBUG_OLT_SCHEDULER
		ev << getFullPath() << ": Ethernet frame removed from PON frame" << endl;
#endif

		//        IpPacket *pkt = (IpPacket *)etherFrame->decapsulate();
		//        send(pkt, "toPacketSink");
		//        delete (EtherFrame *) etherFrame;
		//#ifdef DEBUG_OLT_SCHEDULER
		//        ev << getFullPath() << ": Packet removed from Ethernet Frame" << endl;
		//#endif

		send(etherFrame, "ethg$o", channel);
	}

	// Handle report
	//	int lambda = ponFrameFromOnu->getLambda();
	int report = ponFrameFromOnu->getReport();

	// Generate and schedule a grant frame if needed
	if (report > 0)
	{

#ifdef DEBUG_OLT_SCHEDULER
		ev << getFullPath() << ": Processing report for " << report << " bits from ONU ["
		<< channel << "]" << endl;
#endif
		HybridPonDsGrantFrame *ponFrameToOnu = new HybridPonDsGrantFrame("",
				HYBRID_PON_FRAME);
		//		ponFrameToOnu->setLambda(lambda);
		ponFrameToOnu->setFrameType(1);
		if (report > cwMax)
		{
			// Limit the size of max. grant to 'cwMax'.
			report = cwMax;
		}
		ponFrameToOnu->setGrant(report);
		// Note that the grant here is only for data, i.e.,
		// Ethernet frames, not including overhead & report field.
		ponFrameToOnu->setBitLength(PREAMBLE_SIZE + DELIMITER_SIZE + FLAG_SIZE
				+ GRANT_SIZE + PREAMBLE_SIZE + DELIMITER_SIZE + REPORT_SIZE
				+ report);

#ifdef DEBUG_OLT_SCHEDULER
		ev << getFullPath() << ": scheduling downstream grant for " << report << " bits for ONU ["
		<< channel << "]" << endl;
#endif

		// Handle the generated grant PON frame specific to operation mode.
		handleGrant(channel, ponFrameToOnu);
	}
	delete (HybridPonUsFrame *) ponFrameFromOnu;
}

//------------------------------------------------------------------------------
// Scheduler::transmitPollFrame --
//
//		emulates actual TX of a polling frame by decreasing a queue counter
//      (busyQueuePoll) implementing a virtual FIFO for polling frames.
//
// Arguments:
//      HybridPonMessage *msg;
//
// Results:
//		The queue counter is decreased by the frame length transmitted.
//		The message is deleted.
//------------------------------------------------------------------------------

void OltScheduler::transmitPollFrame(HybridPonMessage *msg)
{
#ifdef DEBUG_OLT_SCHEDULER
	ev << getFullPath() << ": Actual transmission of scheduled polling frame" << endl;
#endif
	busyQueuePoll -= msg->getBitLength();
	if (busyQueuePoll < 0)
	{
		ev << getFullPath()
				<< ": ERROR: something wrong with busyQueuePoll counter!"
				<< endl;
		exit(1);
	}
	delete (cMessage *) msg;
}

///
/// Initializes member variables & activities and allocates memories
/// for them, if needed.
///
void OltScheduler::initialize(void)
{
	// initialize OLT NED parameters
	numReceivers = getParentModule()->par("numReceivers");
	numTransmitters = getParentModule()->par("numTransmitters");
	numOnus = getParentModule()->par("numOnus");

	// initialize OltScheduler NED parameters
	cwMax = par("cwMax");
	maxTxDelay = par("maxTxDelay");
	onuTimeout = par("onuTimeout");
	queueSizePoll = par("queueSizePoll");
//	distances = ((const char *) par("distances"));
	//	numChannels = par("numChannels");
	//	numUsersPerOnu = par("numUsersPerOnu");

	busyQueuePoll = 0;
	RTT.assign(numOnus, simtime_t(0.0));

//	// Parse 'distances' into RTT array.
//	if (distances.size() > 0)
//	{
//		int i = 0;
//		string::size_type idx1 = 0, idx2 = 0;
//		while ((idx1 != string::npos) && (idx2 != string::npos))
//		{
//			idx2 = distances.find(' ', idx1);
//			double dist = (atof((distances.substr(idx1, idx2)).c_str()));
//			double rtt = (dist * 2) / SPEED_OF_LIGHT;
//			RTT[i] = (simtime_t) rtt;
//			if (idx2 != string::npos)
//			{
//				idx1 = distances.find_first_not_of(' ', idx2 + 1);
//			}
//			i++;
//		}
//	}

	CH.assign(numOnus, simtime_t(0.0));
	TX.assign(numTransmitters, simtime_t(0.0));
	RX.assign(numReceivers, simtime_t(0.0));
	pollEvent.assign(numOnus, (HybridPonMessage *) NULL);
	// 	pollOnu.assign(numOnus, simtime_t(onuTimeout));

	// schedule the first poll to all ONUs
	// TODO: Implement ranging procedure to register and measure RTT of ONUs.
	//  simtime_t t;
	for (int i = 0; i < numOnus; i++)
	{
		pollEvent[i] = new HybridPonMessage("", ONU_POLL);
		pollEvent[i]->setOnuIdx(i);	///< set the ONU index
		scheduleOnuPoll(simTime(), pollEvent[i]);

#ifdef DEBUG_OLT_SCHEDULER
		ev << getFullPath() << ": pollEvent [" << i << "] = for ONU[" << pollEvent[i]->length()
		<< "] scheduled at time " << simTime() + t << endl;
#endif
	}
}

///
/// Does post processing and deallocates memories manually allocated
/// for member variables.
///
void OltScheduler::finish(void)
{
}

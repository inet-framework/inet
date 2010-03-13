///
/// @file   OltScheduler.h
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Jun/30/2009
///
/// @brief  Declares 'OltScheduler' and its derived classes for a hybrid
///			TDM/WDM-PON OLT.
///
/// @remarks Copyright (C) 2009-2010 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


// For Trace of TX & RX usage
//#define TRACE_TXRX


#ifndef __OLT_SCHEDULER_H
#define __OLT_SCHEDULER_H

#include "HybridPon.h"

///
/// @class OltScheduler
/// @brief Implements 'OltScheduler' module in a hybrid TDM/WDM-PON OLT.
/// @ingroup hybridpon
///
class OltScheduler: public cSimpleModule
{
protected:
	// OLT NED parameters
	int numReceivers;	///< number of tunable receivers
	int numTransmitters;	///< number of tunable transmitters

	// OltScheduler NED parameters
	int cwMax;	///< maximum grant to ONU [bit]
	simtime_t maxTxDelay;	///< max. limit to TX scheduling delay [sec]
	simtime_t onuTimeout;	///< polling cycle from OLT to ONU [sec]
	int queueSizePoll;	///< size of FIFO queue for polling frames [bit]

    // configuration variables
	double lineRate;	///< line rate of optical channel
	int numOnus;	///< number of ONUs (= number of WDM channels)

	// status variables
	int busyQueuePoll;	///< counter to emulate a FIFO for polling frames
	TimeVector RTT;	///< round-trip times to and from ONUs
	TimeVector CH;	///< WDM channel availability
	TimeVector TX;	///< tunable transmitter availability
	TimeVector RX;	///< tunable receiver availability

	// For trace of grant PON frames (interval, grant size and frame length)
	TimeVector vTxTime;	///< vector of previous grant PON frame TX times

	// For ONU polling and discovery
	HybridPonMsgVector pollEvent;
	/* 	TimeVector  pollOnu; */
	BoolVector onuRegistered;	///< results of ONU discovery processes
	TimeVector rangingTimer;	///< timers used for ONU ranging purpose

protected:
	// Misc.
	void debugSchedulerStatus(void);
	virtual void debugSnapshot(void);
	virtual void handleGrant(int ch, HybridPonDsGrantFrame *grant) = 0;	///< pure virtual function
	inline virtual int scheduleOnuPoll(simtime_t t, HybridPonMessage *msg) ///< wrapper function
	{
		return scheduleAt(t, msg);
	}

	//	// QUICK DEBUG */
	//	simtime_t debugRX(void);
	//	// QUICK DEBUG

	// Event handling
	virtual void handleEthernetFrameFromSni(EtherFrame *frame) = 0; // pure virtual function
	virtual void handleDataPonFrameFromPon(HybridPonUsFrame *msg);
	virtual void sendOnuPoll(HybridPonMessage *msg);
	void transmitPollFrame(HybridPonMessage *msg);

	// Scheduling
	virtual simtime_t seqSchedule(int onu, HybridPonDsFrame *ponFrameToOnu);

	// OMNeT++
	virtual void initialize(void);
	virtual void handleMessage(cMessage *msg) = 0; // pure virtual function
	virtual void finish(void);
};

///
/// @class OltSchedulerSSSF
/// @brief Implements 'OltSchedulerSSSF' (Sequential Scheduling with Schedule-time Framing)
///			module in a hybrid TDM/WDM-PON OLT.
/// @ingroup hybridpon
///
class OltSchedulerSSSF: public OltScheduler
{
protected:
	// OltSchedulerSSSF NED parameters
	int voqSize;	///< Size of VOQ [bit]
	//    int             rsDepth;        // Max. # of checking in ONU timeout rescheduling

	// For VOQs: Indexing is done as follows
	// - Downstream data:       [0...numOnus-1]
	// - Upstream grants/polls: [numOnus...2*numOnus-1]
	Voq *voq;
	IntVector voqBitCtr; // vector of VOQ bit counters [bit]

	// TX queues (for Seq. Scheduler Vers. 4 & 5) to store scheduled
	// downstream PON frames (at most one per TX queue).
	Voq *txQueue;

	// For trace of VOQs
	cOutVector *vQueueLength;	///< array of output vector for VOQ length [frame]
	cOutVector *vQueueOctet;	///< array of output vector for VOQ size [octet]
#ifdef TRACE_TXRX
	// For trace of TX & RX usages
	cOutVector *vTxUsage;	///< array of output vector for TX usage (time, chIdx)
	cOutVector *vRxUsage;	///< array of output vector for RX usage (time, chIdx)
#endif

	// For rescheduling ONU timeout (poll) events
	OnuPollList onuPollList;	///< sorted list of scheduled ONU timeout events

	// For estimating ONU incoming (upstream) rate [0...numOnus-1]
	IntVector grantCtr;	///< vector of queued grant counters
	DoubleVector vRate;	///< vector of estimated ONU incoming rate
	IntVector vGrant;	///< vector of previous grants
	IntVector vReport;	///< vector of previous reports
	TimeVector vRxTime;	///< vector of previous PON frame RX times

	// For estimating OLT incoming (downstream) rate [0...numOnus-1]
	IntVector dsArrvCtr;	///< vector of downstream arrival (bit) counters
	IntVector dsTxCtr;	///< vector of downstream TX (bit) counters

protected:
	// Misc.
	virtual int assignGrants(int ch, int usReport);
	virtual void handleGrant(int ch, HybridPonDsGrantFrame *grant);
	void debugOnuPollListStatus(void);
	cMessage *cancelOnuPoll(HybridPonMessage *msg);	///< wrapper function for cancelEvent()
	virtual int scheduleOnuPoll(simtime_t t, HybridPonMessage *msg);
	inline virtual void rescheduleOnuPolls(void)
	{
	}
	; // to make it optimized away by the compiler

	// Event handling
	virtual void handleEthernetFrameFromSni(EtherFrame *frame);
	virtual void handleDataPonFrameFromPon(HybridPonUsFrame *frame);
	virtual void sendOnuPoll(HybridPonMessage *msg);
	virtual void transmitPonFrame(DummyPacket *msg);

#ifdef TRACE_TXRX
	void releaseTx(DummyPacket *msg);
	void receiveRx(DummyPacket *msg);
	void releaseRx(DummyPacket *msg);
#endif

	// Scheduling:	Note that we split the original scheduling function 'seqSchedule()'
	//				into two parts as follows:
	// - getTxTime(): Find the earliest possible TX time;
	// - scheduleFrame(): Schedule a frame and update global status variables.
	simtime_t getTxTime(const int ch, const bool isGrant, int &txIdx,
			int &rxIdx);
	void scheduleFrame(const simtime_t txTime, const int ch, const int txIdx,
			const int rxIdx, HybridPonDsFrame *ponFrameToOnu);

	// OMNeT++
	virtual void initialize(void);
	virtual void handleMessage(cMessage *msg);
	virtual void finish(void);
};

////------------------------------------------------------------------------------
//// Classes for sequential schedulers
////------------------------------------------------------------------------------

/////
///// @class OltSchedulerSequential
///// @brief Implements 'OltSchedulerSequential' module in a hybrid TDM/WDM-PON OLT.
///// @ingroup hybridpon
/////
//class OltSchedulerSequential: public OltScheduler
//{
//protected:
//	// NED parameters (as defined in NED files)
//	int queueSize; // Size of FIFO queue for data frames [bit]
//
//	// Status variables
//	int busyQueue; // Counter to emulate a FIFO for us & ds data frames
//
//protected:
//	// Misc.
//	virtual void handleGrant(int lambda, HybridPonDsGrantFrame *grant);
//	virtual void initializeSpecific(void);
//	virtual void finishSpecific(void);
//
//	// Event handling
//	//	virtual void receiveIpPacket(IpPacket *pkt);
//	virtual void receiveEthernetFrame(EtherFrame *frame);
//	void transmitPonFrame(DummyPacket *msg);
//
//	// OMNeT++
//	virtual void handleMessage(cMessage *msg);
//};

//class Sequential_v3 : public Sequential_v2     // Sequential scheduler Ver. 3
//{
//	//--------------------------------------------------------------------------
//    //	Member variables
//    //--------------------------------------------------------------------------
//
//
//	//--------------------------------------------------------------------------
//	//	Member functions
//	//--------------------------------------------------------------------------
//
//	// Event handling
//    virtual void	receiveIpPacket(IpPacket *pkt);
//    virtual void	transmitDataFrame(DummyPacket *msg);
//};
//
//
//class Sequential_v5 : public Sequential_v4	// Sequential scheduler Ver. 5
//{
//	//--------------------------------------------------------------------------
//	//	Member functions
//	//--------------------------------------------------------------------------
//
//	// Misc.
//	virtual int		assignGrants(int ch, int usReport);
//};

//------------------------------------------------------------------------------
// Classes based on batch operation mode
//------------------------------------------------------------------------------

//class Batch: public Scheduler {
//protected:
//	//--------------------------------------------------------------------------
//	//	Member variables
//	//--------------------------------------------------------------------------
//
//	// NED parameters (as defined in NED files)
//	simtime_t batchPeriod;
//	int voqSize;
//	double voqThreshold;
//
//	// For VOQs: Indexing is done as follows
//	// - Downstream data:       [0...numOnus-1]
//	// - Upstream grants/polls: [numOnus...2*Onus-1]
//	Voq *voq;
//	IntVector voqBitCtr; // vector of VOQ lengths [bit]
//	int voqStartIdx; // VOQ index to start scheduling with at each batch period
//	// --> used to provide better fairness among VOQs
//
//	// For trace of VOQs
//	cOutVector *vQueueLength; // array of output vector for VOQ length [frame]
//	cOutVector *vQueueOctet; // array of output vector for VOQ size in octet
//
//
//	//--------------------------------------------------------------------------
//	//	Member functions
//	//--------------------------------------------------------------------------
//
//	// Misc.
//	void debugVoqStatus(void);
//	virtual void debugSnapshot(void);
//	virtual void handleGrant(int lambda, HybridPonFrame *grant);
//	virtual void initializeSpecific(void);
//	virtual void finishSpecific(void);
//
//	// Event handling
//	virtual void sendOnuPoll(HybridPonMessage *msg);
////	virtual void receiveIpPacket(IpPacket *pkt);
//    virtual void receiveEthernetFrame(EtherFrame *frame);
//	void batchSchedule(cMessage *batchMsg);
//	void transmitVoqDataFrame(HybridPonMessage *msg);
//
//	// Scheduler to be defined in derived classes (pure virtual function)
//	virtual int scheduler(const simtime_t TX, const simtime_t RX, simtime_t &t,
//			int &voqsToSchedule, BoolVector &schedulableVoq, Voq *voq) = 0;
//
//	// OMNeT++
//	virtual void handleMessage(cMessage *msg);
//};

//class BEDF: public Batch {
//	//--------------------------------------------------------------------------
//	//	Member functions
//	//--------------------------------------------------------------------------
//
//	// Scheduler
//	virtual int scheduler(const simtime_t TX, const simtime_t RX, simtime_t &t,
//			int &voqsToSchedule, BoolVector &schedulableVoq, Voq *voq);
//};

//class LongestQueueFirst : public Batch
//{
//	//--------------------------------------------------------------------------
//	//	Member functions
//	//--------------------------------------------------------------------------
//
//	// Scheduler
//    virtual int scheduler(
//        const simtime_t TX,
//        const simtime_t RX,
//        simtime_t &t,
//        int &voqsToSchedule,
//        BoolVector &schedulableVoq,
//        Voq *voq
//        );
//};
//
//
//class EdfWithLqf : public Batch
//{
//	//--------------------------------------------------------------------------
//	//	Member functions
//	//--------------------------------------------------------------------------
//
//	// Scheduler
//    virtual int scheduler(
//        const simtime_t TX,
//        const simtime_t RX,
//        simtime_t &t,
//        int &voqsToSchedule,
//        BoolVector &schedulableVoq,
//        Voq *voq
//        );
//};


#endif  // __OLT_SCHEDULER_H

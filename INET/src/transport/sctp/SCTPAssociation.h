//
// Copyright (C) 2008 Irene Ruengeler
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __SCTPASSOCIATION_H
#define __SCTPASSOCIATION_H

#include <omnetpp.h>
#include "INETDefs.h"
#include "IPvXAddress.h"
#include "IPAddress.h"
#include "SCTP.h"
#include "RoutingTable.h"
#include "RoutingTableAccess.h"
#include "InterfaceTable.h"
#include "InterfaceTableAccess.h"
#include "SCTPSendStream.h"
#include "SCTPReceiveStream.h"
#include "SCTPMessage.h"
#include "IPControlInfo.h"
#include <list>
#include <iostream>
#include <errno.h>
#include <math.h>
#include <platdep/intxtypes.h>

class SCTPMessage;
class SCTPCommand;
class SCTPOpenCommand;
class SCTPQueue;
class SCTPReceiveStream;
class SCTPSendStream;
class SCTPAlgorithm;
class SCTP;

typedef std::vector<IPvXAddress> AddressVector;

enum SctpState
{
	SCTP_S_CLOSED            = 0,
	SCTP_S_COOKIE_WAIT       = FSM_Steady(1),
	SCTP_S_COOKIE_ECHOED     = FSM_Steady(2),
	SCTP_S_ESTABLISHED       = FSM_Steady(3),    
	SCTP_S_SHUTDOWN_PENDING  = FSM_Steady(4),
	SCTP_S_SHUTDOWN_SENT     = FSM_Steady(5),
	SCTP_S_SHUTDOWN_RECEIVED = FSM_Steady(6),
	SCTP_S_SHUTDOWN_ACK_SENT = FSM_Steady(7)
};


//
// Event, strictly for the FSM state transition purposes.
// DO NOT USE outside performStateTransition()!
//
enum SCTPEventCode
{
	SCTP_E_ASSOCIATE,
	SCTP_E_OPEN_PASSIVE,
	SCTP_E_ABORT,
	SCTP_E_SHUTDOWN,
	SCTP_E_CLOSE,
	SCTP_E_SEND,
	SCTP_E_RCV_INIT,
	SCTP_E_RCV_ABORT,
	SCTP_E_RCV_VALID_COOKIE_ECHO,
	SCTP_E_RCV_INIT_ACK,
	SCTP_E_RCV_COOKIE_ACK,
	SCTP_E_RCV_SHUTDOWN,
	SCTP_E_RCV_SHUTDOWN_ACK,  
	SCTP_E_RCV_SHUTDOWN_COMPLETE,
	SCTP_E_NO_MORE_OUTSTANDING,
	SCTP_E_TIMEOUT_INIT_TIMER,     
	SCTP_E_TIMEOUT_SHUTDOWN_TIMER,
	SCTP_E_TIMEOUT_RTX_TIMER,
	SCTP_E_TIMEOUT_HEARTBEAT_TIMER,
	SCTP_E_IGNORE,
	SCTP_E_RECEIVE,
	SCTP_E_DUP_RECEIVED,
	SCTP_E_PRIMARY,
	SCTP_E_DELIVERED,
	SCTP_E_QUEUE,
	SCTP_E_SEND_SHUTDOWN_ACK,
	SCTP_E_STOP_SENDING
};

enum SCTPChunkTypes
{
	DATA			= 0, 
	INIT			= 1, 
	INIT_ACK		= 2, 
	SACK			= 3,
	HEARTBEAT		= 4, 
	HEARTBEAT_ACK		= 5, 
	ABORT			= 6, 
	SHUTDOWN		= 7, 
	SHUTDOWN_ACK		= 8, 
	ERRORTYPE		= 9, 
	COOKIE_ECHO		= 10, 
	COOKIE_ACK		= 11, 
	SHUTDOWN_COMPLETE	= 14,

};


enum SCTPFlags
{
	COMPLETE_MESG_UNORDERED	= 1,
	COMPLETE_MESG_ORDERED	= 0
};


enum SCTPParameterTypes
{
	UNRECOGNIZED_PARAMETER			= 8,
	SUPPORTED_ADDRESS_TYPES			= 12,
};




enum SCTPCCModules
{
	RFC4960					= 0
};

enum SCTPStreamSchedulers
{
	ROUND_ROBIN				= 0
};


#define SCTP_COMMON_HEADER  				12    // without options
#define SCTP_INIT_CHUNK_LENGTH				20
#define SCTP_DATA_CHUNK_LENGTH  			16
#define SCTP_SACK_CHUNK_LENGTH				16
#define SCTP_HEARTBEAT_CHUNK_LENGTH			4
#define SCTP_ABORT_CHUNK_LENGTH				4
#define SCTP_COOKIE_ACK_LENGTH				4
#define SCTP_FORWARD_TSN_CHUNK_LENGTH			8
#define SCTP_SHUTDOWN_CHUNK_LENGTH			8
#define SCTP_SHUTDOWN_ACK_LENGTH			4
#define SCTP_ERROR_CHUNK_LENGTH				4    // without parameters

#define SCTP_DEFAULT_ARWND 				(1<<16)
#define SCTP_DEFAULT_INBOUND_STREAMS 			10 //17
#define SCTP_DEFAULT_OUTBOUND_STREAMS 			5 //17
#define VALID_COOKIE_LIFE_TIME  			10
#define SCTP_COOKIE_LENGTH				76
#define HB_INTERVAL					30
#define PATH_MAX_RETRANS				5

#define SCTP_TIMEOUT_INIT_REXMIT     			3    // initially 3 seconds
#define SCTP_TIMEOUT_INIT_REXMIT_MAX 			240  // 4 mins 
#define SACK_DELAY              			0.2
#define RTO_BETA					0.25
#define RTO_ALPHA					0.125
#define RTO_INITIAL					3
#define IPTOS_DEFAULT           			0x10    /* IPTOS_LOWDELAY */

#define DEFAULT_MAX_SENDQUEUE   			0       /* unlimited send queue */
#define DEFAULT_MAX_RECVQUEUE   			0       /* unlimited recv queue - unused really */

#define MAX_ASSOCS					10

#define SCTP_MAX_PAYLOAD				1488 // 12 bytes for common header

#define MAX_GAP_COUNT					360
#define MAX_GAP_REPORTS					4
#define ADD_PADDING(x)					((((x) + 3) >> 2) << 2)

//#define RFC						0
#define DEBUG						1

#define SHUTDOWN_GUARD_TIMEOUT				180

/**
 * Returns the minimum of a and b.
 */
inline double min(double a, double b) {return a<b ? a : b;}

/**
 * Returns the maximum of a and b.
 */
inline double max(double a, double b) {return a<b ? b : a;}


class SCTPDataVariables : public cPolymorphic
{
	public:
		SCTPDataVariables();
		~SCTPDataVariables();
		cPacket* userData;
		bool ordered;
		uint16 len;
		uint32 tsn;
		uint16 sid;
		uint16 ssn;
		uint32 ppid;
		uint32 gapReports;
		simtime_t enqueuingTime;
		simtime_t sendTime;
		simtime_t ackTime;
		simtime_t expiryTime;
		bool hasBeenAcked;
		bool hasBeenRemoved; //chunk has been removed from receiveStream
		bool hasBeenAbandoned;
		bool hasBeenFastRetransmitted;
		bool countsAsOutstanding;
		IPvXAddress lastDestination;
		IPvXAddress initialDestination;
		IPvXAddress nextDestination;
		uint32 numberOfTransmissions;
		uint32 numberOfRetransmissions;
		uint32 allowedNoRetransmissions;
		uint32 booksize;
};



class SCTPPathVariables : public cPolymorphic
{
	public:
		SCTPPathVariables(IPvXAddress addr, SCTPAssociation* assoc);
		~SCTPPathVariables();
		SCTPAssociation* association;
		IPvXAddress remoteAddress;
		bool activePath;
		bool confirmed;
		bool requiresRtx;
		bool hbWasAcked;
		bool primaryPathCandidate;
		bool forceHb;
		uint32 pathErrorCount;
		uint32 pathErrorThreshold;
		uint32 pmtu;
		uint32 cwnd;
		uint32 ssthresh;
		uint32 partialBytesAcked;
		uint32 outstandingBytes;
		simtime_t heartbeatTimeout;
		simtime_t heartbeatIntervalTimeout;
		simtime_t rtxTimeout;
		simtime_t cwndTimeout;
		simtime_t updateTime;
		simtime_t lastAckTime;
		simtime_t cwndAdjustmentTime;
		simtime_t pathRto;  //double
		simtime_t srtt;
		simtime_t rttvar;
		cMessage *HeartbeatTimer;
		cMessage *HeartbeatIntervalTimer;
		cMessage *CwndTimer; 
		cMessage *T3_RtxTimer;
		cOutVector *pathSsthresh;
		cOutVector *pathCwnd;
		cOutVector *pathTSN;
		cOutVector *pathRcvdTSN;
		cOutVector *pathRTO;
		cOutVector *pathRTT;
};




class SCTPStateVariables : public cPolymorphic
{
	public:
		SCTPStateVariables();
		~SCTPStateVariables();
	public:
		bool active;
		bool fork;
		bool ackPointAdvanced;
		bool dataChunkReceived;
		bool initReceived;
		bool cookieEchoReceived;
		bool newChunkReceived;
		bool firstChunkReceived;
		bool swsAvoidanceInvoked;
		bool probingIsAllowed;
		bool zeroWindowProbing;
		bool alwaysBundleSack;
		bool fastRecoveryActive;
		bool fastRecoverySupported;
		bool nagleEnabled;
		bool sackAllowed;
		bool reactivatePrimaryPath;
		bool fragment;
		bool resetPending;
		bool stopReceiving; // incoming data will be discarded
		bool stopOldData;	  // data with TSN<peerTsnAfterReset will be discarded
		bool queueUpdate;
		bool firstDataSent;
		bool peerWindowFull;
		bool zeroWindow;
		bool stopSending;   //will be called when SCTP_E_SHUTDOWN arrived
		bool inOut;
		bool noMoreOutstanding;
		IPvXAddress primaryPathIndex;
		IPvXAddress initialPrimaryPath;	
		IPvXAddress lastUsedDataPath;
		IPvXAddress lastDataSourceAddress;
		IPvXAddress nextDest;
		AddressVector localAddresses;
		std::list<uint32> dupList;
		uint32 errorCount;  // overall error counter
		uint64 peerRwnd;
		uint64 initialPeerRwnd;
		uint64 localRwnd;
		uint32 nextTSN; // TSN to be sent
		uint32 lastTsnAck; //stored at the sender side; cumTSNAck announced in a SACK
		uint32 cTsnAck; // will be put in the SACK chunk
		uint32 highestTsnReceived; // will be set when DATA chunk arrived
		uint32 highestTsnAcked;
		uint32 highestTsnStored; //used to compare Tsns in makeRoomForTsn
		uint32 lastTsnReceived; //SACK
		uint32 lastTSN; // my very last TSN to be sent
		uint32 ackState; // number of packets to be acknowledged
		uint32 numGaps;
		uint32 gapStartList[MAX_GAP_COUNT];
		uint32 gapStopList[MAX_GAP_COUNT];
		uint64 queuedBytes;
		uint64 queuedRcvBytes;	//number of messages in receive queues
		uint32 fastRecoveryExitPoint;
		uint32 lastStreamScheduled;
		uint32 assocPmtu; //smallest overall path mtu
		uint32 msgNum; //indicates the sequence number of the message
		uint64 bytesRcvd;
		uint32 numRequests;
		uint32 messagesToPush;
		int32 pushMessagesLeft;
		uint32 count;
		uint8 localTieTag[32];
		uint8 peerTieTag[32];
		uint64 queuedMessages;  // Messages buffered at the sender side
		uint32 messageAcceptLimit;
		uint32 queueLimit;
		uint16 header;
		int32 probingTimeout;
		int32 numMsgsReq[100];
		int32 cookieLifeTime;
		/** Counter for init and cookie retransmissions */
		int16 initRetransCounter;
		//int16 numParam; 
		simtime_t initRexmitTimeout;
		/** pointer to the init chunk data structure (for retransmissions) */
		SCTPInitChunk *initChunk;
		/** pointer to the cookie chunk data structure (for retransmissions) */
		SCTPCookieEchoChunk *cookieChunk;
		/** pointer to the resetChunk (for retransmission) */
		SCTPShutdownChunk* shutdownChunk;
		SCTPShutdownAckChunk* shutdownAckChunk;
		SCTPMessage* sctpmsg;
		int32 sendQueueLimit;
		bool appSendAllowed;
		uint32 nextRSid;
		uint32 swsLimit;
};
	


class SCTPAssociation : public cObject
{
	public:
		// connection identification by apps: appgateIndex+assocId
		int32 appGateIndex; // application gate index
		int32 assocId;       // identifies connection within the app
		IPvXAddress remoteAddr;
		IPvXAddress localAddr;
		uint16 localPort;
		uint16 remotePort;
		bool listen;
		uint32 localVTag;   // local verification tag
		uint32 peerVTag;    // remote verification tag
		
		// timers
		cMessage *T1_InitTimer;
		cMessage *T2_ShutdownTimer;
		cMessage *T5_ShutdownGuardTimer;
		cMessage *SackTimer;
		cMessage *StartTesting;
	
	protected:

		AddressVector localAddressList;
		AddressVector remoteAddressList;

		int32	status;
		uint32 numberOfRemoteAddresses;

		uint32 initTsn; //DATA
		uint32 initPeerTsn; 
		uint32 inboundStreams;
		uint32 outboundStreams;
		uint32 sackFrequency;
		double sackPeriod;
	// variables associated with the state of this Association
		SCTPStateVariables *state;

		cOutVector *advRwnd;
		cOutVector *quBytes;
		cOutVector *cumTsnAck;
		cOutVector* sendQueue;
		
		SCTP *sctpMain;  // SCTP module
	
		// SCTP state machine
		cFSM* fsm;	
		
		// map for storing the path parameters
		typedef std::map<IPvXAddress,SCTPPathVariables*> SCTPPathMap;
		SCTPPathMap sctpPathMap;
		
		//map for storing the queued bytes per path
		typedef std::map<IPvXAddress, uint32> CounterMap;
		typedef struct counter {
			uint64 roomSumSendStreams;
			uint64 bookedSumSendStreams;
			uint64 roomSumRcvStreams;
			CounterMap roomTransQ;
			CounterMap bookedTransQ;
			CounterMap roomRetransQ;
			} QueueCounter;
		QueueCounter qCounter;
		// SCTP queues
		SCTPQueue *transmissionQ;
		SCTPQueue *retransmissionQ;
	
		typedef std::map<uint32, SCTPSendStream*> SCTPSendStreamMap;
		SCTPSendStreamMap sendStreams;
		
		typedef std::map<uint32, SCTPReceiveStream*> SCTPReceiveStreamMap;
	
		SCTPReceiveStreamMap receiveStreams;
		// SCTP behavior in data transfer state
		SCTPAlgorithm *sctpAlgorithm;
		typedef struct calcBytesToSend {
			bool chunk;
			bool packet;
			uint32 bytesToSend;
		} BytesToBeSent;
		BytesToBeSent bytes;
		

		typedef struct congestionControlFunctions {
			void (SCTPAssociation::*ccInitParams)(SCTPPathVariables* path);
			void (SCTPAssociation::*ccUpdateAfterSack)(bool rtxNecessary, SCTPPathVariables* path);
			void (SCTPAssociation::*ccUpdateAfterCwndTimeout)(SCTPPathVariables* path);
			void (SCTPAssociation::*ccUpdateAfterRtxTimeout)(SCTPPathVariables* path);
			void (SCTPAssociation::*ccUpdateMaxBurst)(SCTPPathVariables* path);
			void (SCTPAssociation::*ccUpdateBytesAcked)(uint32 ackedBytes, uint32 osb, bool ctsnaAdvanced, IPvXAddress pathId, uint32 pathOsb, uint32 newOsb);
		} CCFunctions;
		CCFunctions ccFunctions;
		uint16 ccModule;
	public:
		/**
		* Constructor.
		*/
		SCTPAssociation(SCTP *mod, int32 appGateIndex, int32 assocId);
	
		/**
		* Destructor.
		*/
		~SCTPAssociation();
		/**
		* Utility: Send data from sendQueue, at most maxNumBytes (-1 means no limit).
		* If fullSegments is set, don't send segments smaller than MSS (needed for Nagle).
		* Returns true if some data was actually sent.
		*/
		
		void sendAll(IPvXAddress pathId);
		
		/** Utility: returns name of SCTP_I_xxx constants */
		static const char *indicationName(int32 code);
		
		/* @name Various getters */
		//@{
		int32 getFsmState() const {return fsm->getState();};
		SCTPStateVariables *getState() {return state;};
		SCTPQueue *getTransmissionQueue() {return transmissionQ;};
		SCTPQueue *getRetransmissionQueue() {return retransmissionQ;};
		SCTPAlgorithm *getSctpAlgorithm() {return sctpAlgorithm;};
		SCTP *getSctpMain() {return sctpMain;};
		cFSM* getFsm() {return fsm;};
		
		cMessage *getInitTimer() {return T1_InitTimer;};
		cMessage *getShutdownTimer() {return T2_ShutdownTimer;};
		cMessage *getSackTimer() {return SackTimer;};
		
		/** Utility: returns name of SCTP_S_xxx constants */
		static const char *stateName(int32 state);
		
		uint32 chunkToInt(char* type);
		
		
		/* Process self-messages (timers).
		* Normally returns true. A return value of false means that the
		* connection structure must be deleted by the caller (SCTPMain).
		*/
		bool processTimer(cMessage *msg);
		
			
		/**
		* Process incoming SCTP segment. Normally returns true. A return value
		* of false means that the connection structure must be deleted by the
		* caller (SCTP).
		*/
		bool processSCTPMessage(SCTPMessage *sctpmsg, IPvXAddress srcAddr, IPvXAddress destAddr);
		
		/**
		* Process commands from the application.
		* Normally returns true. A return value of false means that the
		* connection structure must be deleted by the caller (SCTP).
		*/
		bool processAppCommand(cPacket *msg);
		
		void removePath();
		void removePath(IPvXAddress addr);
		void removeLastPath(IPvXAddress addr);
		void deleteStreams();
		void stopTimer(cMessage* timer);
		void stopTimers();
		
		SCTPPathVariables* getPath(IPvXAddress pid);
		void printSctpPathMap();		

	protected:
		
				


		/** @name FSM transitions: analysing events and executing state transitions */
		//@{
		/** Maps app command codes (msg kind of app command msgs) to SCTP_E_xxx event codes */
		SCTPEventCode preanalyseAppCommandEvent(int32 commandCode);
		/** Implemements the pure SCTP state machine */
		bool performStateTransition(const SCTPEventCode& event);
		void stateEntered(int32 state);
		//@}
	
		/** @name Processing app commands. Invoked from processAppCommand(). */
		//@{
		void process_ASSOCIATE(SCTPEventCode& event, SCTPCommand *sctpCommand, cPacket *msg); 
		void process_OPEN_PASSIVE(SCTPEventCode& event, SCTPCommand *sctpCommand, cPacket *msg);
		void process_SEND(SCTPEventCode& event, SCTPCommand *sctpCommand, cPacket *msg);
		void process_CLOSE(SCTPEventCode& event);
		void process_ABORT(SCTPEventCode& event);
		void process_STATUS(SCTPEventCode& event, SCTPCommand *sctpCommand, cPacket *msg);
		void process_RECEIVE_REQUEST(SCTPEventCode& event, SCTPCommand *sctpCommand);
		void process_PRIMARY(SCTPEventCode& event, SCTPCommand *sctpCommand);
		//@}
	
		/** @name Processing SCTP message arrivals. Invoked from processSCTPMessage(). */
		//@{
		bool process_RCV_Message(SCTPMessage *sctpseg, IPvXAddress src, IPvXAddress dest);
		/**
		* Process incoming SCTP packets. Invoked from process_RCV_Message
		*/
		bool processInitArrived(SCTPInitChunk* initChunk, int32 sport, int32 dport);
		bool processInitAckArrived(SCTPInitAckChunk* initAckChunk);
		bool processCookieEchoArrived(SCTPCookieEchoChunk* cookieEcho, IPvXAddress addr);
		bool processCookieAckArrived();
		SCTPEventCode processDataArrived(SCTPDataChunk *dataChunk, uint32 count);
		SCTPEventCode processSackArrived(SCTPSackChunk *sackChunk);
		SCTPEventCode processHeartbeatAckArrived(SCTPHeartbeatAckChunk* heartbeatack, SCTPPathVariables* path);
		//@}
	
		/** @name Processing timeouts. Invoked from processTimer(). */
		//@{
		int32 process_TIMEOUT_RTX(SCTPPathVariables* path);
		void process_TIMEOUT_HEARTBEAT(SCTPPathVariables* path);
		void process_TIMEOUT_HEARTBEAT_INTERVAL(SCTPPathVariables* path, bool force);
		void process_TIMEOUT_INIT_REXMIT(SCTPEventCode& event);
		void process_TIMEOUT_CWND(SCTPPathVariables* path);
		void process_TIMEOUT_PROBING();
		void process_TIMEOUT_SHUTDOWN(SCTPEventCode& event);
		int32 updateCounters(SCTPPathVariables* path);
		//@}
		
		void startTimer(cMessage* timer, simtime_t timeout);
	
		/** Utility: clone a listening association. Used for forking. */
		SCTPAssociation *cloneAssociation();
	
		/** Utility: creates send/receive queues and sctpAlgorithm */
		void initAssociation(SCTPOpenCommand *openCmd);
			
		/** Methods dealing with the handling of TSNs  **/
		bool tsnIsDuplicate(uint32 tsn);	
		bool advanceCtsna();
		bool updateGapList(uint32 tsn);
		void removeFromGapList(uint32 removedTsn);
		bool makeRoomForTsn(uint32 tsn, uint32 length, bool uBit);
	
		/** Methods for creating and sending chunks */
		void sendInit();
		void sendInitAck(SCTPInitChunk* initchunk);
		void sendCookieEcho(SCTPInitAckChunk* initackchunk);
		void sendCookieAck(IPvXAddress dest);
		void sendAbort();
		void sendHeartbeat(SCTPPathVariables *path, bool local);
		void sendHeartbeatAck(SCTPHeartbeatChunk *heartbeatChunk, IPvXAddress src, IPvXAddress dest);
		void sendSack();
		void sendShutdown();
		void sendShutdownAck(IPvXAddress dest);
		void sendShutdownComplete();
		SCTPSackChunk* createSack();
		/** Retransmitting chunks */
		void retransmitInit();
		void retransmitCookieEcho();
		void retransmitShutdown();
		void retransmitShutdownAck();

	
		/** Utility: adds control info to message and sends it to IP */
		//void sendToIP(SCTPMessage *sctpmsg);
		//void sendToIP(SCTPMessage *sctpmsg);
		void sendToIP(SCTPMessage *sctpmsg);
		void sendToIP(SCTPMessage *sctpmsg, IPvXAddress dest);
		void sendToIP(SCTPMessage *sctpmsg, IPvXAddress src, IPvXAddress dest);

		void scheduleSack();
		/** Utility: signal to user that connection timed out */
		void signalConnectionTimeout();
	
		/** Utility: start a timer */
		void scheduleTimeout(cMessage *msg, simtime_t timeout)
			{sctpMain->scheduleAt(simulation.getSimTime()+timeout, msg);}
	
		/** Utility: cancel a timer */
		cMessage *cancelEvent(cMessage *msg)  {return sctpMain->cancelEvent(msg);}
	
		/** Utility: sends packet to application */
		void sendToApp(cPacket *msg);
	
		/** Utility: sends status indication (SCTP_I_xxx) to application */
		void sendIndicationToApp(int32 code);
	
		/** Utility: sends SCTP_I_ESTABLISHED indication with SCTPConnectInfo to application */
		void sendEstabIndicationToApp();
		void pushUlp();
		void sendDataArrivedNotification(uint16 sid);
		void putInDeliveryQ(uint16 sid);
		/** Utility: prints local/remote addr/port and app gate index/assocId */
		void printConnBrief();
		/** Utility: prints important header fields */
		static void printSegmentBrief(SCTPMessage *sctpmsg);
		
		

		/** Utility: returns name of SCTP_E_xxx constants */
		static const char *eventName(int32 event);

		void addPath(IPvXAddress addr);
		IPvXAddress getNextDestination(SCTPDataVariables* chk);
		IPvXAddress getNextAddress(IPvXAddress dpi);
		
		void bytesAllowedToSend(IPvXAddress dpi);
			
		void pathStatusIndication(IPvXAddress pid, bool status);

		bool allPathsInactive(void);
		uint32 getLevel(IPvXAddress addr);
		
		/**
		* Manipulating chunks
		*/
		SCTPDataChunk* transformDataChunk(SCTPDataVariables* datVar);
		SCTPDataVariables* makeVarFromMsg(SCTPDataChunk* datachunk);
		
		/**
		*Dealing with streams 
		*/

		int32 streamScheduler(bool peek);
		void initStreams(uint32 inStreams, uint32 outStreams);
		int32 numUsableStreams(void);

		typedef struct streamSchedulingFunctions {
			void (SCTPAssociation::*ssInitStreams)(uint32 inStreams, uint32 outStreams);
			int32 (SCTPAssociation::*ssGetNextSid)(bool peek);
			int32 (SCTPAssociation::*ssUsableStreams)(void);
		} SSFunctions;
		SSFunctions ssFunctions;
		uint16 ssModule;

		/**
		*	Queue Management
		*/
		int32 getQueuedBytes(void);
		int32 getOutstandingBytes(void);
		int32 dequeueAckedChunks(uint32 tsna, IPvXAddress pathId, simtime_t* rttEstimation);
		uint32 getOutstandingBytesOnPath(IPvXAddress pathId);
		SCTPDataVariables* peekOutboundDataChunk(IPvXAddress pid);
		SCTPDataMsg* peekOutboundDataMsg(void);
		SCTPDataVariables* peekAbandonedChunk(IPvXAddress pid);
		SCTPDataVariables* getOutboundDataChunk(IPvXAddress pid, int32 bytes);
		SCTPDataMsg* dequeueOutboundDataMsg(int32 bytes);
		void process_QUEUE(SCTPCommand *sctpCommand);
		/**
		* Flow control
		*/
		void pmDataIsSentOn(IPvXAddress pathId);
		void pmStartPathManagement(void);
		void pmClearPathCounter(IPvXAddress pid);
		void pmRttMeasurement(IPvXAddress pathId, simtime_t rttEstimate, int32 acknowledgedBytes);
		void fcAdjustCounters(uint32 ackedBytes, uint32 osb, bool ctsnaAdvanced, IPvXAddress pathId, uint32 pathOsb, uint32 newOsb);
		
		/**
		* Compare TSNs
		*/
		int32 tsnLt (uint32 tsn1, uint32 tsn2) { return ((int32)(tsn1-tsn2)<0); }
		int32 tsnLe (uint32 tsn1, uint32 tsn2) { return ((int32)(tsn1-tsn2)<=0); }
		int32 tsnGe (uint32 tsn1, uint32 tsn2) { return ((int32)(tsn1-tsn2)>=0); }
		int32 tsnGt (uint32 tsn1, uint32 tsn2) { return ((int32)(tsn1-tsn2)>0); }
		int32 tsnBetween (uint32 tsn1, uint32 midtsn, uint32 tsn2) { return ((tsn2-tsn1)>=(midtsn-tsn1)); }

		int16 ssnGt (uint16 ssn1, uint16 ssn2) { return ((int16)(ssn1-ssn2)>0); }
		
		uint32 subBytes(uint32 osb, uint32 bytes) {return (((int32)(osb-bytes)>0)?osb-bytes:0);}
		
		void disposeOf(SCTPMessage* sctpmsg);
		void printOutstandingTsns();

		/** SCTPCCFunctions  **/
		void initCCParameters(SCTPPathVariables* path);

		void cwndUpdateAfterSack(bool rtxNecessary, SCTPPathVariables* path);
	
		void cwndUpdateAfterCwndTimeout(SCTPPathVariables* path);

		void cwndUpdateAfterRtxTimeout(SCTPPathVariables* path);

		void cwndUpdateMaxBurst(SCTPPathVariables* path);

		void cwndUpdateBytesAcked(uint32 ackedBytes, uint32 osb, bool ctsnaAdvanced, IPvXAddress pathId, uint32 pathOsb, uint32 newOsb);
};

#endif



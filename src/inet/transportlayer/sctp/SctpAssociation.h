//
// Copyright (C) 2005-2010 Irene Ruengeler
// Copyright (C) 2009-2015 Thomas Dreibholz
// Copyright (C) 2015 Martin Becke
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_SCTPASSOCIATION_H
#define __INET_SCTPASSOCIATION_H

#include <errno.h>
#include <iostream>
#include <list>
#include <math.h>

#include "inet/common/INETDefs.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/networklayer/ipv6/Ipv6RoutingTable.h"
#include "inet/transportlayer/sctp/Sctp.h"
#include "inet/transportlayer/sctp/SctpGapList.h"
#include "inet/transportlayer/sctp/SctpHeader.h"
#include "inet/transportlayer/sctp/SctpQueue.h"
#include "inet/transportlayer/sctp/SctpReceiveStream.h"
#include "inet/transportlayer/sctp/SctpSendStream.h"

namespace inet {

class SctpCommand;
class SctpOpenCommand;

namespace sctp {

class SctpHeader;
class SctpReceiveStream;
class SctpSendStream;
class SctpAlgorithm;
class Sctp;

typedef std::vector<L3Address> AddressVector;

enum SctpState {
    SCTP_S_CLOSED = 0,
    SCTP_S_COOKIE_WAIT = FSM_Steady(1),
    SCTP_S_COOKIE_ECHOED = FSM_Steady(2),
    SCTP_S_ESTABLISHED = FSM_Steady(3),
    SCTP_S_SHUTDOWN_PENDING = FSM_Steady(4),
    SCTP_S_SHUTDOWN_SENT = FSM_Steady(5),
    SCTP_S_SHUTDOWN_RECEIVED = FSM_Steady(6),
    SCTP_S_SHUTDOWN_ACK_SENT = FSM_Steady(7)
};

//
// Event, strictly for the FSM state transition purposes.
// DO NOT USE outside performStateTransition()!
//
enum SctpEventCode {
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
    SCTP_E_QUEUE_MSGS_LIMIT,
    SCTP_E_QUEUE_BYTES_LIMIT,
    SCTP_E_SEND_QUEUE_LIMIT,
    SCTP_E_SEND_SHUTDOWN_ACK,
    SCTP_E_STOP_SENDING,
    SCTP_E_STREAM_RESET,
    SCTP_E_RESET_ASSOC,
    SCTP_E_ADD_STREAMS,
    SCTP_E_SEND_ASCONF,
    SCTP_E_SET_STREAM_PRIO,
    SCTP_E_ACCEPT,
    SCTP_E_SET_RTO_INFO,
    SCTP_E_ACCEPT_SOCKET_ID
};

enum SCTPChunkTypes {
    DATA = 0,
    INIT = 1,
    INIT_ACK = 2,
    SACK = 3,
    HEARTBEAT = 4,
    HEARTBEAT_ACK = 5,
    ABORT = 6,
    SHUTDOWN = 7,
    SHUTDOWN_ACK = 8,
    ERRORTYPE = 9,
    COOKIE_ECHO = 10,
    COOKIE_ACK = 11,
    SHUTDOWN_COMPLETE = 14,
    AUTH = 15,
    NR_SACK = 16,
    ASCONF_ACK = 128,
    PKTDROP = 129,
    RE_CONFIG = 130,
    FORWARD_TSN = 192,
    ASCONF = 193,
    IFORWARD_TSN = 194
};

enum SctpPrMethods {
    PR_NONE = 0,
    PR_TTL = 1,
    PR_RTX = 2,
    PR_PRIO = 3,
    PR_STRRST = 4
};

enum SctpStreamResetConstants {
    NOTHING_TO_DO = 0,
    PERFORMED = 1,
    DENIED = 2,
    WRONG_SSN = 3,
    REQUEST_IN_PROGRESS = 4,
    NO_RESET = 5,
    DEFERRED = 6,
    RESET_OUTGOING = 7,
    RESET_INCOMING = 8,
    RESET_BOTH = 9,
    SSN_TSN = 10,
    ADD_INCOMING = 11,
    ADD_OUTGOING = 12,
    OUTGOING_RESET_REQUEST_PARAMETER = 13,
    INCOMING_RESET_REQUEST_PARAMETER = 14,
    SSN_TSN_RESET_REQUEST_PARAMETER = 15,
    STREAM_RESET_RESPONSE_PARAMETER = 16,
    ADD_OUTGOING_STREAMS_REQUEST_PARAMETER = 17,
    ADD_INCOMING_STREAMS_REQUEST_PARAMETER = 18,
    ADD_BOTH = 19,
    PERFORMED_WITH_OPTION = 20,
    PERFORMED_WITH_ADDOUT = 21
};

enum SctpAddIPCorrelatedTypes {
    SET_PRIMARY_ADDRESS = 49156,
    ADAPTATION_LAYER_INDICATION = 49158,
    SUPPORTED_EXTENSIONS = 32776,
    ADD_IP_ADDRESS = 49153,
    DELETE_IP_ADDRESS = 49154,
    ERROR_CAUSE_INDICATION = 49155,
    SUCCESS_INDICATION = 49157,
    ERROR_DELETE_LAST_IP_ADDRESS = 160,
    ERROR_DELETE_SOURCE_ADDRESS = 162
};

enum SctpParameterTypes {
    UNRECOGNIZED_PARAMETER = 8,
    SUPPORTED_ADDRESS_TYPES = 12,
    FORWARD_TSN_SUPPORTED_PARAMETER = 49152,
    RANDOM = 32770,
    CHUNKS = 32771,
    HMAC_ALGO = 32772
};

enum SctpErrorCauses {
    INVALID_STREAM_IDENTIFIER = 1,
    NO_USER_DATA = 9,
    UNSUPPORTED_HMAC = 261,
    MISSING_NAT_ENTRY = 177
};

enum SctpCcModules {
    RFC4960 = 0
};

enum SctpStreamSchedulers {
    ROUND_ROBIN = 0,
    ROUND_ROBIN_PACKET = 1,
    RANDOM_SCHEDULE = 2,
    RANDOM_SCHEDULE_PACKET = 3,
    FAIR_BANDWITH = 4,
    FAIR_BANDWITH_PACKET = 5,
    PRIORITY = 6,
    FCFS = 7,
    PATH_MANUAL = 8,
    PATH_MAP_TO_PATH = 9
};

#define SCTP_COMMON_HEADER                              12      // without options
#define SCTP_INIT_CHUNK_LENGTH                          20
#define SCTP_DATA_CHUNK_LENGTH                          16
#define SCTP_SACK_CHUNK_LENGTH                          16
#define SCTP_NRSACK_CHUNK_LENGTH                        20
#define SCTP_HEARTBEAT_CHUNK_LENGTH                     4
#define SCTP_ABORT_CHUNK_LENGTH                         4
#define SCTP_COOKIE_ACK_LENGTH                          4
#define SCTP_FORWARD_TSN_CHUNK_LENGTH                   8
#define SCTP_SHUTDOWN_CHUNK_LENGTH                      8
#define SCTP_SHUTDOWN_ACK_LENGTH                        4
#define SCTP_ERROR_CHUNK_LENGTH                         4       // without parameters
#define SCTP_STREAM_RESET_CHUNK_LENGTH                  4   // without parameters
#define SCTP_OUTGOING_RESET_REQUEST_PARAMETER_LENGTH    16   // without streams
#define SCTP_INCOMING_RESET_REQUEST_PARAMETER_LENGTH    8   // without streams
#define SCTP_SSN_TSN_RESET_REQUEST_PARAMETER_LENGTH     8
#define SCTP_STREAM_RESET_RESPONSE_PARAMETER_LENGTH     12
#define SCTP_ADD_STREAMS_REQUEST_PARAMETER_LENGTH       12
#define SCTP_SUPPORTED_EXTENSIONS_PARAMETER_LENGTH      4
#define SCTP_ADD_IP_CHUNK_LENGTH                        8
#define SCTP_ADD_IP_PARAMETER_LENGTH                    8
#define SCTP_AUTH_CHUNK_LENGTH                          8
#define SCTP_PKTDROP_CHUNK_LENGTH                       16  // without included dropped packet
#define SHA_LENGTH                                      20
#define IP_HEADER_LENGTH                                20
#define SCTP_DEFAULT_ARWND                              (1 << 16)
#define SCTP_DEFAULT_INBOUND_STREAMS                    17
#define SCTP_DEFAULT_OUTBOUND_STREAMS                   17
#define VALID_COOKIE_LIFE_TIME                          10
#define SCTP_COOKIE_LENGTH                              76
#define HB_INTERVAL                                     30
#define PATH_MAX_RETRANS                                5
#define SCTP_COMPRESSED_NRSACK_CHUNK_LENGTH             16

#define SCTP_TIMEOUT_INIT_REXMIT                        3     // initially 3 seconds
#define SCTP_TIMEOUT_INIT_REXMIT_MAX                    240   // 4 mins
#define SACK_DELAY                                      0.2
#define RTO_BETA                                        0.25
#define RTO_ALPHA                                       0.125
#define RTO_INITIAL                                     3
#define IPTOS_DEFAULT                                   0x10      // IPTOS_LOWDELAY

#define DEFAULT_MAX_SENDQUEUE                           0           /* unlimited send queue */
#define DEFAULT_MAX_RECVQUEUE                           0           /* unlimited recv queue - unused really */

#define MAX_ASSOCS                                      10

#define SCTP_MAX_PAYLOAD                                1488 // 12 bytes for common header

#define ADD_PADDING(x)                                  ((((x) + 3) >> 2) << 2)

#define DEBUG                                           1

#define SHUTDOWN_GUARD_TIMEOUT                          180

/**
 * Returns the minimum of a and b.
 */
inline double min(const double a, const double b) { return (a < b) ? a : b; }

/**
 * Returns the maximum of a and b.
 */
inline double max(const double a, const double b) { return (a < b) ? b : a; }

class INET_API SctpPathVariables : public cObject
{
  public:
    SctpPathVariables(const L3Address& addr, SctpAssociation *assoc, const IRoutingTable *rt);
    ~SctpPathVariables();

    SctpAssociation *association;
    L3Address remoteAddress;

    // ====== Path Management =============================================
    bool activePath;
    bool confirmed;
    bool requiresRtx;
    bool primaryPathCandidate;
    bool forceHb;
    // ====== Last SACK ===================================================
    simtime_t lastSACKSent;
    // ====== T3 Timer Handling ===========================================
    // Set to TRUE when CumAck has acknowledged TSNs on this path.
    // Needed to reset T3 timer.
    bool newCumAck;
    // ====== Path Status =================================================
    uint32 pathErrorCount;
    uint32 pathErrorThreshold;
    uint32 pmtu;
    // ====== Congestion Control ==========================================
    uint32 cwnd;
    uint32 tempCwnd;
    uint32 ssthresh;
    uint32 partialBytesAcked;
    uint32 queuedBytes;
    uint32 outstandingBytes;
    // ~~~~~~ Temporary storage for SACK handling ~~~~~~~
    uint32 outstandingBytesBeforeUpdate;
    uint32 newlyAckedBytes;
    // ====== Fast Recovery ===============================================
    bool fastRecoveryActive;
    uint32 fastRecoveryExitPoint;
    simtime_t fastRecoveryEnteringTime;
    // ====== Lowest TSN (used for triggering T3 RTX Timer Restart) =======
    bool findLowestTsn;
    bool lowestTsnRetransmitted;

    // ====== Timers ======================================================
    cMessage *HeartbeatTimer;
    cMessage *HeartbeatIntervalTimer;
    cMessage *CwndTimer;
    cMessage *T3_RtxTimer;
    cMessage *BlockingTimer;
    cPacket *ResetTimer;
    cMessage *AsconfTimer;
    // ====== High-Speed CC ===============================================
    unsigned int highSpeedCCThresholdIdx;
    // ====== Max Burst ===================================================
    uint32 packetsInBurst;
    // ====== CMT Split Fast Retransmission ===============================
    bool sawNewAck;
    uint32 lowestNewAckInSack;
    uint32 highestNewAckInSack;
    // ====== CMT Pseudo CumAck (CUCv1) ===================================
    bool findPseudoCumAck;
    bool newPseudoCumAck;
    uint32 pseudoCumAck;
    // ====== CMT RTX Pseudo CumAck (CUCv2) ===============================
    bool findRTXPseudoCumAck;
    bool newRTXPseudoCumAck;
    uint32 rtxPseudoCumAck;
    uint32 oldestChunkTsn;
    simtime_t oldestChunkSendTime;
    simtime_t newOldestChunkSendTime;
    // ====== CMT Round-Robin among Paths =================================
    simtime_t lastTransmission;
    unsigned int sendAllRandomizer;
    // ====== CMT/RP-SCTP =================================================
    unsigned int cmtCCGroup;
    unsigned int cmtGroupPaths;
    uint32 utilizedCwnd;
    uint32 cmtGroupTotalUtilizedCwnd;
    uint32 cmtGroupTotalCwnd;
    uint32 cmtGroupTotalSsthresh;
    double cmtGroupTotalCwndBandwidth;
    double cmtGroupTotalUtilizedCwndBandwidth;
    double cmtGroupAlpha;
    // ====== CMT Sender Buffer Control ===================================
    simtime_t blockingTimeout;    // do not use path until given time
    // ====== CMT Slow Path RTT Calculation ===============================
    bool waitingForRTTCalculaton;
    simtime_t txTimeForRTTCalculation;
    uint32 tsnForRTTCalculation;
    // ====== OLIA TMP Variable ===========================================
    uint32 oliaSentBytes;
    // ====== Path Status =================================================
    simtime_t heartbeatTimeout;
    simtime_t heartbeatIntervalTimeout;
    simtime_t rtxTimeout;
    simtime_t cwndTimeout;
    simtime_t rttUpdateTime;
    simtime_t lastAckTime;
    simtime_t pathRto;
    simtime_t srtt;
    simtime_t rttvar;

    // ====== Path Statistics =============================================
    unsigned int gapAckedChunksInLastSACK;    // Per-path GapAck'ed chunks in last SACK (R+NR)
    unsigned int gapNRAckedChunksInLastSACK;    // Per-path GapAck'ed chunks in last SACK (only NR)
    unsigned int gapUnackedChunksInLastSACK;    // Per-path Not-GapAck'ed chunks in last SACK (i.e. chunks between GapAcks)
    unsigned int numberOfDuplicates;
    unsigned int numberOfFastRetransmissions;
    unsigned int numberOfTimerBasedRetransmissions;
    unsigned int numberOfHeartbeatsSent;
    unsigned int numberOfHeartbeatAcksSent;
    unsigned int numberOfHeartbeatsRcvd;
    unsigned int numberOfHeartbeatAcksRcvd;
    uint64 numberOfBytesReceived;

    // ====== Output Vectors ==============================================
    cOutVector *vectorPathFastRecoveryState;
    cOutVector *vectorPathPbAcked;
    cOutVector *vectorPathTsnFastRTX;
    cOutVector *vectorPathTsnTimerBased;
    cOutVector *vectorPathAckedTsnCumAck;
    cOutVector *vectorPathAckedTsnGapAck;
    cOutVector *vectorPathPseudoCumAck;
    cOutVector *vectorPathRTXPseudoCumAck;
    cOutVector *vectorPathBlockingTsnsMoved;
    cOutVector *vectorPathSentTsn;
    cOutVector *vectorPathReceivedTsn;
    cOutVector *vectorPathHb;
    cOutVector *vectorPathRcvdHb;
    cOutVector *vectorPathHbAck;
    cOutVector *vectorPathRcvdHbAck;
    cOutVector *statisticsPathRTO;
    cOutVector *statisticsPathRTT;
    cOutVector *statisticsPathSSthresh;
    cOutVector *statisticsPathCwnd;
    cOutVector *statisticsPathOutstandingBytes;
    cOutVector *statisticsPathQueuedSentBytes;
    cOutVector *statisticsPathSenderBlockingFraction;
    cOutVector *statisticsPathReceiverBlockingFraction;
    cOutVector *statisticsPathGapAckedChunksInLastSACK;
    cOutVector *statisticsPathGapNRAckedChunksInLastSACK;
    cOutVector *statisticsPathGapUnackedChunksInLastSACK;
    cOutVector *statisticsPathBandwidth;
};

class INET_API SctpDataVariables : public cObject
{
  public:
    SctpDataVariables();
    ~SctpDataVariables();

    void setInitialDestination(SctpPathVariables *path)
    {
        initialDestination = path;
    }

    const L3Address& getInitialDestination() const
    {
        if (initialDestination != nullptr) {
            return initialDestination->remoteAddress;
        }
        return zeroAddress;
    }

    SctpPathVariables *getInitialDestinationPath() const
    {
        return initialDestination;
    }

    void setLastDestination(SctpPathVariables *path)
    {
        lastDestination = path;
    }

    const L3Address& getLastDestination() const
    {
        if (lastDestination != nullptr) {
            return lastDestination->remoteAddress;
        }
        return zeroAddress;
    }

    SctpPathVariables *getLastDestinationPath() const
    {
        return lastDestination;
    }

    void setNextDestination(SctpPathVariables *path)
    {
        nextDestination = path;
    }

    const L3Address& getNextDestination() const
    {
        if (nextDestination != nullptr) {
            return nextDestination->remoteAddress;
        }
        return zeroAddress;
    }

    SctpPathVariables *getNextDestinationPath() const
    {
        return nextDestination;
    }

    uint16 getSid() { return sid; };

    // ====== Chunk Data Management =======================================
    cPacket *userData;
    uint32 len;    // Different from wire
    uint32 booksize;
    uint32 tsn;
    uint16 sid;
    uint16 ssn;
    uint32 ppid;
    uint32 fragments;    // Number of fragments => TSNs={tsn, ..., tsn+fragments-1}
    bool enqueuedInTransmissionQ;    // In transmissionQ? Otherwise, it is just in retransmissionQ.
    bool countsAsOutstanding;    // Is chunk outstanding?
    bool hasBeenFastRetransmitted;
    bool hasBeenAbandoned;
    bool hasBeenReneged;    // Has chunk been reneged?
    bool hasBeenAcked;    // Has chunk been SACK'ed?
    bool hasBeenCountedAsNewlyAcked;    // Chunk has been counted as newly SACK'ed
    bool bbit;
    bool ebit;
    bool ordered;
    bool ibit;

    // ====== Retransmission Management ===================================
    uint32 gapReports;
    simtime_t enqueuingTime;
    simtime_t sendTime;
    simtime_t expiryTime;
    uint32 numberOfRetransmissions;
    uint32 numberOfTransmissions;
    uint32 allowedNoRetransmissions;

    // ====== Advanced Chunk Information ==================================
    SctpPathVariables *queuedOnPath;    // The path to account this chunk in qCounters.queuedOnPath
    SctpPathVariables *ackedOnPath;    // The path this chunk has been acked on
    bool hasBeenTimerBasedRtxed;    // Has chunk been timer-based retransmitted?
    bool hasBeenMoved;    // Chunk has been moved to solve buffer blocking
    simtime_t firstSendTime;
    bool wasDropped;    // For receiving side of PKTDROP: chunk dropped
    bool wasPktDropped;    // Stays true even if the TSN has been transmitted
    bool strReset;
    uint32 prMethod;
    uint32 priority;
    bool sendForwardIfAbandoned;

  public:
    static const L3Address zeroAddress;

    // ====== Private Control Information =================================

  private:
    SctpPathVariables *initialDestination;
    SctpPathVariables *lastDestination;
    SctpPathVariables *nextDestination;
};

class INET_API SctpStateVariables : public cObject
{
  public:
    SctpStateVariables();
    ~SctpStateVariables();

  public:
    void setPrimaryPath(SctpPathVariables *path)
    {
        primaryPath = path;
    }

    const L3Address& getPrimaryPathIndex() const
    {
        if (primaryPath != nullptr) {
            return primaryPath->remoteAddress;
        }
        return SctpDataVariables::zeroAddress;
    }

    SctpPathVariables *getPrimaryPath() const
    {
        return primaryPath;
    }

    struct RequestData
    {
        uint32 sn;
        uint16 result;
        uint16 type;
        std::list<uint16> streams;
        uint32 lastTsn;
    };

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
    bool fastRecoverySupported;
    bool nagleEnabled;
    bool sackAllowed;
    bool sackAlreadySent;
    bool reactivatePrimaryPath;
    bool resetPending;
    bool resetRequested;
    bool stopReceiving;    // incoming data will be discarded
    bool stopOldData;    // data with TSN<peerTsnAfterReset will be discarded
    bool queueUpdate;
    bool firstDataSent;
    bool peerWindowFull;
    bool zeroWindow;
    bool stopSending;    // will be called when SCTP_E_SHUTDOWN arrived
    bool stopReading;    // will be called when CLOSE was called and no data will be accepted
    bool inOut;
    bool noMoreOutstanding;
    bool fragInProgress;
    bool incomingRequestSet;
    uint32 numGapReports;
    L3Address initialPrimaryPath;
    std::list<SctpPathVariables *> lastDataSourceList;    // DATA chunk sources for new SACK
    SctpPathVariables *lastDataSourcePath;
    AddressVector localAddresses;
    std::list<uint32> dupList;    // Duplicates list for incoming DATA chunks
    uint32 errorCount;    // overall error counter
    uint64 peerRwnd;
    uint64 initialPeerRwnd;
    uint64 localRwnd;
    uint32 nextTsn;    // TSN to be sent
    uint32 lastTsnAck;    // stored at the sender side; cumTSNAck announced in a SACK
    uint32 highestTsnAcked;
    uint32 lastTsnReceived;    // SACK
    uint32 lastTsn;    // my very last TSN to be sent
    uint32 ackState;    // number of packets to be acknowledged
    SctpGapList gapList;    // GapAck list for incoming DATA chunks
    uint32 packetsInTotalBurst;
    simtime_t lastTransmission;
    uint64 outstandingBytes;    // Number of bytes outstanding
    uint64 queuedSentBytes;    // Number of bytes in sender queue
    uint64 queuedDroppableBytes;    // Bytes in send queue droppable by PR-SCTP
    uint64 queuedReceivedBytes;    // Number of bytes in receiver queue
    uint32 lastStreamScheduled;
    uint32 assocPmtu;    // smallest overall path mtu
    uint32 fragPoint;    // maximum size of a fragment
    uint32 msgNum;    // indicates the sequence number of the message
    uint64 bytesRcvd;
    uint32 numRequests;
    uint32 bytesToRetransmit;
    uint32 messagesToPush;
    int32 pushMessagesLeft;
    uint32 count;
    uint8 localTieTag[32];
    uint8 peerTieTag[32];
    uint64 queuedMessages;    // Messages buffered at the sender side
    uint32 messageAcceptLimit;
    uint32 queueLimit;
    uint16 header;
    uint16 sendResponse;
    uint32 responseSn;
    uint16 numResetRequests;
    int32 probingTimeout;
    std::vector<int32> numMsgsReq;
    int32 cookieLifeTime;
    /** Counter for init and cookie retransmissions */
    int16 initRetransCounter;
    simtime_t initRexmitTimeout;
    /** pointer to the init chunk data structure (for retransmissions) */
    SctpInitChunk *initChunk;
    /** pointer to the cookie chunk data structure (for retransmissions) */
    SctpCookieEchoChunk *cookieChunk;
    /** pointer to the resetChunk (for retransmission) */
    SctpShutdownChunk *shutdownChunk;
    SctpShutdownAckChunk *shutdownAckChunk;
    uint64 sendQueueLimit;
    uint64 sendBuffer;
    bool appSendAllowed;
    simtime_t lastSendQueueAbated;
    uint32 nextRSid;
    uint32 swsLimit;
    bool lastMsgWasFragment;
    bool enableHeartbeats;
    bool sendHeartbeatsOnActivePaths;
    Ptr<SctpHeader> sctpMsg;
    uint16 chunksAdded;
    uint16 dataChunksAdded;
    uint32 packetBytes;
    uint16 numAddedOutStreams;
    uint16 numAddedInStreams;
    bool authAdded;

    // ====== NR-SACK =====================================================
    bool nrSack;
    uint32 gapReportLimit;
    enum GLOVariant {
        GLOV_None = 0,
        GLOV_Optimized1 = 1,
        GLOV_Optimized2 = 2,
        GLOV_Shrunken = 3
    };
    uint32 gapListOptimizationVariant;
    bool smartOverfullSACKHandling;
    bool disableReneging;

    // ====== Retransmission Method =======================================
    uint32 rtxMethod;
    // ====== Max Burst ===================================================
    uint32 maxBurst;
    enum MBVariant {
        MBV_UseItOrLoseIt = 0,
        MBV_CongestionWindowLimiting = 1,
        MBV_UseItOrLoseItTempCwnd = 2,
        MBV_CongestionWindowLimitingTempCwnd = 3,
        MBV_MaxBurst = 4,
        MBV_AggressiveMaxBurst = 5,
        MBV_TotalMaxBurst = 6
    };
    MBVariant maxBurstVariant;
    uint32 initialWindow;
    // ====== CMT-SCTP ====================================================
    bool allowCMT;
    bool (*cmtSendAllComparisonFunction)(const SctpPathVariables *left, const SctpPathVariables *right);
    const char *cmtRetransmissionVariant;

    enum CUCVariant {
        CUCV_Normal = 0,
        CUCV_PseudoCumAck = 1,
        CUCV_PseudoCumAckV2 = 2
    };
    CUCVariant cmtCUCVariant;    // Cwnd Update for CMT (CUC)

    enum BufferSplitVariant {
        CBSV_None = 0,
        CBSV_SenderOnly = 1,
        CBSV_ReceiverOnly = 2,
        CBSV_BothSides = 3
    };
    BufferSplitVariant cmtBufferSplitVariant;    // Buffer Splitting for CMT
    bool cmtBufferSplittingUsesOSB;    // Use outstanding instead of queued bytes for Buffer Splitting

    enum ChunkReschedulingVariant {
        CCRV_None = 0,
        CCRV_SenderOnly = 1,
        CCRV_ReceiverOnly = 2,
        CCRV_BothSides = 3,
        CCRV_Test = 99    // Test only!
    };
    ChunkReschedulingVariant cmtChunkReschedulingVariant;    // Chunk Rescheduling
    double cmtChunkReschedulingThreshold;    // Blocking Threshold for Chunk Rescheduling

    bool cmtSmartT3Reset;    // Smart T3 Reset for CMT
    bool cmtSmartFastRTX;    // Smart Fast RTX for CMT
    bool cmtSmartReneging;    // Smart Reneging for CMT
    bool cmtSlowPathRTTUpdate;    // Slow Path RTT Update for CMT
    bool cmtUseSFR;    // Split Fast Retransmission (SFR) for CMT
    bool cmtUseDAC;    // Delayed Ack for CMT (DAC)
    bool cmtUseFRC;    // Fast Recovery for CMT (FRC)
    bool cmtMovedChunksReduceCwnd;    // Subtract moved chunk from cwnd of old path
    double movedChunkFastRTXFactor;
    unsigned int blockingTsnsMoved;
    bool strictCwndBooking;    // Strict overbooking handling
    enum CSackPath {
        CSP_Standard = 0,
        CSP_Primary = 1,
        CSP_RoundRobin = 2,
        CSP_SmallestSRTT = 3
    };
    CSackPath cmtSackPath;    // SACK path selection variant for CMT
    // ====== High-Speed SCTP =============================================
    bool highSpeedCC;    // HighSpeed CC (RFC 3649)

    // ====== CMT/RP-SCTP =================================================
    enum CCCVariant {
        CCCV_Off = 0,    // Standard SCTP
        CCCV_CMT = 1,    // CMT-SCTP
        CCCV_CMTRPv1 = 2,    // CMT/RPv1-SCTP (old CMT/RP)
        CCCV_CMTRPv2 = 3,    // CMT/RPv2-SCTP (new CMT/RP with bandwidth consideration)
        CCCV_CMT_LIA = 4,    // LIA CC
        CCCV_CMT_OLIA = 5,    // OLIA CC
        CCCV_CMTRP_Test1 = 100,
        CCCV_CMTRP_Test2 = 101
    };
    CCCVariant cmtCCVariant;
    bool rpPathBlocking;    // T.D. 10.08.2011: CMT/RP path blocking
    bool rpScaleBlockingTimeout;    // T.D. 15.08.2011: Scale blocking timeout by number of paths
    uint32 rpMinCwnd;    // T.D. 15.08.2011: Minimum cwnd in MTUs

    // ====== SACK Sequence Number Checker ================================
    bool checkSackSeqNumber;    // Ensure handling SACKs in original sequence
    uint32 outgoingSackSeqNum;
    uint32 incomingSackSeqNum;

    // ====== Partial Reliability SCTP ====================================
    uint32 asconfSn;    // own AddIP serial number
    uint16 numberAsconfReceived;
    uint32 corrIdNum;
    bool asconfOutstanding;
    SctpAsconfChunk *asconfChunk;

    // ====== Stream Reset ================================================
    bool streamReset;
    bool peerStreamReset;
    bool resetDeferred;
    bool bundleReset;
    bool waitForResponse;
    bool firstPeerRequest;
    bool appLimited;
    bool requestsOverlap;
    uint32 streamResetSequenceNumber;
    uint32 expectedStreamResetSequenceNumber;
    uint32 peerRequestSn;
    uint32 inRequestSn;
    uint32 peerTsnAfterReset;
    uint32 lastTsnBeforeReset;    // lastTsn announced in OutgoingStreamResetParameter
    SctpStreamResetChunk *resetChunk; // pointer to the resetChunk (for retransmission)
    SctpParameter *incomingRequest;
    std::list<uint16> resetOutStreams;
    std::list<uint16> resetInStreams;
    std::list<uint16> streamsPending;
    std::list<uint16> streamsToReset;
    std::list<uint16> peerStreamsToReset;
    std::map<uint32, RequestData> requests;
    std::map<uint32, RequestData> peerRequests;
    SctpResetReq *resetInfo;
    uint16 peerRequestType;
    uint16 localRequestType;

    bool findRequestNum(uint32 num);
    bool findPeerRequestNum(uint32 num);
    bool findPeerStreamToReset(uint16 sn);
    bool findMatch(uint16 sn);
    RequestData* findTypeInRequests(uint16 type);
    uint16 getNumRequestsNotPerformed();

    // ====== SCTP Authentication =========================================
    uint16 hmacType;
    bool peerAuth;
    bool auth;
    std::vector<uint16> chunkList;
    std::vector<uint16> peerChunkList;
    uint8 keyVector[512];
    uint32 sizeKeyVector;
    uint8 peerKeyVector[512];
    uint32 sizePeerKeyVector;
    uint8 sharedKey[512];

    // ====== Further features ============================================
    bool osbWithHeader;
    bool padding;
    bool pktDropSent;
    bool peerPktDrop;
    uint32 advancedPeerAckPoint;
    uint32 prMethod;
    bool peerAllowsChunks;    // Flowcontrol: indicates whether the peer adjusts the window according to a number of messages
    uint32 initialPeerMsgRwnd;
    uint32 localMsgRwnd;
    uint32 peerMsgRwnd;    // Flowcontrol: corresponds to peerRwnd
    uint32 bufferedMessages;    // Messages buffered at the receiver side
    uint32 outstandingMessages;    // Outstanding messages on the sender side; used for flowControl; including retransmitted messages
    uint32 bytesToAddPerRcvdChunk;
    uint32 bytesToAddPerPeerChunk;
    bool tellArwnd;
    bool swsMsgInvoked;    // Flowcontrol: corresponds to swsAvoidanceInvoked
    simtime_t lastThroughputTime;
    std::map<uint16, uint32> streamThroughput;
    simtime_t lastAssocThroughputTime;
    uint32 assocThroughput;
    double throughputInterval;
    bool ssNextStream;
    bool ssLastDataChunkSizeSet;
    bool ssOneStreamLeft;
    std::map<uint16, uint32> ssPriorityMap;
    std::map<uint16, int32> ssFairBandwidthMap;
    std::map<uint16, int32> ssStreamToPathMap;

  private:
    SctpPathVariables *primaryPath;
};

class INET_API SctpAssociation : public cObject
{
    friend class Sctp;
    friend class SctpPathVariables;

    // map for storing the path parameters
    typedef std::map<L3Address, SctpPathVariables *> SctpPathMap;
    // map for storing the queued bytes per path
    typedef std::map<L3Address, uint32> CounterMap;

    struct QueueCounter
    {
        uint64 roomSumSendStreams;
        uint64 bookedSumSendStreams;
        uint64 roomSumRcvStreams;
        CounterMap roomTransQ;
        CounterMap bookedTransQ;
        CounterMap roomRetransQ;
    };

    struct BytesToBeSent
    {
        bool chunk;
        bool packet;
        uint32 bytesToSend;
    };

    struct CCFunctions
    {
        void (SctpAssociation::*ccInitParams)(SctpPathVariables *path);
        void (SctpAssociation::*ccUpdateBeforeSack)();
        void (SctpAssociation::*ccUpdateAfterSack)();
        void (SctpAssociation::*ccUpdateAfterCwndTimeout)(SctpPathVariables *path);
        void (SctpAssociation::*ccUpdateAfterRtxTimeout)(SctpPathVariables *path);
        void (SctpAssociation::*ccUpdateMaxBurst)(SctpPathVariables *path);
        void (SctpAssociation::*ccUpdateBytesAcked)(SctpPathVariables *path, const uint32 ackedBytes, const bool ctsnaAdvanced);
    };

    typedef std::map<uint32, SctpSendStream *> SctpSendStreamMap;
    typedef std::map<uint32, SctpReceiveStream *> SctpReceiveStreamMap;
    typedef std::map<uint32,SctpPathVariables*> SctpPathCollection;
    SctpPathCollection assocBestPaths;
    SctpPathCollection assocMaxWndPaths;
    SctpPathCollection assocCollectedPaths;

  public:
    // connection identification by apps: appgateIndex+assocId
    int32 appGateIndex;    // Application gate index
    int32 assocId;    // Identifies connection within the app
    int32 listeningAssocId;
    int32 fd;
    bool listening;
    L3Address remoteAddr;    // Remote address from last message
    L3Address localAddr;    // Local address from last message
    uint16 localPort;    // Remote port from last message
    uint16 remotePort;    // Local port from last message
    uint32 localVTag;    // Local verification tag
    uint32 peerVTag;    // Remote verification tag

    // Timers
    cMessage *T1_InitTimer;
    cMessage *T2_ShutdownTimer;
    cMessage *T5_ShutdownGuardTimer;
    cMessage *SackTimer;
    cMessage *StartTesting;
    cMessage *StartAddIP;
    cOutVector *advMsgRwnd;
    cOutVector *EndToEndDelay;
    bool fairTimer;
    std::map<uint16, cOutVector *> streamThroughputVectors;
    cOutVector *assocThroughputVector;
    cMessage *FairStartTimer;
    cMessage *FairStopTimer;
    // ------ CMT Delayed Ack (DAC) ---------------------
    uint8_t dacPacketsRcvd;

  protected:
    IRoutingTable *rt;
    IInterfaceTable *ift;

    AddressVector localAddressList;
    AddressVector remoteAddressList;
    uint32 numberOfRemoteAddresses;
    uint32 inboundStreams;
    uint32 outboundStreams;
    uint32 initInboundStreams;

    int32 status;
    uint32 initTsn;
    uint32 initPeerTsn;
    uint32 sackFrequency;
    double sackPeriod;
    CCFunctions ccFunctions;
    uint16 ccModule;

    cOutVector *advRwnd;
    cOutVector *cumTsnAck;
    cOutVector *sendQueue;
    cOutVector *numGapBlocks;

    // Variables associated with the state of this association
    SctpStateVariables *state;
    BytesToBeSent bytes;
    Sctp *sctpMain;    // SCTP module
    cFSM *fsm;    // SCTP state machine
    SctpPathMap sctpPathMap;
    QueueCounter qCounter;
    SctpQueue *transmissionQ;
    SctpQueue *retransmissionQ;
    SctpSendStreamMap sendStreams;
    SctpReceiveStreamMap receiveStreams;
    SctpAlgorithm *sctpAlgorithm;

    // ------ Transmission Statistics -------------------------------------
    cOutVector *statisticsOutstandingBytes;
    cOutVector *statisticsQueuedReceivedBytes;
    cOutVector *statisticsQueuedSentBytes;
    cOutVector *statisticsTotalSSthresh;
    cOutVector *statisticsTotalCwnd;
    cOutVector *statisticsTotalBandwidth;
    // ------ Received SACK Statistics ------------------------------------
    cOutVector *statisticsRevokableGapBlocksInLastSACK;    // Revokable GapAck blocks in last received SACK
    cOutVector *statisticsNonRevokableGapBlocksInLastSACK;    // Non-Revokable GapAck blocks in last received SACK
    cOutVector *statisticsArwndInLastSACK;
    cOutVector *statisticsPeerRwnd;
    // ------ Sent SACK Statistics ----------------------------------------
    cOutVector *statisticsNumTotalGapBlocksStored;    // Number of GapAck blocks stored (NOTE: R + NR!)
    cOutVector *statisticsNumRevokableGapBlocksStored;    // Number of Revokable GapAck blocks stored
    cOutVector *statisticsNumNonRevokableGapBlocksStored;    // Number of Non-Revokable GapAck blocks stored
    cOutVector *statisticsNumDuplicatesStored;    // Number of duplicate TSNs stored
    cOutVector *statisticsNumRevokableGapBlocksSent;    // Number of Revokable GapAck blocks sent in last SACK
    cOutVector *statisticsNumNonRevokableGapBlocksSent;    // Number of Non-Revokable GapAck blocks sent in last SACK
    cOutVector *statisticsNumDuplicatesSent;    // Number of duplicate TSNs sent in last SACK
    cOutVector *statisticsSACKLengthSent;    // Length of last sent SACK

  public:
    /**
     * Constructor.
     */
    SctpAssociation(Sctp *mod, int32 appGateIndex, int32 assocId, IRoutingTable *rt, IInterfaceTable *ift);
    /**
     * Destructor.
     */
    ~SctpAssociation();
    /**
     * Utility: Send data from sendQueue.
     */
    void sendOnPath(SctpPathVariables *pathId, const bool firstPass = true);
    void sendOnAllPaths(SctpPathVariables *firstPath);

    /** Utility: returns name of SCTP_I_xxx constants */
    static const char *indicationName(int32 code);

    /** Utility: return IPv4 or IPv6 address level */
    static int getAddressLevel(const L3Address& addr);

    /* @name Various getters */
    //@{
    int32 getFsmState() const { return fsm->getState(); };
    SctpStateVariables *getState() const { return state; };
    SctpQueue *getTransmissionQueue() const { return transmissionQ; };
    SctpQueue *getRetransmissionQueue() const { return retransmissionQ; };
    SctpAlgorithm *getSctpAlgorithm() const { return sctpAlgorithm; };
    Sctp *getSctpMain() const { return sctpMain; };
    cFSM *getFsm() const { return fsm; };
    cMessage *getInitTimer() const { return T1_InitTimer; };
    cMessage *getShutdownTimer() const { return T2_ShutdownTimer; };
    cMessage *getSackTimer() const { return SackTimer; };

    /** Utility: returns name of SCTP_S_xxx constants */
    static const char *stateName(int32 state);

    static uint16 chunkToInt(const char *type);

    /* Process self-messages (timers).
     * Normally returns true. A return value of false means that the
     * connection structure must be deleted by the caller (SctpMain).
     */
    bool processTimer(cMessage *msg);
    /**
     * Process incoming SCTP segment. Normally returns true. A return value
     * of false means that the connection structure must be deleted by the
     * caller (SCTP).
     */
   // bool processSctpMessage(const Ptr<const SctpHeader>& sctpmsg, const L3Address& srcAddr, const L3Address& destAddr);
   bool processSctpMessage(SctpHeader *sctpmsg, const L3Address& srcAddr, const L3Address& destAddr);
    /**
     * Process commands from the application.
     * Normally returns true. A return value of false means that the
     * connection structure must be deleted by the caller (SCTP).
     */
    bool processAppCommand(cMessage *msg, SctpCommandReq *sctpCommand);
    void removePath();
    void removePath(const L3Address& addr);
    void deleteStreams();
    void stopTimer(cMessage *timer);
    void stopTimers();
    SctpPathVariables *getPath(const L3Address& pathId) const
    {
        SctpPathMap::const_iterator iterator = sctpPathMap.find(pathId);
        if (iterator != sctpPathMap.end()) {
            return iterator->second;
        }
        return nullptr;
    }

    void printSctpPathMap() const;

    /**
     * Compare TSNs
     */
    /* Defines see RFC1982 for details */
    static bool SCTP_UINT16_GT(uint16 a, uint16 b) { return (((a < b) && ((uint16)(b - a) > (1U<<15))) || \
                                  ((a > b) && ((uint16)(a - b) < (1U<<15)))); }
    static bool SCTP_UINT16_GE(uint16 a, uint16 b) { return (a == b ) || SCTP_UINT16_GT(a, b); }
    static bool SCTP_UINT32_GT(uint32 a, uint32 b) { return ((a < b) && ((uint32)(b - a) > (1UL<<31))) ||
                                  ((a > b) && ((uint32)(a - b) < (1UL<<31))); }
    static bool SCTP_UINT32_GE(uint32 a, uint32 b) { return SCTP_UINT32_GT(a, b) || (a == b); }
    #define SCTP_TSN_GT(a, b) SCTP_UINT32_GT(a, b)
    #define SCTP_TSN_GE(a, b) SCTP_UINT32_GE(a, b)
    #define SCTP_SSN_GT(a, b) SCTP_UINT16_GT(a, b)
    #define SCTP_SSN_GE(a, b) SCTP_UINT16_GE(a, b)
    static bool tsnGe(const uint32 tsn1, const uint32 tsn2) { return SCTP_TSN_GE(tsn1, tsn2); }
    static bool tsnGt(const uint32 tsn1, const uint32 tsn2) { return SCTP_TSN_GT(tsn1, tsn2); }
    static bool tsnLe(const uint32 tsn1, const uint32 tsn2) { return SCTP_TSN_GE(tsn2, tsn1); }
    static bool tsnLt(const uint32 tsn1, const uint32 tsn2) { return SCTP_TSN_GT(tsn2, tsn1); }
    static bool tsnBetween(const uint32 tsn1, const uint32 midtsn, const uint32 tsn2) { return (SCTP_TSN_GE(midtsn, tsn1) && SCTP_TSN_GE(tsn2, midtsn)); }
    static bool ssnGt(const uint16 ssn1, const uint16 ssn2) { return SCTP_SSN_GT(ssn1, ssn2); }
    static bool midGt(const uint32 mid1, const uint32 mid2) { return SCTP_TSN_GT(mid1, mid2); }

  protected:
    /** @name FSM transitions: analysing events and executing state transitions */
    //@{
    /** Maps app command codes (msg kind of app command msgs) to SCTP_E_xxx event codes */
    SctpEventCode preanalyseAppCommandEvent(int32 commandCode);
    /** Implemements the pure SCTP state machine */
    bool performStateTransition(const SctpEventCode& event);
    void stateEntered(int32 state);
    //@}
    /** @name Processing app commands. Invoked from processAppCommand(). */
    //@{
    void process_ASSOCIATE(SctpEventCode& event, SctpCommandReq *sctpCommand, cMessage *msg);
    void process_OPEN_PASSIVE(SctpEventCode& event, SctpCommandReq *sctpCommand, cMessage *msg);
    void process_SEND(SctpEventCode& event, SctpCommandReq *sctpCommand, cMessage *msg);
    void process_CLOSE(SctpEventCode& event);
    void process_ABORT(SctpEventCode& event);
    void process_STATUS(SctpEventCode& event, SctpCommandReq *sctpCommand, cMessage *msg);
    void process_RECEIVE_REQUEST(SctpEventCode& event, SctpCommandReq *sctpCommand);
    void process_PRIMARY(SctpEventCode& event, SctpCommandReq *sctpCommand);
    void process_STREAM_RESET(SctpCommandReq *sctpCommand);
    //@}

    /** @name Processing Sctp message arrivals. Invoked from processSctpHeader(). */
    //@{
   // bool process_RCV_Message(const Ptr<const SctpHeader>& sctpmsg, const L3Address& src, const L3Address& dest);
   bool process_RCV_Message(SctpHeader *sctpmsg, const L3Address& src, const L3Address& dest);
    /**
     * Process incoming SCTP packets. Invoked from process_RCV_Message
     */
    bool processInitArrived(SctpInitChunk *initChunk, int32 sport, int32 dport);
    bool processInitAckArrived(SctpInitAckChunk *initAckChunk);
    bool processCookieEchoArrived(SctpCookieEchoChunk *cookieEcho, L3Address addr);
    bool processCookieAckArrived();
    SctpEventCode processDataArrived(SctpDataChunk *dataChunk);
    SctpEventCode processSackArrived(SctpSackChunk *sackChunk);
    SctpEventCode processHeartbeatAckArrived(SctpHeartbeatAckChunk *heartbeatack, SctpPathVariables *path);
    SctpEventCode processForwardTsnArrived(SctpForwardTsnChunk *forChunk);
    bool processPacketDropArrived(SctpPacketDropChunk *pktdrop);
    void processErrorArrived(SctpErrorChunk *error);
    //@}

    /** @name Processing timeouts. Invoked from processTimer(). */
    //@{
    void process_TIMEOUT_RTX(SctpPathVariables *path);
    void process_TIMEOUT_BLOCKING(SctpPathVariables *path);
    void process_TIMEOUT_HEARTBEAT(SctpPathVariables *path);
    void process_TIMEOUT_HEARTBEAT_INTERVAL(SctpPathVariables *path, bool force);
    void process_TIMEOUT_INIT_REXMIT(SctpEventCode& event);
    void process_TIMEOUT_PROBING();
    void process_TIMEOUT_SHUTDOWN(SctpEventCode& event);
    int32 updateCounters(SctpPathVariables *path);
    void process_TIMEOUT_RESET(SctpPathVariables *path);
    void process_TIMEOUT_ASCONF(SctpPathVariables *path);
    //@}

    void startTimer(cMessage *timer, const simtime_t& timeout);

    /** Utility: clone a listening association. Used for forking. */
    SctpAssociation *cloneAssociation();

    /** Utility: creates send/receive queues and sctpAlgorithm */
    void initAssociation(SctpOpenReq *openCmd);

    /** Methods dealing with the handling of TSNs  **/
    bool tsnIsDuplicate(const uint32 tsn) const;
    bool makeRoomForTsn(const uint32 tsn, const uint32 length, const bool uBit);

    /** Methods for creating and sending chunks */
    void sendInit();
    void sendInitAck(SctpInitChunk *initchunk);
    void sendCookieEcho(SctpInitAckChunk *initackchunk);
    void sendCookieAck(const L3Address& dest);
    void sendAbort(uint16 tBit = 0);
    void sendHeartbeat(const SctpPathVariables *path);
    void sendHeartbeatAck(const SctpHeartbeatChunk *heartbeatChunk,
            const L3Address& src,
            const L3Address& dest);
    void sendSack();
    void sendShutdown();
    void sendShutdownAck(const L3Address& dest);
    void sendShutdownComplete();
    SctpSackChunk *createSack();
    /** Retransmitting chunks */
    void retransmitInit();
    void retransmitCookieEcho();
    void retransmitReset();
    void retransmitShutdown();
    void retransmitShutdownAck();

    /** Utility: adds control info to message and sends it to IP */
    void sendToIP(Packet *pkt, const Ptr<SctpHeader>& sctpmsg, L3Address dest);

    void sendToIP(Packet *pkt, const Ptr<SctpHeader>& sctpmsg)
    {
        sendToIP(pkt, sctpmsg, remoteAddr);
    }

    void scheduleSack();
    /** Utility: signal to user that connection timed out */
    void signalConnectionTimeout();

    /** Utility: start a timer */
    void scheduleTimeout(cMessage *msg, const simtime_t& timeout)
    {
        sctpMain->scheduleAt(simTime() + timeout, msg);
    }

    /** Utility: cancel a timer */
    cMessage *cancelEvent(cMessage *msg)
    {
        return sctpMain->cancelEvent(msg);
    }

    /** Utility: sends packet to application */
    void sendToApp(cMessage *msg);

    /** Utility: sends status indication (SCTP_I_xxx) to application */
    void sendIndicationToApp(int32 code, int32 value = 0);

    /** Utility: sends SCTP_I_ESTABLISHED indication with SctpConnectInfo to application */
    void sendEstabIndicationToApp();
    void sendAvailableIndicationToApp();
    bool isToBeAccepted() const { return listeningAssocId != -1; }
    void pushUlp();
    void sendDataArrivedNotification(uint16 sid);
    void putInDeliveryQ(uint16 sid);
    /** Utility: prints local/remote addr/port and app gate index/assocId */
    void printAssocBrief();
    /** Utility: prints important header fields */
    static void printSegmentBrief(SctpHeader *sctpmsg);

    /** Utility: returns name of SCTP_E_xxx constants */
    static const char *eventName(int32 event);

    void addPath(const L3Address& addr);
    SctpPathVariables *getNextPath(const SctpPathVariables *oldPath) const;

    const L3Address& getNextAddress(const SctpPathVariables *oldPath) const
    {
        const SctpPathVariables *nextPath = getNextPath(oldPath);
        if (nextPath != nullptr) {
            return nextPath->remoteAddress;
        }
        return SctpDataVariables::zeroAddress;
    }

    SctpPathVariables *getNextDestination(SctpDataVariables *chunk) const;

    void bytesAllowedToSend(SctpPathVariables *path, bool firstPass);

    void pathStatusIndication(const SctpPathVariables *path, bool status);

    bool allPathsInactive() const;

    void sendStreamResetRequest(SctpResetReq *info);
    void sendStreamResetResponse(uint32 srrsn, int result);
    void sendStreamResetResponse(SctpSsnTsnResetRequestParameter *requestParam, int result,
            bool options);
    void sendOutgoingResetRequest(SctpIncomingSsnResetRequestParameter *requestParam);
    void sendAddOutgoingStreamsRequest(uint16 numStreams);
    void sendBundledOutgoingResetAndResponse(SctpIncomingSsnResetRequestParameter *requestParam);
    void sendAddInAndOutStreamsRequest(SctpResetReq *info);
    void sendDoubleStreamResetResponse(uint32 insrrsn, uint16 inresult, uint32 outsrrsn, uint16 outresult);
    void checkStreamsToReset();
    bool streamIsPending(int32 sid);
    void sendPacketDrop(const bool flag);
    void sendHMacError(const uint16 id);
    void sendInvalidStreamError(uint16 sid);
    void resetGapLists();

    SctpForwardTsnChunk *createForwardTsnChunk(const L3Address& pid);

    bool msgMustBeAbandoned(SctpDataMsg *msg, int32 stream, bool ordered);    //PR-SCTP
    bool chunkMustBeAbandoned(SctpDataVariables *chunk,
            SctpPathVariables *sackPath);
    void advancePeerTsn();

    void cucProcessGapReports(const SctpDataVariables *chunk,
            SctpPathVariables *path,
            const bool isAcked);    // CMT-SCTP
    /**
     * Manipulating chunks
     */
    SctpDataChunk *transformDataChunk(SctpDataVariables *chunk);
    SctpDataVariables *makeVarFromMsg(SctpDataChunk *datachunk);

    /**
     * Dealing with streams
     */

    int32 streamScheduler(SctpPathVariables *path, bool peek);
    void initStreams(uint32 inStreams, uint32 outStreams);
    void addInStreams(uint32 inStreams);
    void addOutStreams(uint32 outStreams);
    int32 numUsableStreams();
    int32 streamSchedulerRoundRobinPacket(SctpPathVariables *path, bool peek);
    int32 streamSchedulerRandom(SctpPathVariables *path, bool peek);
    int32 streamSchedulerRandomPacket(SctpPathVariables *path, bool peek);
    int32 streamSchedulerFairBandwidth(SctpPathVariables *path, bool peek);
    int32 streamSchedulerFairBandwidthPacket(SctpPathVariables *path, bool peek);
    int32 streamSchedulerPriority(SctpPathVariables *path, bool peek);
    int32 streamSchedulerFCFS(SctpPathVariables *path, bool peek);
    int32 pathStreamSchedulerManual(SctpPathVariables *path, bool peek);
    int32 pathStreamSchedulerMapToPath(SctpPathVariables *path, bool peek);

    struct SSFunctions
    {
        void (SctpAssociation::*ssInitStreams)(uint32 inStreams, uint32 outStreams);
        void (SctpAssociation::*ssAddInStreams)(uint32 inStreams);
        void (SctpAssociation::*ssAddOutStreams)(uint32 outStreams);
        int32 (SctpAssociation::*ssGetNextSid)(SctpPathVariables *path, bool peek);
        int32 (SctpAssociation::*ssUsableStreams)();
    };
    SSFunctions ssFunctions;
    uint16 ssModule;

    /**
     *    Queue Management
     */
    void process_QUEUE_MSGS_LIMIT(const SctpCommandReq *sctpCommand);
    void process_QUEUE_BYTES_LIMIT(const SctpCommandReq *sctpCommand);
    int32 getOutstandingBytes() const;
    void dequeueAckedChunks(const uint32 tsna,
            SctpPathVariables *path,
            simtime_t& rttEstimation);
    SctpDataMsg *peekOutboundDataMsg();
    SctpDataVariables *peekAbandonedChunk(const SctpPathVariables *path);
    SctpDataVariables *getOutboundDataChunk(const SctpPathVariables *path,
            int32 availableSpace,
            int32 availableCwnd);
    void fragmentOutboundDataMsgs();
    SctpDataMsg *dequeueOutboundDataMsg(SctpPathVariables *path,
            int32 availableSpace,
            int32 availableCwnd);
    bool nextChunkFitsIntoPacket(SctpPathVariables *path, int32 bytes);
    void putInTransmissionQ(uint32 tsn, SctpDataVariables *chunk);

    uint32 getAllTransQ();

    /**
     * Flow control
     */
    void pmStartPathManagement();
    void pmDataIsSentOn(SctpPathVariables *path);
    void pmClearPathCounter(SctpPathVariables *path);
    void pmRttMeasurement(SctpPathVariables *path,
            const simtime_t& rttEstimation);

    void disposeOf(SctpHeader *sctpmsg);
    void removeFirstChunk(SctpHeader *sctpmsg);

    /** Methods for Stream Reset **/
    void resetSsns();
    void resetExpectedSsns();
    bool sendStreamPresent(uint16 sid);
    bool receiveStreamPresent(uint16 sid);
    void resetSsn(uint16 id);
    void resetExpectedSsn(uint16 id);
    uint32 getExpectedSsnOfStream(uint16 id);
    uint32 getSsnOfStream(uint16 id);
    SctpParameter *makeOutgoingStreamResetParameter(uint32 srsn, SctpResetReq *info);
    SctpParameter *makeIncomingStreamResetParameter(uint32 srsn, SctpResetReq *info);
    SctpParameter *makeSsnTsnResetParameter(uint32 srsn);
    SctpParameter *makeAddStreamsRequestParameter(uint32 srsn, SctpResetReq *info);
    void sendOutgoingRequestAndResponse(uint32 inRequestSn, uint32 outRequestSn);
    void sendOutgoingRequestAndResponse(SctpIncomingSsnResetRequestParameter *inRequestParam, SctpOutgoingSsnResetRequestParameter *outRequestParam);
    SctpEventCode processInAndOutResetRequestArrived(SctpIncomingSsnResetRequestParameter *inRequestParam, SctpOutgoingSsnResetRequestParameter *outRequestParam);
    SctpEventCode processOutAndResponseArrived(SctpOutgoingSsnResetRequestParameter *outRequestParam, SctpStreamResetResponseParameter *responseParam);
    SctpEventCode processStreamResetArrived(SctpStreamResetChunk *strResChunk);
    void processOutgoingResetRequestArrived(SctpOutgoingSsnResetRequestParameter *requestParam);
    void processIncomingResetRequestArrived(SctpIncomingSsnResetRequestParameter *requestParam);
    void processSsnTsnResetRequestArrived(SctpSsnTsnResetRequestParameter *requestParam);
    void processResetResponseArrived(SctpStreamResetResponseParameter *responseParam);
    void processAddInAndOutResetRequestArrived(const SctpAddStreamsRequestParameter *addInRequestParam, SctpAddStreamsRequestParameter *addOutRequestParam);
    uint32 getBytesInFlightOfStream(uint16 sid);
    bool getFragInProgressOfStream(uint16 sid);
    void setFragInProgressOfStream(uint16 sid, bool frag);
    bool orderedQueueEmptyOfStream(uint16 sid);
    bool unorderedQueueEmptyOfStream(uint16 sid);

    /** Methods for Add-IP and AUTH **/
    void sendAsconf(const char *type, bool remote = false);
    void sendAsconfAck(uint32 serialNumber);
    SctpEventCode processAsconfArrived(SctpAsconfChunk *asconfChunk);
    SctpEventCode processAsconfAckArrived(SctpAsconfAckChunk *asconfAckChunk);
    void retransmitAsconf();
    bool typeInChunkList(uint16 type);
    bool typeInOwnChunkList(uint16 type);
    SctpAsconfAckChunk *createAsconfAckChunk(uint32 serialNumber);
    SctpAuthenticationChunk *createAuthChunk();
    SctpSuccessIndication *createSuccessIndication(uint32 correlationId);
    void calculateAssocSharedKey();
    bool compareRandom();

    void calculateRcvBuffer();
    void listOrderedQ();
    void tsnWasReneged(SctpDataVariables *chunk,
            const SctpPathVariables *sackPath,
            const int type);
    void printOutstandingTsns();

    /** SctpCcFunctions **/
    void initCcParameters(SctpPathVariables *path);
    void updateFastRecoveryStatus(uint32 lastTsnAck);
    void cwndUpdateBeforeSack();
    void cwndUpdateAfterSack();
    void cwndUpdateAfterCwndTimeout(SctpPathVariables *path);
    void cwndUpdateAfterRtxTimeout(SctpPathVariables *path);
    void cwndUpdateMaxBurst(SctpPathVariables *path);
    void cwndUpdateBytesAcked(SctpPathVariables *path,
            uint32 ackedBytes,
            bool ctsnaAdvanced);
    int32 rpPathBlockingControl(SctpPathVariables *path, double reduction);

  private:
    SctpDataVariables *makeDataVarFromDataMsg(SctpDataMsg *datMsg,
            SctpPathVariables *path);
    SctpPathVariables *choosePathForRetransmission();
    void timeForSack(bool& sackOnly, bool& sackWithData);
    void recordCwndUpdate(SctpPathVariables *path);
    void sendSACKviaSelectedPath(const Ptr<SctpHeader>& sctpMsg);
    void checkOutstandingBytes();
    void updateHighSpeedCCThresholdIdx(SctpPathVariables *path);
    uint32 getInitialCwnd(const SctpPathVariables *path) const;
    void generateSendQueueAbatedIndication(uint64 bytes);
    void renegablyAckChunk(SctpDataVariables *chunk,
            SctpPathVariables *sackPath);
    void nonRenegablyAckChunk(SctpDataVariables *chunk,
            SctpPathVariables *sackPath,
            simtime_t& rttEstimation,
            Sctp::AssocStat *assocStat);
    void handleChunkReportedAsAcked(uint32& highestNewAck,
            simtime_t& rttEstimation,
            SctpDataVariables *myChunk,
            SctpPathVariables *sackPath,
            const bool sackIsNonRevokable);
    void handleChunkReportedAsMissing(const SctpSackChunk *sackChunk,
            const uint32 highestNewAck,
            SctpDataVariables *myChunk,
            SctpPathVariables *sackPath);
    void moveChunkToOtherPath(SctpDataVariables *chunk,
            SctpPathVariables *newPath);
    void decreaseOutstandingBytes(SctpDataVariables *chunk);
    void increaseOutstandingBytes(SctpDataVariables *chunk,
            SctpPathVariables *path);
    int32 calculateBytesToSendOnPath(const SctpPathVariables *pathVar);
    void storePacket(SctpPathVariables *pathVar,
            const Ptr<SctpHeader>& sctpMsg,
            uint16 chunksAdded,
            uint16 dataChunksAdded,
            bool authAdded);
    void loadPacket(SctpPathVariables *pathVar,
            Ptr<SctpHeader> *sctpMsg,
            uint16 *chunksAdded,
            uint16 *dataChunksAdded,
            bool *authAdded);

    void ackChunk(SctpDataVariables *chunk, SctpPathVariables *sackPath)
    {
        chunk->hasBeenAcked = true;
        chunk->ackedOnPath = sackPath;
    }

    void unackChunk(SctpDataVariables *chunk)
    {
        chunk->hasBeenAcked = false;
    }

    bool chunkHasBeenAcked(const SctpDataVariables *chunk) const
    {
        return chunk->hasBeenAcked;
    }

    bool chunkHasBeenAcked(uint32 tsn) const
    {
        const SctpDataVariables *chunk = retransmissionQ->getChunk(tsn);
        if (chunk) {
            return chunkHasBeenAcked(chunk);
        }
        return false;
    }

    void checkPseudoCumAck(const SctpPathVariables *path);
    static bool pathMapLargestSSThreshold(const SctpPathVariables *left, const SctpPathVariables *right);
    static bool pathMapLargestSpace(const SctpPathVariables *left, const SctpPathVariables *right);
    static bool pathMapLargestSpaceAndSSThreshold(const SctpPathVariables *left, const SctpPathVariables *right);
    static bool pathMapSmallestLastTransmission(const SctpPathVariables *left, const SctpPathVariables *right);
    static bool pathMapRandomized(const SctpPathVariables *left, const SctpPathVariables *right);
    std::vector<SctpPathVariables *> getSortedPathMap();
    void chunkReschedulingControl(SctpPathVariables *path);
    void recalculateOLIABasis();

    /**
     * w: cwnd of the path
     * s: ssthresh of the path
     * totalW: Sum of all cwnds of the association
     * a: factor alpha of olia calculation - see https://tools.ietf.org/html/draft-khalili-mptcp-congestion-control-05
     * mtu: mtu of the path
     * ackedBytes: ackednowlged bytes
     * path: path variable (for further investigation, debug, etc)
     */
    uint32 updateOLIA(uint32 w, uint32 s, uint32 totalW, double a, uint32 mtu, uint32 ackedBytes, SctpPathVariables* path);

    bool addAuthChunkIfNecessary(Ptr<SctpHeader> sctpMsg, uint16 chunkType, bool authAdded)
    {
        if ((state->auth) && (state->peerAuth) && (typeInChunkList(chunkType)) && (authAdded == false)) {
            SctpAuthenticationChunk *authChunk = createAuthChunk();
            sctpMsg->appendSctpChunks(authChunk);
            auto it = sctpMain->assocStatMap.find(assocId);
            it->second.numAuthChunksSent++;
            return true;
        }
        return false;
    }
};

} // namespace sctp
} // namespace inet

#endif // ifndef __INET_SCTPASSOCIATION_H


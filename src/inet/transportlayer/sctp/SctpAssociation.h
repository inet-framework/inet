//
// Copyright (C) 2005-2010 Irene Ruengeler
// Copyright (C) 2009-2015 Thomas Dreibholz
// Copyright (C) 2015 Martin Becke
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCTPASSOCIATION_H
#define __INET_SCTPASSOCIATION_H

#include <errno.h>
#include <math.h>

#include <iostream>
#include <list>

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
    DATA              = 0,
    INIT              = 1,
    INIT_ACK          = 2,
    SACK              = 3,
    HEARTBEAT         = 4,
    HEARTBEAT_ACK     = 5,
    ABORT             = 6,
    SHUTDOWN          = 7,
    SHUTDOWN_ACK      = 8,
    ERRORTYPE         = 9,
    COOKIE_ECHO       = 10,
    COOKIE_ACK        = 11,
    SHUTDOWN_COMPLETE = 14,
    AUTH              = 15,
    NR_SACK           = 16,
    ASCONF_ACK        = 128,
    PKTDROP           = 129,
    RE_CONFIG         = 130,
    FORWARD_TSN       = 192,
    ASCONF            = 193,
    IFORWARD_TSN      = 194
};

enum SctpPrMethods {
    PR_NONE   = 0,
    PR_TTL    = 1,
    PR_RTX    = 2,
    PR_PRIO   = 3,
    PR_STRRST = 4
};

enum SctpStreamResetConstants {
    NOTHING_TO_DO                          = 0,
    PERFORMED                              = 1,
    DENIED                                 = 2,
    WRONG_SSN                              = 3,
    REQUEST_IN_PROGRESS                    = 4,
    NO_RESET                               = 5,
    DEFERRED                               = 6,
    RESET_OUTGOING                         = 7,
    RESET_INCOMING                         = 8,
    RESET_BOTH                             = 9,
    SSN_TSN                                = 10,
    ADD_INCOMING                           = 11,
    ADD_OUTGOING                           = 12,
    OUTGOING_RESET_REQUEST_PARAMETER       = 13,
    INCOMING_RESET_REQUEST_PARAMETER       = 14,
    SSN_TSN_RESET_REQUEST_PARAMETER        = 15,
    STREAM_RESET_RESPONSE_PARAMETER        = 16,
    ADD_OUTGOING_STREAMS_REQUEST_PARAMETER = 17,
    ADD_INCOMING_STREAMS_REQUEST_PARAMETER = 18,
    ADD_BOTH                               = 19,
    PERFORMED_WITH_OPTION                  = 20,
    PERFORMED_WITH_ADDOUT                  = 21
};

enum SctpAddIPCorrelatedTypes {
    SET_PRIMARY_ADDRESS          = 49156,
    ADAPTATION_LAYER_INDICATION  = 49158,
    SUPPORTED_EXTENSIONS         = 32776,
    ADD_IP_ADDRESS               = 49153,
    DELETE_IP_ADDRESS            = 49154,
    ERROR_CAUSE_INDICATION       = 49155,
    SUCCESS_INDICATION           = 49157,
    ERROR_DELETE_LAST_IP_ADDRESS = 160,
    ERROR_DELETE_SOURCE_ADDRESS  = 162
};

enum SctpParameterTypes {
    UNRECOGNIZED_PARAMETER          = 8,
    SUPPORTED_ADDRESS_TYPES         = 12,
    FORWARD_TSN_SUPPORTED_PARAMETER = 49152,
    RANDOM                          = 32770,
    CHUNKS                          = 32771,
    HMAC_ALGO                       = 32772
};

enum SctpErrorCauses {
    INVALID_STREAM_IDENTIFIER = 1,
    NO_USER_DATA              = 9,
    UNSUPPORTED_HMAC          = 261,
    MISSING_NAT_ENTRY         = 177
};

enum SctpCcModules {
    RFC4960 = 0
};

enum SctpStreamSchedulers {
    ROUND_ROBIN            = 0,
    ROUND_ROBIN_PACKET     = 1,
    RANDOM_SCHEDULE        = 2,
    RANDOM_SCHEDULE_PACKET = 3,
    FAIR_BANDWITH          = 4,
    FAIR_BANDWITH_PACKET   = 5,
    PRIORITY               = 6,
    FCFS                   = 7,
    PATH_MANUAL            = 8,
    PATH_MAP_TO_PATH       = 9
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
    uint32_t pathErrorCount;
    uint32_t pathErrorThreshold;
    uint32_t pmtu;
    // ====== Congestion Control ==========================================
    uint32_t cwnd;
    uint32_t tempCwnd;
    uint32_t ssthresh;
    uint32_t partialBytesAcked;
    uint32_t queuedBytes;
    uint32_t outstandingBytes;
    // ~~~~~~ Temporary storage for SACK handling ~~~~~~~
    uint32_t outstandingBytesBeforeUpdate;
    uint32_t newlyAckedBytes;
    // ====== Fast Recovery ===============================================
    bool fastRecoveryActive;
    uint32_t fastRecoveryExitPoint;
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
    uint32_t packetsInBurst;
    // ====== CMT Split Fast Retransmission ===============================
    bool sawNewAck;
    uint32_t lowestNewAckInSack;
    uint32_t highestNewAckInSack;
    // ====== CMT Pseudo CumAck (CUCv1) ===================================
    bool findPseudoCumAck;
    bool newPseudoCumAck;
    uint32_t pseudoCumAck;
    // ====== CMT RTX Pseudo CumAck (CUCv2) ===============================
    bool findRTXPseudoCumAck;
    bool newRTXPseudoCumAck;
    uint32_t rtxPseudoCumAck;
    uint32_t oldestChunkTsn;
    simtime_t oldestChunkSendTime;
    simtime_t newOldestChunkSendTime;
    // ====== CMT Round-Robin among Paths =================================
    simtime_t lastTransmission;
    unsigned int sendAllRandomizer;
    // ====== CMT/RP-SCTP =================================================
    unsigned int cmtCCGroup;
    unsigned int cmtGroupPaths;
    uint32_t utilizedCwnd;
    uint32_t cmtGroupTotalUtilizedCwnd;
    uint32_t cmtGroupTotalCwnd;
    uint32_t cmtGroupTotalSsthresh;
    double cmtGroupTotalCwndBandwidth;
    double cmtGroupTotalUtilizedCwndBandwidth;
    double cmtGroupAlpha;
    // ====== CMT Sender Buffer Control ===================================
    simtime_t blockingTimeout; // do not use path until given time
    // ====== CMT Slow Path RTT Calculation ===============================
    bool waitingForRTTCalculaton;
    simtime_t txTimeForRTTCalculation;
    uint32_t tsnForRTTCalculation;
    // ====== OLIA TMP Variable ===========================================
    uint32_t oliaSentBytes;
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
    unsigned int gapAckedChunksInLastSACK; // Per-path GapAck'ed chunks in last SACK (R+NR)
    unsigned int gapNRAckedChunksInLastSACK; // Per-path GapAck'ed chunks in last SACK (only NR)
    unsigned int gapUnackedChunksInLastSACK; // Per-path Not-GapAck'ed chunks in last SACK (i.e. chunks between GapAcks)
    unsigned int numberOfDuplicates;
    unsigned int numberOfFastRetransmissions;
    unsigned int numberOfTimerBasedRetransmissions;
    unsigned int numberOfHeartbeatsSent;
    unsigned int numberOfHeartbeatAcksSent;
    unsigned int numberOfHeartbeatsRcvd;
    unsigned int numberOfHeartbeatAcksRcvd;
    uint64_t numberOfBytesReceived;

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

    uint16_t getSid() { return sid; };

    // ====== Chunk Data Management =======================================
    cPacket *userData;
    uint32_t len; // Different from wire
    uint32_t booksize;
    uint32_t tsn;
    uint16_t sid;
    uint16_t ssn;
    uint32_t ppid;
    uint32_t fragments; // Number of fragments => TSNs={tsn, ..., tsn+fragments-1}
    bool enqueuedInTransmissionQ; // In transmissionQ? Otherwise, it is just in retransmissionQ.
    bool countsAsOutstanding; // Is chunk outstanding?
    bool hasBeenFastRetransmitted;
    bool hasBeenAbandoned;
    bool hasBeenReneged; // Has chunk been reneged?
    bool hasBeenAcked; // Has chunk been SACK'ed?
    bool hasBeenCountedAsNewlyAcked; // Chunk has been counted as newly SACK'ed
    bool bbit;
    bool ebit;
    bool ordered;
    bool ibit;

    // ====== Retransmission Management ===================================
    uint32_t gapReports;
    simtime_t enqueuingTime;
    simtime_t sendTime;
    simtime_t expiryTime;
    uint32_t numberOfRetransmissions;
    uint32_t numberOfTransmissions;
    uint32_t allowedNoRetransmissions;

    // ====== Advanced Chunk Information ==================================
    SctpPathVariables *queuedOnPath; // The path to account this chunk in qCounters.queuedOnPath
    SctpPathVariables *ackedOnPath; // The path this chunk has been acked on
    bool hasBeenTimerBasedRtxed; // Has chunk been timer-based retransmitted?
    bool hasBeenMoved; // Chunk has been moved to solve buffer blocking
    simtime_t firstSendTime;
    bool wasDropped; // For receiving side of PKTDROP: chunk dropped
    bool wasPktDropped; // Stays true even if the TSN has been transmitted
    bool strReset;
    uint32_t prMethod;
    uint32_t priority;
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

    struct RequestData {
        uint32_t sn;
        uint16_t result;
        uint16_t type;
        std::list<uint16_t> streams;
        uint32_t lastTsn;
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
    bool stopReceiving; // incoming data will be discarded
    bool stopOldData; // data with TSN<peerTsnAfterReset will be discarded
    bool queueUpdate;
    bool firstDataSent;
    bool peerWindowFull;
    bool zeroWindow;
    bool stopSending; // will be called when SCTP_E_SHUTDOWN arrived
    bool stopReading; // will be called when CLOSE was called and no data will be accepted
    bool inOut;
    bool noMoreOutstanding;
    bool fragInProgress;
    bool incomingRequestSet;
    uint32_t numGapReports;
    L3Address initialPrimaryPath;
    std::list<SctpPathVariables *> lastDataSourceList; // DATA chunk sources for new SACK
    SctpPathVariables *lastDataSourcePath;
    AddressVector localAddresses;
    std::list<uint32_t> dupList; // Duplicates list for incoming DATA chunks
    uint32_t errorCount; // overall error counter
    uint64_t peerRwnd;
    uint64_t initialPeerRwnd;
    uint64_t localRwnd;
    uint32_t nextTsn; // TSN to be sent
    uint32_t lastTsnAck; // stored at the sender side; cumTSNAck announced in a SACK
    uint32_t highestTsnAcked;
    uint32_t lastTsnReceived; // SACK
    uint32_t lastTsn; // my very last TSN to be sent
    uint32_t ackState; // number of packets to be acknowledged
    SctpGapList gapList; // GapAck list for incoming DATA chunks
    uint32_t packetsInTotalBurst;
    simtime_t lastTransmission;
    uint64_t outstandingBytes; // Number of bytes outstanding
    uint64_t queuedSentBytes; // Number of bytes in sender queue
    uint64_t queuedDroppableBytes; // Bytes in send queue droppable by PR-SCTP
    uint64_t queuedReceivedBytes; // Number of bytes in receiver queue
    uint32_t lastStreamScheduled;
    uint32_t assocPmtu; // smallest overall path mtu
    uint32_t fragPoint; // maximum size of a fragment
    uint32_t msgNum; // indicates the sequence number of the message
    uint64_t bytesRcvd;
    uint32_t numRequests;
    uint32_t bytesToRetransmit;
    uint32_t messagesToPush;
    int32_t pushMessagesLeft;
    uint32_t count;
    uint8_t localTieTag[32];
    uint8_t peerTieTag[32];
    uint64_t queuedMessages; // Messages buffered at the sender side
    uint32_t messageAcceptLimit;
    uint32_t queueLimit;
    uint16_t header;
    uint16_t sendResponse;
    uint32_t responseSn;
    uint16_t numResetRequests;
    int32_t probingTimeout;
    std::vector<int32_t> numMsgsReq;
    int32_t cookieLifeTime;
    /** Counter for init and cookie retransmissions */
    int16_t initRetransCounter;
    simtime_t initRexmitTimeout;
    /** pointer to the init chunk data structure (for retransmissions) */
    SctpInitChunk *initChunk;
    /** pointer to the cookie chunk data structure (for retransmissions) */
    SctpCookieEchoChunk *cookieChunk;
    /** pointer to the resetChunk (for retransmission) */
    SctpShutdownChunk *shutdownChunk;
    SctpShutdownAckChunk *shutdownAckChunk;
    uint64_t sendQueueLimit;
    uint64_t sendBuffer;
    bool appSendAllowed;
    simtime_t lastSendQueueAbated;
    uint32_t nextRSid;
    uint32_t swsLimit;
    bool lastMsgWasFragment;
    bool enableHeartbeats;
    bool sendHeartbeatsOnActivePaths;
    Ptr<SctpHeader> sctpMsg;
    uint16_t chunksAdded;
    uint16_t dataChunksAdded;
    uint32_t packetBytes;
    uint16_t numAddedOutStreams;
    uint16_t numAddedInStreams;
    bool authAdded;

    // ====== NR-SACK =====================================================
    bool nrSack;
    uint32_t gapReportLimit;
    enum GLOVariant {
        GLOV_None       = 0,
        GLOV_Optimized1 = 1,
        GLOV_Optimized2 = 2,
        GLOV_Shrunken   = 3
    };
    uint32_t gapListOptimizationVariant;
    bool smartOverfullSACKHandling;
    bool disableReneging;

    // ====== Retransmission Method =======================================
    uint32_t rtxMethod;
    // ====== Max Burst ===================================================
    uint32_t maxBurst;
    enum MBVariant {
        MBV_UseItOrLoseIt                    = 0,
        MBV_CongestionWindowLimiting         = 1,
        MBV_UseItOrLoseItTempCwnd            = 2,
        MBV_CongestionWindowLimitingTempCwnd = 3,
        MBV_MaxBurst                         = 4,
        MBV_AggressiveMaxBurst               = 5,
        MBV_TotalMaxBurst                    = 6
    };
    MBVariant maxBurstVariant;
    uint32_t initialWindow;
    // ====== CMT-SCTP ====================================================
    bool allowCMT;
    bool (*cmtSendAllComparisonFunction)(const SctpPathVariables *left, const SctpPathVariables *right);
    const char *cmtRetransmissionVariant;

    enum CUCVariant {
        CUCV_Normal         = 0,
        CUCV_PseudoCumAck   = 1,
        CUCV_PseudoCumAckV2 = 2
    };
    CUCVariant cmtCUCVariant; // Cwnd Update for CMT (CUC)

    enum BufferSplitVariant {
        CBSV_None         = 0,
        CBSV_SenderOnly   = 1,
        CBSV_ReceiverOnly = 2,
        CBSV_BothSides    = 3
    };
    BufferSplitVariant cmtBufferSplitVariant; // Buffer Splitting for CMT
    bool cmtBufferSplittingUsesOSB; // Use outstanding instead of queued bytes for Buffer Splitting

    enum ChunkReschedulingVariant {
        CCRV_None         = 0,
        CCRV_SenderOnly   = 1,
        CCRV_ReceiverOnly = 2,
        CCRV_BothSides    = 3,
        CCRV_Test         = 99    // Test only!
    };
    ChunkReschedulingVariant cmtChunkReschedulingVariant; // Chunk Rescheduling
    double cmtChunkReschedulingThreshold; // Blocking Threshold for Chunk Rescheduling

    bool cmtSmartT3Reset; // Smart T3 Reset for CMT
    bool cmtSmartFastRTX; // Smart Fast RTX for CMT
    bool cmtSmartReneging; // Smart Reneging for CMT
    bool cmtSlowPathRTTUpdate; // Slow Path RTT Update for CMT
    bool cmtUseSFR; // Split Fast Retransmission (SFR) for CMT
    bool cmtUseDAC; // Delayed Ack for CMT (DAC)
    bool cmtUseFRC; // Fast Recovery for CMT (FRC)
    bool cmtMovedChunksReduceCwnd; // Subtract moved chunk from cwnd of old path
    double movedChunkFastRTXFactor;
    unsigned int blockingTsnsMoved;
    bool strictCwndBooking; // Strict overbooking handling
    enum CSackPath {
        CSP_Standard     = 0,
        CSP_Primary      = 1,
        CSP_RoundRobin   = 2,
        CSP_SmallestSRTT = 3
    };
    CSackPath cmtSackPath; // SACK path selection variant for CMT
    // ====== High-Speed SCTP =============================================
    bool highSpeedCC; // HighSpeed CC (RFC 3649)

    // ====== CMT/RP-SCTP =================================================
    enum CCCVariant {
        CCCV_Off         = 0,    // Standard SCTP
        CCCV_CMT         = 1,    // CMT-SCTP
        CCCV_CMTRPv1     = 2,    // CMT/RPv1-SCTP (old CMT/RP)
        CCCV_CMTRPv2     = 3,    // CMT/RPv2-SCTP (new CMT/RP with bandwidth consideration)
        CCCV_CMT_LIA     = 4,    // LIA CC
        CCCV_CMT_OLIA    = 5,    // OLIA CC
        CCCV_CMTRP_Test1 = 100,
        CCCV_CMTRP_Test2 = 101
    };
    CCCVariant cmtCCVariant;
    bool rpPathBlocking; // T.D. 10.08.2011: CMT/RP path blocking
    bool rpScaleBlockingTimeout; // T.D. 15.08.2011: Scale blocking timeout by number of paths
    uint32_t rpMinCwnd; // T.D. 15.08.2011: Minimum cwnd in MTUs

    // ====== SACK Sequence Number Checker ================================
    bool checkSackSeqNumber; // Ensure handling SACKs in original sequence
    uint32_t outgoingSackSeqNum;
    uint32_t incomingSackSeqNum;

    // ====== Partial Reliability SCTP ====================================
    uint32_t asconfSn; // own AddIP serial number
    uint16_t numberAsconfReceived;
    uint32_t corrIdNum;
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
    uint32_t streamResetSequenceNumber;
    uint32_t expectedStreamResetSequenceNumber;
    uint32_t peerRequestSn;
    uint32_t inRequestSn;
    uint32_t peerTsnAfterReset;
    uint32_t lastTsnBeforeReset; // lastTsn announced in OutgoingStreamResetParameter
    SctpStreamResetChunk *resetChunk; // pointer to the resetChunk (for retransmission)
    SctpParameter *incomingRequest;
    std::list<uint16_t> resetOutStreams;
    std::list<uint16_t> resetInStreams;
    std::list<uint16_t> streamsPending;
    std::list<uint16_t> streamsToReset;
    std::list<uint16_t> peerStreamsToReset;
    std::map<uint32_t, RequestData> requests;
    std::map<uint32_t, RequestData> peerRequests;
    SctpResetReq *resetInfo;
    uint16_t peerRequestType;
    uint16_t localRequestType;

    bool findRequestNum(uint32_t num);
    bool findPeerRequestNum(uint32_t num);
    bool findPeerStreamToReset(uint16_t sn);
    bool findMatch(uint16_t sn);
    RequestData *findTypeInRequests(uint16_t type);
    uint16_t getNumRequestsNotPerformed();

    // ====== SCTP Authentication =========================================
    uint16_t hmacType;
    bool peerAuth;
    bool auth;
    std::vector<uint16_t> chunkList;
    std::vector<uint16_t> peerChunkList;
    uint8_t keyVector[512];
    uint32_t sizeKeyVector;
    uint8_t peerKeyVector[512];
    uint32_t sizePeerKeyVector;
    uint8_t sharedKey[512];

    // ====== Further features ============================================
    bool osbWithHeader;
    bool padding;
    bool pktDropSent;
    bool peerPktDrop;
    uint32_t advancedPeerAckPoint;
    uint32_t prMethod;
    bool peerAllowsChunks; // Flowcontrol: indicates whether the peer adjusts the window according to a number of messages
    uint32_t initialPeerMsgRwnd;
    uint32_t localMsgRwnd;
    uint32_t peerMsgRwnd; // Flowcontrol: corresponds to peerRwnd
    uint32_t bufferedMessages; // Messages buffered at the receiver side
    uint32_t outstandingMessages; // Outstanding messages on the sender side; used for flowControl; including retransmitted messages
    uint32_t bytesToAddPerRcvdChunk;
    uint32_t bytesToAddPerPeerChunk;
    bool tellArwnd;
    bool swsMsgInvoked; // Flowcontrol: corresponds to swsAvoidanceInvoked
    simtime_t lastThroughputTime;
    std::map<uint16_t, uint32_t> streamThroughput;
    simtime_t lastAssocThroughputTime;
    uint32_t assocThroughput;
    double throughputInterval;
    bool ssNextStream;
    bool ssLastDataChunkSizeSet;
    bool ssOneStreamLeft;
    std::map<uint16_t, uint32_t> ssPriorityMap;
    std::map<uint16_t, int32_t> ssFairBandwidthMap;
    std::map<uint16_t, int32_t> ssStreamToPathMap;

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
    typedef std::map<L3Address, uint32_t> CounterMap;

    struct QueueCounter {
        uint64_t roomSumSendStreams;
        uint64_t bookedSumSendStreams;
        uint64_t roomSumRcvStreams;
        CounterMap roomTransQ;
        CounterMap bookedTransQ;
        CounterMap roomRetransQ;
    };

    struct BytesToBeSent {
        bool chunk;
        bool packet;
        uint32_t bytesToSend;
    };

    struct CCFunctions {
        void (SctpAssociation::*ccInitParams)(SctpPathVariables *path);
        void (SctpAssociation::*ccUpdateBeforeSack)();
        void (SctpAssociation::*ccUpdateAfterSack)();
        void (SctpAssociation::*ccUpdateAfterCwndTimeout)(SctpPathVariables *path);
        void (SctpAssociation::*ccUpdateAfterRtxTimeout)(SctpPathVariables *path);
        void (SctpAssociation::*ccUpdateMaxBurst)(SctpPathVariables *path);
        void (SctpAssociation::*ccUpdateBytesAcked)(SctpPathVariables *path, const uint32_t ackedBytes, const bool ctsnaAdvanced);
    };

    typedef std::map<uint32_t, SctpSendStream *> SctpSendStreamMap;
    typedef std::map<uint32_t, SctpReceiveStream *> SctpReceiveStreamMap;
    typedef std::map<uint32_t, SctpPathVariables *> SctpPathCollection;
    SctpPathCollection assocBestPaths;
    SctpPathCollection assocMaxWndPaths;
    SctpPathCollection assocCollectedPaths;

  public:
    // connection identification by apps: appgateIndex+assocId
    int32_t appGateIndex; // Application gate index
    int32_t assocId; // Identifies connection within the app
    int32_t listeningAssocId;
    int32_t fd;
    bool listening;
    L3Address remoteAddr; // Remote address from last message
    L3Address localAddr; // Local address from last message
    uint16_t localPort; // Remote port from last message
    uint16_t remotePort; // Local port from last message
    uint32_t localVTag; // Local verification tag
    uint32_t peerVTag; // Remote verification tag

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
    std::map<uint16_t, cOutVector *> streamThroughputVectors;
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
    uint32_t numberOfRemoteAddresses;
    uint32_t inboundStreams;
    uint32_t outboundStreams;
    uint32_t initInboundStreams;

    int32_t status;
    uint32_t initTsn;
    uint32_t initPeerTsn;
    uint32_t sackFrequency;
    double sackPeriod;
    CCFunctions ccFunctions;
    uint16_t ccModule;

    cOutVector *advRwnd;
    cOutVector *cumTsnAck;
    cOutVector *sendQueue;
    cOutVector *numGapBlocks;

    // Variables associated with the state of this association
    SctpStateVariables *state;
    BytesToBeSent bytes;
    Sctp *sctpMain; // SCTP module
    cFSM *fsm; // SCTP state machine
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
    cOutVector *statisticsRevokableGapBlocksInLastSACK; // Revokable GapAck blocks in last received SACK
    cOutVector *statisticsNonRevokableGapBlocksInLastSACK; // Non-Revokable GapAck blocks in last received SACK
    cOutVector *statisticsArwndInLastSACK;
    cOutVector *statisticsPeerRwnd;
    // ------ Sent SACK Statistics ----------------------------------------
    cOutVector *statisticsNumTotalGapBlocksStored; // Number of GapAck blocks stored (NOTE: R + NR!)
    cOutVector *statisticsNumRevokableGapBlocksStored; // Number of Revokable GapAck blocks stored
    cOutVector *statisticsNumNonRevokableGapBlocksStored; // Number of Non-Revokable GapAck blocks stored
    cOutVector *statisticsNumDuplicatesStored; // Number of duplicate TSNs stored
    cOutVector *statisticsNumRevokableGapBlocksSent; // Number of Revokable GapAck blocks sent in last SACK
    cOutVector *statisticsNumNonRevokableGapBlocksSent; // Number of Non-Revokable GapAck blocks sent in last SACK
    cOutVector *statisticsNumDuplicatesSent; // Number of duplicate TSNs sent in last SACK
    cOutVector *statisticsSACKLengthSent; // Length of last sent SACK

  public:
    /**
     * Constructor.
     */
    SctpAssociation(Sctp *mod, int32_t appGateIndex, int32_t assocId, IRoutingTable *rt, IInterfaceTable *ift);
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
    static const char *indicationName(int32_t code);

    /** Utility: return IPv4 or IPv6 address level */
    static int getAddressLevel(const L3Address& addr);

    /* @name Various getters */
    //@{
    int32_t getFsmState() const { return fsm->getState(); };
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
    static const char *stateName(int32_t state);

    static uint16_t chunkToInt(const char *type);

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
//    bool processSctpMessage(const Ptr<const SctpHeader>& sctpmsg, const L3Address& srcAddr, const L3Address& destAddr);
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
        return (iterator != sctpPathMap.end()) ? iterator->second : nullptr;
    }

    void printSctpPathMap() const;

    /**
     * Compare TSNs
     */
    /* Defines see RFC1982 for details */
    static bool SCTP_UINT16_GT(uint16_t a, uint16_t b) {
        return ((a < b) && ((uint16_t)(b - a) > (1U << 15))) || \
               ((a > b) && ((uint16_t)(a - b) < (1U << 15)));
    }

    static bool SCTP_UINT16_GE(uint16_t a, uint16_t b) { return (a == b) || SCTP_UINT16_GT(a, b); }
    static bool SCTP_UINT32_GT(uint32_t a, uint32_t b) {
        return ((a < b) && ((uint32_t)(b - a) > (1UL << 31))) ||
               ((a > b) && ((uint32_t)(a - b) < (1UL << 31)));
    }

    static bool SCTP_UINT32_GE(uint32_t a, uint32_t b) { return SCTP_UINT32_GT(a, b) || (a == b); }
    #define SCTP_TSN_GT(a, b)    SCTP_UINT32_GT(a, b)
    #define SCTP_TSN_GE(a, b)    SCTP_UINT32_GE(a, b)
    #define SCTP_SSN_GT(a, b)    SCTP_UINT16_GT(a, b)
    #define SCTP_SSN_GE(a, b)    SCTP_UINT16_GE(a, b)
    static bool tsnGe(const uint32_t tsn1, const uint32_t tsn2) { return SCTP_TSN_GE(tsn1, tsn2); }
    static bool tsnGt(const uint32_t tsn1, const uint32_t tsn2) { return SCTP_TSN_GT(tsn1, tsn2); }
    static bool tsnLe(const uint32_t tsn1, const uint32_t tsn2) { return SCTP_TSN_GE(tsn2, tsn1); }
    static bool tsnLt(const uint32_t tsn1, const uint32_t tsn2) { return SCTP_TSN_GT(tsn2, tsn1); }
    static bool tsnBetween(const uint32_t tsn1, const uint32_t midtsn, const uint32_t tsn2) { return SCTP_TSN_GE(midtsn, tsn1) && SCTP_TSN_GE(tsn2, midtsn); }
    static bool ssnGt(const uint16_t ssn1, const uint16_t ssn2) { return SCTP_SSN_GT(ssn1, ssn2); }
    static bool midGt(const uint32_t mid1, const uint32_t mid2) { return SCTP_TSN_GT(mid1, mid2); }

  protected:
    /** @name FSM transitions: analysing events and executing state transitions */
    //@{
    /** Maps app command codes (msg kind of app command msgs) to SCTP_E_xxx event codes */
    SctpEventCode preanalyseAppCommandEvent(int32_t commandCode);
    /** Implemements the pure SCTP state machine */
    bool performStateTransition(const SctpEventCode& event);
    void stateEntered(int32_t state);
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
//    bool process_RCV_Message(const Ptr<const SctpHeader>& sctpmsg, const L3Address& src, const L3Address& dest);
    bool process_RCV_Message(SctpHeader *sctpmsg, const L3Address& src, const L3Address& dest);
    /**
     * Process incoming SCTP packets. Invoked from process_RCV_Message
     */
    bool processInitArrived(SctpInitChunk *initChunk, int32_t sport, int32_t dport);
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
    int32_t updateCounters(SctpPathVariables *path);
    void process_TIMEOUT_RESET(SctpPathVariables *path);
    void process_TIMEOUT_ASCONF(SctpPathVariables *path);
    //@}

    void startTimer(cMessage *timer, const simtime_t& timeout);

    /** Utility: clone a listening association. Used for forking. */
    SctpAssociation *cloneAssociation();

    /** Utility: creates send/receive queues and sctpAlgorithm */
    void initAssociation(SctpOpenReq *openCmd);

    /** Methods dealing with the handling of TSNs  **/
    bool tsnIsDuplicate(const uint32_t tsn) const;
    bool makeRoomForTsn(const uint32_t tsn, const uint32_t length, const bool uBit);

    /** Methods for creating and sending chunks */
    void sendInit();
    void sendInitAck(SctpInitChunk *initchunk);
    void sendCookieEcho(SctpInitAckChunk *initackchunk);
    void sendCookieAck(const L3Address& dest);
    void sendAbort(uint16_t tBit = 0);
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
        sctpMain->scheduleAfter(timeout, msg);
    }

    /** Utility: cancel a timer */
    cMessage *cancelEvent(cMessage *msg)
    {
        return sctpMain->cancelEvent(msg);
    }

    /** Utility: sends packet to application */
    void sendToApp(cMessage *msg);

    /** Utility: sends status indication (SCTP_I_xxx) to application */
    void sendIndicationToApp(int32_t code, int32_t value = 0);

    /** Utility: sends SCTP_I_ESTABLISHED indication with SctpConnectInfo to application */
    void sendEstabIndicationToApp();
    void sendAvailableIndicationToApp();
    bool isToBeAccepted() const { return listeningAssocId != -1; }
    void pushUlp();
    void sendDataArrivedNotification(uint16_t sid);
    void putInDeliveryQ(uint16_t sid);
    /** Utility: prints local/remote addr/port and app gate index/assocId */
    void printAssocBrief();
    /** Utility: prints important header fields */
    static void printSegmentBrief(SctpHeader *sctpmsg);

    /** Utility: returns name of SCTP_E_xxx constants */
    static const char *eventName(int32_t event);

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
    void sendStreamResetResponse(uint32_t srrsn, int result);
    void sendStreamResetResponse(SctpSsnTsnResetRequestParameter *requestParam, int result,
            bool options);
    void sendOutgoingResetRequest(SctpIncomingSsnResetRequestParameter *requestParam);
    void sendAddOutgoingStreamsRequest(uint16_t numStreams);
    void sendBundledOutgoingResetAndResponse(SctpIncomingSsnResetRequestParameter *requestParam);
    void sendAddInAndOutStreamsRequest(SctpResetReq *info);
    void sendDoubleStreamResetResponse(uint32_t insrrsn, uint16_t inresult, uint32_t outsrrsn, uint16_t outresult);
    void checkStreamsToReset();
    bool streamIsPending(int32_t sid);
    void sendPacketDrop(const bool flag);
    void sendHMacError(const uint16_t id);
    void sendInvalidStreamError(uint16_t sid);
    void resetGapLists();

    SctpForwardTsnChunk *createForwardTsnChunk(const L3Address& pid);

    bool msgMustBeAbandoned(SctpDataMsg *msg, int32_t stream, bool ordered); // PR-SCTP
    bool chunkMustBeAbandoned(SctpDataVariables *chunk,
            SctpPathVariables *sackPath);
    void advancePeerTsn();

    void cucProcessGapReports(const SctpDataVariables *chunk,
            SctpPathVariables *path,
            const bool isAcked); // CMT-SCTP
    /**
     * Manipulating chunks
     */
    SctpDataChunk *transformDataChunk(SctpDataVariables *chunk);
    SctpDataVariables *makeVarFromMsg(SctpDataChunk *datachunk);

    /**
     * Dealing with streams
     */

    int32_t streamScheduler(SctpPathVariables *path, bool peek);
    void initStreams(uint32_t inStreams, uint32_t outStreams);
    void addInStreams(uint32_t inStreams);
    void addOutStreams(uint32_t outStreams);
    int32_t numUsableStreams();
    int32_t streamSchedulerRoundRobinPacket(SctpPathVariables *path, bool peek);
    int32_t streamSchedulerRandom(SctpPathVariables *path, bool peek);
    int32_t streamSchedulerRandomPacket(SctpPathVariables *path, bool peek);
    int32_t streamSchedulerFairBandwidth(SctpPathVariables *path, bool peek);
    int32_t streamSchedulerFairBandwidthPacket(SctpPathVariables *path, bool peek);
    int32_t streamSchedulerPriority(SctpPathVariables *path, bool peek);
    int32_t streamSchedulerFCFS(SctpPathVariables *path, bool peek);
    int32_t pathStreamSchedulerManual(SctpPathVariables *path, bool peek);
    int32_t pathStreamSchedulerMapToPath(SctpPathVariables *path, bool peek);

    struct SSFunctions {
        void (SctpAssociation::*ssInitStreams)(uint32_t inStreams, uint32_t outStreams);
        void (SctpAssociation::*ssAddInStreams)(uint32_t inStreams);
        void (SctpAssociation::*ssAddOutStreams)(uint32_t outStreams);
        int32_t (SctpAssociation::*ssGetNextSid)(SctpPathVariables *path, bool peek);
        int32_t (SctpAssociation::*ssUsableStreams)();
    };
    SSFunctions ssFunctions;
    uint16_t ssModule;

    /**
     *    Queue Management
     */
    void process_QUEUE_MSGS_LIMIT(const SctpCommandReq *sctpCommand);
    void process_QUEUE_BYTES_LIMIT(const SctpCommandReq *sctpCommand);
    int32_t getOutstandingBytes() const;
    void dequeueAckedChunks(const uint32_t tsna,
            SctpPathVariables *path,
            simtime_t& rttEstimation);
    SctpDataMsg *peekOutboundDataMsg();
    SctpDataVariables *peekAbandonedChunk(const SctpPathVariables *path);
    SctpDataVariables *getOutboundDataChunk(const SctpPathVariables *path,
            int32_t availableSpace,
            int32_t availableCwnd);
    void fragmentOutboundDataMsgs();
    SctpDataMsg *dequeueOutboundDataMsg(SctpPathVariables *path,
            int32_t availableSpace,
            int32_t availableCwnd);
    bool nextChunkFitsIntoPacket(SctpPathVariables *path, int32_t bytes);
    void putInTransmissionQ(uint32_t tsn, SctpDataVariables *chunk);

    uint32_t getAllTransQ();

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
    bool sendStreamPresent(uint32_t sid);
    bool receiveStreamPresent(uint32_t sid);
    void resetSsn(uint16_t id);
    void resetExpectedSsn(uint16_t id);
    uint32_t getExpectedSsnOfStream(uint16_t id);
    uint32_t getSsnOfStream(uint16_t id);
    SctpParameter *makeOutgoingStreamResetParameter(uint32_t srsn, SctpResetReq *info);
    SctpParameter *makeIncomingStreamResetParameter(uint32_t srsn, SctpResetReq *info);
    SctpParameter *makeSsnTsnResetParameter(uint32_t srsn);
    SctpParameter *makeAddStreamsRequestParameter(uint32_t srsn, SctpResetReq *info);
    void sendOutgoingRequestAndResponse(uint32_t inRequestSn, uint32_t outRequestSn);
    void sendOutgoingRequestAndResponse(SctpIncomingSsnResetRequestParameter *inRequestParam, SctpOutgoingSsnResetRequestParameter *outRequestParam);
    SctpEventCode processInAndOutResetRequestArrived(SctpIncomingSsnResetRequestParameter *inRequestParam, SctpOutgoingSsnResetRequestParameter *outRequestParam);
    SctpEventCode processOutAndResponseArrived(SctpOutgoingSsnResetRequestParameter *outRequestParam, SctpStreamResetResponseParameter *responseParam);
    SctpEventCode processStreamResetArrived(SctpStreamResetChunk *strResChunk);
    void processOutgoingResetRequestArrived(SctpOutgoingSsnResetRequestParameter *requestParam);
    void processIncomingResetRequestArrived(SctpIncomingSsnResetRequestParameter *requestParam);
    void processSsnTsnResetRequestArrived(SctpSsnTsnResetRequestParameter *requestParam);
    void processResetResponseArrived(SctpStreamResetResponseParameter *responseParam);
    void processAddInAndOutResetRequestArrived(const SctpAddStreamsRequestParameter *addInRequestParam, SctpAddStreamsRequestParameter *addOutRequestParam);
    uint32_t getBytesInFlightOfStream(uint16_t sid);
    bool getFragInProgressOfStream(uint16_t sid);
    void setFragInProgressOfStream(uint16_t sid, bool frag);
    bool orderedQueueEmptyOfStream(uint16_t sid);
    bool unorderedQueueEmptyOfStream(uint16_t sid);

    /** Methods for Add-IP and AUTH **/
    void sendAsconf(const char *type, bool remote = false);
    void sendAsconfAck(uint32_t serialNumber);
    SctpEventCode processAsconfArrived(SctpAsconfChunk *asconfChunk);
    SctpEventCode processAsconfAckArrived(SctpAsconfAckChunk *asconfAckChunk);
    void retransmitAsconf();
    bool typeInChunkList(uint16_t type);
    bool typeInOwnChunkList(uint16_t type);
    SctpAsconfAckChunk *createAsconfAckChunk(uint32_t serialNumber);
    SctpAuthenticationChunk *createAuthChunk();
    SctpSuccessIndication *createSuccessIndication(uint32_t correlationId);
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
    void updateFastRecoveryStatus(uint32_t lastTsnAck);
    void cwndUpdateBeforeSack();
    void cwndUpdateAfterSack();
    void cwndUpdateAfterCwndTimeout(SctpPathVariables *path);
    void cwndUpdateAfterRtxTimeout(SctpPathVariables *path);
    void cwndUpdateMaxBurst(SctpPathVariables *path);
    void cwndUpdateBytesAcked(SctpPathVariables *path,
            uint32_t ackedBytes,
            bool ctsnaAdvanced);
    int32_t rpPathBlockingControl(SctpPathVariables *path, double reduction);

  private:
    SctpDataVariables *makeDataVarFromDataMsg(SctpDataMsg *datMsg,
            SctpPathVariables *path);
    SctpPathVariables *choosePathForRetransmission();
    void timeForSack(bool& sackOnly, bool& sackWithData);
    void recordCwndUpdate(SctpPathVariables *path);
    void sendSACKviaSelectedPath(const Ptr<SctpHeader>& sctpMsg);
    void checkOutstandingBytes();
    void updateHighSpeedCCThresholdIdx(SctpPathVariables *path);
    uint32_t getInitialCwnd(const SctpPathVariables *path) const;
    void generateSendQueueAbatedIndication(uint64_t bytes);
    void renegablyAckChunk(SctpDataVariables *chunk,
            SctpPathVariables *sackPath);
    void nonRenegablyAckChunk(SctpDataVariables *chunk,
            SctpPathVariables *sackPath,
            simtime_t& rttEstimation,
            Sctp::AssocStat *assocStat);
    void handleChunkReportedAsAcked(uint32_t& highestNewAck,
            simtime_t& rttEstimation,
            SctpDataVariables *myChunk,
            SctpPathVariables *sackPath,
            const bool sackIsNonRevokable);
    void handleChunkReportedAsMissing(const SctpSackChunk *sackChunk,
            const uint32_t highestNewAck,
            SctpDataVariables *myChunk,
            SctpPathVariables *sackPath);
    void moveChunkToOtherPath(SctpDataVariables *chunk,
            SctpPathVariables *newPath);
    void decreaseOutstandingBytes(SctpDataVariables *chunk);
    void increaseOutstandingBytes(SctpDataVariables *chunk,
            SctpPathVariables *path);
    int32_t calculateBytesToSendOnPath(const SctpPathVariables *pathVar);
    void storePacket(SctpPathVariables *pathVar,
            const Ptr<SctpHeader>& sctpMsg,
            uint16_t chunksAdded,
            uint16_t dataChunksAdded,
            bool authAdded);
    void loadPacket(SctpPathVariables *pathVar,
            Ptr<SctpHeader> *sctpMsg,
            uint16_t *chunksAdded,
            uint16_t *dataChunksAdded,
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

    bool chunkHasBeenAcked(uint32_t tsn) const
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
    uint32_t updateOLIA(uint32_t w, uint32_t s, uint32_t totalW, double a, uint32_t mtu, uint32_t ackedBytes, SctpPathVariables *path);

    bool addAuthChunkIfNecessary(Ptr<SctpHeader> sctpMsg, uint16_t chunkType, bool authAdded)
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

#endif


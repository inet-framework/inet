//
// Copyright (C) 2005-2010 Irene Ruengeler
// Copyright (C) 2009-2015 Thomas Dreibholz
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/sctp/SctpAssociation.h"

#include <assert.h>
#include <string.h>

#include <sstream>

#include "inet/common/stlutils.h"
#include "inet/transportlayer/contract/sctp/SctpCommand_m.h"
#include "inet/transportlayer/sctp/Sctp.h"
#include "inet/transportlayer/sctp/SctpAlgorithm.h"
#include "inet/transportlayer/sctp/SctpQueue.h"

namespace inet {
namespace sctp {

SctpPathVariables::SctpPathVariables(const L3Address& addr, SctpAssociation *assoc, const IRoutingTable *rt)
{
    // ====== Path Variable Initialization ===================================
    association = assoc;
    remoteAddress = addr;
    activePath = true;
    confirmed = false;
    primaryPathCandidate = false;
    pathErrorCount = 0;
    const NetworkInterface *rtie;
    pathErrorThreshold = assoc->getSctpMain()->getPathMaxRetrans();

    if (!pathErrorThreshold) {
        pathErrorThreshold = PATH_MAX_RETRANS;
    }

    pathRto = assoc->getSctpMain()->getRtoInitial();
    heartbeatTimeout = pathRto;
    double interval = assoc->getSctpMain()->par("hbInterval");

    if (!interval) {
        interval = HB_INTERVAL;
    }

    heartbeatIntervalTimeout = pathRto + interval;
    srtt = pathRto;
    lastAckTime = 0;
    forceHb = false;
    partialBytesAcked = 0;
    queuedBytes = 0;
    outstandingBytes = 0;

    rtie = rt->getOutputInterfaceForDestination(remoteAddress);

    if (rtie == nullptr) {
        throw cRuntimeError("No interface for remote address %s found!", remoteAddress.str().c_str());
    }

    pmtu = rtie->getMtu();
    rttvar = 0.0;

    cwndTimeout = pathRto;
    cwnd = 0;
    ssthresh = 0;
    tempCwnd = 0;
    rttUpdateTime = 0.0;
    fastRecoveryExitPoint = 0;
    fastRecoveryActive = false;

    cmtCCGroup = 0;
    lastTransmission = simTime();
    sendAllRandomizer = RNGCONTEXT uniform(0, (1u << 31));
    pseudoCumAck = 0;
    newPseudoCumAck = false;
    findPseudoCumAck = true; // Set findPseudoCumAck to TRUE for new destination.
    rtxPseudoCumAck = 0;
    newRTXPseudoCumAck = false;
    findRTXPseudoCumAck = true; // Set findRTXPseudoCumAck to TRUE for new destination.
    oldestChunkTsn = 0;
    oldestChunkSendTime = simTime();
    highestNewAckInSack = 0;
    lowestNewAckInSack = 0;
    waitingForRTTCalculaton = false;
    tsnForRTTCalculation = 0;
    txTimeForRTTCalculation = 0;
    blockingTimeout = simTime();
    packetsInBurst = 0;
    highSpeedCCThresholdIdx = 0;

    requiresRtx = false;
    newCumAck = false;
    outstandingBytesBeforeUpdate = 0;
    newlyAckedBytes = 0;
    findLowestTsn = true;
    lowestTsnRetransmitted = false;
    sawNewAck = false;
    cmtGroupPaths = 0;
    utilizedCwnd = 0;
    cmtGroupTotalUtilizedCwnd = 0;
    cmtGroupTotalCwnd = 0;
    cmtGroupTotalSsthresh = 0;
    cmtGroupTotalCwndBandwidth = 0.0;
    cmtGroupTotalUtilizedCwndBandwidth = 0.0;
    cmtGroupAlpha = 0.0;
    gapAckedChunksInLastSACK = 0;
    gapNRAckedChunksInLastSACK = 0;
    gapUnackedChunksInLastSACK = 0;

    oliaSentBytes = 0;
    numberOfFastRetransmissions = 0;
    numberOfTimerBasedRetransmissions = 0;
    numberOfHeartbeatsSent = 0;
    numberOfHeartbeatsRcvd = 0;
    numberOfHeartbeatAcksSent = 0;
    numberOfHeartbeatAcksRcvd = 0;
    numberOfDuplicates = 0;
    numberOfBytesReceived = 0;

    // ====== Path Info ======================================================
    SctpPathInfo *pinfo = new SctpPathInfo("pinfo");
    pinfo->setRemoteAddress(addr);

    // ====== Timers =========================================================
    char str[128];
    snprintf(str, sizeof(str), "HB_TIMER %d:%s", assoc->assocId, addr.str().c_str());
    HeartbeatTimer = new cMessage(str);
    snprintf(str, sizeof(str), "HB_INT_TIMER %d:%s", assoc->assocId, addr.str().c_str());
    HeartbeatIntervalTimer = new cMessage(str);
    snprintf(str, sizeof(str), "CWND_TIMER %d:%s", assoc->assocId, addr.str().c_str());
    CwndTimer = new cMessage(str);
    snprintf(str, sizeof(str), "RTX_TIMER %d:%s", assoc->assocId, addr.str().c_str());
    T3_RtxTimer = new cMessage(str);
    snprintf(str, sizeof(str), "Reset_TIMER %d:%s", assoc->assocId, addr.str().c_str());
    ResetTimer = new cPacket(str);
    ResetTimer->setContextPointer(association);
    snprintf(str, sizeof(str), "ASCONF_TIMER %d:%s", assoc->assocId, addr.str().c_str());
    AsconfTimer = new cMessage(str);
    AsconfTimer->setContextPointer(association);
    snprintf(str, sizeof(str), "BLOCKING_TIMER %d:%s", assoc->assocId, addr.str().c_str());
    BlockingTimer = new cMessage(str);
    HeartbeatTimer->setContextPointer(association);
    HeartbeatIntervalTimer->setContextPointer(association);
    CwndTimer->setContextPointer(association);
    T3_RtxTimer->setContextPointer(association);
    T3_RtxTimer->setControlInfo(pinfo);
    HeartbeatTimer->setControlInfo(pinfo->dup());
    HeartbeatIntervalTimer->setControlInfo(pinfo->dup());
    CwndTimer->setControlInfo(pinfo->dup());
    ResetTimer->setControlInfo(pinfo->dup());
    AsconfTimer->setControlInfo(pinfo->dup());
    BlockingTimer->setControlInfo(pinfo->dup());

    snprintf(str, sizeof(str), "RTO %d:%s", assoc->assocId, addr.str().c_str());
    statisticsPathRTO = new cOutVector(str);
    snprintf(str, sizeof(str), "RTT %d:%s", assoc->assocId, addr.str().c_str());
    statisticsPathRTT = new cOutVector(str);

    snprintf(str, sizeof(str), "Slow Start Threshold %d:%s", assoc->assocId, addr.str().c_str());
    statisticsPathSSthresh = new cOutVector(str);
    snprintf(str, sizeof(str), "Congestion Window %d:%s", assoc->assocId, addr.str().c_str());
    statisticsPathCwnd = new cOutVector(str);
    snprintf(str, sizeof(str), "Bandwidth %d:%s", assoc->assocId, addr.str().c_str());
    statisticsPathBandwidth = new cOutVector(str);

    snprintf(str, sizeof(str), "TSN Sent %d:%s", assoc->assocId, addr.str().c_str());
    vectorPathSentTsn = new cOutVector(str);
    snprintf(str, sizeof(str), "TSN Received %d:%s", assoc->assocId, addr.str().c_str());
    vectorPathReceivedTsn = new cOutVector(str);

    snprintf(str, sizeof(str), "HB Sent %d:%s", assoc->assocId, addr.str().c_str());
    vectorPathHb = new cOutVector(str);
    snprintf(str, sizeof(str), "HB ACK Sent %d:%s", assoc->assocId, addr.str().c_str());
    vectorPathHbAck = new cOutVector(str);
    snprintf(str, sizeof(str), "HB Received %d:%s", assoc->assocId, addr.str().c_str());
    vectorPathRcvdHb = new cOutVector(str);
    snprintf(str, sizeof(str), "HB ACK Received %d:%s", assoc->assocId, addr.str().c_str());
    vectorPathRcvdHbAck = new cOutVector(str);

    snprintf(str, sizeof(str), "Queued Sent Bytes %d:%s", assoc->assocId, addr.str().c_str());
    statisticsPathQueuedSentBytes = new cOutVector(str);
    snprintf(str, sizeof(str), "Outstanding Bytes %d:%s", assoc->assocId, addr.str().c_str());
    statisticsPathOutstandingBytes = new cOutVector(str);
    snprintf(str, sizeof(str), "Sender Blocking Fraction %d:%s", assoc->assocId, addr.str().c_str());
    statisticsPathSenderBlockingFraction = new cOutVector(str);
    snprintf(str, sizeof(str), "Receiver Blocking Fraction %d:%s", assoc->assocId, addr.str().c_str());
    statisticsPathReceiverBlockingFraction = new cOutVector(str);
    snprintf(str, sizeof(str), "Number of Gap Acked Chunks in Last SACK %d:%s", assoc->assocId, addr.str().c_str());
    statisticsPathGapAckedChunksInLastSACK = new cOutVector(str);
    snprintf(str, sizeof(str), "Number of Non-Revokable Gap Acked Chunks in Last SACK %d:%s", assoc->assocId, addr.str().c_str());
    statisticsPathGapNRAckedChunksInLastSACK = new cOutVector(str);
    snprintf(str, sizeof(str), "Number of Gap Missed Chunks in Last SACK %d:%s", assoc->assocId, addr.str().c_str());
    statisticsPathGapUnackedChunksInLastSACK = new cOutVector(str);

    snprintf(str, sizeof(str), "Partial Bytes Acked %d:%s", assoc->assocId, addr.str().c_str());
    vectorPathPbAcked = new cOutVector(str);

    snprintf(str, sizeof(str), "Fast Recovery State %d:%s", assoc->assocId, addr.str().c_str());
    vectorPathFastRecoveryState = new cOutVector(str);
    vectorPathFastRecoveryState->record(0);

    snprintf(str, sizeof(str), "TSN Sent Fast RTX %d:%s", assoc->assocId, addr.str().c_str());
    vectorPathTsnFastRTX = new cOutVector(str);
    snprintf(str, sizeof(str), "TSN Sent Timer-Based RTX %d:%s", assoc->assocId, addr.str().c_str());
    vectorPathTsnTimerBased = new cOutVector(str);
    snprintf(str, sizeof(str), "TSN Acked CumAck %d:%s", assoc->assocId, addr.str().c_str());
    vectorPathAckedTsnCumAck = new cOutVector(str);
    snprintf(str, sizeof(str), "TSN Acked GapAck %d:%s", assoc->assocId, addr.str().c_str());
    vectorPathAckedTsnGapAck = new cOutVector(str);

    snprintf(str, sizeof(str), "TSN PseudoCumAck %d:%s", assoc->assocId, addr.str().c_str());
    vectorPathPseudoCumAck = new cOutVector(str);
    snprintf(str, sizeof(str), "TSN RTXPseudoCumAck %d:%s", assoc->assocId, addr.str().c_str());
    vectorPathRTXPseudoCumAck = new cOutVector(str);
    snprintf(str, sizeof(str), "Blocking TSNs Moved %d:%s", assoc->assocId, addr.str().c_str());
    vectorPathBlockingTsnsMoved = new cOutVector(str);
}

SctpPathVariables::~SctpPathVariables()
{
    delete statisticsPathSSthresh;
    delete statisticsPathCwnd;
    delete statisticsPathBandwidth;
    delete statisticsPathRTO;
    delete statisticsPathRTT;

    delete vectorPathSentTsn;
    delete vectorPathReceivedTsn;
    delete vectorPathHb;
    delete vectorPathRcvdHb;
    delete vectorPathHbAck;
    delete vectorPathRcvdHbAck;

    delete statisticsPathQueuedSentBytes;
    delete statisticsPathOutstandingBytes;
    delete statisticsPathGapAckedChunksInLastSACK;
    delete statisticsPathGapNRAckedChunksInLastSACK;
    delete statisticsPathGapUnackedChunksInLastSACK;
    delete statisticsPathSenderBlockingFraction;
    delete statisticsPathReceiverBlockingFraction;

    delete vectorPathPbAcked;
    delete vectorPathFastRecoveryState;

    delete vectorPathTsnFastRTX;
    delete vectorPathTsnTimerBased;
    delete vectorPathAckedTsnCumAck;
    delete vectorPathAckedTsnGapAck;

    delete vectorPathPseudoCumAck;
    delete vectorPathRTXPseudoCumAck;
    delete vectorPathBlockingTsnsMoved;
}

const L3Address SctpDataVariables::zeroAddress = L3Address();

SctpDataVariables::SctpDataVariables()
{
    userData = nullptr;
    ordered = true;
    len = 0;
    tsn = 0;
    sid = 0;
    ssn = 0;
    ppid = 0;
    fragments = 1;
    gapReports = 0;
    enqueuingTime = 0;
    sendTime = 0;
    expiryTime = 0;
    enqueuedInTransmissionQ = false;
    hasBeenAcked = false;
    hasBeenCountedAsNewlyAcked = false;
    hasBeenReneged = false;
    hasBeenAbandoned = false;
    hasBeenFastRetransmitted = false;
    countsAsOutstanding = false;
    ibit = false;
    queuedOnPath = nullptr;
    ackedOnPath = nullptr;
    hasBeenMoved = false;
    hasBeenTimerBasedRtxed = false;
    wasDropped = false;
    wasPktDropped = false;
    firstSendTime = 0;
    sendForwardIfAbandoned = false;
    lastDestination = nullptr;
    nextDestination = nullptr;
    initialDestination = nullptr;
    numberOfTransmissions = 0;
    numberOfRetransmissions = 0;
    booksize = 0;
    bbit = false;
    ebit = false;
    allowedNoRetransmissions = 0;
    strReset = false;
    prMethod = 0;
    priority = 0;
}

SctpDataVariables::~SctpDataVariables()
{
}

SctpStateVariables::SctpStateVariables()
{
    active = false;
    fork = false;
    initReceived = false;
    cookieEchoReceived = false;
    ackPointAdvanced = false;
    swsAvoidanceInvoked = false;
    firstChunkReceived = false;
    probingIsAllowed = false;
    zeroWindowProbing = true;
    alwaysBundleSack = true;
    fastRecoverySupported = true;
    reactivatePrimaryPath = false;
    newChunkReceived = false;
    dataChunkReceived = false;
    sackAllowed = false;
    sackAlreadySent = false;
    resetPending = false;
    resetRequested = false;
    stopReceiving = false;
    stopOldData = false;
    stopSending = false;
    stopReading = false;
    inOut = false;
    asconfOutstanding = false;
    streamReset = false;
    peerStreamReset = false;
    queueUpdate = false;
    firstDataSent = false;
    peerWindowFull = false;
    zeroWindow = false;
    padding = false;
    pktDropSent = false;
    peerPktDrop = false;
    appSendAllowed = true;
    noMoreOutstanding = false;
    fragInProgress = false;
    resetDeferred = false;
    bundleReset = false;
    waitForResponse = false;
    firstPeerRequest = true;
    incomingRequestSet = false;
    appLimited = false;
    requestsOverlap = false;
    primaryPath = nullptr;
    lastDataSourcePath = nullptr;
    resetChunk = nullptr;
    asconfChunk = nullptr;
    shutdownChunk = nullptr;
    shutdownAckChunk = nullptr;
    initChunk = nullptr;
    cookieChunk = nullptr;
    resetInfo = nullptr;
    incomingRequest = nullptr;
    peerRequestType = 0;
    localRequestType = 0;
    bytesToRetransmit = 0;
    initRexmitTimeout = SCTP_TIMEOUT_INIT_REXMIT;
    localRwnd = SCTP_DEFAULT_ARWND;
    errorCount = 0;
    initRetransCounter = 0;
    nextTsn = 0;
    chunksAdded = 0;
    dataChunksAdded = 0;
    packetBytes = 0;
    lastTsnAck = 0;
    highestTsnAcked = 0;
    nextRSid = 0;
    lastTsnBeforeReset = 0;
    advancedPeerAckPoint = 0;
    ackState = 0;
    lastStreamScheduled = 0;
    peerRwnd = 0;
    initialPeerRwnd = 0;
    assocPmtu = 0;
    fragPoint = 0;
    outstandingBytes = 0;
    messagesToPush = 1;
    pushMessagesLeft = 0;
    msgNum = 0;
    bytesRcvd = 0;
    sendBuffer = 0;
    queuedReceivedBytes = 0;
    prMethod = 0;
    assocThroughput = 0;
    queuedSentBytes = 0;
    queuedDroppableBytes = 0;
    numAddedOutStreams = 0;
    numAddedInStreams = 0;
    lastMsgWasFragment = false;
    enableHeartbeats = true;
    sendHeartbeatsOnActivePaths = false;
    sizeKeyVector = 0;
    sizePeerKeyVector = 0;
    auth = false;
    peerAuth = false;
    hmacType = 0;
    bufferedMessages = 0;
    initialPeerMsgRwnd = 0;
    localMsgRwnd = 0;
    peerMsgRwnd = 0;
    peerAllowsChunks = false;
    bytesToAddPerRcvdChunk = 0;
    bytesToAddPerPeerChunk = 0;
    tellArwnd = false;
    swsMsgInvoked = false;
    outstandingMessages = 0;
    ssNextStream = true;
    ssOneStreamLeft = false;
    ssLastDataChunkSizeSet = false;
    lastSendQueueAbated = simTime();
    queuedMessages = 0;
    queueLimit = 0;
    probingTimeout = 1;
    numRequests = 0;
    sendQueueLimit = 0;
    swsLimit = 0;
    authAdded = false;
    nrSack = false;
    gapReportLimit = 0;
    gapListOptimizationVariant = 0;
    smartOverfullSACKHandling = false;
    disableReneging = false;
    rtxMethod = 0;
    maxBurst = 0;
    sendResponse = 0;
    responseSn = 0;
    numResetRequests = 0;
    maxBurstVariant = SctpStateVariables::MBV_MaxBurst;
    initialWindow = 0;
    allowCMT = false;
    cmtSendAllComparisonFunction = nullptr;
    cmtRetransmissionVariant = nullptr;
    cmtCUCVariant = SctpStateVariables::CUCV_Normal;
    cmtBufferSplitVariant = SctpStateVariables::CBSV_None;
    cmtBufferSplittingUsesOSB = false;
    cmtChunkReschedulingVariant = SctpStateVariables::CCRV_None;
    cmtChunkReschedulingThreshold = 0.5;
    cmtSmartT3Reset = true;
    cmtSmartFastRTX = true;
    cmtSmartReneging = false;
    cmtSlowPathRTTUpdate = false;
    cmtUseSFR = true;
    numMsgsReq.resize(65536);

    for (unsigned int i = 0; i < 65536; i++) {
        numMsgsReq[i] = 0;
    }
    for (unsigned int i = 0; i < 32; i++) {
        localTieTag[i] = 0;
        peerTieTag[i] = 0;
    }

    count = 0;
    blockingTsnsMoved = 0;

    cmtUseDAC = true;
    cmtUseFRC = true;
    cmtMovedChunksReduceCwnd = true;
    movedChunkFastRTXFactor = 2.0;
    strictCwndBooking = false;
    cmtSackPath = CSP_Standard;
    highSpeedCC = false;
    cmtCCVariant = CCCV_Off;
    rpPathBlocking = false;
    rpScaleBlockingTimeout = false;
    rpMinCwnd = 1;
    checkSackSeqNumber = false;
    outgoingSackSeqNum = 0;
    incomingSackSeqNum = 0;
    asconfSn = 0;
    numberAsconfReceived = 0;
    corrIdNum = 0;
    streamResetSequenceNumber = 0;
    expectedStreamResetSequenceNumber = 0;
    peerRequestSn = 0;
    inRequestSn = 0;
    peerTsnAfterReset = 0;
    osbWithHeader = false;
    throughputInterval = 1.0;
}

SctpStateVariables::~SctpStateVariables()
{
    if (incomingRequestSet) {
        delete incomingRequest;
    }
    if (initChunk != nullptr)
        delete initChunk;
}

bool SctpStateVariables::findRequestNum(uint32_t num)
{
    return containsKey(requests, num);
}

bool SctpStateVariables::findPeerRequestNum(uint32_t num)
{
    return containsKey(peerRequests, num);
}

bool SctpStateVariables::findPeerStreamToReset(uint16_t num)
{
    std::list<uint16_t>::iterator it;
    for (it = peerStreamsToReset.begin(); it != peerStreamsToReset.end(); it++) {
        if ((*it) == num)
            return true;
    }
    return false;
}

bool SctpStateVariables::findMatch(uint16_t num)
{
    std::list<uint16_t>::iterator it;
    for (it = resetOutStreams.begin(); it != resetOutStreams.end(); it++) {
        if ((*it) == num)
            return true;
    }
    return false;
}

SctpStateVariables::RequestData *SctpStateVariables::findTypeInRequests(uint16_t type)
{
    for (auto& elem : requests) {
        if (elem.second.type == type) {
            return &(elem.second);
        }
    }
    return nullptr;
}

uint16_t SctpStateVariables::getNumRequestsNotPerformed()
{
    uint16_t count = 0;
    for (auto& elem : requests) {
        if (elem.second.result != PERFORMED && elem.second.result != DEFERRED) {
            count++;
        }
    }
    return count;
}

//
// FSM framework, SCTP FSM
//

SctpAssociation::SctpAssociation(Sctp *_module, int32_t _appGateIndex, int32_t _assocId, IRoutingTable *_rt, IInterfaceTable *_ift)
{
    // ====== Initialize variables ===========================================
    rt = _rt;
    ift = _ift;
    sctpMain = _module;
    appGateIndex = _appGateIndex;
    assocId = _assocId;
    listeningAssocId = -1;
    fd = -1;
    listening = false;
    localPort = 0;
    remotePort = 0;
    localVTag = 0;
    peerVTag = 0;
    numberOfRemoteAddresses = 0;
    inboundStreams = SCTP_DEFAULT_INBOUND_STREAMS;
    outboundStreams = SCTP_DEFAULT_OUTBOUND_STREAMS;
    initInboundStreams = 0;
    // queues and algorithm will be created on active or passive open
    transmissionQ = nullptr;
    retransmissionQ = nullptr;
    sctpAlgorithm = nullptr;
    state = nullptr;
    sackPeriod = SACK_DELAY;

    cumTsnAck = nullptr;
    sendQueue = nullptr;
    numGapBlocks = nullptr;

    qCounter.roomSumSendStreams = 0;
    qCounter.bookedSumSendStreams = 0;
    qCounter.roomSumRcvStreams = 0;
    bytes.chunk = false;
    bytes.packet = false;
    bytes.bytesToSend = 0;

    fairTimer = false;
    status = SCTP_S_CLOSED;
    initTsn = 0;
    initPeerTsn = 0;
    sackFrequency = 2;
    ccFunctions.ccInitParams = nullptr;
    ccFunctions.ccUpdateBeforeSack = nullptr;
    ccFunctions.ccUpdateAfterSack = nullptr;
    ccFunctions.ccUpdateAfterCwndTimeout = nullptr;
    ccFunctions.ccUpdateAfterRtxTimeout = nullptr;
    ccFunctions.ccUpdateMaxBurst = nullptr;
    ccFunctions.ccUpdateBytesAcked = nullptr;
    ccModule = 0;
    ssFunctions.ssInitStreams = nullptr;
    ssFunctions.ssAddInStreams = nullptr;
    ssFunctions.ssAddOutStreams = nullptr;
    ssFunctions.ssGetNextSid = nullptr;
    ssFunctions.ssUsableStreams = nullptr;

    EV_INFO << "SctpAssociationBase::SctpAssociation(): new assocId="
            << assocId << endl;

    // ====== FSM ============================================================
    char fsmName[64];
    snprintf(fsmName, sizeof(fsmName), "fsm-%d", assocId);
    fsm = new cFSM();
    fsm->setName(fsmName);
    fsm->setState(SCTP_S_CLOSED);

    // ====== Path Info ======================================================
    SctpPathInfo *pinfo = new SctpPathInfo("pathInfo");
    pinfo->setRemoteAddress(L3Address());

    // ====== Timers =========================================================
    char timerName[128];
    snprintf(timerName, sizeof(timerName), "T1_INIT of Association %d", assocId);
    T1_InitTimer = new cMessage(timerName);
    snprintf(timerName, sizeof(timerName), "T2_SHUTDOWN of Association %d", assocId);
    T2_ShutdownTimer = new cMessage(timerName);
    snprintf(timerName, sizeof(timerName), "T5_SHUTDOWN_GUARD of Association %d", assocId);
    T5_ShutdownGuardTimer = new cMessage(timerName);
    snprintf(timerName, sizeof(timerName), "SACK_TIMER of Association %d", assocId);
    SackTimer = new cMessage(timerName);

    StartTesting = nullptr;
    if (sctpMain->testTimeout > 0) {
        StartTesting = new cMessage("StartTesting");
        StartTesting->setContextPointer(this);
        StartTesting->setControlInfo(pinfo->dup());
        scheduleTimeout(StartTesting, sctpMain->testTimeout);
    }

    T1_InitTimer->setContextPointer(this);
    T2_ShutdownTimer->setContextPointer(this);
    SackTimer->setContextPointer(this);
    T5_ShutdownGuardTimer->setContextPointer(this);

    T1_InitTimer->setControlInfo(pinfo);
    T2_ShutdownTimer->setControlInfo(pinfo->dup());
    SackTimer->setControlInfo(pinfo->dup());
    T5_ShutdownGuardTimer->setControlInfo(pinfo->dup());

    // ====== Output vectors =================================================
    char vectorName[128];
    snprintf(vectorName, sizeof(vectorName), "Advertised Receiver Window %d", assocId);
    advRwnd = new cOutVector(vectorName);

    snprintf(vectorName, sizeof(vectorName), "Slow Start Threshold %d:Total", assocId);
    statisticsTotalSSthresh = new cOutVector(vectorName);
    snprintf(vectorName, sizeof(vectorName), "Congestion Window %d:Total", assocId);
    statisticsTotalCwnd = new cOutVector(vectorName);
    snprintf(vectorName, sizeof(vectorName), "Bandwidth %d:Total", assocId);
    statisticsTotalBandwidth = new cOutVector(vectorName);
    snprintf(vectorName, sizeof(vectorName), "Queued Received Bytes %d:Total", assocId);
    statisticsQueuedReceivedBytes = new cOutVector(vectorName);
    snprintf(vectorName, sizeof(vectorName), "Queued Sent Bytes %d:Total", assocId);
    statisticsQueuedSentBytes = new cOutVector(vectorName);
    snprintf(vectorName, sizeof(vectorName), "Outstanding Bytes %d:Total", assocId);
    statisticsOutstandingBytes = new cOutVector(vectorName);

    snprintf(vectorName, sizeof(vectorName), "Number of Revokable Gap Blocks in SACK %d", assocId);
    statisticsRevokableGapBlocksInLastSACK = new cOutVector(vectorName);
    snprintf(vectorName, sizeof(vectorName), "Number of Non-Revokable Gap Blocks in SACK %d", assocId);
    statisticsNonRevokableGapBlocksInLastSACK = new cOutVector(vectorName);

    snprintf(vectorName, sizeof(vectorName), "Number of Total Gap Blocks Stored %d", assocId);
    statisticsNumTotalGapBlocksStored = new cOutVector(vectorName);
    snprintf(vectorName, sizeof(vectorName), "Number of Revokable Gap Blocks Stored %d", assocId);
    statisticsNumRevokableGapBlocksStored = new cOutVector(vectorName);
    snprintf(vectorName, sizeof(vectorName), "Number of Non-Revokable Gap Blocks Stored %d", assocId);
    statisticsNumNonRevokableGapBlocksStored = new cOutVector(vectorName);
    snprintf(vectorName, sizeof(vectorName), "Number of Duplicate TSNs Stored %d", assocId);
    statisticsNumDuplicatesStored = new cOutVector(vectorName);

    snprintf(vectorName, sizeof(vectorName), "Number of Revokable Gap Blocks Sent %d", assocId);
    statisticsNumRevokableGapBlocksSent = new cOutVector(vectorName);
    snprintf(vectorName, sizeof(vectorName), "Number of Non-Revokable Gap Blocks Sent %d", assocId);
    statisticsNumNonRevokableGapBlocksSent = new cOutVector(vectorName);
    snprintf(vectorName, sizeof(vectorName), "Number of Duplicate TSNs Sent %d", assocId);
    statisticsNumDuplicatesSent = new cOutVector(vectorName);
    snprintf(vectorName, sizeof(vectorName), "Length of SACK Sent %d", assocId);
    statisticsSACKLengthSent = new cOutVector(vectorName);

    snprintf(vectorName, sizeof(vectorName), "Arwnd in Last SACK %d", assocId);
    statisticsArwndInLastSACK = new cOutVector(vectorName);
    snprintf(vectorName, sizeof(vectorName), "Peer Rwnd %d", assocId);
    statisticsPeerRwnd = new cOutVector(vectorName);

    // ====== Extensions =====================================================
    StartAddIP = new cMessage("addIP");
    StartAddIP->setContextPointer(this);
    StartAddIP->setControlInfo(pinfo->dup());
    FairStartTimer = new cMessage("fairStart");
    FairStartTimer->setContextPointer(this);
    FairStartTimer->setControlInfo(pinfo->dup());
    FairStopTimer = new cMessage("fairStop");
    FairStopTimer->setContextPointer(this);
    FairStopTimer->setControlInfo(pinfo->dup());
    snprintf(vectorName, sizeof(vectorName), "Advertised Message Receiver Window of Association %d", assocId);
    advMsgRwnd = new cOutVector(vectorName);
    snprintf(vectorName, sizeof(vectorName), "End to End Delay of Association %d", assocId);
    EndToEndDelay = new cOutVector(vectorName);

    // ====== Assoc throughput ===============================================
    snprintf(vectorName, sizeof(vectorName), "Throughput of Association %d", assocId);
    assocThroughputVector = new cOutVector(vectorName);
    assocThroughputVector->record(0.0);

    // ====== CMT Delayed Ack ================================================
    dacPacketsRcvd = 0;

    // ====== Stream scheduling ==============================================
    ssModule = sctpMain->par("ssModule");

    switch (ssModule) {
        case ROUND_ROBIN:
            ssFunctions.ssInitStreams = &SctpAssociation::initStreams;
            ssFunctions.ssAddInStreams = &SctpAssociation::addInStreams;
            ssFunctions.ssAddOutStreams = &SctpAssociation::addOutStreams;
            ssFunctions.ssGetNextSid = &SctpAssociation::streamScheduler;
            ssFunctions.ssUsableStreams = &SctpAssociation::numUsableStreams;
            EV_DETAIL << "Setting Stream Scheduler: ROUND_ROBIN" << endl;
            break;

        case ROUND_ROBIN_PACKET:
            ssFunctions.ssInitStreams = &SctpAssociation::initStreams;
            ssFunctions.ssAddInStreams = &SctpAssociation::addInStreams;
            ssFunctions.ssAddOutStreams = &SctpAssociation::addOutStreams;
            ssFunctions.ssGetNextSid = &SctpAssociation::streamSchedulerRoundRobinPacket;
            ssFunctions.ssUsableStreams = &SctpAssociation::numUsableStreams;
            EV_DETAIL << "Setting Stream Scheduler: ROUND_ROBIN_PACKET" << endl;
            break;

        case RANDOM_SCHEDULE:
            ssFunctions.ssInitStreams = &SctpAssociation::initStreams;
            ssFunctions.ssAddInStreams = &SctpAssociation::addInStreams;
            ssFunctions.ssAddOutStreams = &SctpAssociation::addOutStreams;
            ssFunctions.ssGetNextSid = &SctpAssociation::streamSchedulerRandom;
            ssFunctions.ssUsableStreams = &SctpAssociation::numUsableStreams;
            EV_DETAIL << "Setting Stream Scheduler: RANDOM_SCHEDULE" << endl;
            break;

        case RANDOM_SCHEDULE_PACKET:
            ssFunctions.ssInitStreams = &SctpAssociation::initStreams;
            ssFunctions.ssAddInStreams = &SctpAssociation::addInStreams;
            ssFunctions.ssAddOutStreams = &SctpAssociation::addOutStreams;
            ssFunctions.ssGetNextSid = &SctpAssociation::streamSchedulerRandomPacket;
            ssFunctions.ssUsableStreams = &SctpAssociation::numUsableStreams;
            EV_DETAIL << "Setting Stream Scheduler: RANDOM_SCHEDULE_PACKET" << endl;
            break;

        case FAIR_BANDWITH:
            ssFunctions.ssInitStreams = &SctpAssociation::initStreams;
            ssFunctions.ssAddInStreams = &SctpAssociation::addInStreams;
            ssFunctions.ssAddOutStreams = &SctpAssociation::addOutStreams;
            ssFunctions.ssGetNextSid = &SctpAssociation::streamSchedulerFairBandwidth;
            ssFunctions.ssUsableStreams = &SctpAssociation::numUsableStreams;
            EV_DETAIL << "Setting Stream Scheduler: FAIR_BANDWITH" << endl;
            break;

        case FAIR_BANDWITH_PACKET:
            ssFunctions.ssInitStreams = &SctpAssociation::initStreams;
            ssFunctions.ssAddInStreams = &SctpAssociation::addInStreams;
            ssFunctions.ssAddOutStreams = &SctpAssociation::addOutStreams;
            ssFunctions.ssGetNextSid = &SctpAssociation::streamSchedulerFairBandwidthPacket;
            ssFunctions.ssUsableStreams = &SctpAssociation::numUsableStreams;
            EV_DETAIL << "Setting Stream Scheduler: FAIR_BANDWITH_PACKET" << endl;
            break;

        case PRIORITY:
            ssFunctions.ssInitStreams = &SctpAssociation::initStreams;
            ssFunctions.ssAddInStreams = &SctpAssociation::addInStreams;
            ssFunctions.ssAddOutStreams = &SctpAssociation::addOutStreams;
            ssFunctions.ssGetNextSid = &SctpAssociation::streamSchedulerPriority;
            ssFunctions.ssUsableStreams = &SctpAssociation::numUsableStreams;
            EV_DETAIL << "Setting Stream Scheduler: PRIORITY" << endl;
            break;

        case FCFS:
            ssFunctions.ssInitStreams = &SctpAssociation::initStreams;
            ssFunctions.ssGetNextSid = &SctpAssociation::streamSchedulerFCFS;
            ssFunctions.ssUsableStreams = &SctpAssociation::numUsableStreams;
            EV_DETAIL << "Setting Stream Scheduler: FCFS" << endl;
            break;

        case PATH_MANUAL:
            ssFunctions.ssInitStreams = &SctpAssociation::initStreams;
            ssFunctions.ssAddInStreams = &SctpAssociation::addInStreams;
            ssFunctions.ssAddOutStreams = &SctpAssociation::addOutStreams;
            ssFunctions.ssGetNextSid = &SctpAssociation::pathStreamSchedulerManual;
            ssFunctions.ssUsableStreams = &SctpAssociation::numUsableStreams;
            EV_DETAIL << "Setting Stream Scheduler: PATH_MANUAL" << endl;
            break;

        case PATH_MAP_TO_PATH:
            ssFunctions.ssInitStreams = &SctpAssociation::initStreams;
            ssFunctions.ssAddInStreams = &SctpAssociation::addInStreams;
            ssFunctions.ssAddOutStreams = &SctpAssociation::addOutStreams;
            ssFunctions.ssGetNextSid = &SctpAssociation::pathStreamSchedulerMapToPath;
            ssFunctions.ssUsableStreams = &SctpAssociation::numUsableStreams;
            EV_DETAIL << "Setting Stream Scheduler: PATH_MAP_TO_PATH" << endl;
            break;
    }
}

SctpAssociation::~SctpAssociation()
{
    EV_TRACE << "Destructor SctpAssociation " << assocId << endl;

    delete T1_InitTimer;
    delete T2_ShutdownTimer;
    delete T5_ShutdownGuardTimer;
    delete SackTimer;

    delete advRwnd;
    delete cumTsnAck;
    delete numGapBlocks;
    delete sendQueue;

    delete statisticsOutstandingBytes;
    delete statisticsQueuedReceivedBytes;
    delete statisticsQueuedSentBytes;
    delete statisticsTotalSSthresh;
    delete statisticsTotalCwnd;
    delete statisticsTotalBandwidth;

    delete statisticsRevokableGapBlocksInLastSACK;
    delete statisticsNonRevokableGapBlocksInLastSACK;
    delete statisticsNumTotalGapBlocksStored;
    delete statisticsNumRevokableGapBlocksStored;
    delete statisticsNumNonRevokableGapBlocksStored;
    delete statisticsNumDuplicatesStored;
    delete statisticsNumRevokableGapBlocksSent;
    delete statisticsNumNonRevokableGapBlocksSent;
    delete statisticsNumDuplicatesSent;
    delete statisticsSACKLengthSent;

    delete statisticsArwndInLastSACK;
    delete statisticsPeerRwnd;

    delete StartAddIP;
    delete advMsgRwnd;
    delete EndToEndDelay;

    int i = 0;
    while (streamThroughputVectors[i] != nullptr) {
        delete streamThroughputVectors[i++];
    }
    if (assocThroughputVector != nullptr)
        delete assocThroughputVector;
    if (FairStartTimer)
        delete cancelEvent(FairStartTimer);
    if (FairStopTimer)
        delete cancelEvent(FairStopTimer);

    if (state->asconfOutstanding && state->asconfChunk)
        delete state->asconfChunk;

    delete fsm;
    delete state;
    delete sctpAlgorithm;
}

bool SctpAssociation::processTimer(cMessage *msg)
{
    SctpPathVariables *path = nullptr;

    EV_INFO << msg->getName() << " timer expired at " << simTime() << "\n";

    SctpPathInfo *pinfo = check_and_cast<SctpPathInfo *>(msg->getControlInfo());
    L3Address addr = pinfo->getRemoteAddress();

    if (!addr.isUnspecified())
        path = getPath(addr);

    // first do actions
    SctpEventCode event;
    event = SCTP_E_IGNORE;

    if (msg == T1_InitTimer) {
        process_TIMEOUT_INIT_REXMIT(event);
    }
    else if (msg == SackTimer) {
        EV_DETAIL << simTime() << " delayed Sack: cTsnAck=" << state->gapList.getCumAckTsn() << " highestTsnReceived=" << state->gapList.getHighestTsnReceived() << " lastTsnReceived=" << state->lastTsnReceived << " ackState=" << state->ackState << " numGaps=" << state->gapList.getNumGaps(SctpGapList::GT_Any) << "\n";
        sendSack();
    }
    else if (msg == T2_ShutdownTimer) {
        stopTimer(T2_ShutdownTimer);
        process_TIMEOUT_SHUTDOWN(event);
    }
    else if (msg == T5_ShutdownGuardTimer) {
        stopTimer(T5_ShutdownGuardTimer);
        if (state->shutdownChunk) {
            delete state->shutdownChunk;
            state->shutdownChunk = nullptr;
        }
        sendIndicationToApp(SCTP_I_CONN_LOST);
        sendAbort();
        sctpMain->removeAssociation(this);
        return true;
    }
    else if (path != nullptr && msg == path->HeartbeatIntervalTimer) {
        process_TIMEOUT_HEARTBEAT_INTERVAL(path, path->forceHb);
    }
    else if (path != nullptr && msg == path->HeartbeatTimer) {
        process_TIMEOUT_HEARTBEAT(path);
    }
    else if (path != nullptr && msg == path->T3_RtxTimer) {
        process_TIMEOUT_RTX(path);
    }
    else if (path != nullptr && msg == path->CwndTimer) {
        (this->*ccFunctions.ccUpdateAfterCwndTimeout)(path);
    }
    else if (strcmp(msg->getName(), "StartTesting") == 0) {
//        if (sctpMain->testing == false)
//        {
//            sctpMain->testing = true;
        EV_DEBUG << "set testing to true\n";
//        }
        // todo: testing was removed.
    }
    else if (path != nullptr && msg == path->ResetTimer) {
        process_TIMEOUT_RESET(path);
    }
    else if (path != nullptr && msg == path->AsconfTimer) {
        process_TIMEOUT_ASCONF(path);
    }
    else if (msg == StartAddIP) {
        state->corrIdNum = state->asconfSn;
        const char *type = sctpMain->par("addIpType").stringValue();
        sendAsconf(type);
    }
    else if (msg == FairStartTimer) {
        auto it = sctpMain->assocStatMap.find(assocId);
        if (it != sctpMain->assocStatMap.end()) {
            it->second.fairStart = simTime();
            fairTimer = true;
        }
    }
    else if (msg == FairStopTimer) {
        auto it = sctpMain->assocStatMap.find(assocId);
        if (it != sctpMain->assocStatMap.end()) {
            it->second.fairStop = simTime();
            it->second.fairLifeTime = it->second.fairStop - it->second.fairStart;
            it->second.fairThroughput = it->second.fairAckedBytes / it->second.fairLifeTime.dbl();
            fairTimer = false;
        }
    }
    else {
        sctpAlgorithm->processTimer(msg, event);
    }

    // then state transitions
    return performStateTransition(event);
}

// bool SctpAssociation::processSctpMessage(const Ptr<const SctpHeader>& sctpmsg,
bool SctpAssociation::processSctpMessage(SctpHeader *sctpmsg,
        const L3Address& msgSrcAddr,
        const L3Address& msgDestAddr)
{
    printAssocBrief();
    localAddr = msgDestAddr;
    localPort = sctpmsg->getDestPort();
    remoteAddr = msgSrcAddr;
    remotePort = sctpmsg->getSrcPort();

    if (fsm->getState() == SCTP_S_ESTABLISHED) {
        bool found = false;
        for (auto& elem : state->localAddresses) {
            if ((elem) == msgDestAddr) {
                found = true;
                break;
            }
        }
        if (!found) {
            EV_INFO << "destAddr " << msgDestAddr << " is not bound to host\n";
            return true;
        }
    }

    return process_RCV_Message(sctpmsg, msgSrcAddr, msgDestAddr);
}

SctpEventCode SctpAssociation::preanalyseAppCommandEvent(int32_t commandCode)
{
    switch (commandCode) {
        case SCTP_C_ASSOCIATE:
            return SCTP_E_ASSOCIATE;

        case SCTP_C_OPEN_PASSIVE:
            return SCTP_E_OPEN_PASSIVE;

        case SCTP_C_SEND:
            return SCTP_E_SEND;

        case SCTP_C_CLOSE:
            return SCTP_E_CLOSE;

        case SCTP_C_ABORT:
            return SCTP_E_ABORT;

        case SCTP_C_RECEIVE:
            return SCTP_E_RECEIVE;

        case SCTP_C_SEND_UNORDERED:
            return SCTP_E_SEND;

        case SCTP_C_SEND_ORDERED:
            return SCTP_E_SEND;

        case SCTP_C_PRIMARY:
            return SCTP_E_PRIMARY;

        case SCTP_C_QUEUE_MSGS_LIMIT:
            return SCTP_E_QUEUE_MSGS_LIMIT;

        case SCTP_C_QUEUE_BYTES_LIMIT:
            return SCTP_E_QUEUE_BYTES_LIMIT;

        case SCTP_C_SHUTDOWN:
            return SCTP_E_SHUTDOWN;

        case SCTP_C_NO_OUTSTANDING:
            return SCTP_E_SEND_SHUTDOWN_ACK;

        case SCTP_C_STREAM_RESET:
            return SCTP_E_STREAM_RESET;

        case SCTP_C_RESET_ASSOC:
            return SCTP_E_RESET_ASSOC;

        case SCTP_C_ADD_STREAMS:
            return SCTP_E_ADD_STREAMS;

        case SCTP_C_SEND_ASCONF:
            return SCTP_E_SEND_ASCONF; // Needed for multihomed NAT

        case SCTP_C_SET_STREAM_PRIO:
            return SCTP_E_SET_STREAM_PRIO;

        case SCTP_C_ACCEPT:
            return SCTP_E_ACCEPT;

        case SCTP_C_ACCEPT_SOCKET_ID:
            return SCTP_E_ACCEPT_SOCKET_ID;

        case SCTP_C_SET_RTO_INFO:
            return SCTP_E_SET_RTO_INFO;

        default:
            EV_DETAIL << "commandCode=" << commandCode << "\n";
            throw cRuntimeError("Unknown message kind in app command");
    }
}

bool SctpAssociation::processAppCommand(cMessage *msg, SctpCommandReq *sctpCommand)
{
    printAssocBrief();

    SctpEventCode event = preanalyseAppCommandEvent(msg->getKind());

    EV_INFO << "App command: " << eventName(event) << "\n";

    switch (event) {
        case SCTP_E_ASSOCIATE:
            process_ASSOCIATE(event, sctpCommand, msg);
            break;

        case SCTP_E_OPEN_PASSIVE:
            process_OPEN_PASSIVE(event, sctpCommand, msg);
            break;

        case SCTP_E_SEND:
            process_SEND(event, sctpCommand, msg);
            break;

        case SCTP_E_ABORT:
            process_ABORT(event);
            break;

        case SCTP_E_RECEIVE:
            process_RECEIVE_REQUEST(event, sctpCommand);
            break;

        case SCTP_E_PRIMARY:
            process_PRIMARY(event, sctpCommand);
            break;

        case SCTP_E_STREAM_RESET:
        case SCTP_E_RESET_ASSOC:
        case SCTP_E_ADD_STREAMS:
            if (state->peerStreamReset == true) {
                process_STREAM_RESET(sctpCommand);
            }
            event = SCTP_E_IGNORE;
            break;

        case SCTP_E_SEND_ASCONF:
            sendAsconf(sctpMain->par("addIpType"));
            break;

        case SCTP_E_SET_STREAM_PRIO: {
            auto sctpSendReq = check_and_cast<SctpSendReq *>(sctpCommand);
            state->ssPriorityMap[sctpSendReq->getSid()] = sctpSendReq->getPpid();
            break;
        }

        case SCTP_E_QUEUE_BYTES_LIMIT:
            process_QUEUE_BYTES_LIMIT(sctpCommand);
            break;

        case SCTP_E_QUEUE_MSGS_LIMIT:
            process_QUEUE_MSGS_LIMIT(sctpCommand);
            break;

        case SCTP_E_CLOSE:
            if (listening) {
                event = SCTP_E_IGNORE;
                break;
            }
            state->stopReading = true;
            /* fall through */

        case SCTP_E_SHUTDOWN: /*sendShutdown*/
            EV_INFO << "SCTP_E_SHUTDOWN in state " << stateName(fsm->getState()) << "\n";

            if (fsm->getState() == SCTP_S_SHUTDOWN_RECEIVED) {
                EV_INFO << "send shutdown ack\n";
                sendShutdownAck(remoteAddr);
            }
            break;

        case SCTP_E_STOP_SENDING:
            break;

        case SCTP_E_SEND_SHUTDOWN_ACK:
            break;

        case SCTP_E_ACCEPT:
            fd = sctpCommand->getFd();
            EV_DETAIL << "Accepted fd " << fd << " for assoc " << assocId << endl;
            break;

        case SCTP_E_ACCEPT_SOCKET_ID:
            sendEstabIndicationToApp();
            break;

        case SCTP_E_SET_RTO_INFO: {
            auto sctpRtoReq = check_and_cast<SctpRtoReq *>(sctpCommand);
            sctpMain->setRtoInitial(sctpRtoReq->getRtoInitial());
            sctpMain->setRtoMin(sctpRtoReq->getRtoMin());
            sctpMain->setRtoMax(sctpRtoReq->getRtoMax());
            break;
        }
        default:
            throw cRuntimeError("Wrong event code");
    }

//    delete sctpCommand;
    // then state transitions
    return performStateTransition(event);
}

bool SctpAssociation::performStateTransition(const SctpEventCode& event)
{
    EV_TRACE << "performStateTransition\n";

    if (event == SCTP_E_IGNORE) { // e.g. discarded segment
        EV_DETAIL << "Staying in state: " << stateName(fsm->getState()) << " (no FSM event)\n";
        return true;
    }

    // state machine
    int32_t oldState = fsm->getState();

    switch (fsm->getState()) {
        case SCTP_S_CLOSED:
            switch (event) {
                case SCTP_E_ABORT:
                    FSM_Goto((*fsm), SCTP_S_CLOSED);
                    break;

                case SCTP_E_OPEN_PASSIVE:
                    FSM_Goto((*fsm), SCTP_S_CLOSED);
                    break;

                case SCTP_E_ASSOCIATE:
                    FSM_Goto((*fsm), SCTP_S_COOKIE_WAIT);
                    break;

                case SCTP_E_RCV_INIT:
                    FSM_Goto((*fsm), SCTP_S_CLOSED);
                    break;

                case SCTP_E_RCV_ABORT:
                    FSM_Goto((*fsm), SCTP_S_CLOSED);
                    break;

                case SCTP_E_RCV_VALID_COOKIE_ECHO:
                    FSM_Goto((*fsm), SCTP_S_ESTABLISHED);
                    break;

                case SCTP_E_CLOSE:
                    FSM_Goto((*fsm), SCTP_S_CLOSED);
                    break;

                default:
                    break;
            }
            break;

        case SCTP_S_COOKIE_WAIT:
            switch (event) {
                case SCTP_E_RCV_ABORT:
                    FSM_Goto((*fsm), SCTP_S_CLOSED);
                    break;

                case SCTP_E_ABORT:
                    FSM_Goto((*fsm), SCTP_S_CLOSED);
                    break;

                case SCTP_E_RCV_INIT_ACK:
                    FSM_Goto((*fsm), SCTP_S_COOKIE_ECHOED);
                    break;

                case SCTP_E_RCV_VALID_COOKIE_ECHO:
                    FSM_Goto((*fsm), SCTP_S_ESTABLISHED);
                    break;

                default:
                    break;
            }
            break;

        case SCTP_S_COOKIE_ECHOED:
            switch (event) {
                case SCTP_E_RCV_ABORT:
                    FSM_Goto((*fsm), SCTP_S_CLOSED);
                    break;

                case SCTP_E_ABORT:
                    FSM_Goto((*fsm), SCTP_S_CLOSED);
                    break;

                case SCTP_E_RCV_COOKIE_ACK:
                    FSM_Goto((*fsm), SCTP_S_ESTABLISHED);
//                    sendEstabIndicationToApp();
                    break;

                default:
                    break;
            }
            break;

        case SCTP_S_ESTABLISHED:
            switch (event) {
                case SCTP_E_SEND:
                    FSM_Goto((*fsm), SCTP_S_ESTABLISHED);
                    break;

                case SCTP_E_ABORT:
                    FSM_Goto((*fsm), SCTP_S_CLOSED);
                    break;

                case SCTP_E_RCV_ABORT:
                    FSM_Goto((*fsm), SCTP_S_CLOSED);
                    break;

                case SCTP_E_CLOSE:
                case SCTP_E_SHUTDOWN:
                    FSM_Goto((*fsm), SCTP_S_SHUTDOWN_PENDING);
                    break;

                case SCTP_E_STOP_SENDING:
                    FSM_Goto((*fsm), SCTP_S_SHUTDOWN_PENDING);
                    state->stopSending = true;
                    state->lastTsn = state->nextTsn - 1;
                    break;

                case SCTP_E_RCV_SHUTDOWN:
                    FSM_Goto((*fsm), SCTP_S_SHUTDOWN_RECEIVED);
                    break;

                default:
                    break;
            }
            break;

        case SCTP_S_SHUTDOWN_PENDING:
            switch (event) {
                case SCTP_E_RCV_ABORT:
                    FSM_Goto((*fsm), SCTP_S_CLOSED);
                    break;

                case SCTP_E_ABORT:
                    FSM_Goto((*fsm), SCTP_S_CLOSED);
                    break;

                case SCTP_E_NO_MORE_OUTSTANDING:
                    FSM_Goto((*fsm), SCTP_S_SHUTDOWN_SENT);
                    break;

                case SCTP_E_RCV_SHUTDOWN:
                    FSM_Goto((*fsm), SCTP_S_SHUTDOWN_RECEIVED);
                    break;

                case SCTP_E_RCV_SHUTDOWN_ACK:
                    FSM_Goto((*fsm), SCTP_S_CLOSED);
                    break;

                default:
                    break;
            }
            break;

        case SCTP_S_SHUTDOWN_RECEIVED:
            switch (event) {
                case SCTP_E_ABORT:
                    FSM_Goto((*fsm), SCTP_S_CLOSED);
                    break;

                case SCTP_E_RCV_ABORT:
                    FSM_Goto((*fsm), SCTP_S_CLOSED);
                    break;

                case SCTP_E_NO_MORE_OUTSTANDING:
                    FSM_Goto((*fsm), SCTP_S_SHUTDOWN_ACK_SENT);
                    break;

                case SCTP_E_SHUTDOWN:
                    sendShutdownAck(remoteAddr);
                    break;

                default:
                    break;
            }
            break;

        case SCTP_S_SHUTDOWN_SENT:
            switch (event) {
                case SCTP_E_ABORT:
                    FSM_Goto((*fsm), SCTP_S_CLOSED);
                    break;

                case SCTP_E_RCV_ABORT:
                    FSM_Goto((*fsm), SCTP_S_CLOSED);
                    break;

                case SCTP_E_RCV_SHUTDOWN_ACK:
                    FSM_Goto((*fsm), SCTP_S_CLOSED);
                    break;

                case SCTP_E_RCV_SHUTDOWN:
                    sendShutdownAck(remoteAddr);
                    FSM_Goto((*fsm), SCTP_S_SHUTDOWN_ACK_SENT);
                    break;

                default:
                    break;
            }
            break;

        case SCTP_S_SHUTDOWN_ACK_SENT:
            switch (event) {
                case SCTP_E_ABORT:
                    FSM_Goto((*fsm), SCTP_S_CLOSED);
                    break;

                case SCTP_E_RCV_ABORT:
                    FSM_Goto((*fsm), SCTP_S_CLOSED);
                    break;

                case SCTP_E_RCV_SHUTDOWN_COMPLETE:
                    FSM_Goto((*fsm), SCTP_S_CLOSED);
                    break;

                default:
                    break;
            }
            break;
    }

    if (oldState != fsm->getState()) {
        EV_DETAIL << "Transition: " << stateName(oldState) << " --> " << stateName(fsm->getState())
                  << "    (event was: " << eventName(event) << ")\n";
        EV_DETAIL << sctpMain->getName() << ": " << stateName(oldState) << " --> "
                  << stateName(fsm->getState()) << "  (on " << eventName(event) << ")\n";
        stateEntered(fsm->getState());
    }
    else {
        EV << "Staying in state: " << stateName(fsm->getState())
           << " (event was: " << eventName(event) << ")\n";
    }

    if (event == SCTP_E_ABORT && oldState == fsm->getState() && fsm->getState() == SCTP_S_CLOSED)
        return true;

    if (oldState != fsm->getState() && fsm->getState() == SCTP_S_CLOSED) {
        EV_DETAIL << "return false because oldState=" << oldState << " and new state is closed\n";
        return false;
    }
    else
        return true;
}

void SctpAssociation::stateEntered(int32_t status)
{
    switch (status) {
        case SCTP_S_COOKIE_WAIT:
            break;

        case SCTP_S_ESTABLISHED: {
            EV_INFO << "State ESTABLISHED entered" << endl;
            stopTimer(T1_InitTimer);

            if (state->initChunk) {
                delete state->initChunk;
                state->initChunk = nullptr;
            }

            state->nagleEnabled = (bool)sctpMain->getNagle();
            state->enableHeartbeats = sctpMain->getEnableHeartbeats();
            state->sendHeartbeatsOnActivePaths = sctpMain->par("sendHeartbeatsOnActivePaths");
            state->numGapReports = sctpMain->par("numGapReports");
            state->maxBurst = (uint32_t)sctpMain->getMaxBurst();
            state->rtxMethod = sctpMain->par("RTXMethod");
            state->nrSack = sctpMain->par("nrSack");
            state->disableReneging = sctpMain->par("disableReneging");
            state->checkSackSeqNumber = sctpMain->par("checkSackSeqNumber");
            state->outgoingSackSeqNum = 0;
            state->incomingSackSeqNum = 0;
            state->fragPoint = (uint32_t)sctpMain->getFragPoint();
            state->highSpeedCC = sctpMain->par("highSpeedCC");
            state->initialWindow = sctpMain->par("initialWindow");
            const char *maxBurstVariantPar = sctpMain->par("maxBurstVariant").stringValue();
            if (strcmp(maxBurstVariantPar, "useItOrLoseIt") == 0) {
                state->maxBurstVariant = SctpStateVariables::MBV_UseItOrLoseIt;
            }
            else if (strcmp(maxBurstVariantPar, "congestionWindowLimiting") == 0) {
                state->maxBurstVariant = SctpStateVariables::MBV_CongestionWindowLimiting;
            }
            else if (strcmp(maxBurstVariantPar, "maxBurst") == 0) {
                state->maxBurstVariant = SctpStateVariables::MBV_MaxBurst;
            }
            else if (strcmp(maxBurstVariantPar, "aggressiveMaxBurst") == 0) {
                state->maxBurstVariant = SctpStateVariables::MBV_AggressiveMaxBurst;
            }
            else if (strcmp(maxBurstVariantPar, "totalMaxBurst") == 0) {
                state->maxBurstVariant = SctpStateVariables::MBV_TotalMaxBurst;
            }
            else if (strcmp(maxBurstVariantPar, "useItOrLoseItTempCwnd") == 0) {
                state->maxBurstVariant = SctpStateVariables::MBV_UseItOrLoseItTempCwnd;
            }
            else if (strcmp(maxBurstVariantPar, "congestionWindowLimitingTempCwnd") == 0) {
                state->maxBurstVariant = SctpStateVariables::MBV_CongestionWindowLimitingTempCwnd;
            }
            else {
                throw cRuntimeError("Invalid setting of maxBurstVariant: %s.",
                        maxBurstVariantPar);
            }

            const char *cmtSendAllVariantPar = sctpMain->par("cmtSendAllVariant").stringValue();
            if (strcmp(cmtSendAllVariantPar, "normal") == 0) {
                state->cmtSendAllComparisonFunction = nullptr;
            }
            else if (strcmp(cmtSendAllVariantPar, "smallestLastTransmission") == 0) {
                state->cmtSendAllComparisonFunction = pathMapSmallestLastTransmission;
            }
            else if (strcmp(cmtSendAllVariantPar, "randomized") == 0) {
                state->cmtSendAllComparisonFunction = pathMapRandomized;
            }
            else if (strcmp(cmtSendAllVariantPar, "largestSSThreshold") == 0) {
                state->cmtSendAllComparisonFunction = pathMapLargestSSThreshold;
            }
            else if (strcmp(cmtSendAllVariantPar, "largestSpace") == 0) {
                state->cmtSendAllComparisonFunction = pathMapLargestSpace;
            }
            else if (strcmp(cmtSendAllVariantPar, "largestSpaceAndSSThreshold") == 0) {
                state->cmtSendAllComparisonFunction = pathMapLargestSpaceAndSSThreshold;
            }
            else {
                throw cRuntimeError("Invalid setting of cmtSendAllVariant: %s.",
                        cmtSendAllVariantPar);
            }

            state->cmtRetransmissionVariant = sctpMain->par("cmtRetransmissionVariant");
            const char *cmtCUCVariantPar = sctpMain->par("cmtCUCVariant").stringValue();
            if (strcmp(cmtCUCVariantPar, "normal") == 0) {
                state->cmtCUCVariant = SctpStateVariables::CUCV_Normal;
            }
            else if (strcmp(cmtCUCVariantPar, "pseudoCumAck") == 0) {
                state->cmtCUCVariant = SctpStateVariables::CUCV_PseudoCumAck;
            }
            else if (strcmp(cmtCUCVariantPar, "pseudoCumAckV2") == 0) {
                state->cmtCUCVariant = SctpStateVariables::CUCV_PseudoCumAckV2;
            }
            else {
                throw cRuntimeError("Bad setting for cmtCUCVariant: %s\n",
                        cmtCUCVariantPar);
            }
            state->smartOverfullSACKHandling = sctpMain->par("smartOverfullSACKHandling");

            const char *cmtChunkReschedulingVariantPar = sctpMain->par("cmtChunkReschedulingVariant").stringValue();
            if (strcmp(cmtChunkReschedulingVariantPar, "none") == 0) {
                state->cmtChunkReschedulingVariant = SctpStateVariables::CCRV_None;
            }
            else if (strcmp(cmtChunkReschedulingVariantPar, "senderOnly") == 0) {
                state->cmtChunkReschedulingVariant = SctpStateVariables::CCRV_SenderOnly;
            }
            else if (strcmp(cmtChunkReschedulingVariantPar, "receiverOnly") == 0) {
                state->cmtChunkReschedulingVariant = SctpStateVariables::CCRV_ReceiverOnly;
            }
            else if (strcmp(cmtChunkReschedulingVariantPar, "bothSides") == 0) {
                state->cmtChunkReschedulingVariant = SctpStateVariables::CCRV_BothSides;
            }
            else if (strcmp(cmtChunkReschedulingVariantPar, "test") == 0) {
                state->cmtChunkReschedulingVariant = SctpStateVariables::CCRV_Test;
            }
            else {
                throw cRuntimeError("Bad setting for cmtChunkReschedulingVariant: %s\n",
                        cmtChunkReschedulingVariantPar);
            }

            const char *cmtBufferSplitVariantPar = sctpMain->par("cmtBufferSplitVariant").stringValue();
            if (strcmp(cmtBufferSplitVariantPar, "none") == 0) {
                state->cmtBufferSplitVariant = SctpStateVariables::CBSV_None;
            }
            else if (strcmp(cmtBufferSplitVariantPar, "senderOnly") == 0) {
                state->cmtBufferSplitVariant = SctpStateVariables::CBSV_SenderOnly;
            }
            else if (strcmp(cmtBufferSplitVariantPar, "receiverOnly") == 0) {
                state->cmtBufferSplitVariant = SctpStateVariables::CBSV_ReceiverOnly;
            }
            else if (strcmp(cmtBufferSplitVariantPar, "bothSides") == 0) {
                state->cmtBufferSplitVariant = SctpStateVariables::CBSV_BothSides;
            }
            else {
                throw cRuntimeError("Bad setting for cmtBufferSplitVariant: %s\n",
                        cmtBufferSplitVariantPar);
            }
            state->cmtBufferSplittingUsesOSB = sctpMain->par("cmtBufferSplittingUsesOSB");

            const char *gapListOptimizationVariantPar = sctpMain->par("gapListOptimizationVariant").stringValue();
            if (strcmp(gapListOptimizationVariantPar, "none") == 0) {
                state->gapListOptimizationVariant = SctpStateVariables::GLOV_None;
            }
            else if (strcmp(gapListOptimizationVariantPar, "optimized1") == 0) {
                state->gapListOptimizationVariant = SctpStateVariables::GLOV_Optimized1;
            }
            else if (strcmp(gapListOptimizationVariantPar, "optimized2") == 0) {
                state->gapListOptimizationVariant = SctpStateVariables::GLOV_Optimized2;
            }
            else if (strcmp(gapListOptimizationVariantPar, "shrunken") == 0) {
                state->gapListOptimizationVariant = SctpStateVariables::GLOV_Shrunken;
            }
            else {
                throw cRuntimeError("Bad setting for gapListOptimizationVariant: %s\n",
                        gapListOptimizationVariantPar);
            }

            state->cmtUseSFR = sctpMain->par("cmtUseSFR");
            state->cmtUseDAC = sctpMain->par("cmtUseDAC");
            state->cmtUseFRC = sctpMain->par("cmtUseFRC");
            state->gapReportLimit = sctpMain->par("gapReportLimit");
            state->cmtSmartT3Reset = sctpMain->par("cmtSmartT3Reset");
            state->cmtSmartReneging = sctpMain->par("cmtSmartReneging");
            state->cmtSmartFastRTX = sctpMain->par("cmtSmartFastRTX");
            state->cmtSlowPathRTTUpdate = sctpMain->par("cmtSlowPathRTTUpdate");
            state->cmtMovedChunksReduceCwnd = sctpMain->par("cmtMovedChunksReduceCwnd");
            state->cmtChunkReschedulingThreshold = sctpMain->par("cmtChunkReschedulingThreshold");
            state->movedChunkFastRTXFactor = sctpMain->par("movedChunkFastRTXFactor");
            state->strictCwndBooking = sctpMain->par("strictCwndBooking");

            const char *cmtSackPathPar = sctpMain->par("cmtSackPath").stringValue();
            if (strcmp(cmtSackPathPar, "standard") == 0) {
                state->cmtSackPath = SctpStateVariables::CSP_Standard;
            }
            else if (strcmp(cmtSackPathPar, "primary") == 0) {
                state->cmtSackPath = SctpStateVariables::CSP_Primary;
            }
            else if (strcmp(cmtSackPathPar, "roundRobin") == 0) {
                state->cmtSackPath = SctpStateVariables::CSP_RoundRobin;
            }
            else if (strcmp(cmtSackPathPar, "smallestSRTT") == 0) {
                state->cmtSackPath = SctpStateVariables::CSP_SmallestSRTT;
            }
            else {
                throw cRuntimeError("Bad setting for cmtSackPath: %s\n",
                        cmtSackPathPar);
            }

            const char *cmtCCVariantPar = sctpMain->par("cmtCCVariant").stringValue();
            if (strcmp(cmtCCVariantPar, "off") == 0) {
                state->cmtCCVariant = SctpStateVariables::CCCV_Off;
                state->allowCMT = false;
            }
            else if (strcmp(cmtCCVariantPar, "cmt") == 0) {
                state->cmtCCVariant = SctpStateVariables::CCCV_CMT;
                state->allowCMT = true;
            }
            else if (strcmp(sctpMain->par("cmtCCVariant").stringValue(), "lia") == 0) {
                state->cmtCCVariant = SctpStateVariables::CCCV_CMT_LIA;
                state->allowCMT = true;
            }
            else if (strcmp(sctpMain->par("cmtCCVariant").stringValue(), "olia") == 0) {
                state->cmtCCVariant = SctpStateVariables::CCCV_CMT_OLIA;
                state->allowCMT = true;
            }
            else if ((strcmp(cmtCCVariantPar, "cmtrp") == 0) ||
                     (strcmp(cmtCCVariantPar, "cmtrpv1") == 0))
            {
                state->cmtCCVariant = SctpStateVariables::CCCV_CMTRPv1;
                state->allowCMT = true;
            }
            else if (strcmp(cmtCCVariantPar, "cmtrpv2") == 0) {
                state->cmtCCVariant = SctpStateVariables::CCCV_CMTRPv2;
                state->allowCMT = true;
            }
            else if (strcmp(cmtCCVariantPar, "cmtrp-t1") == 0) {
                state->cmtCCVariant = SctpStateVariables::CCCV_CMTRP_Test1;
                state->allowCMT = true;
            }
            else if (strcmp(cmtCCVariantPar, "cmtrp-t2") == 0) {
                state->cmtCCVariant = SctpStateVariables::CCCV_CMTRP_Test2;
                state->allowCMT = true;
            }
            else {
                throw cRuntimeError("Bad setting for cmtCCVariant: %s\n",
                        cmtCCVariantPar);
            }

            state->rpPathBlocking = sctpMain->par("rpPathBlocking");
            state->rpScaleBlockingTimeout = sctpMain->par("rpScaleBlockingTimeout");
            state->rpMinCwnd = sctpMain->par("rpMinCwnd");

            cStringTokenizer pathGroupsTokenizer(sctpMain->par("cmtCCPathGroups").stringValue());
            if (pathGroupsTokenizer.hasMoreTokens()) {
                auto pathIterator = sctpPathMap.begin();
                while (pathIterator != sctpPathMap.end()) {
                    const char *token = pathGroupsTokenizer.nextToken();
                    if (token == nullptr) {
                        throw cRuntimeError("Too few cmtCCGroup values to cover all paths!");
                    }
                    SctpPathVariables *path = pathIterator->second;
                    path->cmtCCGroup = atol(token);
                    pathIterator++;
                }
            }

            state->osbWithHeader = sctpMain->par("osbWithHeader");
            state->padding = sctpMain->par("padding");
            if (state->osbWithHeader)
                state->header = SCTP_DATA_CHUNK_LENGTH;
            else
                state->header = 0;
            state->swsLimit = sctpMain->par("swsLimit");
            state->fastRecoverySupported = sctpMain->par("fastRecoverySupported");
            state->reactivatePrimaryPath = sctpMain->par("reactivatePrimaryPath");
            state->packetsInTotalBurst = 0;
            state->auth = sctpMain->auth;
            state->messageAcceptLimit = sctpMain->par("messageAcceptLimit");
            state->bytesToAddPerRcvdChunk = sctpMain->par("bytesToAddPerRcvdChunk");
            state->bytesToAddPerPeerChunk = sctpMain->par("bytesToAddPerPeerChunk");
            state->tellArwnd = sctpMain->par("tellArwnd");
            state->throughputInterval = sctpMain->par("throughputInterval");
            sackPeriod = sctpMain->getSackPeriod();
            sackFrequency = sctpMain->getSackFrequency();
            Sctp::AssocStat stat;
            stat.assocId = assocId;
            stat.start = simTime();
            stat.stop = 0;
            stat.rcvdBytes = 0;
            stat.ackedBytes = 0;
            stat.sentBytes = 0;
            stat.transmittedBytes = 0;
            stat.numFastRtx = 0;
            stat.numT3Rtx = 0;
            stat.numDups = 0;
            stat.numPathFailures = 0;
            stat.numForwardTsn = 0;
            stat.sumRGapRanges = 0;
            stat.sumNRGapRanges = 0;
            stat.numOverfullSACKs = 0;
            stat.lifeTime = 0;
            stat.throughput = 0;
            stat.numDropsBecauseNewTsnGreaterThanHighestTsn = 0;
            stat.numDropsBecauseNoRoomInBuffer = 0;
            stat.numChunksReneged = 0;
            stat.numAuthChunksSent = 0;
            stat.numAuthChunksAccepted = 0;
            stat.numAuthChunksRejected = 0;
            stat.numResetRequestsSent = 0;
            stat.numResetRequestsPerformed = 0;
            fairTimer = false;
            stat.fairStart = 0;
            stat.fairStop = 0;
            stat.fairLifeTime = 0;
            stat.fairThroughput = 0;
            stat.fairAckedBytes = 0;
            stat.numEndToEndMessages = 0;
            stat.cumEndToEndDelay = 0;
            stat.startEndToEndDelay = sctpMain->par("startEndToEndDelay");
            stat.stopEndToEndDelay = sctpMain->par("stopEndToEndDelay");
            sctpMain->assocStatMap[stat.assocId] = stat;
            ccModule = sctpMain->par("ccModule");

            switch (ccModule) {
                case RFC4960: {
                    ccFunctions.ccInitParams = &SctpAssociation::initCcParameters;
                    ccFunctions.ccUpdateBeforeSack = &SctpAssociation::cwndUpdateBeforeSack;
                    ccFunctions.ccUpdateAfterSack = &SctpAssociation::cwndUpdateAfterSack;
                    ccFunctions.ccUpdateAfterCwndTimeout = &SctpAssociation::cwndUpdateAfterCwndTimeout;
                    ccFunctions.ccUpdateAfterRtxTimeout = &SctpAssociation::cwndUpdateAfterRtxTimeout;
                    ccFunctions.ccUpdateMaxBurst = &SctpAssociation::cwndUpdateMaxBurst;
                    ccFunctions.ccUpdateBytesAcked = &SctpAssociation::cwndUpdateBytesAcked;
                    break;
                }
            }

            pmStartPathManagement();
            state->sendQueueLimit = sctpMain->par("sendQueueLimit");
            EV_INFO << "stateEntered: Established socketId= " << assocId << endl;
            if (isToBeAccepted()) {
                EV_INFO << "Listening socket can accept now\n";
                sendAvailableIndicationToApp();
            }
            else {
                sendEstabIndicationToApp();
            }
            if (sctpMain->hasPar("addIP")) {
                const bool addIP = sctpMain->par("addIP");
                simtime_t addTime = sctpMain->par("addTime");
                EV_DETAIL << getFullPath() << ": addIP = " << addIP << " time = " << addTime << "\n";
                if (addIP == true && addTime > SIMTIME_ZERO) {
                    EV_DETAIL << "startTimer addTime to expire at " << simTime() + addTime << "\n";

                    scheduleTimeout(StartAddIP, addTime);
                }
            }
            if ((double)sctpMain->par("fairStart") > 0) {
                sctpMain->scheduleAt(sctpMain->par("fairStart"), FairStartTimer);
                sctpMain->scheduleAt(sctpMain->par("fairStop"), FairStopTimer);
                sctpMain->recordScalar("rtoMin", sctpMain->par("rtoMin").doubleValue());
            }
            char str[128];
            snprintf(str, sizeof(str), "Cumulated TSN Ack of Association %d", assocId);
            cumTsnAck = new cOutVector(str);
            snprintf(str, sizeof(str), "Number of Gap Blocks in Last SACK of Association %d", assocId);
            numGapBlocks = new cOutVector(str);
            snprintf(str, sizeof(str), "SendQueue of Association %d", assocId);
            sendQueue = new cOutVector(str);
            state->sendQueueLimit = sctpMain->par("sendQueueLimit");
            Sctp::VTagPair vtagPair;
            vtagPair.peerVTag = peerVTag;
            vtagPair.localVTag = localVTag;
            vtagPair.localPort = localPort;
            vtagPair.remotePort = remotePort;
            sctpMain->sctpVTagMap[assocId] = vtagPair;
            break;
        }

        case SCTP_S_CLOSED: {
            sendIndicationToApp(SCTP_I_CLOSED);
            break;
        }

        case SCTP_S_SHUTDOWN_PENDING: {
            if (getOutstandingBytes() == 0 && transmissionQ->getQueueSize() == 0 && qCounter.roomSumSendStreams == 0)
                sendShutdown();
            break;
        }

        case SCTP_S_SHUTDOWN_RECEIVED: {
            EV_INFO << "Entered state SHUTDOWN_RECEIVED, osb=" << getOutstandingBytes()
                    << ", transQ=" << transmissionQ->getQueueSize()
                    << ", scount=" << qCounter.roomSumSendStreams << endl;

            if (getOutstandingBytes() == 0 && transmissionQ->getQueueSize() == 0 && qCounter.roomSumSendStreams == 0) {
                sendShutdownAck(remoteAddr);
            }
            else {
                sendOnAllPaths(state->getPrimaryPath());
            }
            break;
        }
    }
}

void SctpAssociation::removePath()
{
    while (!sctpPathMap.empty()) {
        auto pathIterator = sctpPathMap.begin();
        SctpPathVariables *path = pathIterator->second;
        for (auto j = remoteAddressList.begin(); j != remoteAddressList.end(); j++) {
            if ((*j) == path->remoteAddress) {
                remoteAddressList.erase(j);
                break;
            }
        }
        EV_INFO << getFullPath() << " remove path " << path->remoteAddress << endl;
        stopTimer(path->HeartbeatTimer);
        delete path->HeartbeatTimer;
        stopTimer(path->HeartbeatIntervalTimer);
        delete path->HeartbeatIntervalTimer;
        stopTimer(path->T3_RtxTimer);
        delete path->T3_RtxTimer;
        stopTimer(path->CwndTimer);
        delete path->CwndTimer;
        stopTimer(path->ResetTimer);
        delete path->ResetTimer;
        stopTimer(path->AsconfTimer);
        delete path->AsconfTimer;
        stopTimer(path->BlockingTimer);
        delete path->BlockingTimer;
        delete path;
        sctpPathMap.erase(pathIterator);
    }
}

} // namespace sctp
} // namespace inet


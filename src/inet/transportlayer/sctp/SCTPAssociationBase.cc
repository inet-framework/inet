//
// Copyright (C) 2005-2010 Irene Ruengeler
// Copyright (C) 2009-2015 Thomas Dreibholz
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

#include <string.h>
#include <assert.h>
#include <sstream>

#include "inet/transportlayer/sctp/SCTPAssociation.h"

#include "inet/transportlayer/sctp/SCTP.h"
#include "inet/transportlayer/contract/sctp/SCTPCommand_m.h"
#include "inet/transportlayer/sctp/SCTPQueue.h"
#include "inet/transportlayer/sctp/SCTPAlgorithm.h"

namespace inet {

namespace sctp {

SCTPPathVariables::SCTPPathVariables(const L3Address& addr, SCTPAssociation *assoc, const IRoutingTable *rt)
{
    // ====== Path Variable Initialization ===================================
    association = assoc;
    remoteAddress = addr;
    activePath = true;
    confirmed = false;
    primaryPathCandidate = false;
    pathErrorCount = 0;
    const InterfaceEntry *rtie;
    pathErrorThreshold = assoc->getSctpMain()->par("pathMaxRetrans");

    if (!pathErrorThreshold) {
        pathErrorThreshold = PATH_MAX_RETRANS;
    }

    pathRto = assoc->getSctpMain()->par("rtoInitial");
    heartbeatTimeout = pathRto;
    double interval = (double)assoc->getSctpMain()->par("hbInterval");

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

    pmtu = rtie->getMTU();
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
    sendAllRandomizer = RNGCONTEXT uniform(0, (1 << 31));
    pseudoCumAck = 0;
    newPseudoCumAck = false;
    findPseudoCumAck = true;    // Set findPseudoCumAck to TRUE for new destination.
    rtxPseudoCumAck = 0;
    newRTXPseudoCumAck = false;
    findRTXPseudoCumAck = true;    // Set findRTXPseudoCumAck to TRUE for new destination.
    oldestChunkTSN = 0;
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
    findLowestTSN = true;
    lowestTSNRetransmitted = false;
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
    SCTPPathInfo *pinfo = new SCTPPathInfo("pinfo");
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
    vectorPathSentTSN = new cOutVector(str);
    snprintf(str, sizeof(str), "TSN Received %d:%s", assoc->assocId, addr.str().c_str());
    vectorPathReceivedTSN = new cOutVector(str);

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
    vectorPathTSNFastRTX = new cOutVector(str);
    snprintf(str, sizeof(str), "TSN Sent Timer-Based RTX %d:%s", assoc->assocId, addr.str().c_str());
    vectorPathTSNTimerBased = new cOutVector(str);
    snprintf(str, sizeof(str), "TSN Acked CumAck %d:%s", assoc->assocId, addr.str().c_str());
    vectorPathAckedTSNCumAck = new cOutVector(str);
    snprintf(str, sizeof(str), "TSN Acked GapAck %d:%s", assoc->assocId, addr.str().c_str());
    vectorPathAckedTSNGapAck = new cOutVector(str);

    snprintf(str, sizeof(str), "TSN PseudoCumAck %d:%s", assoc->assocId, addr.str().c_str());
    vectorPathPseudoCumAck = new cOutVector(str);
    snprintf(str, sizeof(str), "TSN RTXPseudoCumAck %d:%s", assoc->assocId, addr.str().c_str());
    vectorPathRTXPseudoCumAck = new cOutVector(str);
    snprintf(str, sizeof(str), "Blocking TSNs Moved %d:%s", assoc->assocId, addr.str().c_str());
    vectorPathBlockingTSNsMoved = new cOutVector(str);
}

SCTPPathVariables::~SCTPPathVariables()
{
    delete statisticsPathSSthresh;
    delete statisticsPathCwnd;
    delete statisticsPathBandwidth;
    delete statisticsPathRTO;
    delete statisticsPathRTT;

    delete vectorPathSentTSN;
    delete vectorPathReceivedTSN;
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

    delete vectorPathTSNFastRTX;
    delete vectorPathTSNTimerBased;
    delete vectorPathAckedTSNCumAck;
    delete vectorPathAckedTSNGapAck;

    delete vectorPathPseudoCumAck;
    delete vectorPathRTXPseudoCumAck;
    delete vectorPathBlockingTSNsMoved;
}

const L3Address SCTPDataVariables::zeroAddress = L3Address();

SCTPDataVariables::SCTPDataVariables()
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

SCTPDataVariables::~SCTPDataVariables()
{
}

SCTPStateVariables::SCTPStateVariables()
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
    resetPending = false;
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
    primaryPath = nullptr;
    lastDataSourcePath = nullptr;
    resetChunk = nullptr;
    asconfChunk = nullptr;
    shutdownChunk = nullptr;
    shutdownAckChunk = nullptr;
    initChunk = nullptr;
    cookieChunk = nullptr;
    sctpmsg = nullptr;
    sctpMsg = nullptr;
    bytesToRetransmit = 0;
    initRexmitTimeout = SCTP_TIMEOUT_INIT_REXMIT;
    localRwnd = SCTP_DEFAULT_ARWND;
    errorCount = 0;
    initRetransCounter = 0;
    nextTSN = 0;
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
    messagesToPush = 0;
    pushMessagesLeft = 0;
    msgNum = 0;
    bytesRcvd = 0;
    sendBuffer = 0;
    queuedReceivedBytes = 0;
    prMethod = 0;
    assocThroughput = 0;
    queuedSentBytes = 0;
    queuedDroppableBytes = 0;
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
    ssNextStream = false;
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
    maxBurstVariant = SCTPStateVariables::MBV_UseItOrLoseIt;
    initialWindow = 0;
    allowCMT = false;
    cmtSendAllComparisonFunction = nullptr;
    cmtRetransmissionVariant = 0;
    cmtCUCVariant = SCTPStateVariables::CUCV_Normal;
    cmtBufferSplitVariant = SCTPStateVariables::CBSV_None;
    cmtBufferSplittingUsesOSB = false;
    cmtChunkReschedulingVariant = SCTPStateVariables::CCRV_None;
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
    blockingTSNsMoved = 0;

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

SCTPStateVariables::~SCTPStateVariables()
{
}

//
// FSM framework, SCTP FSM
//

SCTPAssociation::SCTPAssociation(SCTP *_module, int32 _appGateIndex, int32 _assocId, IRoutingTable *_rt, IInterfaceTable *_ift)
{
    // ====== Initialize variables ===========================================
    rt = _rt;
    ift = _ift;
    sctpMain = _module;
    appGateIndex = _appGateIndex;
    assocId = _assocId;
    localPort = 0;
    remotePort = 0;
    localVTag = 0;
    peerVTag = 0;
    numberOfRemoteAddresses = 0;
    inboundStreams = SCTP_DEFAULT_INBOUND_STREAMS;
    outboundStreams = SCTP_DEFAULT_OUTBOUND_STREAMS;
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
    ssFunctions.ssGetNextSid = nullptr;
    ssFunctions.ssUsableStreams = nullptr;

    EV_INFO << "SCTPAssociationBase::SCTPAssociation(): new assocId="
            << assocId << endl;

    // ====== FSM ============================================================
    char fsmName[64];
    snprintf(fsmName, sizeof(fsmName), "fsm-%d", assocId);
    fsm = new cFSM();
    fsm->setName(fsmName);
    fsm->setState(SCTP_S_CLOSED);

    // ====== Path Info ======================================================
    SCTPPathInfo *pinfo = new SCTPPathInfo("pathInfo");
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
            ssFunctions.ssInitStreams = &SCTPAssociation::initStreams;
            ssFunctions.ssGetNextSid = &SCTPAssociation::streamScheduler;
            ssFunctions.ssUsableStreams = &SCTPAssociation::numUsableStreams;
            break;

        case ROUND_ROBIN_PACKET:
            ssFunctions.ssInitStreams = &SCTPAssociation::initStreams;
            ssFunctions.ssGetNextSid = &SCTPAssociation::streamSchedulerRoundRobinPacket;
            ssFunctions.ssUsableStreams = &SCTPAssociation::numUsableStreams;
            break;

        case RANDOM_SCHEDULE:
            ssFunctions.ssInitStreams = &SCTPAssociation::initStreams;
            ssFunctions.ssGetNextSid = &SCTPAssociation::streamSchedulerRandom;
            ssFunctions.ssUsableStreams = &SCTPAssociation::numUsableStreams;
            break;

        case RANDOM_SCHEDULE_PACKET:
            ssFunctions.ssInitStreams = &SCTPAssociation::initStreams;
            ssFunctions.ssGetNextSid = &SCTPAssociation::streamSchedulerRandomPacket;
            ssFunctions.ssUsableStreams = &SCTPAssociation::numUsableStreams;
            break;

        case FAIR_BANDWITH:
            ssFunctions.ssInitStreams = &SCTPAssociation::initStreams;
            ssFunctions.ssGetNextSid = &SCTPAssociation::streamSchedulerFairBandwidth;
            ssFunctions.ssUsableStreams = &SCTPAssociation::numUsableStreams;
            break;

        case FAIR_BANDWITH_PACKET:
            ssFunctions.ssInitStreams = &SCTPAssociation::initStreams;
            ssFunctions.ssGetNextSid = &SCTPAssociation::streamSchedulerFairBandwidthPacket;
            ssFunctions.ssUsableStreams = &SCTPAssociation::numUsableStreams;
            break;

        case PRIORITY:
            ssFunctions.ssInitStreams = &SCTPAssociation::initStreams;
            ssFunctions.ssGetNextSid = &SCTPAssociation::streamSchedulerPriority;
            ssFunctions.ssUsableStreams = &SCTPAssociation::numUsableStreams;
            break;

        case FCFS:
            ssFunctions.ssInitStreams = &SCTPAssociation::initStreams;
            ssFunctions.ssGetNextSid = &SCTPAssociation::streamSchedulerFCFS;
            ssFunctions.ssUsableStreams = &SCTPAssociation::numUsableStreams;
            break;

        case PATH_MANUAL:
            ssFunctions.ssInitStreams = &SCTPAssociation::initStreams;
            ssFunctions.ssGetNextSid = &SCTPAssociation::pathStreamSchedulerManual;
            ssFunctions.ssUsableStreams = &SCTPAssociation::numUsableStreams;
            break;

        case PATH_MAP_TO_PATH:
            ssFunctions.ssInitStreams = &SCTPAssociation::initStreams;
            ssFunctions.ssGetNextSid = &SCTPAssociation::pathStreamSchedulerMapToPath;
            ssFunctions.ssUsableStreams = &SCTPAssociation::numUsableStreams;
            break;
    }
}

SCTPAssociation::~SCTPAssociation()
{
    EV_TRACE << "Destructor SCTPAssociation " << assocId << endl;

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

bool SCTPAssociation::processTimer(cMessage *msg)
{
    SCTPPathVariables *path = nullptr;

    EV_INFO << msg->getName() << " timer expired at " << simTime() << "\n";

    SCTPPathInfo *pinfo = check_and_cast<SCTPPathInfo *>(msg->getControlInfo());
    L3Address addr = pinfo->getRemoteAddress();

    if (!addr.isUnspecified())
        path = getPath(addr);

    // first do actions
    SCTPEventCode event;
    event = SCTP_E_IGNORE;

    if (msg == T1_InitTimer) {
        process_TIMEOUT_INIT_REXMIT(event);
    }
    else if (msg == SackTimer) {
        EV_DETAIL << simTime() << " delayed Sack: cTsnAck=" << state->gapList.getCumAckTSN() << " highestTsnReceived=" << state->gapList.getHighestTSNReceived() << " lastTsnReceived=" << state->lastTsnReceived << " ackState=" << state->ackState << " numGaps=" << state->gapList.getNumGaps(SCTPGapList::GT_Any) << "\n";
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
        //if (sctpMain->testing == false)
        //{
        //sctpMain->testing = true;
        EV_DEBUG << "set testing to true\n";
        //}
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
        const char *type = (const char *)sctpMain->par("addIpType");
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

bool SCTPAssociation::processSCTPMessage(SCTPMessage *sctpmsg,
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
        for (auto k = state->localAddresses.begin(); k != state->localAddresses.end(); ++k) {
            if ((*k) == msgDestAddr) {
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

SCTPEventCode SCTPAssociation::preanalyseAppCommandEvent(int32 commandCode)
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

        case SCTP_C_SEND_ASCONF:
            return SCTP_E_SEND_ASCONF;    // Needed for multihomed NAT

        case SCTP_C_SET_STREAM_PRIO:
            return SCTP_E_SET_STREAM_PRIO;

        default:
            EV_DETAIL << "commandCode=" << commandCode << "\n";
            throw cRuntimeError("Unknown message kind in app command");
    }
}

bool SCTPAssociation::processAppCommand(cMessage *msg)
{
    printAssocBrief();

    // first do actions
    SCTPCommand *sctpCommand = (SCTPCommand *)(msg->removeControlInfo());
    SCTPEventCode event = preanalyseAppCommandEvent(msg->getKind());

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
            if (state->peerStreamReset == true) {
                process_STREAM_RESET(sctpCommand);
            }
            event = SCTP_E_IGNORE;
            break;

        case SCTP_E_SEND_ASCONF:
            sendAsconf(sctpMain->par("addIpType"));
            break;

        case SCTP_E_SET_STREAM_PRIO:
            state->ssPriorityMap[((SCTPSendInfo *)sctpCommand)->getSid()] =
                ((SCTPSendInfo *)sctpCommand)->getPpid();
            break;

        case SCTP_E_QUEUE_BYTES_LIMIT:
            process_QUEUE_BYTES_LIMIT(sctpCommand);
            break;

        case SCTP_E_QUEUE_MSGS_LIMIT:
            process_QUEUE_MSGS_LIMIT(sctpCommand);
            break;

        case SCTP_E_CLOSE:
            state->stopReading = true;
            /* fall through */

        case SCTP_E_SHUTDOWN:    /*sendShutdown*/
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

        default:
            throw cRuntimeError("Wrong event code");
    }

    delete sctpCommand;
    // then state transitions
    return performStateTransition(event);
}

bool SCTPAssociation::performStateTransition(const SCTPEventCode& event)
{
    EV_TRACE << "performStateTransition\n";

    if (event == SCTP_E_IGNORE) {    // e.g. discarded segment
        EV_DETAIL << "Staying in state: " << stateName(fsm->getState()) << " (no FSM event)\n";
        return true;
    }

    // state machine
    int32 oldState = fsm->getState();

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
                    state->lastTSN = state->nextTSN - 1;
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

void SCTPAssociation::stateEntered(int32 status)
{
    switch (status) {
        case SCTP_S_COOKIE_WAIT:
            break;

        case SCTP_S_ESTABLISHED: {
            EV_INFO << "State ESTABLISHED entered" << endl;
            stopTimer(T1_InitTimer);

            if (state->initChunk) {
                delete state->initChunk;
            }

            state->nagleEnabled = (bool)sctpMain->par("nagleEnabled");
            state->enableHeartbeats = (bool)sctpMain->par("enableHeartbeats");
            state->sendHeartbeatsOnActivePaths = (bool)sctpMain->par("sendHeartbeatsOnActivePaths");
            state->numGapReports = sctpMain->par("numGapReports");
            state->maxBurst = (uint32)sctpMain->par("maxBurst");
            state->rtxMethod = sctpMain->par("RTXMethod");
            state->nrSack = (bool)sctpMain->par("nrSack");
            state->disableReneging = (bool)sctpMain->par("disableReneging");
            state->checkSackSeqNumber = (bool)sctpMain->par("checkSackSeqNumber");
            state->outgoingSackSeqNum = 0;
            state->incomingSackSeqNum = 0;
            state->fragPoint = (uint32)sctpMain->par("fragPoint");
            state->highSpeedCC = (bool)sctpMain->par("highSpeedCC");
            state->initialWindow = (uint32)sctpMain->par("initialWindow");
            if (strcmp((const char *)sctpMain->par("maxBurstVariant"), "useItOrLoseIt") == 0) {
                state->maxBurstVariant = SCTPStateVariables::MBV_UseItOrLoseIt;
            }
            else if (strcmp((const char *)sctpMain->par("maxBurstVariant"), "congestionWindowLimiting") == 0) {
                state->maxBurstVariant = SCTPStateVariables::MBV_CongestionWindowLimiting;
            }
            else if (strcmp((const char *)sctpMain->par("maxBurstVariant"), "maxBurst") == 0) {
                state->maxBurstVariant = SCTPStateVariables::MBV_MaxBurst;
            }
            else if (strcmp((const char *)sctpMain->par("maxBurstVariant"), "aggressiveMaxBurst") == 0) {
                state->maxBurstVariant = SCTPStateVariables::MBV_AggressiveMaxBurst;
            }
            else if (strcmp((const char *)sctpMain->par("maxBurstVariant"), "totalMaxBurst") == 0) {
                state->maxBurstVariant = SCTPStateVariables::MBV_TotalMaxBurst;
            }
            else if (strcmp((const char *)sctpMain->par("maxBurstVariant"), "useItOrLoseItTempCwnd") == 0) {
                state->maxBurstVariant = SCTPStateVariables::MBV_UseItOrLoseItTempCwnd;
            }
            else if (strcmp((const char *)sctpMain->par("maxBurstVariant"), "congestionWindowLimitingTempCwnd") == 0) {
                state->maxBurstVariant = SCTPStateVariables::MBV_CongestionWindowLimitingTempCwnd;
            }
            else {
                throw cRuntimeError("Invalid setting of maxBurstVariant: %s.",
                        (const char *)sctpMain->par("maxBurstVariant"));
            }

            if (strcmp((const char *)sctpMain->par("cmtSendAllVariant"), "normal") == 0) {
                state->cmtSendAllComparisonFunction = nullptr;
            }
            else if (strcmp((const char *)sctpMain->par("cmtSendAllVariant"), "smallestLastTransmission") == 0) {
                state->cmtSendAllComparisonFunction = pathMapSmallestLastTransmission;
            }
            else if (strcmp((const char *)sctpMain->par("cmtSendAllVariant"), "randomized") == 0) {
                state->cmtSendAllComparisonFunction = pathMapRandomized;
            }
            else if (strcmp((const char *)sctpMain->par("cmtSendAllVariant"), "largestSSThreshold") == 0) {
                state->cmtSendAllComparisonFunction = pathMapLargestSSThreshold;
            }
            else if (strcmp((const char *)sctpMain->par("cmtSendAllVariant"), "largestSpace") == 0) {
                state->cmtSendAllComparisonFunction = pathMapLargestSpace;
            }
            else if (strcmp((const char *)sctpMain->par("cmtSendAllVariant"), "largestSpaceAndSSThreshold") == 0) {
                state->cmtSendAllComparisonFunction = pathMapLargestSpaceAndSSThreshold;
            }
            else {
                throw cRuntimeError("Invalid setting of cmtSendAllVariant: %s.",
                        (const char *)sctpMain->par("cmtSendAllVariant"));
            }

            state->cmtRetransmissionVariant = sctpMain->par("cmtRetransmissionVariant");
            if (strcmp((const char *)sctpMain->par("cmtCUCVariant"), "normal") == 0) {
                state->cmtCUCVariant = SCTPStateVariables::CUCV_Normal;
            }
            else if (strcmp((const char *)sctpMain->par("cmtCUCVariant"), "pseudoCumAck") == 0) {
                state->cmtCUCVariant = SCTPStateVariables::CUCV_PseudoCumAck;
            }
            else if (strcmp((const char *)sctpMain->par("cmtCUCVariant"), "pseudoCumAckV2") == 0) {
                state->cmtCUCVariant = SCTPStateVariables::CUCV_PseudoCumAckV2;
            }
            else {
                throw cRuntimeError("Bad setting for cmtCUCVariant: %s\n",
                        (const char *)sctpMain->par("cmtCUCVariant"));
            }
            state->smartOverfullSACKHandling = (bool)sctpMain->par("smartOverfullSACKHandling");

            if (strcmp((const char *)sctpMain->par("cmtChunkReschedulingVariant"), "none") == 0) {
                state->cmtChunkReschedulingVariant = SCTPStateVariables::CCRV_None;
            }
            else if (strcmp((const char *)sctpMain->par("cmtChunkReschedulingVariant"), "senderOnly") == 0) {
                state->cmtChunkReschedulingVariant = SCTPStateVariables::CCRV_SenderOnly;
            }
            else if (strcmp((const char *)sctpMain->par("cmtChunkReschedulingVariant"), "receiverOnly") == 0) {
                state->cmtChunkReschedulingVariant = SCTPStateVariables::CCRV_ReceiverOnly;
            }
            else if (strcmp((const char *)sctpMain->par("cmtChunkReschedulingVariant"), "bothSides") == 0) {
                state->cmtChunkReschedulingVariant = SCTPStateVariables::CCRV_BothSides;
            }
            else if (strcmp((const char *)sctpMain->par("cmtChunkReschedulingVariant"), "test") == 0) {
                state->cmtChunkReschedulingVariant = SCTPStateVariables::CCRV_Test;
            }
            else {
                throw cRuntimeError("Bad setting for cmtChunkReschedulingVariant: %s\n",
                        (const char *)sctpMain->par("cmtChunkReschedulingVariant"));
            }

            if (strcmp((const char *)sctpMain->par("cmtBufferSplitVariant"), "none") == 0) {
                state->cmtBufferSplitVariant = SCTPStateVariables::CBSV_None;
            }
            else if (strcmp((const char *)sctpMain->par("cmtBufferSplitVariant"), "senderOnly") == 0) {
                state->cmtBufferSplitVariant = SCTPStateVariables::CBSV_SenderOnly;
            }
            else if (strcmp((const char *)sctpMain->par("cmtBufferSplitVariant"), "receiverOnly") == 0) {
                state->cmtBufferSplitVariant = SCTPStateVariables::CBSV_ReceiverOnly;
            }
            else if (strcmp((const char *)sctpMain->par("cmtBufferSplitVariant"), "bothSides") == 0) {
                state->cmtBufferSplitVariant = SCTPStateVariables::CBSV_BothSides;
            }
            else {
                throw cRuntimeError("Bad setting for cmtBufferSplitVariant: %s\n",
                        (const char *)sctpMain->par("cmtBufferSplitVariant"));
            }
            state->cmtBufferSplittingUsesOSB = (bool)sctpMain->par("cmtBufferSplittingUsesOSB");

            if (strcmp((const char *)sctpMain->par("gapListOptimizationVariant"), "none") == 0) {
                state->gapListOptimizationVariant = SCTPStateVariables::GLOV_None;
            }
            else if (strcmp((const char *)sctpMain->par("gapListOptimizationVariant"), "optimized1") == 0) {
                state->gapListOptimizationVariant = SCTPStateVariables::GLOV_Optimized1;
            }
            else if (strcmp((const char *)sctpMain->par("gapListOptimizationVariant"), "optimized2") == 0) {
                state->gapListOptimizationVariant = SCTPStateVariables::GLOV_Optimized2;
            }
            else if (strcmp((const char *)sctpMain->par("gapListOptimizationVariant"), "shrunken") == 0) {
                state->gapListOptimizationVariant = SCTPStateVariables::GLOV_Shrunken;
            }
            else {
                throw cRuntimeError("Bad setting for gapListOptimizationVariant: %s\n",
                        (const char *)sctpMain->par("gapListOptimizationVariant"));
            }

            state->cmtUseSFR = (bool)sctpMain->par("cmtUseSFR");
            state->cmtUseDAC = (bool)sctpMain->par("cmtUseDAC");
            state->cmtUseFRC = (bool)sctpMain->par("cmtUseFRC");
            state->gapReportLimit = (uint32)sctpMain->par("gapReportLimit");
            state->cmtSmartT3Reset = (bool)sctpMain->par("cmtSmartT3Reset");
            state->cmtSmartReneging = (bool)sctpMain->par("cmtSmartReneging");
            state->cmtSmartFastRTX = (bool)sctpMain->par("cmtSmartFastRTX");
            state->cmtSlowPathRTTUpdate = (bool)sctpMain->par("cmtSlowPathRTTUpdate");
            state->cmtMovedChunksReduceCwnd = (bool)sctpMain->par("cmtMovedChunksReduceCwnd");
            state->cmtChunkReschedulingThreshold = (double)sctpMain->par("cmtChunkReschedulingThreshold");
            state->movedChunkFastRTXFactor = (double)sctpMain->par("movedChunkFastRTXFactor");
            state->strictCwndBooking = (bool)sctpMain->par("strictCwndBooking");

            if (strcmp((const char *)sctpMain->par("cmtSackPath"), "standard") == 0) {
                state->cmtSackPath = SCTPStateVariables::CSP_Standard;
            }
            else if (strcmp((const char *)sctpMain->par("cmtSackPath"), "primary") == 0) {
                state->cmtSackPath = SCTPStateVariables::CSP_Primary;
            }
            else if (strcmp((const char *)sctpMain->par("cmtSackPath"), "roundRobin") == 0) {
                state->cmtSackPath = SCTPStateVariables::CSP_RoundRobin;
            }
            else if (strcmp((const char *)sctpMain->par("cmtSackPath"), "smallestSRTT") == 0) {
                state->cmtSackPath = SCTPStateVariables::CSP_SmallestSRTT;
            }
            else {
                throw cRuntimeError("Bad setting for cmtSackPath: %s\n",
                        (const char *)sctpMain->par("cmtSackPath"));
            }

            if (strcmp((const char *)sctpMain->par("cmtCCVariant"), "off") == 0) {
                state->cmtCCVariant = SCTPStateVariables::CCCV_Off;
                state->allowCMT = false;
            }
            else if (strcmp((const char *)sctpMain->par("cmtCCVariant"), "cmt") == 0) {
                state->cmtCCVariant = SCTPStateVariables::CCCV_CMT;
                state->allowCMT = true;
            }
            else if(strcmp((const char*)sctpMain->par("cmtCCVariant"), "lia") == 0){
               state->cmtCCVariant = SCTPStateVariables::CCCV_CMT_LIA;
               state->allowCMT     = true;
            }
            else if(strcmp((const char*)sctpMain->par("cmtCCVariant"), "olia") == 0){
               state->cmtCCVariant = SCTPStateVariables::CCCV_CMT_OLIA;
               state->allowCMT     = true;
            }
            else if ((strcmp((const char *)sctpMain->par("cmtCCVariant"), "cmtrp") == 0) ||
                     (strcmp((const char *)sctpMain->par("cmtCCVariant"), "cmtrpv1") == 0))
            {
                state->cmtCCVariant = SCTPStateVariables::CCCV_CMTRPv1;
                state->allowCMT = true;
            }
            else if (strcmp((const char *)sctpMain->par("cmtCCVariant"), "cmtrpv2") == 0) {
                state->cmtCCVariant = SCTPStateVariables::CCCV_CMTRPv2;
                state->allowCMT = true;
            }
            else if (strcmp((const char *)sctpMain->par("cmtCCVariant"), "cmtrp-t1") == 0) {
                state->cmtCCVariant = SCTPStateVariables::CCCV_CMTRP_Test1;
                state->allowCMT = true;
            }
            else if (strcmp((const char *)sctpMain->par("cmtCCVariant"), "cmtrp-t2") == 0) {
                state->cmtCCVariant = SCTPStateVariables::CCCV_CMTRP_Test2;
                state->allowCMT = true;
            }
            else {
                throw cRuntimeError("Bad setting for cmtCCVariant: %s\n",
                        (const char *)sctpMain->par("cmtCCVariant"));
            }

            state->rpPathBlocking = (bool)sctpMain->par("rpPathBlocking");
            state->rpScaleBlockingTimeout = (bool)sctpMain->par("rpScaleBlockingTimeout");
            state->rpMinCwnd = sctpMain->par("rpMinCwnd");

            cStringTokenizer pathGroupsTokenizer(sctpMain->par("cmtCCPathGroups").stringValue());
            if (pathGroupsTokenizer.hasMoreTokens()) {
                auto pathIterator = sctpPathMap.begin();
                while (pathIterator != sctpPathMap.end()) {
                    const char *token = pathGroupsTokenizer.nextToken();
                    if (token == nullptr) {
                        throw cRuntimeError("Too few cmtCCGroup values to cover all paths!");
                    }
                    SCTPPathVariables *path = pathIterator->second;
                    path->cmtCCGroup = atol(token);
                    pathIterator++;
                }
            }

            state->osbWithHeader = (bool)sctpMain->par("osbWithHeader");
            state->padding = (bool)sctpMain->par("padding");
            if (state->osbWithHeader)
                state->header = SCTP_DATA_CHUNK_LENGTH;
            else
                state->header = 0;
            state->swsLimit = (uint32)sctpMain->par("swsLimit");
            state->fastRecoverySupported = (bool)sctpMain->par("fastRecoverySupported");
            state->reactivatePrimaryPath = (bool)sctpMain->par("reactivatePrimaryPath");
            state->packetsInTotalBurst = 0;
            state->auth = sctpMain->auth;
            state->messageAcceptLimit = sctpMain->par("messageAcceptLimit");
            state->bytesToAddPerRcvdChunk = sctpMain->par("bytesToAddPerRcvdChunk");
            state->bytesToAddPerPeerChunk = sctpMain->par("bytesToAddPerPeerChunk");
            state->tellArwnd = sctpMain->par("tellArwnd");
            state->throughputInterval = (double)sctpMain->par("throughputInterval");
            sackPeriod = (double)sctpMain->par("sackPeriod");
            sackFrequency = sctpMain->par("sackFrequency");
            SCTP::AssocStat stat;
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
            stat.numDropsBecauseNewTSNGreaterThanHighestTSN = 0;
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
            stat.startEndToEndDelay = (uint32)sctpMain->par("startEndToEndDelay");
            stat.stopEndToEndDelay = (uint32)sctpMain->par("stopEndToEndDelay");
            sctpMain->assocStatMap[stat.assocId] = stat;
            ccModule = sctpMain->par("ccModule");

            switch (ccModule) {
                case RFC4960: {
                    ccFunctions.ccInitParams = &SCTPAssociation::initCCParameters;
                    ccFunctions.ccUpdateBeforeSack = &SCTPAssociation::cwndUpdateBeforeSack;
                    ccFunctions.ccUpdateAfterSack = &SCTPAssociation::cwndUpdateAfterSack;
                    ccFunctions.ccUpdateAfterCwndTimeout = &SCTPAssociation::cwndUpdateAfterCwndTimeout;
                    ccFunctions.ccUpdateAfterRtxTimeout = &SCTPAssociation::cwndUpdateAfterRtxTimeout;
                    ccFunctions.ccUpdateMaxBurst = &SCTPAssociation::cwndUpdateMaxBurst;
                    ccFunctions.ccUpdateBytesAcked = &SCTPAssociation::cwndUpdateBytesAcked;
                    break;
                }
            }

            pmStartPathManagement();
            state->sendQueueLimit = (uint32)sctpMain->par("sendQueueLimit");
            sendEstabIndicationToApp();
            if (sctpMain->hasPar("addIP")) {
                const bool addIP = (bool)sctpMain->par("addIP");
                EV_DETAIL << getFullPath() << ": addIP = " << addIP << " time = " << (double)sctpMain->par("addTime") << "\n";
                if (addIP == true && (double)sctpMain->par("addTime") > 0) {
                    EV_DETAIL << "startTimer addTime to expire at " << simTime() + (double)sctpMain->par("addTime") << "\n";

                    scheduleTimeout(StartAddIP, (double)sctpMain->par("addTime"));
                }
            }
            if ((double)sctpMain->par("fairStart") > 0) {
                sctpMain->scheduleAt(sctpMain->par("fairStart"), FairStartTimer);
                sctpMain->scheduleAt(sctpMain->par("fairStop"), FairStopTimer);
                sctpMain->recordScalar("rtoMin", (double)sctpMain->par("rtoMin"));
            }
            char str[128];
            snprintf(str, sizeof(str), "Cumulated TSN Ack of Association %d", assocId);
            cumTsnAck = new cOutVector(str);
            snprintf(str, sizeof(str), "Number of Gap Blocks in Last SACK of Association %d", assocId);
            numGapBlocks = new cOutVector(str);
            snprintf(str, sizeof(str), "SendQueue of Association %d", assocId);
            sendQueue = new cOutVector(str);
            state->sendQueueLimit = (uint32)sctpMain->par("sendQueueLimit");
            SCTP::VTagPair vtagPair;
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

void SCTPAssociation::removePath()
{
    while (!sctpPathMap.empty()) {
        auto pathIterator = sctpPathMap.begin();
        SCTPPathVariables *path = pathIterator->second;
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


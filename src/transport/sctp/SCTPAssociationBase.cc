//
// Copyright (C) 2005-2010 Irene Ruengeler
// Copyright (C) 2009-2010 Thomas Dreibholz
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
#include "SCTP.h"
#include "SCTPAssociation.h"
#include "SCTPCommand_m.h"
#include "IPControlInfo_m.h"
#include "SCTPQueue.h"
#include "SCTPAlgorithm.h"
#include "IPv4InterfaceData.h"

#include <sstream>


SCTPPathVariables::SCTPPathVariables(const IPvXAddress& addr, SCTPAssociation* assoc)
{
    InterfaceTableAccess interfaceTableAccess;

    association                  = assoc;
    remoteAddress                = addr;
    activePath                   = true;
    confirmed                    = false;
    primaryPathCandidate         = false;
    pathErrorCount               = 0;
    pathErrorThreshold       = assoc->getSctpMain()->par("pathMaxRetrans");
    if (!pathErrorThreshold) {
        pathErrorThreshold = PATH_MAX_RETRANS;
    }
    pathRto                      = assoc->getSctpMain()->par("rtoInitial");
    heartbeatTimeout             = pathRto;
    double interval              = (double)assoc->getSctpMain()->par("hbInterval");
    if (!interval) {
        interval = HB_INTERVAL;
    }
    heartbeatIntervalTimeout = pathRto+interval;
    srtt                         = pathRto;
    lastAckTime                  = 0;
    forceHb                      = false;
    partialBytesAcked            = 0;
    queuedBytes                  = 0;
    outstandingBytes             = 0;

    RoutingTableAccess routingTableAccess;
    const InterfaceEntry* rtie = routingTableAccess.get()->getInterfaceForDestAddr(remoteAddress.get4());
    if(rtie == NULL) {
        opp_error("No interface for remote address %s found!", remoteAddress.get4().str().c_str());
    }
    pmtu                         = rtie->getMTU();
    rttvar                       = 0.0;

    cwndTimeout                  = pathRto;
    cwnd                         = 0;
    ssthresh                     = 0;
    updateTime                   = 0.0;
    fastRecoveryExitPoint        = 0;
    fastRecoveryActive           = false;

    numberOfFastRetransmissions      = 0;
    numberOfTimerBasedRetransmissions = 0;
    numberOfHeartbeatsSent = 0;
    numberOfHeartbeatsRcvd = 0;
    numberOfHeartbeatAcksSent = 0;
    numberOfHeartbeatAcksRcvd = 0;

    char str[128];
    snprintf(str, sizeof(str), "HB_TIMER %d:%s",assoc->assocId,addr.str().c_str());
    HeartbeatTimer = new cMessage(str);
    snprintf(str, sizeof(str), "HB_INT_TIMER %d:%s",assoc->assocId,addr.str().c_str());
    HeartbeatIntervalTimer = new cMessage(str);
    snprintf(str, sizeof(str), "CWND_TIMER %d:%s",assoc->assocId,addr.str().c_str());
    CwndTimer = new cMessage(str);
    snprintf(str, sizeof(str), "RTX_TIMER %d:%s",assoc->assocId,addr.str().c_str());
    T3_RtxTimer = new cMessage(str);
    HeartbeatTimer->setContextPointer(association);
    HeartbeatIntervalTimer->setContextPointer(association);
    CwndTimer->setContextPointer(association);
    T3_RtxTimer->setContextPointer(association);

    snprintf(str, sizeof(str), "RTO %d:%s",assoc->assocId,addr.str().c_str());
    statisticsPathRTO = new cOutVector(str);
    snprintf(str, sizeof(str), "RTT %d:%s",assoc->assocId,addr.str().c_str());
    statisticsPathRTT = new cOutVector(str);

    snprintf(str, sizeof(str), "Slow Start Threshold %d:%s",assoc->assocId,addr.str().c_str());
    statisticsPathSSthresh = new cOutVector(str);
    snprintf(str, sizeof(str), "Congestion Window %d:%s",assoc->assocId,addr.str().c_str());
    statisticsPathCwnd = new cOutVector(str);

    snprintf(str, sizeof(str), "TSN Sent %d:%s",assoc->assocId,addr.str().c_str());
    pathTSN = new cOutVector(str);
    snprintf(str, sizeof(str), "TSN Received %d:%s",assoc->assocId,addr.str().c_str());
    pathRcvdTSN = new cOutVector(str);

    snprintf(str, sizeof(str), "HB Sent %d:%s",assoc->assocId,addr.str().c_str());
    pathHb = new cOutVector(str);
    snprintf(str, sizeof(str), "HB ACK Sent %d:%s",assoc->assocId,addr.str().c_str());
    pathHbAck = new cOutVector(str);
    snprintf(str, sizeof(str), "HB Received %d:%s",assoc->assocId,addr.str().c_str());
    pathRcvdHb = new cOutVector(str);
    snprintf(str, sizeof(str), "HB ACK Received %d:%s",assoc->assocId,addr.str().c_str());
    pathRcvdHbAck = new cOutVector(str);



    SCTPPathInfo* pinfo = new SCTPPathInfo("pinfo");
    pinfo->setRemoteAddress(addr);
    T3_RtxTimer->setControlInfo(pinfo);
    HeartbeatTimer->setControlInfo(pinfo->dup());
    HeartbeatIntervalTimer->setControlInfo(pinfo->dup());
    CwndTimer->setControlInfo(pinfo->dup());

}

SCTPPathVariables::~SCTPPathVariables()
{
    statisticsPathSSthresh->record(0);
    statisticsPathCwnd->record(0);
    delete statisticsPathSSthresh;
    delete statisticsPathCwnd;
    statisticsPathRTO->record(0);
    statisticsPathRTT->record(0);
    delete statisticsPathRTO;
    delete statisticsPathRTT;

    delete pathTSN;
    delete pathRcvdTSN;
    delete pathHb;
    delete pathRcvdHb;
    delete pathHbAck;
    delete pathRcvdHbAck;

}


const IPvXAddress SCTPDataVariables::zeroAddress = IPvXAddress("0.0.0.0");

SCTPDataVariables::SCTPDataVariables()
{
    userData                     = NULL;
    ordered                      = true;
    len                          = 0;
    tsn                          = 0;
    sid                          = 0;
    ssn                          = 0;
    ppid                         = 0;
    gapReports                   = 0;
    enqueuingTime                = 0;
    sendTime                     = 0;
    ackTime                      = 0;
    expiryTime                   = 0;
    enqueuedInTransmissionQ      = false;
    hasBeenAcked                 = false;
    hasBeenReneged               = false;
    hasBeenAbandoned             = false;
    hasBeenFastRetransmitted     = false;
    countsAsOutstanding          = false;
    lastDestination              = NULL;
    nextDestination              = NULL;
    initialDestination           = NULL;
    numberOfTransmissions        = 0;
    numberOfRetransmissions      = 0;
    booksize                     = 0;
}

SCTPDataVariables::~SCTPDataVariables()
{
}

SCTPStateVariables::SCTPStateVariables()
{
    active                    = false;
    fork                          = false;
    initReceived              = false;
    cookieEchoReceived    = false;
    ackPointAdvanced          = false;
    swsAvoidanceInvoked   = false;
    firstChunkReceived    = false;
    probingIsAllowed          = false;
    zeroWindowProbing         = true;
    alwaysBundleSack          = true;
    fastRecoverySupported  = true;
    reactivatePrimaryPath  = false;
    newChunkReceived          = false;
    dataChunkReceived         = false;
    sackAllowed               = false;
    resetPending              = false;
    stopReceiving             = false;
    stopOldData               = false;
    stopSending               = false;
    inOut                         = false;
    queueUpdate               = false;
    firstDataSent             = false;
    peerWindowFull            = false;
    zeroWindow                = false;
    appSendAllowed            = true;
    noMoreOutstanding         = false;
    primaryPath               = NULL;
    lastDataSourceAddress  = IPvXAddress("0.0.0.0");
    shutdownChunk             = NULL;
    initChunk                 = NULL;
    cookieChunk               = NULL;
    sctpmsg                   = NULL;
    sctpMsg                   = NULL;
    bytesToRetransmit         = 0;
    initRexmitTimeout         = SCTP_TIMEOUT_INIT_REXMIT;
    localRwnd                 = SCTP_DEFAULT_ARWND;
    errorCount                = 0;
    initRetransCounter    = 0;
    nextTSN                   = 0;
    cTsnAck                   = 0;
    lastTsnAck                = 0;
    highestTsnReceived    = 0;
    highestTsnAcked       = 0;
    highestTsnStored          = 0;
    nextRSid                      = 0;
    ackState                      = 0;
    lastStreamScheduled   = 0;
    peerRwnd                      = 0;
    initialPeerRwnd       = 0;
    assocPmtu                 = 0;
    outstandingBytes          = 0;
    messagesToPush            = 0;
    pushMessagesLeft          = 0;
    numGaps                   = 0;
    msgNum                    = 0;
    bytesRcvd                 = 0;
    sendBuffer                = 0;
    queuedReceivedBytes   = 0;
    lastSendQueueAbated   = simTime();
    queuedMessages            = 0;
    queueLimit                = 0;
    probingTimeout            = 1;
    numRequests               = 0;
    numMsgsReq.resize(65536);
    for (unsigned int i = 0; i < 65536; i++) {
        numMsgsReq[i] = 0;
    }
    for (unsigned int i = 0; i < MAX_GAP_COUNT; i++) {
        gapStartList[i] = 0;
        gapStopList[i]   = 0;
    }
    for (unsigned int i = 0; i < 32; i++) {
        localTieTag[i] = 0;
        peerTieTag[i]   = 0;
    }
    count = 0;
}

SCTPStateVariables::~SCTPStateVariables()
{
}


//
// FSM framework, SCTP FSM
//

SCTPAssociation::SCTPAssociation(SCTP* _module, int32 _appGateIndex, int32 _assocId)
{
    // ====== Initialize variables ===========================================
    sctpMain                            = _module;
    appGateIndex                        = _appGateIndex;
    assocId                             = _assocId;
    localPort                           = 0;
    remotePort                          = 0;
    localVTag                           = 0;
    peerVTag                            = 0;
    numberOfRemoteAddresses             = 0;
    inboundStreams                      = SCTP_DEFAULT_INBOUND_STREAMS;
    outboundStreams                     = SCTP_DEFAULT_OUTBOUND_STREAMS;
    // queues and algorithm will be created on active or passive open
    transmissionQ                       = NULL;
    retransmissionQ                     = NULL;
    sctpAlgorithm                       = NULL;
    state                               = NULL;
    sackPeriod                          = SACK_DELAY;
/*
    totalCwndAdjustmentTime         = simTime();
    lastTotalSSthresh                   = ~0;
    lastTotalCwnd                       = ~0;*/

    cumTsnAck                           = NULL;
    sendQueue                           = NULL;
    numGapBlocks                        = NULL;

    qCounter.roomSumSendStreams = 0;
    qCounter.bookedSumSendStreams = 0;
    qCounter.roomSumRcvStreams      = 0;
    bytes.chunk                         = false;
    bytes.packet                        = false;
    bytes.bytesToSend                   = 0;

    sctpEV3 << "SCTPAssociationBase::SCTPAssociation(): new assocId="
              << assocId << endl;

    // ====== FSM ============================================================
    char fsmName[64];
    snprintf(fsmName, sizeof(fsmName), "fsm-%d", assocId);
    fsm = new cFSM();
    fsm->setName(fsmName);
    fsm->setState(SCTP_S_CLOSED);


    // ====== Path Info ======================================================
    SCTPPathInfo* pinfo = new SCTPPathInfo("pathInfo");
    pinfo->setRemoteAddress(IPvXAddress("0.0.0.0"));

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

    if (sctpMain->testTimeout > 0){
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

    // ====== Stream scheduling ==============================================
    ssModule = sctpMain->par("ssModule");
    switch (ssModule)
    {
        case ROUND_ROBIN:
            ssFunctions.ssInitStreams    = &SCTPAssociation::initStreams;
            ssFunctions.ssGetNextSid     = &SCTPAssociation::streamScheduler;
            ssFunctions.ssUsableStreams  = &SCTPAssociation::numUsableStreams;
            break;
    }
}

SCTPAssociation::~SCTPAssociation()
{
    sctpEV3 << "Destructor SCTPAssociation" << endl;

    delete T1_InitTimer;
    delete T2_ShutdownTimer;
    delete T5_ShutdownGuardTimer;
    delete SackTimer;

    delete advRwnd;
    delete cumTsnAck;
    delete numGapBlocks;
    delete sendQueue;


    delete fsm;
    delete state;
    delete sctpAlgorithm;
}

bool SCTPAssociation::processTimer(cMessage *msg)
{
    SCTPPathVariables* path = NULL;

    sctpEV3 << msg->getName() << " timer expired at "<<simulation.getSimTime()<<"\n";

    SCTPPathInfo* pinfo = check_and_cast<SCTPPathInfo*>(msg->getControlInfo());
    IPvXAddress addr = pinfo->getRemoteAddress();

    if (addr != IPvXAddress("0.0.0.0"))
        path = getPath(addr);
     // first do actions
    SCTPEventCode event;
    event = SCTP_E_IGNORE;
    if (msg==T1_InitTimer)
    {
        process_TIMEOUT_INIT_REXMIT(event);
    }
    else if (msg==SackTimer)
    {
    sctpEV3<<simulation.getSimTime()<<" delayed Sack: cTsnAck="<<state->cTsnAck<<" highestTsnReceived="<<state->highestTsnReceived<<" lastTsnReceived="<<state->lastTsnReceived<<" ackState="<<state->ackState<<" numGaps="<<state->numGaps<<"\n";
        sendSack();
    }
    else if (msg==T2_ShutdownTimer)
    {
        stopTimer(T2_ShutdownTimer);
        process_TIMEOUT_SHUTDOWN(event);
    }
    else if (msg==T5_ShutdownGuardTimer)
    {
        stopTimer(T5_ShutdownGuardTimer);
        delete state->shutdownChunk;
        sendIndicationToApp(SCTP_I_CONN_LOST);
        sendAbort();
        sctpMain->removeAssociation(this);
    }
    else if (path!=NULL && msg==path->HeartbeatIntervalTimer)
    {
        process_TIMEOUT_HEARTBEAT_INTERVAL(path, path->forceHb);
    }
    else if (path!=NULL && msg==path->HeartbeatTimer)
    {
        process_TIMEOUT_HEARTBEAT(path);
    }
    else if (path!=NULL && msg==path->T3_RtxTimer)
    {
        process_TIMEOUT_RTX(path);
    }
    else if (path!=NULL && msg==path->CwndTimer)
    {
        (this->*ccFunctions.ccUpdateAfterCwndTimeout)(path);
    }
    else if (strcmp(msg->getName(), "StartTesting")==0)
    {
        if (sctpMain->testing == false)
        {
            sctpMain->testing = true;
            sctpEV3<<"set testing to true\n";
        }
    }
    else
    {
        sctpAlgorithm->processTimer(msg, event);
    }
     // then state transitions
     return performStateTransition(event);
}

bool SCTPAssociation::processSCTPMessage(SCTPMessage*           sctpmsg,
                                                      const IPvXAddress& msgSrcAddr,
                                                      const IPvXAddress& msgDestAddr)
{
    printConnBrief();

    localAddr  = msgDestAddr;
    localPort  = sctpmsg->getDestPort();
    remoteAddr = msgSrcAddr;
    remotePort = sctpmsg->getSrcPort();

    return process_RCV_Message(sctpmsg, msgSrcAddr, msgDestAddr);
}

SCTPEventCode SCTPAssociation::preanalyseAppCommandEvent(int32 commandCode)
{
    switch (commandCode) {
    case SCTP_C_ASSOCIATE:           return SCTP_E_ASSOCIATE;
    case SCTP_C_OPEN_PASSIVE:        return SCTP_E_OPEN_PASSIVE;
    case SCTP_C_SEND:                return SCTP_E_SEND;
    case SCTP_C_CLOSE:               return SCTP_E_CLOSE;
    case SCTP_C_ABORT:               return SCTP_E_ABORT;
    case SCTP_C_RECEIVE:             return SCTP_E_RECEIVE;
    case SCTP_C_SEND_UNORDERED:      return SCTP_E_SEND;
    case SCTP_C_SEND_ORDERED:        return SCTP_E_SEND;
    case SCTP_C_PRIMARY:             return SCTP_E_PRIMARY;
    case SCTP_C_QUEUE_MSGS_LIMIT:    return SCTP_E_QUEUE_MSGS_LIMIT;
    case SCTP_C_QUEUE_BYTES_LIMIT:   return SCTP_E_QUEUE_BYTES_LIMIT;
    case SCTP_C_SHUTDOWN:            return SCTP_E_SHUTDOWN;
    case SCTP_C_NO_OUTSTANDING:      return SCTP_E_SEND_SHUTDOWN_ACK;
    default:
        sctpEV3<<"commandCode="<<commandCode<<"\n";
        opp_error("Unknown message kind in app command");
                      return (SCTPEventCode)0; // to satisfy compiler
    }
}

bool SCTPAssociation::processAppCommand(cPacket *msg)
{
    printConnBrief();

    // first do actions
    SCTPCommand *sctpCommand = (SCTPCommand *)(msg->removeControlInfo());
    SCTPEventCode event = preanalyseAppCommandEvent(msg->getKind());

    sctpEV3 << "App command: " << eventName(event) << "\n";
    switch (event)
    {
        case SCTP_E_ASSOCIATE: process_ASSOCIATE(event, sctpCommand, msg); break;
        case SCTP_E_OPEN_PASSIVE: process_OPEN_PASSIVE(event, sctpCommand, msg); break;
        case SCTP_E_SEND: process_SEND(event, sctpCommand, msg); break;
        case SCTP_E_CLOSE: process_CLOSE(event); break;
        case SCTP_E_ABORT: process_ABORT(event); break;
        case SCTP_E_RECEIVE: process_RECEIVE_REQUEST(event, sctpCommand); break;
        case SCTP_E_PRIMARY: process_PRIMARY(event, sctpCommand); break;
        case SCTP_E_QUEUE_BYTES_LIMIT: process_QUEUE_BYTES_LIMIT(sctpCommand); break;
        case SCTP_E_QUEUE_MSGS_LIMIT:    process_QUEUE_MSGS_LIMIT(sctpCommand); break;
        case SCTP_E_SHUTDOWN: /*sendShutdown*/
        sctpEV3<<"SCTP_E_SHUTDOWN in state "<<stateName(fsm->getState())<<"\n";
            if (fsm->getState()==SCTP_S_SHUTDOWN_RECEIVED) {
            sctpEV3<<"send shutdown ack\n";
                sendShutdownAck(remoteAddr);
                }
            break;  //I.R.
        case SCTP_E_STOP_SENDING: break;
        case SCTP_E_SEND_SHUTDOWN_ACK:
                /*if (fsm->getState()==SCTP_S_SHUTDOWN_RECEIVED && getOutstandingBytes()==0
                    && qCounter.roomSumSendStreams==0 && transmissionQ->getQueueSize()==0)
                {
                    sendShutdownAck(state->primaryPathIndex);
                }*/
                break;
        default: opp_error("wrong event code");
    }
    delete sctpCommand;
    // then state transitions
    return performStateTransition(event);
}


bool SCTPAssociation::performStateTransition(const SCTPEventCode& event)
{
    sctpEV3<<"performStateTransition\n";
    if (event==SCTP_E_IGNORE)   // e.g. discarded segment
    {
        ev << "Staying in state: " << stateName(fsm->getState()) << " (no FSM event)\n";
        return true;
    }

    // state machine
    int32 oldState = fsm->getState();
    switch (fsm->getState())
    {
        case SCTP_S_CLOSED:
            switch (event)
            {
                case SCTP_E_ABORT: FSM_Goto((*fsm), SCTP_S_CLOSED); break;
                case SCTP_E_OPEN_PASSIVE: FSM_Goto((*fsm), SCTP_S_CLOSED); break;
                case SCTP_E_ASSOCIATE: FSM_Goto((*fsm), SCTP_S_COOKIE_WAIT); break;
                case SCTP_E_RCV_INIT: FSM_Goto((*fsm), SCTP_S_CLOSED); break;
                case SCTP_E_RCV_ABORT: FSM_Goto((*fsm), SCTP_S_CLOSED); break;
                case SCTP_E_RCV_VALID_COOKIE_ECHO: FSM_Goto((*fsm), SCTP_S_ESTABLISHED); break;
                    case SCTP_E_CLOSE: FSM_Goto((*fsm), SCTP_S_CLOSED); break;
                default:;
            }
            break;

        case SCTP_S_COOKIE_WAIT:
            switch (event)
            {
                case SCTP_E_RCV_ABORT: FSM_Goto((*fsm), SCTP_S_CLOSED); break;
                case SCTP_E_ABORT: FSM_Goto((*fsm), SCTP_S_CLOSED); break;
                case SCTP_E_RCV_INIT_ACK: FSM_Goto((*fsm), SCTP_S_COOKIE_ECHOED); break;
                case SCTP_E_RCV_VALID_COOKIE_ECHO: FSM_Goto((*fsm), SCTP_S_ESTABLISHED); break;
                default:;
            }
            break;

        case SCTP_S_COOKIE_ECHOED:
            switch (event)
            {
                case SCTP_E_RCV_ABORT: FSM_Goto((*fsm), SCTP_S_CLOSED); break;
                case SCTP_E_ABORT: FSM_Goto((*fsm), SCTP_S_CLOSED); break;
                case SCTP_E_RCV_COOKIE_ACK:FSM_Goto((*fsm), SCTP_S_ESTABLISHED); break;
                default:;
            }
            break;
        case SCTP_S_ESTABLISHED:
            switch (event)
            {
                case SCTP_E_SEND: FSM_Goto((*fsm), SCTP_S_ESTABLISHED); break;
                case SCTP_E_ABORT: FSM_Goto((*fsm), SCTP_S_CLOSED); break;
                case SCTP_E_RCV_ABORT: FSM_Goto((*fsm), SCTP_S_CLOSED); break;
                case SCTP_E_SHUTDOWN: FSM_Goto((*fsm), SCTP_S_SHUTDOWN_PENDING);    break;
                case SCTP_E_STOP_SENDING: FSM_Goto((*fsm), SCTP_S_SHUTDOWN_PENDING); state->stopSending = true; state->lastTSN = state->nextTSN-1; break;    //I.R.
                case SCTP_E_RCV_SHUTDOWN: FSM_Goto((*fsm), SCTP_S_SHUTDOWN_RECEIVED); break;
                case SCTP_E_CLOSE: FSM_Goto((*fsm), SCTP_S_CLOSED); break;
                default:;
            }
            break;

        case SCTP_S_SHUTDOWN_PENDING:
            switch (event)
            {
                case SCTP_E_RCV_ABORT: FSM_Goto((*fsm), SCTP_S_CLOSED); break;
                case SCTP_E_ABORT: FSM_Goto((*fsm), SCTP_S_CLOSED); break;
                case SCTP_E_NO_MORE_OUTSTANDING: FSM_Goto((*fsm), SCTP_S_SHUTDOWN_SENT); break;
                case SCTP_E_RCV_SHUTDOWN: FSM_Goto((*fsm), SCTP_S_SHUTDOWN_RECEIVED); break;
                case SCTP_E_RCV_SHUTDOWN_ACK: FSM_Goto((*fsm), SCTP_S_CLOSED); break;
                default:;
            }
            break;

        case SCTP_S_SHUTDOWN_RECEIVED:
            switch (event)
            {
                case SCTP_E_ABORT: FSM_Goto((*fsm), SCTP_S_CLOSED); break;
                case SCTP_E_RCV_ABORT: FSM_Goto((*fsm), SCTP_S_CLOSED); break;
                case SCTP_E_NO_MORE_OUTSTANDING:
                            FSM_Goto((*fsm), SCTP_S_SHUTDOWN_ACK_SENT);
                    break;
                case SCTP_E_SHUTDOWN: sendShutdownAck(remoteAddr); /*FSM_Goto((*fsm), SCTP_S_SHUTDOWN_ACK_SENT);*/ break;
                default:;
            }
            break;

        case SCTP_S_SHUTDOWN_SENT:
            switch (event)
            {
                case SCTP_E_ABORT: FSM_Goto((*fsm), SCTP_S_CLOSED); break;
                case SCTP_E_RCV_ABORT: FSM_Goto((*fsm), SCTP_S_CLOSED); break;
                case SCTP_E_RCV_SHUTDOWN_ACK: FSM_Goto((*fsm), SCTP_S_CLOSED); break;
                case SCTP_E_RCV_SHUTDOWN: sendShutdownAck(remoteAddr); FSM_Goto((*fsm), SCTP_S_SHUTDOWN_ACK_SENT); break;
                default:;
            }
            break;

        case SCTP_S_SHUTDOWN_ACK_SENT:
            switch (event)
            {
                case SCTP_E_ABORT: FSM_Goto((*fsm), SCTP_S_CLOSED); break;
                case SCTP_E_RCV_ABORT: FSM_Goto((*fsm), SCTP_S_CLOSED); break;
                case SCTP_E_RCV_SHUTDOWN_COMPLETE:      FSM_Goto((*fsm), SCTP_S_CLOSED); break;
                default:;
            }
            break;

    }

    if (oldState!=fsm->getState())
    {
        ev << "Transition: " << stateName(oldState) << " --> " << stateName(fsm->getState()) << "    (event was: " << eventName(event) << ")\n";
        sctpEV3 << sctpMain->getName() << ": " << stateName(oldState) << " --> " << stateName(fsm->getState()) << "  (on " << eventName(event) << ")\n";
        stateEntered(fsm->getState());
    }
    else
    {
        ev<< "Staying in state: " << stateName(fsm->getState()) << " (event was: " << eventName(event) << ")\n";
    }
    if (event==SCTP_E_ABORT && oldState==fsm->getState() && fsm->getState()==SCTP_S_CLOSED)
        return true;

    if (oldState!=fsm->getState() && fsm->getState()==SCTP_S_CLOSED)
    {
        sctpEV3<<"return false because oldState="<<oldState<<" and new state is closed\n";
        return false;
    }
    else
        return true;
}

void SCTPAssociation::stateEntered(int32 status)
{
    switch (status)
    {
        case SCTP_S_COOKIE_WAIT:
            break;
        case SCTP_S_ESTABLISHED:
        {
            sctpEV3 << "State ESTABLISHED entered" << endl;
            stopTimer(T1_InitTimer);
            if (state->initChunk) {
                delete state->initChunk;
            }
            state->nagleEnabled                   = (bool)sctpMain->par("nagleEnabled");
            state->enableHeartbeats               = (bool)sctpMain->par("enableHeartbeats");
            state->numGapReports                  = sctpMain->par("numGapReports");
            state->maxBurst                       = (uint32)sctpMain->par("maxBurst");
                state->header = 0;
            state->swsLimit                      = (uint32)sctpMain->par("swsLimit");
            state->fastRecoverySupported         = (bool)sctpMain->par("fastRecoverySupported");
            state->reactivatePrimaryPath         = (bool)sctpMain->par("reactivatePrimaryPath");
            sackPeriod                           = (double)sctpMain->par("sackPeriod");
            sackFrequency                        = sctpMain->par("sackFrequency");
            SCTP::AssocStat stat;
            stat.assocId                         = assocId;
            stat.start                           = simulation.getSimTime();
            stat.stop                            = 0;
            stat.rcvdBytes                       = 0;
            stat.ackedBytes                      = 0;
            stat.sentBytes                       = 0;
            stat.transmittedBytes                = 0;
            stat.numFastRtx                      = 0;
            stat.numT3Rtx                        = 0;
            stat.numDups                         = 0;
            stat.numPathFailures                 = 0;
            stat.numForwardTsn                   = 0;
            stat.lifeTime                        = 0;
            stat.throughput                      = 0;
            sctpMain->assocStatMap[stat.assocId] = stat;
            ccModule = sctpMain->par("ccModule");
            switch (ccModule)
            {
                case RFC4960:
                {
                    ccFunctions.ccInitParams                 = &SCTPAssociation::initCCParameters;
                    ccFunctions.ccUpdateAfterSack            = &SCTPAssociation::cwndUpdateAfterSack;
                    ccFunctions.ccUpdateAfterCwndTimeout     = &SCTPAssociation::cwndUpdateAfterCwndTimeout;
                    ccFunctions.ccUpdateAfterRtxTimeout      = &SCTPAssociation::cwndUpdateAfterRtxTimeout;
                    ccFunctions.ccUpdateMaxBurst             = &SCTPAssociation::cwndUpdateMaxBurst;
                    ccFunctions.ccUpdateBytesAcked           = &SCTPAssociation::cwndUpdateBytesAcked;
                    break;
                }
            }
            pmStartPathManagement();
            state->sendQueueLimit = (uint32)sctpMain->par("sendQueueLimit");
            sendEstabIndicationToApp();
            char str[128];
            snprintf(str, sizeof(str), "Cumulated TSN Ack of Association %d", assocId);
            cumTsnAck = new cOutVector(str);
            snprintf(str, sizeof(str), "Number of Gap Blocks in Last SACK of Association %d", assocId);
            numGapBlocks = new cOutVector(str);
            snprintf(str, sizeof(str), "SendQueue of Association %d", assocId);
            sendQueue = new cOutVector(str);
            state->sendQueueLimit = (uint32)sctpMain->par("sendQueueLimit");
            SCTP::VTagPair vtagPair;
            vtagPair.peerVTag     = peerVTag;
            vtagPair.localVTag  = localVTag;
            vtagPair.localPort  = localPort;
            vtagPair.remotePort = remotePort;
            sctpMain->sctpVTagMap[assocId] = vtagPair;
            break;
        }
        case SCTP_S_CLOSED:
        {
            sendIndicationToApp(SCTP_I_CLOSED);
            break;
        }
        case SCTP_S_SHUTDOWN_PENDING:
        {
            if (getOutstandingBytes()==0 && transmissionQ->getQueueSize()==0 && qCounter.roomSumSendStreams==0)
                sendShutdown();
            break;
        }
        case SCTP_S_SHUTDOWN_RECEIVED:
        {
            sctpEV3 << "Entered state SHUTDOWN_RECEIVED, osb=" << getOutstandingBytes()
                      << ", transQ=" << transmissionQ->getQueueSize()
                      << ", scount=" << qCounter.roomSumSendStreams << endl;
            if (getOutstandingBytes()==0 && transmissionQ->getQueueSize()==0 && qCounter.roomSumSendStreams==0) {
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
    SCTPPathMap::iterator pathIterator;
    while((pathIterator = sctpPathMap.begin()) != sctpPathMap.end())
    {
        SCTPPathVariables* path = pathIterator->second;
        sctpEV3 << getFullPath() << " remove path " << path->remoteAddress << endl;
        stopTimer(path->HeartbeatTimer);
        delete path->HeartbeatTimer;
        stopTimer(path->HeartbeatIntervalTimer);
        sctpEV3 << "delete timer " << path->HeartbeatIntervalTimer->getName() << endl;
        delete path->HeartbeatIntervalTimer;
        stopTimer(path->T3_RtxTimer);
        delete path->T3_RtxTimer;
        stopTimer(path->CwndTimer);
        delete path->CwndTimer;
        delete path;
        sctpPathMap.erase(pathIterator);
    }
}

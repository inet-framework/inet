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


#include <string.h>
#include <assert.h>
#include "SCTP.h"
#include "SCTPAssociation.h"
#include "SCTPCommand_m.h"
#include "IPControlInfo_m.h"
#include "SCTPQueue.h"
#include "SCTPAlgorithm.h"

#include <sstream>


SCTPPathVariables:: SCTPPathVariables(IPvXAddress addr, SCTPAssociation* assoc)
{
	association = assoc;
	remoteAddress = addr;
	activePath = true;
	confirmed = false;
	primaryPathCandidate = false;
	pathErrorCount =0;
	pathErrorThreshold = assoc->getSctpMain()->par("pathMaxRetrans");
	if (!pathErrorThreshold)
		pathErrorThreshold = PATH_MAX_RETRANS;
	pathRto = assoc->getSctpMain()->par("rtoInitial");
	heartbeatTimeout = pathRto;
	double interval = (double)assoc->getSctpMain()->par("hbInterval");

	if (!interval)
		interval = HB_INTERVAL;
	heartbeatIntervalTimeout = pathRto+interval;
	srtt = pathRto;
	lastAckTime = 0;
	forceHb = false;
	partialBytesAcked = 0;
	outstandingBytes = 0;
	RoutingTableAccess routingTableAccess;
    	InterfaceEntry *rtie = routingTableAccess.get()->getInterfaceForDestAddr(remoteAddress.get4());
	pmtu = rtie->getMTU();
	hbWasAcked = false;
	rttvar = 0.0;

	cwndTimeout = pathRto;
	cwnd = 0;
	updateTime = 0.0;
	cwndAdjustmentTime=0;
	char str[70];
	sprintf(str, "HB_TIMER %d:%s",assoc->assocId,addr.str().c_str());
	HeartbeatTimer = new cMessage(str);
	sprintf(str, "HB_INT_TIMER %d:%s",assoc->assocId,addr.str().c_str());
	HeartbeatIntervalTimer = new cMessage(str);
	sprintf(str, "CWND_TIMER %d:%s",assoc->assocId,addr.str().c_str());
	CwndTimer = new cMessage(str);
	sprintf(str, "RTX_TIMER %d:%s",assoc->assocId,addr.str().c_str());
	T3_RtxTimer = new cMessage(str);
	HeartbeatTimer->setContextPointer(association);
	HeartbeatIntervalTimer->setContextPointer(association);
	CwndTimer->setContextPointer(association);
	T3_RtxTimer->setContextPointer(association);
	sprintf(str, "Slow Start Threshold %d:%s",assoc->assocId,addr.str().c_str());
	pathSsthresh = new cOutVector(str);
	sprintf(str, "Congestion Window %d:%s",assoc->assocId,addr.str().c_str());
	pathCwnd = new cOutVector(str);
	sprintf(str, "TSN sent for assoc %d on path %s",assoc->assocId,addr.str().c_str());
	pathTSN = new cOutVector(str);
	sprintf(str, "TSN received for assoc %d on path %s",assoc->assocId,addr.str().c_str());
	pathRcvdTSN = new cOutVector(str);
	sprintf(str, "RTO for assoc %d on path %s",assoc->assocId,addr.str().c_str());
	pathRTO = new cOutVector(str);
	sprintf(str, "RTT for assoc %d on path %s",assoc->assocId,addr.str().c_str());
	pathRTT = new cOutVector(str);
	sprintf(str, "peerRwnd/RTT for assoc %d on path %s",assoc->assocId,addr.str().c_str());
	SCTPPathInfo* pinfo = new SCTPPathInfo();
	pinfo->setRemoteAddress(addr);
	T3_RtxTimer->setControlInfo(pinfo);
	HeartbeatTimer->setControlInfo(pinfo->dup());
	HeartbeatIntervalTimer->setControlInfo(pinfo->dup());
	CwndTimer->setControlInfo(pinfo->dup());

}

SCTPPathVariables::~SCTPPathVariables()
{

}

SCTPDataVariables::SCTPDataVariables()
{
	userData=NULL;
	ordered = true;
	len = 0;
	tsn = 0;
	sid = 0;
	ssn = 0;
	ppid = 0;
	gapReports = 0;
	enqueuingTime = 0;
	sendTime = 0;
	ackTime = 0;
	expiryTime = 0;
	hasBeenAcked = false;
	hasBeenRemoved = false;
	hasBeenAbandoned = false;
	hasBeenFastRetransmitted = false;
	countsAsOutstanding = false;
	lastDestination = IPvXAddress("0.0.0.0");
	nextDestination = IPvXAddress("0.0.0.0");
	initialDestination = IPvXAddress("0.0.0.0");
	numberOfTransmissions = 0;
	numberOfRetransmissions = 0;
	booksize = 0;
}

SCTPDataVariables::~SCTPDataVariables()
{

}

SCTPStateVariables::SCTPStateVariables()
{
uint32 i;
	active 			= false;
	fork 			= false;
	initReceived 		= false;
	cookieEchoReceived	= false;
	ackPointAdvanced 	= false;
	swsAvoidanceInvoked	= false;
	firstChunkReceived 	= false;
	probingIsAllowed	= false;
	zeroWindowProbing	= true;
	alwaysBundleSack	= true;
	fastRecoverySupported	= true;
	reactivatePrimaryPath 	= false;
	fastRecoveryActive 	= false;
	newChunkReceived 	= false;
	dataChunkReceived 	= false;
	sackAllowed 		= false;
	fragment		= false;
	resetPending		= false;
	stopReceiving		= false;
	stopOldData		= false;
	stopSending		= false;
	inOut			= false;
	queueUpdate		= false;
	firstDataSent		= false;
	peerWindowFull		= false;
	zeroWindow 		= false;
	appSendAllowed		= true;
	noMoreOutstanding 	= false;
	primaryPathIndex 	= IPvXAddress("0.0.0.0");
	lastUsedDataPath 	= IPvXAddress("0.0.0.0");
	lastDataSourceAddress 	= IPvXAddress("0.0.0.0");
	nextDest		= IPvXAddress("0.0.0.0");
	shutdownChunk		= NULL;
	initChunk		= NULL;
	cookieChunk		= NULL;
	sctpmsg			= NULL;
	initRexmitTimeout 	= SCTP_TIMEOUT_INIT_REXMIT;
	localRwnd 		= SCTP_DEFAULT_ARWND;
	errorCount		= 0;
	initRetransCounter 	= 0;
	nextTSN 		= 0;
	cTsnAck 		= 0;
	lastTsnAck		= 0;
	highestTsnReceived 	= 0;
	highestTsnAcked 	= 0;
	highestTsnStored	= 0;
	nextRSid		= 0;
	ackState 		= 0;
	lastStreamScheduled 	= 0;
	fastRecoveryExitPoint	= 0;
	peerRwnd 		= 0;
	initialPeerRwnd		= 0;
	assocPmtu 		= 0;
	queuedBytes 		= 0;
	messagesToPush		= 0;
	pushMessagesLeft	= 0;
	numGaps 		= 0;
	msgNum 			= 0;
	bytesRcvd 		= 0;
	queuedMessages		= 0;
	queueLimit		= 0;
	queuedRcvBytes		= 0;
	probingTimeout		= 1;
	numRequests 		= 0;
	for (i=0; i<100; i++)
		numMsgsReq[i] = 0;

	for (i=0; i<MAX_GAP_COUNT; i++)
	{
		gapStartList[i] = 0;
		gapStopList[i] = 0;
	}

	for (i=0; i<32;i++)
	{
		localTieTag[i]=0;
		peerTieTag[i]=0;
	}

	count = 0;
}

SCTPStateVariables::~SCTPStateVariables()
{

}

//
// FSM framework, SCTP FSM
//

SCTPAssociation::SCTPAssociation(SCTP *_mod, int32 _appGateIndex, int32 _assocId)
{
	sctpMain = _mod;
	appGateIndex = _appGateIndex;
	assocId = _assocId;

	ev<<"SCTPAssociationBase:SCTPAssociation new assocId="<<assocId<<"\n";

	localPort = remotePort = 0;
	localVTag = 0;
	peerVTag = 0;
	numberOfRemoteAddresses=0;
	char fsmname[24];
	sprintf(fsmname, "fsm-%d", assocId);
	fsm = new cFSM();
	fsm->setName(fsmname);
	fsm->setState(SCTP_S_CLOSED);
	inboundStreams = SCTP_DEFAULT_INBOUND_STREAMS;
	outboundStreams = SCTP_DEFAULT_OUTBOUND_STREAMS;
	// queues and algorithm will be created on active or passive open
	transmissionQ = NULL;
	retransmissionQ = NULL;
	sctpAlgorithm = NULL;
	state = NULL;
	SCTPPathInfo* pinfo = new SCTPPathInfo();
	pinfo->setRemoteAddress(IPvXAddress("0.0.0.0"));
	char timername[70];
	sprintf(timername, "T1_INIT of assoc %d", assocId);
	T1_InitTimer = new cMessage(timername);
	sprintf(timername, "T2_SHUTDOWN of assoc %d", assocId);
	T2_ShutdownTimer = new cMessage(timername);
	sprintf(timername, "T5_SHUTDOWN_GUARD of assoc %d", assocId);
	T5_ShutdownGuardTimer = new cMessage(timername);
	sprintf(timername, "SACK_TIMER of assoc %d", assocId);
	SackTimer = new cMessage(timername);

	if (sctpMain->testTimeout>0)
	{
		StartTesting = new cMessage("StartTesting");
		StartTesting->setContextPointer(this);
		StartTesting->setControlInfo(pinfo->dup());
		scheduleTimeout(StartTesting, sctpMain->testTimeout);
	}
	sackPeriod = SACK_DELAY;
	T1_InitTimer->setContextPointer(this);
	T2_ShutdownTimer->setContextPointer(this);
	SackTimer->setContextPointer(this);
	T5_ShutdownGuardTimer->setContextPointer(this);

	T1_InitTimer->setControlInfo(pinfo);
	T2_ShutdownTimer->setControlInfo(pinfo->dup());
	SackTimer->setControlInfo(pinfo->dup());
	T5_ShutdownGuardTimer->setControlInfo(pinfo->dup());
	advRwnd = NULL;
	quBytes = NULL;
	cumTsnAck = NULL;
	sendQueue = NULL;
	qCounter.roomSumSendStreams = 0;
	qCounter.bookedSumSendStreams = 0;
	qCounter.roomSumRcvStreams = 0;
	bytes.chunk = false;
	bytes.packet = false;
	bytes.bytesToSend = 0;
	advRwnd = new cOutVector();
	sprintf(timername, "Advertised Receiver Window of assoc %d", assocId);
	advRwnd->setName(timername);
	quBytes = new cOutVector();
	sprintf(timername, "Queued Bytes of assoc %d", assocId);
	quBytes->setName(timername);
	ssModule = sctpMain->par("ssModule");
	switch (ssModule)
	{
		case ROUND_ROBIN:
		{
			ssFunctions.ssInitStreams = &SCTPAssociation::initStreams;
			ssFunctions.ssGetNextSid = &SCTPAssociation::streamScheduler;
			ssFunctions.ssUsableStreams = &SCTPAssociation::numUsableStreams;
		}
	}
}

SCTPAssociation::~SCTPAssociation()
{
	delete T1_InitTimer;
	delete T2_ShutdownTimer;
	delete T5_ShutdownGuardTimer;
	delete SackTimer;
	delete fsm;
	delete advRwnd;
	delete quBytes;
	delete cumTsnAck;
	delete sendQueue;
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
	else if (msg==StartTesting)
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

bool SCTPAssociation::processSCTPMessage(SCTPMessage *sctpmsg, IPvXAddress msgSrcAddr, IPvXAddress msgDestAddr)
{

	printConnBrief();

	localAddr=msgDestAddr;
	localPort=sctpmsg->getDestPort();

	remoteAddr=msgSrcAddr;
	remotePort=sctpmsg->getSrcPort();

	return process_RCV_Message(sctpmsg, msgSrcAddr, msgDestAddr);
}

SCTPEventCode SCTPAssociation::preanalyseAppCommandEvent(int32 commandCode)
{
    switch (commandCode)
    {
	case SCTP_C_ASSOCIATE:	return SCTP_E_ASSOCIATE;
	case SCTP_C_OPEN_PASSIVE:	return SCTP_E_OPEN_PASSIVE;
	case SCTP_C_SEND:		return SCTP_E_SEND;
	case SCTP_C_CLOSE:		return SCTP_E_CLOSE;
	case SCTP_C_ABORT:		return SCTP_E_ABORT;
	case SCTP_C_RECEIVE:	  	return SCTP_E_RECEIVE;
	case SCTP_C_SEND_UNORDERED:	return SCTP_E_SEND;
	case SCTP_C_SEND_ORDERED: 	return SCTP_E_SEND;
	case SCTP_C_PRIMARY:		return SCTP_E_PRIMARY;
	case SCTP_C_QUEUE:		return SCTP_E_QUEUE;
	case SCTP_C_SHUTDOWN:		return SCTP_E_SHUTDOWN;
	case SCTP_C_NO_OUTSTANDING:	return SCTP_E_SEND_SHUTDOWN_ACK;
        default: sctpEV3<<"commandCode="<<commandCode<<"\n";
		opp_error("Unknown message kind in app command");
                 return (SCTPEventCode)0; // to satisfy compiler
    }
}

bool SCTPAssociation::processAppCommand(cPacket *msg)
{
	printConnBrief();

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
		case SCTP_E_QUEUE: process_QUEUE(sctpCommand); break;
		case SCTP_E_SHUTDOWN: /*sendShutdown*/ break;  //I.R.
		case SCTP_E_STOP_SENDING: break;
		case SCTP_E_SEND_SHUTDOWN_ACK: if (fsm->getState()==SCTP_S_SHUTDOWN_RECEIVED && getOutstandingBytes()==0) sendShutdownAck(state->primaryPathIndex); break;
		default: opp_error("wrong event code");
	}
	delete sctpCommand;
	// then state transitions
	return performStateTransition(event);
}


bool SCTPAssociation::performStateTransition(const SCTPEventCode& event)
{
	sctpEV3<<"performStateTransition\n";
	if (event==SCTP_E_IGNORE)
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
				case SCTP_E_SHUTDOWN: FSM_Goto((*fsm), SCTP_S_SHUTDOWN_PENDING);  break;
				case SCTP_E_STOP_SENDING: FSM_Goto((*fsm), SCTP_S_SHUTDOWN_PENDING); state->stopSending = true; state->lastTSN = state->nextTSN-1; break;  //I.R.
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
				case SCTP_E_NO_MORE_OUTSTANDING: FSM_Goto((*fsm), SCTP_S_SHUTDOWN_ACK_SENT); break;
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
				case SCTP_E_RCV_SHUTDOWN_COMPLETE:     FSM_Goto((*fsm), SCTP_S_CLOSED); break;
				default:;
			}
			break;

	}

	if (oldState!=fsm->getState())
	{
		ev << "Transition: " << stateName(oldState) << " --> " << stateName(fsm->getState()) << "  (event was: " << eventName(event) << ")\n";
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
			sctpEV3<<"State ESTABLISHED entered\n";
			char str[70];
			stopTimer(T1_InitTimer);
			if (state->initChunk)
				delete state->initChunk;
			state->nagleEnabled = (bool)sctpMain->par("nagleEnabled");
				state->header = 0;
			state->swsLimit = (uint32)sctpMain->par("swsLimit");
			state->reactivatePrimaryPath = (bool)sctpMain->par("reactivatePrimaryPath");
			state->fragment = (bool)sctpMain->par("fragment");
			if (!state->fragment)
				state->fragment = false;
			sackPeriod = (double)sctpMain->par("sackPeriod");
			sackFrequency = sctpMain->par("sackFrequency");
			SCTP::AssocStat stat;
			stat.assocId = assocId;
			stat.start = simulation.getSimTime();
			stat.stop = 0;
			stat.rcvdBytes=0;
			stat.ackedBytes=0;
			stat.sentBytes=0;
			stat.transmittedBytes=0;
			stat.numFastRtx=0;
			stat.numT3Rtx=0;
			stat.numPathFailures=0;
			stat.numForwardTsn=0;
			stat.lifeTime=0;
			stat.throughput=0;
			sctpMain->assocStatMap[stat.assocId] = stat;
			ccModule = sctpMain->par("ccModule");
			switch (ccModule)
			{
				case RFC4960:
				{
					ccFunctions.ccInitParams = &SCTPAssociation::initCCParameters;
					ccFunctions.ccUpdateAfterSack = &SCTPAssociation::cwndUpdateAfterSack;
					ccFunctions.ccUpdateAfterCwndTimeout = &SCTPAssociation::cwndUpdateAfterCwndTimeout;
					ccFunctions.ccUpdateAfterRtxTimeout = &SCTPAssociation::cwndUpdateAfterRtxTimeout;
					ccFunctions.ccUpdateMaxBurst = &SCTPAssociation::cwndUpdateMaxBurst;
					ccFunctions.ccUpdateBytesAcked = &SCTPAssociation::cwndUpdateBytesAcked;
					break;
				}
			}
			sendEstabIndicationToApp();
			pmStartPathManagement();
			sprintf(str, "Cumulated TSN Ack of assoc %d", assocId);
			cumTsnAck = new cOutVector(str);
			sprintf(str, "SendQueue of assoc %d", assocId);
			sendQueue = new cOutVector(str);
			state->sendQueueLimit = (uint32)sctpMain->par("sendQueueLimit");
			SCTP::VTagPair vtagPair;
			vtagPair.peerVTag = peerVTag;
			vtagPair.localPort = localPort;
			vtagPair.remotePort = remotePort;
			sctpMain->sctpVTagMap[vtagPair] = this->assocId;
			break;
	}
		case SCTP_S_CLOSED:
		{
			sendIndicationToApp(SCTP_I_CLOSED);
			break;
		}
		case SCTP_S_SHUTDOWN_PENDING:
		{
			if (getOutstandingBytes()==0 && transmissionQ->getQueueSize()==0 && qCounter.roomSumSendStreams==0 && fsm->getState() == SCTP_S_SHUTDOWN_PENDING) sendShutdown();
			break;
		}
	}
}

void SCTPAssociation::removePath()
{
SCTPPathMap::iterator j;
	while((j=sctpPathMap.begin())!=sctpPathMap.end())
	{
		stopTimer(j->second->HeartbeatTimer);
		delete j->second->HeartbeatTimer;
		stopTimer(j->second->HeartbeatIntervalTimer);
		delete j->second->HeartbeatIntervalTimer;
		stopTimer(j->second->T3_RtxTimer);
		delete j->second->T3_RtxTimer;
		stopTimer(j->second->CwndTimer);
		delete j->second->CwndTimer;
		delete j->second->pathSsthresh;
		delete j->second->pathCwnd;
		delete j->second->pathTSN;
		delete j->second->pathRcvdTSN;
		delete j->second->pathRTO;
		delete j->second->pathRTT;
		delete j->second;
		sctpPathMap.erase(j);
	}
}



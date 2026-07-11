//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/pcep/Pcc.h"

#include <algorithm>

#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet {

Define_Module(Pcc);

simsignal_t Pcc::sessionUpSignal = registerSignal("sessionUp");
simsignal_t Pcc::sessionDownSignal = registerSignal("sessionDown");

Pcc::Pcc()
{
}

Pcc::~Pcc()
{
    cancelAndDelete(reconnectTimer);
    cancelAndDelete(keepAliveSendTimer);
    cancelAndDelete(sessionHoldTimer);
    for (auto *s : deadSockets)
        delete s;
    delete socket;
}

void Pcc::initialize(int stage)
{
    RoutingProtocolBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        pceAddressStr = par("pceAddress").stdstringValue();
        pcePort = par("pcePort");
        localAddressStr = par("localAddress").stdstringValue();
        keepaliveTime = par("keepaliveTime");
        deadTimer = par("deadTimer");
        reconnectInterval = par("reconnectInterval");

        reconnectTimer = new cMessage("PcepReconnectTimer");

        WATCH(numSent);
        WATCH(numReceived);
    }
}

void Pcc::handleStartOperation(LifecycleOperation *operation)
{
    // (Re)start the connection attempt cycle -- covers both the very first start and
    // a later restart alike (mirrors Ldp's own unconditional Hello-timer (re)arm at
    // the end of handleStartOperation).
    scheduleAfter(exponential(0.01), reconnectTimer);
}

void Pcc::handleStopOperation(LifecycleOperation *operation)
{
    clearState();
    if (socket)
        socket->close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void Pcc::handleCrashOperation(LifecycleOperation *operation)
{
    clearState();
    if (socket)
        socket->destroy();
}

void Pcc::clearState()
{
    cancelEvent(reconnectTimer);
    cancelAndDelete(keepAliveSendTimer);
    keepAliveSendTimer = nullptr;
    cancelAndDelete(sessionHoldTimer);
    sessionHoldTimer = nullptr;
    state = PCEP_NONEXISTENT;
    // Phase 2: any PCReq outstanding on the session being torn down will never see
    // its PCRep now -- drop it rather than leaving a permanently-PENDING entry that
    // would silently swallow every future requestPathComputation() retry for the
    // same (source, dest, bandwidth, priority, affinity) tuple without ever
    // resending (there is no session left to answer on anyway; a fresh request is
    // sent once/if the session comes back up).
    pendingRequests.clear();
}

void Pcc::handleMessageWhenUp(cMessage *msg)
{
    // delete sockets torn down inside a previous callback (safe now: we are no
    // longer executing inside their processMessage())
    for (auto *s : deadSockets)
        delete s;
    deadSockets.clear();

    if (msg->isSelfMessage()) {
        if (msg == reconnectTimer)
            processReconnectTimeout(msg);
        else if (!strcmp(msg->getName(), "PcepKeepAliveSendTimer"))
            processKeepAliveSendTimeout(msg);
        else if (!strcmp(msg->getName(), "PcepSessionHoldTimer"))
            processSessionHoldTimeout(msg);
        else
            throw cRuntimeError("Pcc: unknown self message '%s'", msg->getName());
    }
    else if (socket && socket->belongsToSocket(msg)) {
        socket->processMessage(msg);
    }
    else {
        // an in-flight TCP indication may arrive for a socket we just tore down on
        // session loss; drop it instead of aborting
        EV_WARN << "no socket found for msg '" << msg->getName() << "', dropping\n";
        delete msg;
    }

    if (operationalState == State::STOPPING_OPERATION) {
        if (socket && socket->isOpen())
            return;
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
    }
}

void Pcc::connectToPce()
{
    if (socket) {
        // shouldn't normally happen (only one attempt is ever in flight at a time),
        // but be defensive rather than leaking or double-using a stale socket
        deadSockets.push_back(socket);
        socket = nullptr;
    }

    socket = new TcpSocket();
    socket->setOutputGate(gate("socketOut"));
    socket->setCallback(this);

    // See Pcc.ned's doc comment on "localAddress": binding to an explicit, stable
    // local address matters whenever the PCE is more than one hop away in an IGP/
    // TED-routed backbone -- this model's own Ted/LinkStateRouting (like a real IGP)
    // only guarantees reachability to each router's OWN routerId, not to whichever
    // arbitrary outgoing-interface address an unspecified bind would otherwise let
    // the stack pick as this connection's source.
    L3Address localAddress = localAddressStr.empty() ? L3Address() : L3AddressResolver().resolve(localAddressStr.c_str());
    socket->bind(localAddress, -1);

    L3Address pceAddress = L3AddressResolver().resolve(pceAddressStr.c_str());
    EV_INFO << "Connecting to PCE " << pceAddress << ":" << pcePort << "\n";
    socket->connect(pceAddress, pcePort);
}

void Pcc::sendToPce(Packet *pk)
{
    numSent++;
    socket->send(pk);

    // RFC 5440 Section 7.3: sending ANY PCEP message resets the KeepAlive send timer
    // (it only exists once the session is OPERATIONAL)
    if (keepAliveSendTimer != nullptr)
        rescheduleAfter(negotiatedKeepaliveTime, keepAliveSendTimer);
}

void Pcc::sendOpen()
{
    Packet *pk = new Packet("Pcep-Open");
    const auto& open = makeShared<PcepOpen>();
    open->setChunkLength(PCEP_OPEN_MESSAGE_BYTES);
    open->setType(PCEP_OPEN);
    open->setKeepaliveTime((uint8_t)keepaliveTime.inUnit(SIMTIME_S));
    open->setDeadTimer((uint8_t)deadTimer.inUnit(SIMTIME_S));
    open->setSid(sid);
    pk->insertAtBack(open);

    sendToPce(pk);
    EV_INFO << "Open sent to PCE (keepaliveTime=" << keepaliveTime << ", deadTimer=" << deadTimer
            << ", sid=" << (int)sid << ")\n";
}

void Pcc::sendKeepalive()
{
    Packet *pk = new Packet("Pcep-Keepalive");
    const auto& ka = makeShared<PcepKeepalive>();
    ka->setChunkLength(PCEP_KEEPALIVE_MESSAGE_BYTES);
    ka->setType(PCEP_KEEPALIVE);
    pk->insertAtBack(ka);

    sendToPce(pk);
    EV_INFO << "Keepalive sent to PCE\n";
}

void Pcc::sendPcreq(const PceRequest& request)
{
    Packet *pk = new Packet("Pcep-PCReq");
    const auto& req = makeShared<PcepPcreq>();
    req->setChunkLength(PCEP_PCREQ_MESSAGE_BYTES);
    req->setType(PCEP_PCREQ);
    req->setRequestId(request.requestId);
    req->setSrcAddress(request.srcAddress);
    req->setDstAddress(request.destAddress);
    req->setBandwidth(request.bandwidth);
    req->setSetupPriority(request.setupPriority);
    req->setIncludeAny(request.includeAny);
    req->setExcludeAny(request.excludeAny);
    pk->insertAtBack(req);

    sendToPce(pk);
    EV_INFO << "PCReq sent to PCE (requestId=" << request.requestId << ", src=" << request.srcAddress
            << ", dst=" << request.destAddress << ", bandwidth=" << request.bandwidth
            << ", setupPriority=" << request.setupPriority << ")\n";
}

Pcc::PceComputationResult Pcc::requestPathComputation(const Ipv4Address& srcAddress, const Ipv4Address& destAddress,
        double bandwidth, int setupPriority, uint32_t includeAny, uint32_t excludeAny, EroVector& outEro)
{
    // Called cross-module (from ~RsvpTe, a sibling app in the same node, via pccModule) --
    // required so the simulation kernel attributes any send() this call ends up doing
    // (sendPcreq() below) to THIS module, not the caller's; mirrors RsvpTe's own
    // addSession()/delSession() (called cross-module from ScenarioManager).
    Enter_Method("requestPathComputation");

    for (auto it = pendingRequests.begin(); it != pendingRequests.end(); ++it) {
        if (it->srcAddress == srcAddress && it->destAddress == destAddress && it->bandwidth == bandwidth
                && it->setupPriority == setupPriority && it->includeAny == includeAny && it->excludeAny == excludeAny) {
            if (!it->hasResult)
                return PceComputationResult::PENDING; // PCReq already sent, PCRep not received yet

            PceComputationResult result = it->success ? PceComputationResult::COMPUTED : PceComputationResult::NO_PATH;
            if (it->success)
                outEro = it->ero;
            pendingRequests.erase(it); // one-shot: this phase's PCEP is stateless (see class doc)
            return result;
        }
    }

    if (state != PCEP_OPERATIONAL) {
        EV_INFO << "cannot request PCE path computation to " << destAddress << ": PCEP session not OPERATIONAL\n";
        return PceComputationResult::NO_PATH;
    }

    PceRequest request;
    request.srcAddress = srcAddress;
    request.destAddress = destAddress;
    request.bandwidth = bandwidth;
    request.setupPriority = setupPriority;
    request.includeAny = includeAny;
    request.excludeAny = excludeAny;
    request.requestId = ++pceRequestIdCounter;
    pendingRequests.push_back(request);
    sendPcreq(request);
    return PceComputationResult::PENDING;
}

void Pcc::socketEstablished(TcpSocket *sock)
{
    ASSERT(sock == socket);
    EV_INFO << "TCP connection established with PCE " << socket->getRemoteAddress() << "\n";

    // RFC 5440 Section 6.2: the transport connection alone does not make the session
    // usable -- Open/Keepalive negotiation must complete first (see processOPEN/
    // processKEEPALIVE). sessionUp is deferred to that OPERATIONAL transition.
    state = PCEP_INITIALIZED;

    // The Pcc always plays the active role (PCEP has no discovery phase, unlike
    // Ldp's Hello-driven active/passive election): we initiate parameter negotiation
    // immediately once connected.
    sid = ++sidCounter;
    sendOpen();
    state = PCEP_OPENSENT;
}

void Pcc::socketDataArrived(TcpSocket *sock)
{
    ASSERT(sock == socket);
    auto queue = socket->getReadBuffer();
    while (queue->has<PcepMessage>()) {
        numReceived++;
        auto header = queue->pop<PcepMessage>();
        processPcepPacketFromTcp(header);
    }
}

void Pcc::handleTcpConnectionDown(TcpSocket *sock)
{
    if (sock != socket)
        return; // a stale/already-replaced socket's callback; nothing to do

    bool wasOperational = (state == PCEP_OPERATIONAL);
    cancelAndDelete(keepAliveSendTimer);
    keepAliveSendTimer = nullptr;
    cancelAndDelete(sessionHoldTimer);
    sessionHoldTimer = nullptr;
    if (wasOperational)
        emit(sessionDownSignal, (long)sid);
    state = PCEP_NONEXISTENT;
    pendingRequests.clear(); // see clearState()'s doc comment for the rationale

    // defer the delete: we are called from inside socket->processMessage(), which
    // still touches the socket after this callback returns
    deadSockets.push_back(socket);
    socket = nullptr;

    EV_INFO << "PCEP session with PCE lost; reconnecting in " << reconnectInterval << "\n";
    rescheduleAfter(reconnectInterval, reconnectTimer);
}

void Pcc::socketPeerClosed(TcpSocket *sock)
{
    EV_INFO << "PCE " << sock->getRemoteAddress() << " closed the TCP connection, closing here as well\n";
    sock->close();
}

void Pcc::socketClosed(TcpSocket *sock)
{
    EV_WARN << "PCEP TCP connection to PCE closed\n";
    handleTcpConnectionDown(sock);
}

void Pcc::socketFailure(TcpSocket *sock, int code)
{
    EV_WARN << "PCEP TCP connection to PCE broken\n";
    handleTcpConnectionDown(sock);
}

void Pcc::processPcepPacketFromTcp(const Ptr<const PcepMessage>& pcepMsg)
{
    // RFC 5440 Section 7.3: receiving ANY PCEP message resets the session hold timer
    // (it only exists once the session is OPERATIONAL -- see processKEEPALIVE's
    // OPENREC->OPERATIONAL transition)
    if (sessionHoldTimer != nullptr)
        rescheduleAfter(peerDeadTimer, sessionHoldTimer);

    switch (pcepMsg->getType()) {
        case PCEP_OPEN:
            processOPEN(pcepMsg);
            break;

        case PCEP_KEEPALIVE:
            processKEEPALIVE();
            break;

        case PCEP_PCREP:
            processPCREP(pcepMsg);
            break;

        default:
            // an unrecognized message type from a peer must not abort the simulation
            EV_WARN << "ignoring an unrecognized PCEP message of type " << pcepMsg->getType() << endl;
            break;
    }
}

void Pcc::processOPEN(const Ptr<const PcepMessage>& pcepMsg)
{
    const auto& open = CHK(dynamicPtrCast<const PcepOpen>(pcepMsg));
    uint8_t peerKeepaliveTime = open->getKeepaliveTime();
    uint8_t peerDeadTimerSec = open->getDeadTimer();

    EV_INFO << "Open received from PCE (peer keepaliveTime=" << (int)peerKeepaliveTime << "s, peer deadTimer="
            << (int)peerDeadTimerSec << "s)" << endl;

    if (state != PCEP_OPENSENT && state != PCEP_INITIALIZED) {
        // a duplicate/out-of-sequence Open is a peer/protocol anomaly, not a model error
        EV_WARN << "unexpected Open in session state " << pcepSessionStateName(state) << ", ignoring" << endl;
        return;
    }

    if (peerKeepaliveTime == 0) {
        // RFC 5440 Section 7.3/7.15: unacceptable session parameter -> reject and close
        EV_WARN << "PCE proposed an unacceptable KeepAlive Time (0), rejecting session" << endl;
        if (socket)
            socket->close();
        return;
    }

    negotiatedKeepaliveTime = std::min(keepaliveTime, SimTime(peerKeepaliveTime, SIMTIME_S));
    peerDeadTimer = SimTime(peerDeadTimerSec, SIMTIME_S);

    if (state == PCEP_INITIALIZED) {
        // shouldn't normally happen (the Pcc always sends its own Open immediately on
        // socketEstablished, before the Pce could have anything to send us yet) --
        // handled defensively anyway, mirroring Ldp::processINITIALIZATION's
        // symmetric passive-side branch.
        sendOpen();
    }
    sendKeepalive();
    state = PCEP_OPENREC;
    EV_INFO << "session with PCE negotiated keepaliveTime=" << negotiatedKeepaliveTime << ", state OPENREC" << endl;
}

void Pcc::processKEEPALIVE()
{
    EV_INFO << "Keepalive received from PCE" << endl;

    if (state == PCEP_OPENREC) {
        state = PCEP_OPERATIONAL;
        EV_INFO << "PCEP session with PCE " << socket->getRemoteAddress() << " is now OPERATIONAL" << endl;
        emit(sessionUpSignal, (long)sid);

        // start KeepAlive-based session liveness (RFC 5440 Section 7.3): initial arm
        // of both timers
        ASSERT(keepAliveSendTimer == nullptr && sessionHoldTimer == nullptr);
        keepAliveSendTimer = new cMessage("PcepKeepAliveSendTimer");
        scheduleAfter(negotiatedKeepaliveTime, keepAliveSendTimer);
        sessionHoldTimer = new cMessage("PcepSessionHoldTimer");
        scheduleAfter(peerDeadTimer, sessionHoldTimer);
    }
    else if (state == PCEP_OPERATIONAL) {
        // steady-state KeepAlive refresh; the session hold timer reset already
        // happened unconditionally at the top of processPcepPacketFromTcp
        EV_DETAIL << "Keepalive refresh from PCE (session already OPERATIONAL)" << endl;
    }
    else {
        EV_WARN << "unexpected Keepalive in session state " << pcepSessionStateName(state) << ", ignoring" << endl;
    }
}

void Pcc::processPCREP(const Ptr<const PcepMessage>& pcepMsg)
{
    const auto& rep = CHK(dynamicPtrCast<const PcepPcrep>(pcepMsg));

    for (auto& request : pendingRequests) {
        if (request.requestId == rep->getRequestId()) {
            if (request.hasResult) {
                // a duplicate/retransmitted PCRep for an already-answered request is a
                // peer/protocol anomaly, not a model error
                EV_WARN << "duplicate PCRep for requestId " << request.requestId << ", ignoring" << endl;
                return;
            }
            request.hasResult = true;
            request.success = !rep->getNoPath();
            if (request.success) {
                request.ero = rep->getEro();
                EV_INFO << "PCRep received from PCE: requestId=" << request.requestId
                        << ", ERO with " << request.ero.size() << " hop(s)" << endl;
            }
            else
                EV_INFO << "PCRep received from PCE: requestId=" << request.requestId << ", NO-PATH" << endl;
            return;
        }
    }

    // a PCRep naming a requestId we have no record of (e.g. the request was already
    // consumed and erased, or belongs to a session that bounced in between) is a
    // peer/protocol anomaly, not a model error
    EV_WARN << "PCRep for unknown requestId " << rep->getRequestId() << ", ignoring" << endl;
}

void Pcc::processKeepAliveSendTimeout(cMessage *msg)
{
    ASSERT(msg == keepAliveSendTimer);
    // Guard against the narrow window where the socket has already left CONNECTED
    // (e.g. a graceful close in progress) but the teardown callback hasn't fired yet
    // to cancel this timer: skip this cycle instead of sending.
    if (socket == nullptr || socket->getState() != TcpSocket::CONNECTED) {
        EV_DETAIL << "skipping scheduled Keepalive to PCE: session is closing\n";
        return;
    }
    EV_DETAIL << "no PCEP message sent to PCE for " << negotiatedKeepaliveTime << ", sending Keepalive" << endl;
    sendKeepalive();
}

void Pcc::processSessionHoldTimeout(cMessage *msg)
{
    ASSERT(msg == sessionHoldTimer);

    // RFC 5440 Section 7.3: no PCEP message (Keepalive or otherwise) was received
    // from the PCE within the DeadTimer IT advertised in its own Open -- the session
    // is presumed dead.
    EV_WARN << "DeadTimer expired for PCEP session with PCE: no PCEP message received within " << peerDeadTimer
            << "; closing the session (will reconnect in " << reconnectInterval << ")" << endl;

    if (state == PCEP_OPERATIONAL)
        emit(sessionDownSignal, (long)sid);

    cancelAndDelete(keepAliveSendTimer);
    keepAliveSendTimer = nullptr;
    cancelAndDelete(sessionHoldTimer); // deletes 'msg' itself
    sessionHoldTimer = nullptr;
    state = PCEP_NONEXISTENT;
    pendingRequests.clear(); // see clearState()'s doc comment for the rationale

    if (socket) {
        // decisive, synchronous teardown: we cannot trust a graceful close() to ever
        // complete against a PCE that may not even be there any more. We are not
        // inside a socket callback here (this is a timer), so an immediate delete is
        // safe (unlike handleTcpConnectionDown's deferred deadSockets pattern).
        socket->abort();
        delete socket;
        socket = nullptr;
    }

    rescheduleAfter(reconnectInterval, reconnectTimer);
}

void Pcc::processReconnectTimeout(cMessage *msg)
{
    ASSERT(msg == reconnectTimer);
    connectToPce();
}

} // namespace inet

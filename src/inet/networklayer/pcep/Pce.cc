//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/pcep/Pce.h"

#include <algorithm>

#include "inet/networklayer/ted/Ted.h"

namespace inet {

Define_Module(Pce);

simsignal_t Pce::sessionUpSignal = registerSignal("sessionUp");
simsignal_t Pce::sessionDownSignal = registerSignal("sessionDown");

Pce::Pce()
{
}

Pce::~Pce()
{
    for (auto& s : sessions) {
        cancelAndDelete(s.keepAliveSendTimer);
        cancelAndDelete(s.sessionHoldTimer);
    }
    for (auto *s : deadSockets)
        delete s;
    socketMap.deleteSockets();
}

void Pce::initialize(int stage)
{
    RoutingProtocolBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        keepaliveTime = par("keepaliveTime");
        deadTimer = par("deadTimer");

        tedmod.reference(this, "tedModule", true);

        WATCH(numSent);
        WATCH(numReceived);
        WATCH_EXPR("numSessions", sessions.size());
    }
    else if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        setupSocket();
    }
}

void Pce::setupSocket()
{
    // (re)create a fresh socket object: this also runs on node restart, and a closed
    // socket cannot be re-bound
    serverSocket = TcpSocket();

    EV_INFO << "Starting to listen on port " << PCEP_PORT << " for incoming PCEP sessions\n";
    serverSocket.setOutputGate(gate("socketOut"));
    serverSocket.setCallback(this);
    serverSocket.bind(PCEP_PORT);
    serverSocket.listen();
}

void Pce::handleStartOperation(LifecycleOperation *operation)
{
    // OperationalMixin calls this with operation==nullptr during initialize() (before
    // the socket-setup init stage); a real restart passes a non-null operation and
    // must re-open the listening socket that stop tore down.
    if (operation != nullptr)
        setupSocket();
}

void Pce::handleStopOperation(LifecycleOperation *operation)
{
    clearState();
    serverSocket.close();
    for (auto s : socketMap.getMap())
        s.second->close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void Pce::handleCrashOperation(LifecycleOperation *operation)
{
    clearState();
    serverSocket.destroy();
    for (auto s : socketMap.getMap())
        s.second->destroy();
}

void Pce::clearState()
{
    for (auto& s : sessions) {
        cancelAndDelete(s.keepAliveSendTimer);
        cancelAndDelete(s.sessionHoldTimer);
    }
    sessions.clear();
}

void Pce::handleMessageWhenUp(cMessage *msg)
{
    // delete sockets torn down inside a previous callback (safe now: we are no
    // longer executing inside their processMessage())
    for (auto *s : deadSockets)
        delete s;
    deadSockets.clear();

    if (msg->isSelfMessage()) {
        if (!strcmp(msg->getName(), "PcepKeepAliveSendTimer"))
            processKeepAliveSendTimeout(msg);
        else if (!strcmp(msg->getName(), "PcepSessionHoldTimer"))
            processSessionHoldTimeout(msg);
        else
            throw cRuntimeError("Pce: unknown self message '%s'", msg->getName());
    }
    else {
        ISocket *socket = socketMap.findSocketFor(msg);
        if (socket)
            socket->processMessage(msg);
        else if (serverSocket.belongsToSocket(msg))
            serverSocket.processMessage(msg);
        else {
            // an in-flight TCP indication may arrive for a session we just tore down;
            // drop it instead of aborting
            EV_WARN << "no socket found for msg '" << msg->getName() << "', dropping\n";
            delete msg;
        }
    }

    if (operationalState == State::STOPPING_OPERATION) {
        if (serverSocket.isOpen())
            return;
        for (auto s : socketMap.getMap())
            if (s.second->isOpen())
                return;
        socketMap.deleteSockets();
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
    }
}

int Pce::findSession(TcpSocket *socket)
{
    for (size_t i = 0; i < sessions.size(); ++i)
        if (sessions[i].socket == socket)
            return (int)i;
    return -1;
}

void Pce::sendToSession(int i, Packet *pk)
{
    numSent++;
    sessions[i].socket->send(pk);

    // RFC 5440 Section 7.3: sending ANY PCEP message resets the KeepAlive send timer
    // (it only exists once the session is OPERATIONAL)
    if (sessions[i].keepAliveSendTimer != nullptr)
        rescheduleAfter(sessions[i].negotiatedKeepaliveTime, sessions[i].keepAliveSendTimer);
}

void Pce::sendOpen(int i)
{
    Packet *pk = new Packet("Pcep-Open");
    const auto& open = makeShared<PcepOpen>();
    open->setChunkLength(PCEP_OPEN_MESSAGE_BYTES);
    open->setType(PCEP_OPEN);
    open->setKeepaliveTime((uint8_t)keepaliveTime.inUnit(SIMTIME_S));
    open->setDeadTimer((uint8_t)deadTimer.inUnit(SIMTIME_S));
    open->setSid(sessions[i].sid);
    pk->insertAtBack(open);

    sendToSession(i, pk);
    EV_INFO << "Open sent to " << sessions[i].pccAddress << " (keepaliveTime=" << keepaliveTime
            << ", deadTimer=" << deadTimer << ", sid=" << (int)sessions[i].sid << ")\n";
}

void Pce::sendKeepalive(int i)
{
    Packet *pk = new Packet("Pcep-Keepalive");
    const auto& ka = makeShared<PcepKeepalive>();
    ka->setChunkLength(PCEP_KEEPALIVE_MESSAGE_BYTES);
    ka->setType(PCEP_KEEPALIVE);
    pk->insertAtBack(ka);

    sendToSession(i, pk);
    EV_INFO << "Keepalive sent to " << sessions[i].pccAddress << "\n";
}

void Pce::socketAvailable(TcpSocket *listenerSocket, TcpAvailableInfo *availableInfo)
{
    // PCEP has no discovery phase (unlike Ldp's Hello-driven peer discovery): any PCC
    // that can reach us on PCEP_PORT is accepted unconditionally, and a new session
    // entry is created on the spot.
    TcpSocket *newSocket = new TcpSocket(availableInfo);
    newSocket->setOutputGate(gate("socketOut"));
    newSocket->setCallback(this);
    socketMap.addSocket(newSocket);

    PccSession session;
    session.socket = newSocket;
    session.pccAddress = newSocket->getRemoteAddress();
    session.state = PCEP_INITIALIZED;
    sessions.push_back(session);

    EV_INFO << "Accepted PCEP TCP connection from " << session.pccAddress << "\n";
    listenerSocket->accept(availableInfo->getNewSocketId());
}

void Pce::socketDataArrived(TcpSocket *socket)
{
    int i = findSession(socket);
    if (i == -1) {
        EV_WARN << "data arrived on an unknown PCEP socket, ignoring\n";
        return;
    }

    auto queue = socket->getReadBuffer();
    while (queue->has<PcepMessage>()) {
        numReceived++;
        auto header = queue->pop<PcepMessage>();
        processPcepPacketFromTcp(i, header);
    }
}

void Pce::handleTcpConnectionDown(TcpSocket *socket)
{
    // Discard the session: unlike Ldp (whose peer identity survives a lost TCP
    // session via the UDP Hello adjacency), a PCEP PCE has no standing peer
    // identity for a PCC at all -- a reconnecting PCC simply opens a brand new TCP
    // connection, handled afresh by socketAvailable().
    if (socketMap.removeSocket(socket) == nullptr)
        return; // already torn down (e.g. both socketClosed and socketFailure fired)

    for (size_t i = 0; i < sessions.size(); ++i) {
        if (sessions[i].socket == socket) {
            if (sessions[i].state == PCEP_OPERATIONAL)
                emit(sessionDownSignal, (long)sessions[i].sid);
            cancelAndDelete(sessions[i].keepAliveSendTimer);
            cancelAndDelete(sessions[i].sessionHoldTimer);
            EV_INFO << "PCEP session with " << sessions[i].pccAddress << " went down\n";
            sessions.erase(sessions.begin() + i);
            break;
        }
    }
    // defer the delete: we are called from inside socket->processMessage(), which
    // still touches the socket after this callback returns
    deadSockets.push_back(socket);
}

void Pce::socketPeerClosed(TcpSocket *socket)
{
    EV_INFO << "PCC " << socket->getRemoteAddress() << " closed the TCP connection, closing here as well\n";
    socket->close();
}

void Pce::socketClosed(TcpSocket *socket)
{
    EV_WARN << "PCEP TCP connection to " << socket->getRemoteAddress() << " closed\n";
    handleTcpConnectionDown(socket);
}

void Pce::socketFailure(TcpSocket *socket, int code)
{
    EV_WARN << "PCEP TCP connection to " << socket->getRemoteAddress() << " broken\n";
    handleTcpConnectionDown(socket);
}

void Pce::processPcepPacketFromTcp(int i, const Ptr<const PcepMessage>& pcepMsg)
{
    // RFC 5440 Section 7.3: receiving ANY PCEP message resets the session hold timer
    // (it only exists once the session is OPERATIONAL -- see processKEEPALIVE's
    // OPENREC->OPERATIONAL transition)
    if (sessions[i].sessionHoldTimer != nullptr)
        rescheduleAfter(sessions[i].peerDeadTimer, sessions[i].sessionHoldTimer);

    switch (pcepMsg->getType()) {
        case PCEP_OPEN:
            processOPEN(i, pcepMsg);
            break;

        case PCEP_KEEPALIVE:
            processKEEPALIVE(i);
            break;

        case PCEP_PCREQ:
            processPCREQ(i, pcepMsg);
            break;

        default:
            // an unrecognized message type from a peer must not abort the simulation
            EV_WARN << "ignoring an unrecognized PCEP message of type " << pcepMsg->getType() << endl;
            break;
    }
}

void Pce::processOPEN(int i, const Ptr<const PcepMessage>& pcepMsg)
{
    const auto& open = CHK(dynamicPtrCast<const PcepOpen>(pcepMsg));
    uint8_t peerKeepaliveTime = open->getKeepaliveTime();
    uint8_t peerDeadTimerSec = open->getDeadTimer();

    EV_INFO << "Open received from " << sessions[i].pccAddress << " (peer keepaliveTime=" << (int)peerKeepaliveTime
            << "s, peer deadTimer=" << (int)peerDeadTimerSec << "s)" << endl;

    if (sessions[i].state != PCEP_INITIALIZED) {
        // a duplicate/out-of-sequence Open is a peer/protocol anomaly, not a model error
        EV_WARN << "unexpected Open from " << sessions[i].pccAddress << " in session state "
                << pcepSessionStateName(sessions[i].state) << ", ignoring" << endl;
        return;
    }

    if (peerKeepaliveTime == 0) {
        // RFC 5440 Section 7.3/7.15: a KeepAlive Time of 0 from a PCC (this model's
        // PCE never proposes 0 itself) is an unacceptable session parameter --
        // mirrors Ldp::processINITIALIZATION's handling of the equivalent LDP case.
        EV_WARN << "PCC " << sessions[i].pccAddress << " proposed an unacceptable KeepAlive Time (0), rejecting session" << endl;
        if (sessions[i].socket)
            sessions[i].socket->close();
        return;
    }

    sessions[i].negotiatedKeepaliveTime = std::min(keepaliveTime, SimTime(peerKeepaliveTime, SIMTIME_S));
    sessions[i].peerDeadTimer = SimTime(peerDeadTimerSec, SIMTIME_S);
    sessions[i].sid = ++sidCounter;

    // Pce is always the passive side (PCEP has no discovery phase, and only the PCC
    // ever initiates -- see Pcc): this is always the PCC's opening move, so reply
    // with our own Open, then immediately a Keepalive (mirrors
    // Ldp::processINITIALIZATION's passive-side branch).
    sendOpen(i);
    sendKeepalive(i);
    sessions[i].state = PCEP_OPENREC;
    EV_INFO << "session with " << sessions[i].pccAddress << " negotiated keepaliveTime="
            << sessions[i].negotiatedKeepaliveTime << ", state OPENREC" << endl;
}

void Pce::processKEEPALIVE(int i)
{
    EV_INFO << "Keepalive received from " << sessions[i].pccAddress << endl;

    if (sessions[i].state == PCEP_OPENREC) {
        sessions[i].state = PCEP_OPERATIONAL;
        EV_INFO << "PCEP session with " << sessions[i].pccAddress << " is now OPERATIONAL" << endl;
        emit(sessionUpSignal, (long)sessions[i].sid);

        // start KeepAlive-based session liveness (RFC 5440 Section 7.3): initial arm
        // of both timers (before this point they are nullptr, so the unconditional
        // reset attempts in sendToSession/processPcepPacketFromTcp are no-ops)
        ASSERT(sessions[i].keepAliveSendTimer == nullptr && sessions[i].sessionHoldTimer == nullptr);
        sessions[i].keepAliveSendTimer = new cMessage("PcepKeepAliveSendTimer");
        scheduleAfter(sessions[i].negotiatedKeepaliveTime, sessions[i].keepAliveSendTimer);
        sessions[i].sessionHoldTimer = new cMessage("PcepSessionHoldTimer");
        scheduleAfter(sessions[i].peerDeadTimer, sessions[i].sessionHoldTimer);
    }
    else if (sessions[i].state == PCEP_OPERATIONAL) {
        // steady-state KeepAlive refresh; the session hold timer reset already
        // happened unconditionally at the top of processPcepPacketFromTcp
        EV_DETAIL << "Keepalive refresh from " << sessions[i].pccAddress << " (session already OPERATIONAL)" << endl;
    }
    else {
        EV_WARN << "unexpected Keepalive from " << sessions[i].pccAddress << " in session state "
                << pcepSessionStateName(sessions[i].state) << ", ignoring" << endl;
    }
}

void Pce::processPCREQ(int i, const Ptr<const PcepMessage>& pcepMsg)
{
    const auto& req = CHK(dynamicPtrCast<const PcepPcreq>(pcepMsg));

    EV_INFO << "PCReq received from " << sessions[i].pccAddress << " (requestId=" << req->getRequestId()
            << ", src=" << req->getSrcAddress() << ", dst=" << req->getDstAddress()
            << ", bandwidth=" << req->getBandwidth() << ", setupPriority=" << req->getSetupPriority() << ")" << endl;

    // Same CSPF call RsvpTe::createIngressPSB() makes for its own local computation
    // (Workstream C6) -- rooted at the REQUESTER (the PCReq's END-POINTS source
    // address), not at this Pce's own node, since the Pce may be running on a
    // different router than the one that will actually signal the LSP.
    Ipv4AddressVector dest;
    dest.push_back(req->getDstAddress());
    Ipv4AddressVector cspfPath = tedmod->calculateShortestPath(req->getSrcAddress(), dest, tedmod->getLinks(),
            req->getBandwidth(), req->getSetupPriority(), req->getIncludeAny(), req->getExcludeAny());

    Packet *pk = new Packet("Pcep-PCRep");
    const auto& rep = makeShared<PcepPcrep>();
    rep->setType(PCEP_PCREP);
    rep->setRequestId(req->getRequestId());

    if (cspfPath.empty()) {
        EV_INFO << "no feasible path to " << req->getDstAddress() << " for requestId " << req->getRequestId()
                << ", replying with NO-PATH" << endl;
        rep->setNoPath(true);
        rep->setChunkLength(PCEP_PCREP_NOPATH_MESSAGE_BYTES);
    }
    else {
        // cspfPath[0] is the requester itself (the CSPF root); the ERO -- like
        // RsvpTe's own local CSPF result -- carries only the hops AFTER it. CSPF
        // always yields a full STRICT route (every hop L=false).
        EroVector ero;
        for (unsigned int k = 1; k < cspfPath.size(); k++) {
            EroObj hop;
            hop.L = false;
            hop.node = cspfPath[k];
            ero.push_back(hop);
        }
        rep->setNoPath(false);
        rep->setEro(ero);
        rep->setChunkLength(pcepPcrepEroMessageBytes(ero.size()));
        EV_INFO << "computed ERO for requestId " << req->getRequestId() << " (" << ero.size() << " hop(s))" << endl;
    }

    pk->insertAtBack(rep);
    sendToSession(i, pk);
}

void Pce::processKeepAliveSendTimeout(cMessage *msg)
{
    for (size_t i = 0; i < sessions.size(); ++i) {
        if (sessions[i].keepAliveSendTimer == msg) {
            // RFC 5440 Section 7.3: no PCEP message was sent to this PCC for a full
            // negotiatedKeepaliveTime -- send an explicit Keepalive so its session hold
            // timer never expires on us. Guard against the narrow window where the
            // socket has already left CONNECTED (e.g. a graceful close in progress)
            // but the teardown callback hasn't fired yet to cancel this timer: a
            // peer/protocol-adjacent condition must never abort the simulation, so
            // just skip this cycle instead of sending.
            if (sessions[i].socket == nullptr || sessions[i].socket->getState() != TcpSocket::CONNECTED) {
                EV_DETAIL << "skipping scheduled Keepalive to " << sessions[i].pccAddress << ": session is closing\n";
                return;
            }
            EV_DETAIL << "no PCEP message sent to " << sessions[i].pccAddress << " for "
                      << sessions[i].negotiatedKeepaliveTime << ", sending Keepalive" << endl;
            sendKeepalive((int)i);
            return;
        }
    }
    // the timer belongs to a session that no longer exists (already torn down
    // through another path); nothing to do
}

void Pce::processSessionHoldTimeout(cMessage *msg)
{
    for (size_t i = 0; i < sessions.size(); ++i) {
        if (sessions[i].sessionHoldTimer == msg) {
            // RFC 5440 Section 7.3: no PCEP message (Keepalive or otherwise) was
            // received from this PCC within the DeadTimer IT advertised in its own
            // Open -- the session is presumed dead.
            EV_WARN << "DeadTimer expired for PCEP session with " << sessions[i].pccAddress
                    << ": no PCEP message received within " << sessions[i].peerDeadTimer
                    << "; closing the session" << endl;

            if (sessions[i].state == PCEP_OPERATIONAL)
                emit(sessionDownSignal, (long)sessions[i].sid);

            TcpSocket *socket = sessions[i].socket;
            cancelAndDelete(sessions[i].keepAliveSendTimer);
            sessions[i].keepAliveSendTimer = nullptr;
            cancelAndDelete(sessions[i].sessionHoldTimer); // deletes 'msg' itself
            sessions[i].sessionHoldTimer = nullptr;

            if (socket) {
                // decisive, synchronous teardown of the transport connection: we
                // cannot trust a graceful close() to ever complete against a PCC
                // that may not even be there any more. We are not inside a socket
                // callback here (this is a timer), so an immediate delete is safe
                // (unlike handleTcpConnectionDown's deferred deadSockets pattern).
                socketMap.removeSocket(socket);
                socket->abort();
                delete socket;
            }
            sessions.erase(sessions.begin() + i);
            return;
        }
    }
    // the timer belongs to a session that no longer exists (already torn down
    // through another path); nothing to do
}

} // namespace inet

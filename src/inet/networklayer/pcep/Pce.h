//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PCE_H
#define __INET_PCE_H

#include <vector>

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/socket/SocketMap.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/pcep/PcepCommon.h"
#include "inet/networklayer/pcep/PcepMessages_m.h"
#include "inet/routing/base/RoutingProtocolBase.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"

namespace inet {

class Ted;

/**
 * PCE (Path Computation Element) server (RFC 5440): listens on PCEP_PORT (4189) and
 * accepts PCEP sessions from any number of PCCs (~Pcc), each tracked independently
 * in `sessions`. Phase 1 of this workstream (see Pce.ned): session establishment
 * only -- no path computation, LSP delegation, or ERO handling yet.
 */
class INET_API Pce : public RoutingProtocolBase, public TcpSocket::BufferingCallback
{
  public:
    struct PccSession {
        TcpSocket *socket = nullptr;
        L3Address pccAddress;
        PcepSessionState state = PCEP_NONEXISTENT;
        simtime_t negotiatedKeepaliveTime = 0; // min(ours, this PCC's) once negotiated (RFC 5440 Section 7.3); 0 until then

        // RFC 5440 Section 7.3: the DeadTimer THIS PCC advertised in its own Open --
        // i.e. how long it told us it needs before declaring the session dead if it
        // hears nothing from us. We (the receiving side) apply THIS value -- not our
        // own configured ~deadTimer parameter -- to detect when this PCC itself has
        // gone silent for too long (see Pce::processOPEN/processSessionHoldTimeout).
        // Our own ~deadTimer is the reverse: what we advertise to the PCC for it to
        // apply when monitoring US.
        simtime_t peerDeadTimer = 0;

        uint8_t sid = 0; // Session ID assigned by us to this PCC (see Pce::sidCounter)

        // KeepAlive-based session liveness (RFC 5440 Section 7.3), armed only once the
        // session is OPERATIONAL: keepAliveSendTimer fires every negotiatedKeepaliveTime
        // without a PCEP message having been SENT to this PCC (reset in
        // Pce::sendToSession on every send) and triggers an explicit Keepalive;
        // sessionHoldTimer fires peerDeadTimer after the last PCEP message RECEIVED from
        // this PCC (reset in Pce::processPcepPacketFromTcp on every receive) and tears
        // the session down (Pce::processSessionHoldTimeout).
        cMessage *keepAliveSendTimer = nullptr;
        cMessage *sessionHoldTimer = nullptr;
    };
    typedef std::vector<PccSession> SessionVector;

  protected:
    // configuration
    simtime_t keepaliveTime; // KeepAlive Time WE propose in our Open (RFC 5440 Section 7.3); RFC-conventional default
    simtime_t deadTimer; // DeadTimer WE propose in our Open -- what we ask the PCC to use when monitoring US

    // Wired but unused this phase (Phase 1: session establishment only -- see
    // Pce.ned): an "omniscient" module-path reference to the network's Ted, mirroring
    // Ldp::tedmod and Ted::initializeTED's own existing topology-omniscience
    // elsewhere in this codebase. A later (path-computation) phase will consult it.
    ModuleRefByPar<Ted> tedmod;

    TcpSocket serverSocket; // for listening on PCEP_PORT
    SocketMap socketMap; // holds TCP connections with PCCs
    SessionVector sessions;
    std::vector<TcpSocket *> deadSockets; // sockets torn down in a callback, deleted later

    int sidCounter = 0;
    long numSent = 0;
    long numReceived = 0;

    static simsignal_t sessionUpSignal;
    static simsignal_t sessionDownSignal;

  protected:
    virtual int findSession(TcpSocket *socket);
    virtual void sendToSession(int i, Packet *msg);
    virtual void sendOpen(int i);
    virtual void sendKeepalive(int i);

    virtual void processPcepPacketFromTcp(int i, const Ptr<const PcepMessage>& pcepMsg);
    virtual void processOPEN(int i, const Ptr<const PcepMessage>& pcepMsg);
    virtual void processKEEPALIVE(int i);

    virtual void processKeepAliveSendTimeout(cMessage *msg);
    virtual void processSessionHoldTimeout(cMessage *msg);

    virtual void handleTcpConnectionDown(TcpSocket *socket);

  public:
    Pce();
    virtual ~Pce();

    // lifecycle
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    virtual void setupSocket();
    virtual void clearState();

    /** @name TcpSocket::ICallback/BufferingCallback methods */
    //@{
    virtual void socketDataArrived(TcpSocket *socket) override;
    virtual void socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo) override;
    virtual void socketEstablished(TcpSocket *socket) override {} // server side: nothing to do until the PCC's Open arrives (see socketAvailable/socketDataArrived)
    virtual void socketPeerClosed(TcpSocket *socket) override;
    virtual void socketClosed(TcpSocket *socket) override;
    virtual void socketFailure(TcpSocket *socket, int code) override;
    virtual void socketStatusArrived(TcpSocket *socket, TcpStatusInfo *status) override {}
    virtual void socketDeleted(TcpSocket *socket) override {} // TODO
    //@}
};

} // namespace inet

#endif

//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PCC_H
#define __INET_PCC_H

#include <vector>

#include "inet/networklayer/pcep/PcepCommon.h"
#include "inet/networklayer/pcep/PcepMessages_m.h"
#include "inet/routing/base/RoutingProtocolBase.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"

namespace inet {

/**
 * PCC (Path Computation Client) (RFC 5440): opens a TCP connection to a single,
 * statically configured PCE (~Pce) on PCEP_PORT (4189) and, unlike Ldp's Hello-driven
 * peer discovery, always plays the active (connecting) role -- PCEP has no
 * discovery phase. Phase 1 of this workstream (see Pcc.ned): session establishment
 * only -- no path computation, LSP delegation, or ERO application yet.
 */
class INET_API Pcc : public RoutingProtocolBase, public TcpSocket::BufferingCallback
{
  protected:
    // configuration
    std::string pceAddressStr;
    int pcePort = -1;
    simtime_t keepaliveTime; // KeepAlive Time WE propose in our Open (RFC 5440 Section 7.3)
    simtime_t deadTimer; // DeadTimer WE propose in our Open -- what we ask the PCE to use when monitoring US
    simtime_t reconnectInterval;

    TcpSocket *socket = nullptr;
    PcepSessionState state = PCEP_NONEXISTENT;
    simtime_t negotiatedKeepaliveTime = 0; // min(ours, the PCE's) once negotiated (RFC 5440 Section 7.3); 0 until then

    // RFC 5440 Section 7.3: the DeadTimer the PCE advertised in ITS own Open -- see
    // Pce::PccSession::peerDeadTimer's doc comment for the full rationale; the same
    // directionality applies symmetrically here.
    simtime_t peerDeadTimer = 0;

    uint8_t sid = 0; // Session ID we chose for this session (see sidCounter)
    int sidCounter = 0;

    // KeepAlive-based session liveness (RFC 5440 Section 7.3); see Pce::PccSession's
    // matching fields for the full division of labor -- identical here, just for a
    // single (rather than per-PCC) session.
    cMessage *keepAliveSendTimer = nullptr;
    cMessage *sessionHoldTimer = nullptr;

    // PCEP has no discovery/Hello phase to piggyback reconnection on (unlike Ldp,
    // whose surviving Hello adjacency re-establishes a lost session automatically);
    // this explicit timer drives both the very first connection attempt and every
    // later reconnection attempt after a lost session.
    cMessage *reconnectTimer = nullptr;

    std::vector<TcpSocket *> deadSockets; // sockets torn down in a callback, deleted later

    long numSent = 0;
    long numReceived = 0;

    static simsignal_t sessionUpSignal;
    static simsignal_t sessionDownSignal;

  protected:
    virtual void connectToPce();
    virtual void sendToPce(Packet *msg);
    virtual void sendOpen();
    virtual void sendKeepalive();

    virtual void processPcepPacketFromTcp(const Ptr<const PcepMessage>& pcepMsg);
    virtual void processOPEN(const Ptr<const PcepMessage>& pcepMsg);
    virtual void processKEEPALIVE();

    virtual void processKeepAliveSendTimeout(cMessage *msg);
    virtual void processSessionHoldTimeout(cMessage *msg);
    virtual void processReconnectTimeout(cMessage *msg);

    virtual void handleTcpConnectionDown(TcpSocket *socket);

  public:
    Pcc();
    virtual ~Pcc();

    // lifecycle
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    virtual void clearState();

    /** @name TcpSocket::ICallback/BufferingCallback methods */
    //@{
    virtual void socketDataArrived(TcpSocket *socket) override;
    virtual void socketAvailable(TcpSocket *, TcpAvailableInfo *) override {} // Pcc never listens (see Pcc.ned)
    virtual void socketEstablished(TcpSocket *socket) override;
    virtual void socketPeerClosed(TcpSocket *socket) override;
    virtual void socketClosed(TcpSocket *socket) override;
    virtual void socketFailure(TcpSocket *socket, int code) override;
    virtual void socketStatusArrived(TcpSocket *, TcpStatusInfo *) override {}
    virtual void socketDeleted(TcpSocket *) override {} // TODO
    //@}
};

} // namespace inet

#endif

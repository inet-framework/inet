//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PCE_H
#define __INET_PCE_H

#include <map>
#include <vector>

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/scenario/IScriptable.h"
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
 * in `sessions`. Phase 1 of this workstream (see Pce.ned): session establishment.
 * Phase 2 (RFC 5440 Section 6.5/6.6, stateless path computation) adds PCReq/PCRep:
 * `tedmod` (wired since Phase 1) is now actually consulted -- a PCReq is answered by
 * running the identical Ted::calculateShortestPath() CSPF ~RsvpTe's own local
 * computation uses (Workstream C6), rooted at the requester rather than this node.
 * No LSP delegation/stateful PCRpt/PCUpd yet -- that is a later phase.
 */
class INET_API Pce : public RoutingProtocolBase, public TcpSocket::BufferingCallback, public IScriptable
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

    // Wired since Phase 1 (session establishment); actually consulted starting Phase 2
    // (processPCREQ()): an "omniscient" module-path reference to the network's Ted,
    // mirroring Ldp::tedmod and Ted::initializeTED's own existing topology-omniscience
    // elsewhere in this codebase.
    ModuleRefByPar<Ted> tedmod;

    TcpSocket serverSocket; // for listening on PCEP_PORT
    SocketMap socketMap; // holds TCP connections with PCCs
    SessionVector sessions;
    std::vector<TcpSocket *> deadSockets; // sockets torn down in a callback, deleted later

    // Phase 3 (RFC 8231 stateful delegation): one entry per delegated LSP a PCC has
    // reported to us via a PCRpt, keyed by PLSP-ID -- see processPCRPT(). Model
    // simplification: PLSP-ID is assumed globally unique across every PCC this Pce
    // serves (a real PCE would key by the pair (PCC identity, PLSP-ID), since PLSP-ID
    // is only meaningful within a single PCC's own session); acceptable here since
    // this workstream's topologies/tests only ever have a single PCC delegating to a
    // given Pce.
    struct DelegatedLsp {
        int plspId;
        L3Address pccAddress; // which PCC's session this LSP belongs to -- see findSessionByAddress()
        Ipv4Address srcAddress;
        Ipv4Address destAddress;
        double bandwidth;
        int setupPriority;
        uint32_t includeAny;
        uint32_t excludeAny;
        EroVector currentEro; // the route this LSP is currently known to be using
    };
    std::map<int, DelegatedLsp> delegatedLsps;

    int sidCounter = 0;
    uint32_t srpIdCounter = 0; // Phase 3: this Pce's own correlator counter for PCUpd (mirrors ~Pcc::pceRequestIdCounter)
    long numSent = 0;
    long numReceived = 0;

    static simsignal_t sessionUpSignal;
    static simsignal_t sessionDownSignal;

  protected:
    virtual int findSession(TcpSocket *socket);
    // Phase 3: looks a session up by the PCC's address rather than its (possibly
    // reconnected/replaced) TcpSocket -- used to find where to send a PCUpd for a
    // delegated LSP recorded earlier (DelegatedLsp::pccAddress).
    virtual int findSessionByAddress(const L3Address& pccAddress);
    virtual void sendToSession(int i, Packet *msg);
    virtual void sendOpen(int i);
    virtual void sendKeepalive(int i);
    virtual void sendPcupd(int i, uint32_t srpId, int plspId, const EroVector& ero);

    virtual void processPcepPacketFromTcp(int i, const Ptr<const PcepMessage>& pcepMsg);
    virtual void processOPEN(int i, const Ptr<const PcepMessage>& pcepMsg);
    virtual void processKEEPALIVE(int i);
    virtual void processPCREQ(int i, const Ptr<const PcepMessage>& pcepMsg);
    virtual void processPCRPT(int i, const Ptr<const PcepMessage>& pcepMsg);

    virtual void processKeepAliveSendTimeout(cMessage *msg);
    virtual void processSessionHoldTimeout(cMessage *msg);

    // Phase 3: PCE-initiated reoptimization (see Pce.ned's doc comment for why this,
    // rather than a TED-change-triggered version, was chosen): re-runs CSPF for one
    // delegated LSP (or, via processCommand()'s "all", every one currently known) and
    // sends a PCUpd if the result differs from the ERO it is currently known to be
    // using.
    virtual void reoptimizeDelegatedLsp(int plspId);

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

    // IScriptable implementation (Phase 3): understands a single "pce-reoptimize"
    // ~ScenarioManager command, e.g. <pce-reoptimize module="LSR4.app[0]" plspid="all"/>
    // or plspid="<N>" for one specific delegated LSP -- see Pce.ned's doc comment.
    virtual void processCommand(const cXMLElement& node) override;

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

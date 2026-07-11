//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PCC_H
#define __INET_PCC_H

#include <map>
#include <vector>

#include "inet/common/ModuleRefByPar.h"
#include "inet/networklayer/pcep/PcepCommon.h"
#include "inet/networklayer/pcep/PcepMessages_m.h"
#include "inet/networklayer/rsvpte/IntServ_m.h"
#include "inet/routing/base/RoutingProtocolBase.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"

namespace inet {

class RsvpTe;

/**
 * PCC (Path Computation Client) (RFC 5440): opens a TCP connection to a single,
 * statically configured PCE (~Pce) on PCEP_PORT (4189) and, unlike Ldp's Hello-driven
 * peer discovery, always plays the active (connecting) role -- PCEP has no
 * discovery phase. Phase 1 of this workstream (see Pcc.ned): session establishment
 * only. Phase 2 (RFC 5440 Section 6.5/6.6, stateless path computation) adds
 * requestPathComputation(): a synchronous-looking C++ API that ~RsvpTe (its
 * "pccModule") calls in place of its own local CSPF (Ted::calculateShortestPath())
 * when configured for "pce" computationMode, bridging that synchronous call onto
 * the inherently asynchronous PCReq/PCRep TCP exchange -- see requestPathComputation()'s
 * own doc comment for exactly how.
 */
class INET_API Pcc : public RoutingProtocolBase, public TcpSocket::BufferingCallback
{
  public:
    // Outcome of requestPathComputation() below.
    enum class PceComputationResult { PENDING, COMPUTED, NO_PATH };

  protected:
    // Phase 2: one entry per path computation request this Pcc has sent to the PCE
    // and not yet handed back to its caller (~RsvpTe) -- see requestPathComputation().
    // Keyed by the full computation input tuple (there is no other identity a caller
    // outside this module could use to look a request back up by); requestId is this
    // Pcc's OWN correlator, carried in the RP object, used only to match an arriving
    // PCRep back to its entry here (RsvpTe never sees it). Removed as soon as its
    // result is consumed by requestPathComputation() -- this phase's PCEP is
    // stateless (RFC 5440 Section 6.5/6.6), so nothing here outlives a single
    // request/reply/consume cycle.
    struct PceRequest {
        Ipv4Address srcAddress;
        Ipv4Address destAddress;
        double bandwidth = 0;
        int setupPriority = 0;
        uint32_t includeAny = 0;
        uint32_t excludeAny = 0;
        uint32_t requestId = 0;
        bool hasResult = false; // false: PCReq sent, PCRep not received yet
        bool success = false; // valid only if hasResult; true: ero is a computed path, false: PCE reported NO-PATH
        EroVector ero;
    };
    std::vector<PceRequest> pendingRequests;
    uint32_t pceRequestIdCounter = 0;

    // Phase 3 (RFC 8231 stateful delegation): bookkeeping for one delegated LSP this
    // Pcc has reported (or is about to report) to the PCE -- see reportLsp(), called
    // cross-module by ~RsvpTe whenever a delegated ingress LSP it manages comes up,
    // changes route (make-before-break), or goes down. Keyed by PLSP-ID (a small
    // integer ~RsvpTe assigns once per delegated LSP and both sides use for its
    // whole lifetime) in `delegatedLsps` below -- deliberately NOT the tuple-keyed
    // shape `pendingRequests` above uses: that cache exists only because stateless
    // PCEP (Phase 2) has no other identity to key by, whereas stateful delegation
    // has a real, RFC-defined identity (PLSP-ID) to use instead. The two mechanisms
    // are RFC-distinct (RFC 5440 Section 6.5/6.6 vs. RFC 8231 Section 6.1/6.2) and
    // deliberately kept code-distinct here too.
    struct DelegatedLsp {
        bool up = false;
        EroVector ero;
        // SRP-ID of the last PCUpd applied to this LSP that has not yet been
        // acknowledged by a PCRpt -- echoed once (then reset to 0) the next time
        // reportLsp() sends a report for this PLSP-ID (RFC 8231 Section 7.2).
        uint32_t lastAppliedSrpId = 0;
    };
    std::map<int, DelegatedLsp> delegatedLsps;

    // Phase 3: RFC 8231 Section 5.2 state timeout -- how long this PCC waits, after
    // the PCEP session is lost, for the PCE to come back before reverting every
    // delegated LSP to local control (see Pcc.ned's doc comment and
    // processStateTimeout()).
    simtime_t stateTimeout;
    cMessage *stateTimeoutTimer = nullptr;

    // Phase 3: an optional back-reference to the co-located ~RsvpTe this Pcc's
    // delegated LSPs actually belong to -- needed both to apply an incoming PCUpd
    // (applyPceUpdate()) and to tell RsvpTe to fall back to local computation once
    // stateTimeout expires (revertToLocalComputation()). Unlike ~RsvpTe's own
    // pccModule reference (always resolved when computationMode is "pce"), this is
    // only resolved when the "rsvpteModule" parameter is non-empty -- a plain
    // stateless (Phase 1/2-only) deployment never sets it and never touches any of
    // this phase's code.
    ModuleRefByPar<RsvpTe> rsvptemod;

    // configuration
    std::string pceAddressStr;
    int pcePort = -1;
    std::string localAddressStr; // see Pcc.ned's doc comment; "" means "let the stack pick"
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
    virtual void sendPcreq(const PceRequest& request);
    virtual void sendPcrpt(int plspId, uint32_t srpId, bool up, const Ipv4Address& srcAddress,
            const Ipv4Address& destAddress, double bandwidth, int setupPriority,
            uint32_t includeAny, uint32_t excludeAny, const EroVector& ero);

    virtual void processPcepPacketFromTcp(const Ptr<const PcepMessage>& pcepMsg);
    virtual void processOPEN(const Ptr<const PcepMessage>& pcepMsg);
    virtual void processKEEPALIVE();
    virtual void processPCREP(const Ptr<const PcepMessage>& pcepMsg);
    virtual void processPCUPD(const Ptr<const PcepMessage>& pcepMsg);

    virtual void processKeepAliveSendTimeout(cMessage *msg);
    virtual void processSessionHoldTimeout(cMessage *msg);
    virtual void processReconnectTimeout(cMessage *msg);
    // Phase 3: RFC 8231 Section 5.2 -- the PCEP session has been down for a full
    // stateTimeout with delegated LSPs still outstanding; tell ~RsvpTe to fall back
    // to local computation for every one of them (see revertToLocalComputation()'s
    // doc comment on RsvpTe for what "fall back" means precisely).
    virtual void processStateTimeout(cMessage *msg);
    // Starts stateTimeoutTimer, but only if there is at least one delegated LSP to
    // fall back for and the timer isn't already running (called from both
    // handleTcpConnectionDown() and processSessionHoldTimeout() -- the two ways an
    // OPERATIONAL session can be lost).
    virtual void armStateTimeoutIfNeeded();

    virtual void handleTcpConnectionDown(TcpSocket *socket);

  public:
    // Phase 2 (RFC 5440 Section 6.5/6.6): the bridge between ~RsvpTe's synchronous
    // ingress path computation call and this module's asynchronous PCReq/PCRep TCP
    // exchange. Called by ~RsvpTe::createIngressPSB() in place of its own local
    // Ted::calculateShortestPath() CSPF call whenever its "computationMode" is "pce";
    // arguments mirror that same call's parameters exactly (srcAddress is the
    // requesting router's own id -- the CSPF root -- mirroring RsvpTe's routerId).
    //
    // Each call either:
    // - finds an outstanding request for this EXACT (srcAddress, destAddress,
    //   bandwidth, setupPriority, includeAny, excludeAny) tuple whose PCRep has
    //   already arrived: consumes it (one-shot -- erased from pendingRequests right
    //   here, matching this phase's stateless PCEP semantics) and returns COMPUTED
    //   (with outEro filled from the PCE's ERO) or NO_PATH (the PCE's NO-PATH);
    // - finds a matching request still in flight (no PCRep yet): returns PENDING,
    //   without resending -- the caller (RsvpTe) is expected to retry later via its
    //   own existing permanent-path retry timer (see RsvpTe::createPath()'s
    //   PATH_RETRY handling, unchanged by this workstream: "no path yet" and
    //   "awaiting a reply" are treated identically);
    // - finds no matching request at all: if the PCEP session is OPERATIONAL, sends
    //   a fresh PCReq and returns PENDING; otherwise returns NO_PATH immediately
    //   (nothing to wait for -- there is no session to answer on).
    //
    // A tuple-keyed cache (rather than a caller-supplied opaque handle) is a
    // deliberate simplification: RsvpTe has no PCEP-specific request identity of its
    // own to hand back, and two concurrent requests for the identical
    // (source/dest/bandwidth/priority/affinity) inputs would get the identical CSPF
    // answer from a stateless PCE anyway, so sharing one cache slot for them costs
    // nothing but a possible extra PCReq if both retry after the first is consumed.
    virtual PceComputationResult requestPathComputation(const Ipv4Address& srcAddress, const Ipv4Address& destAddress,
            double bandwidth, int setupPriority, uint32_t includeAny, uint32_t excludeAny, EroVector& outEro);

    // Phase 3 (RFC 8231 Section 6.1): called by ~RsvpTe whenever a delegated ingress
    // LSP it manages comes up (up=true, a real ERO), changes route via an internal
    // make-before-break re-route or an applied PCUpd (up=true again, same PLSP-ID,
    // new ERO), or is torn down (up=false, ero ignored). Builds and, if the PCEP
    // session is currently OPERATIONAL, sends a PCRpt -- echoing (once) the SRP-ID of
    // the last PCUpd applied to this PLSP-ID, if any (see DelegatedLsp::lastAppliedSrpId).
    // If the session is NOT operational, the report is simply recorded locally and
    // not sent: RFC 8231's full State Synchronization Sequence (Section 5.6) -- which
    // would resend this and every other delegated LSP's state once the session comes
    // back up -- is out of scope for this phase (a documented simplification).
    // srcAddress/destAddress/bandwidth/setupPriority/includeAny/excludeAny are the
    // same CSPF inputs ~Pce::reoptimizeDelegatedLsp() needs later; see PcepPcrpt's
    // own doc comment for why they ride along on every report rather than being
    // looked up separately.
    virtual void reportLsp(int plspId, bool up, const Ipv4Address& srcAddress, const Ipv4Address& destAddress,
            double bandwidth, int setupPriority, uint32_t includeAny, uint32_t excludeAny, const EroVector& ero);

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

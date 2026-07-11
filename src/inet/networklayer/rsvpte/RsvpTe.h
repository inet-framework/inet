//
// Copyright (C) 2005 Vojtech Janota
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_RSVPTE_H
#define __INET_RSVPTE_H

#include <map>
#include <vector>

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/scenario/IScriptable.h"
#include "inet/networklayer/rsvpte/IRsvpClassifier.h"
#include "inet/networklayer/rsvpte/IntServ_m.h"
#include "inet/networklayer/rsvpte/RsvpHelloMsg_m.h"
#include "inet/networklayer/rsvpte/RsvpPathMsg_m.h"
#include "inet/networklayer/rsvpte/RsvpResvMsg_m.h"
#include "inet/networklayer/rsvpte/RsvpSrefreshMsg_m.h"
#include "inet/networklayer/rsvpte/SignallingMsg_m.h"
#include "inet/routing/base/RoutingProtocolBase.h"

namespace inet {

class RsvpClassifier;
class IIpv4RoutingTable;
class IInterfaceTable;
class Ted;
class LibTable;
class Pcc;

/**
 * TODO documentation
 */
class INET_API RsvpTe : public RoutingProtocolBase, public IScriptable
{
  protected:

    struct TrafficPath {
        SenderTemplateObj sender;
        SenderTspecObj tspec;

        EroVector ERO;

        int owner;
        bool permanent;

        // CSPF affinity constraints (Workstream C6/D3), parsed from this path's optional
        // <include_any>/<exclude_any> XML attributes; 0 = no constraint. Only consulted when
        // computeEro is on and this path's ERO is empty/all-loose (see createIngressPSB()).
        uint32_t includeAny = 0;
        uint32_t excludeAny = 0;

        // C7 (make-before-break, RFC 3209 Section 4.6.4): >= 0 while this path is a
        // pending replacement LSP signaled to take over from an older Lsp_Id on the SAME
        // session (set by triggerMakeBeforeBreak(), cleared by commitResv()'s cutover once
        // the replacement's ingress label is installed and traffic has been switched over).
        // -1 (the default) means "not a pending replacement" -- either an ordinary path, or
        // one that has already completed its cutover and is now the session's normal path.
        int replacesLspId = -1;

        // C7: the flip side of replacesLspId, kept on the OLD (primary) path while its
        // replacement is in flight: >= 0 means "a replacement with this Lsp_Id is already
        // being signaled/retried for me, don't start a second one". Needed because a single
        // path problem can legitimately generate more than one PathErr reaching this same
        // ingress in quick succession (e.g. a multi-hop preemption re-notifies from each hop
        // independently) -- without this guard, each one would spawn its own redundant
        // replacement. Reset implicitly by this path's entry being erased once the
        // replacement's cutover completes (completeMakeBeforeBreakCutover()).
        int pendingReplacementLspId = -1;
    };

    struct TrafficSession {
        SessionObj sobj;

        std::vector<TrafficPath> paths;
    };

    std::vector<TrafficSession> traffic;

    // C8 (RFC 2961 refresh reduction): a MESSAGE_ID_ACK/_NACK a router owes to some
    // peer, queued until it can be piggybacked on the next message this router
    // sends that peer (see PathStateBlock::pendingAckToPeer / ResvStateBlock::
    // pendingAcksByPhop below). Plain data, no owned pointers -- cleaned up for free
    // whenever the owning PSB/RSB is erased.
    struct PendingAck {
        bool isNack = false;
        uint32_t epoch = 0;
        uint32_t messageId = 0;
    };

    /**
     * Path State Block (PSB) structure
     */
    struct PathStateBlock {
        // SESSION object structure
        SessionObj sessionObject;

        // SENDER_TEMPLATE structure
        SenderTemplateObj Sender_Template_Object;

        // SENDER_TSPEC structure
        SenderTspecObj Sender_Tspec_Object;

        // Previous Hop Ipv4 address from PHOP object
        Ipv4Address previousHopAddress;

        // List of outgoing Interfaces for this (sender, destination) single entry for unicast case
        Ipv4Address OutInterface;

        // this must be part of PSB to allow refreshing
        EroVector ERO;

        // PSB unique identifier
        int id;

        // timer/timeout routines
        PsbTimerMsg *timerMsg;
        PsbTimeoutMsg *timeoutMsg;

        // handler module
        int handler;

        // sim time this PSB was created (createPSB/createIngressPSB); used at the
        // ingress to compute LSP setup latency for the lspEstablished signal
        simtime_t pathCreationTime;

        // C8 (RFC 2961 refresh reduction): this PSB's own outbound MESSAGE_ID,
        // identifying the content of the Path WE send downstream (toward this PSB's
        // OutInterface peer) for it. 0 = not yet assigned; assigned once, the first
        // time refreshPath() sends a full Path while refreshReduction is on, and
        // never bumped afterwards -- this model never mutates an existing PSB's
        // content in place (a changed path becomes a brand-new PSB via
        // make-before-break, C7), so the id never needs to change post-assignment.
        uint32_t outMessageId = 0;

        // C8: the MESSAGE_ID our previous-hop peer attached to the most recently
        // received Path/Srefresh for this PSB, remembered so a later Srefresh from
        // that same peer naming this id can be resolved back to this PSB (RFC 2961
        // Section 5.3; see RsvpTe::processSrefreshMsg()).
        bool hasInMessageId = false;
        uint32_t inMessageIdEpoch = 0;
        uint32_t inMessageId = 0;

        // C8: an ACK/NACK owed to this PSB's downstream peer for a MESSAGE_ID THEY
        // attached to a Resv they sent us (see ResvStateBlock::hasInMessageId on the
        // matching RSB) -- piggybacked on the next Path/Srefresh sent for this PSB,
        // then cleared (RsvpTe::refreshPath()/sendPathSrefresh()).
        bool hasPendingAckOut = false;
        PendingAck pendingAckOut;
    };

    typedef std::vector<PathStateBlock> PsbVector;

    /**
     * Reservation State Block (RSB) structure
     */
    struct ResvStateBlock {
        // SESSION object structure
        SessionObj sessionObject;

        // Next Hop Ipv4 address from PHOP object
        Ipv4Address Next_Hop_Address;

        // Outgoing Interface on which reservation is to be made or has been made
        Ipv4Address OI;

        // Flows description
        FlowDescriptorVector FlowDescriptor;

        // input labels we have currently installed in the database
        std::vector<int> inLabelVector;

        // no reservation style field: this model always uses shared explicit (SE)

        // FLOWSPEC structure
        FlowSpecObj Flowspec_Object;

        // RSB unique identifier
        int id;

        // timer/timeout routines
        RsbRefreshTimerMsg *refreshTimerMsg;
        RsbCommitTimerMsg *commitTimerMsg;
        RsbTimeoutMsg *timeoutMsg;

        // C8 (RFC 2961 refresh reduction): this RSB's own outbound MESSAGE_IDs, one
        // per previous hop we send a (possibly SE-merged) Resv to -- refreshResv()
        // already fans out to potentially multiple phops when several upstream
        // senders share this reservation on the same outgoing interface, so unlike
        // a PSB's single outMessageId, this genuinely needs to be per-peer. A phop's
        // absence from the map means "not yet assigned" for that phop (ids start at
        // 1, so 0 is never a stored value either way). Cleared (forcing a full
        // resend + fresh id) whenever this RSB's content changes -- see updateRSB().
        std::map<Ipv4Address, uint32_t> outMessageIdByPhop;

        // C8: the MESSAGE_ID our downstream peer (Next_Hop_Address) attached to the
        // most recently received Resv/Srefresh for this RSB, remembered so a later
        // Srefresh from that peer naming this id resolves back to this RSB.
        bool hasInMessageId = false;
        uint32_t inMessageIdEpoch = 0;
        uint32_t inMessageId = 0;

        // C8: ACKs/NACKs owed to specific previous hops for MESSAGE_IDs THEY
        // attached to a Path they sent us (each phop is a distinct upstream sender
        // feeding this shared reservation -- see the matching PSB's
        // hasInMessageId) -- piggybacked on the next Resv/Srefresh sent to that
        // phop, then cleared (RsvpTe::refreshResv()/sendResvSrefresh()).
        std::map<Ipv4Address, PendingAck> pendingAcksByPhop;
    };

    typedef std::vector<ResvStateBlock> RsbVector;

    /**
     * RSVP Hello State structure
     */
    struct HelloState {
        Ipv4Address peer;

        int srcInstance;
        int dstInstance;

        HelloTimerMsg *timer;
        HelloTimeoutMsg *timeout;

        // next hello message sent should include following flags
        bool request;
        bool ack;

        // up/down status of this peer (true if we're getting regular hellos)
        bool ok;
    };

    typedef std::vector<HelloState> HelloVector;

    simtime_t helloInterval;
    simtime_t helloTimeout;
    simtime_t refreshInterval; // RFC 2205's R
    int stateLifetimeFactor; // RFC 2205's K
    simtime_t retryInterval;
    bool advertiseImplicitNull = true;
    // C6: compute a strict ERO at the ingress via Ted::calculateShortestPath() (CSPF) whenever
    // a path's configured ERO is empty or contains only loose hops. Default off -- CSPF is a
    // purely local ingress computation with no RFC-mandated wire behavior (RFC 3209 leaves
    // route computation unspecified), so defaulting it off is a scope choice, not a compliance
    // gap; it also keeps every shipped example/showcase (which all hand-write EROs) fingerprint-
    // identical.
    bool computeEro = false;
    // Workstream F4 Phase 2: WHERE computeEro's automatic ERO computation runs --
    // "local" (default, unchanged) or "pce" (delegate to a co-located ~Pcc, see
    // pccmod below). See RsvpTe.ned's doc comment for the full rationale.
    std::string computationMode = "local";
    // C7: RFC 4736-lite periodic reoptimization -- every reoptimizeInterval, re-run CSPF for
    // every established ingress path and, if it finds a strictly cheaper route than the one
    // currently in use, make-before-break reroute onto it. 0 (default) disables the feature
    // entirely (no timer is ever scheduled), independent of computeEro's setting, which keeps
    // it fingerprint-inert regardless of computeEro.
    simtime_t reoptimizeInterval;
    // C8 (RFC 2961 Section 5.4): refresh reduction -- attach MESSAGE_ID objects to
    // Path/Resv, compress subsequent periodic refreshes into Srefresh messages, and
    // piggyback ACK/NACK objects on messages already travelling toward a peer.
    // Default off keeps every shipped example/showcase fingerprint-identical.
    bool refreshReduction = false;

  protected:
    ModuleRefByPar<Ted> tedmod;
    ModuleRefByPar<IIpv4RoutingTable> rt;
    ModuleRefByPar<IInterfaceTable> ift;
    ModuleRefByPar<LibTable> lt;
    ModuleRefByPar<IRsvpClassifier> rpct;
    // Workstream F4 Phase 2: only referenced (see initialize()) when computationMode
    // is "pce" -- a plain-"local" network (the default, every pre-existing example/
    // showcase) never touches this and needs no "pccModule" parameter at all.
    ModuleRefByPar<Pcc> pccmod;

    int maxPsbId = 0;
    int maxRsbId = 0;

    int maxSrcInstance = 0;

    // C7: monotonic counter for allocating fresh Lsp_Id values to make-before-break
    // replacement paths (there is no other Lsp_Id generator in this model -- every
    // operator-configured lspid is an explicit XML value; this counter is bumped past every
    // XML/add-session lspid seen so a generated replacement id never collides with one).
    int maxLspId = 0;

    // C7: single periodic self-message driving reoptimization; only allocated/scheduled
    // when reoptimizeInterval > 0.
    ReoptimizeTimerMsg *reoptimizeTimerMsg = nullptr;

    // C8 (RFC 2961 refresh reduction): this router's own MESSAGE_ID epoch, chosen
    // once at initialize() time (a fresh draw each run, like a real router's would
    // change across restarts) and used for every MESSAGE_ID this router originates.
    // maxMessageId is the single monotonic counter allocating fresh ids across every
    // PSB/RSB (mirroring maxPsbId/maxRsbId/maxLspId above) -- there is no need for
    // per-PSB/per-RSB-phop counters since ids only need to be unique per (epoch,
    // originating router), not globally.
    uint32_t messageIdEpoch = 0;
    uint32_t maxMessageId = 0;

    Ipv4Address routerId;

    PsbVector PSBList;
    RsbVector RSBList;
    HelloVector HelloList;

    static simsignal_t lspEstablishedSignal;
    static simsignal_t psbCountSignal;
    static simsignal_t rsbCountSignal;

  protected:
    // emit the current PSBList/RSBList sizes; call after any change to PSBList/RSBList
    virtual void emitPsbCount();
    virtual void emitRsbCount();

    virtual void processSignallingMessage(SignallingMsg *msg);
    virtual void processPSB_TIMER(PsbTimerMsg *msg);
    virtual void processPSB_TIMEOUT(PsbTimeoutMsg *msg);
    virtual void processRSB_REFRESH_TIMER(RsbRefreshTimerMsg *msg);
    virtual void processRSB_COMMIT_TIMER(RsbCommitTimerMsg *msg);
    virtual void processRSB_TIMEOUT(RsbTimeoutMsg *msg);
    virtual void processHELLO_TIMER(HelloTimerMsg *msg);
    virtual void processHELLO_TIMEOUT(HelloTimeoutMsg *msg);
    virtual void processPATH_NOTIFY(PathNotifyMsg *msg);
    virtual void processRSVPMessage(Packet *pk);
    virtual void processHelloMsg(Packet *pk);
    virtual void processPathMsg(Packet *pk);
    virtual void processResvMsg(Packet *pk);
    virtual void processPathTearMsg(Packet *pk);
    virtual void processPathErrMsg(Packet *pk);
    virtual void processResvTearMsg(Packet *pk);
    virtual void processResvErrMsg(Packet *pk);

    virtual PathStateBlock *createPSB(const Ptr<RsvpPathMsg>& msg);
    virtual PathStateBlock *createIngressPSB(const TrafficSession& session, const TrafficPath& path);
    virtual void removePSB(PathStateBlock *psb);
    virtual ResvStateBlock *createRSB(const Ptr<const RsvpResvMsg>& msg);
    virtual ResvStateBlock *createEgressRSB(PathStateBlock *psb);
    virtual void updateRSB(ResvStateBlock *rsb, const RsvpResvMsg *msg);
    virtual void removeRSB(ResvStateBlock *rsb);
    virtual void removeRsbFilter(ResvStateBlock *rsb, unsigned int index);

    virtual void refreshPath(PathStateBlock *psbEle);
    virtual void refreshResv(ResvStateBlock *rsbEle);
    virtual void refreshResv(ResvStateBlock *rsbEle, Ipv4Address PHOP);
    virtual void commitResv(ResvStateBlock *rsb);

    // C8 (RFC 2961 refresh reduction): compressed-refresh counterparts of
    // refreshPath()/refreshResv(rsb, PHOP), sent instead of them by
    // processPSB_TIMER()/processRSB_REFRESH_TIMER() once a PSB/(RSB, phop) already
    // has an assigned outbound MESSAGE_ID.
    virtual void sendPathSrefresh(PathStateBlock *psbEle);
    virtual void sendResvSrefresh(ResvStateBlock *rsbEle, Ipv4Address PHOP);
    virtual void processSrefreshMsg(Packet *pk);
    // Handles a piggybacked MESSAGE_ID_ACK/_NACK found on an incoming Path, Resv, or
    // Srefresh (RsvpPacket/RsvpSrefreshMsg's hasMessageIdAck fields): a positive ack
    // is purely informational (logged only -- this model does not track "has this
    // peer confirmed receipt" beyond that); a nack forces an immediate full resend
    // of the state the acked (epoch, id) identifies, if this router still owns one
    // matching it.
    virtual void handleMessageIdAck(Ipv4Address peer, bool isNack, uint32_t epoch, uint32_t id);
    // An Srefresh named an (epoch, id) this router does not recognize as having been
    // sent (by us) to "peer": queue and, best-effort, promptly send a NACK back
    // toward "peer" so it falls back to a full refresh (RFC 2961 Section 5.3). If
    // this router has no state at all going toward "peer" yet, the NACK has no
    // vehicle and is dropped (logged) -- a documented corner case of the
    // piggyback-only scope choice.
    virtual void sendMessageIdNack(Ipv4Address peer, uint32_t epoch, uint32_t id);

    virtual void scheduleRefreshTimer(PathStateBlock *psbEle, simtime_t delay);
    virtual void scheduleTimeout(PathStateBlock *psbEle);
    virtual void scheduleRefreshTimer(ResvStateBlock *rsbEle, simtime_t delay);
    virtual void scheduleCommitTimer(ResvStateBlock *rsbEle);
    virtual void scheduleTimeout(ResvStateBlock *rsbEle);

    virtual void sendPathErrorMessage(PathStateBlock *psb, int errCode);
    virtual void sendPathErrorMessage(SessionObj session, SenderTemplateObj sender, SenderTspecObj tspec, Ipv4Address nextHop, int errCode);
    virtual void sendPathTearMessage(Ipv4Address peerIP, const SessionObj& session, const SenderTemplateObj& sender, Ipv4Address LIH, Ipv4Address NHOP, bool force);
    virtual void sendPathNotify(int handler, const SessionObj& session, const SenderTemplateObj& sender, int status, simtime_t delay);
    virtual void sendResvTearMessage(ResvStateBlock *rsbEle);
    virtual void sendResvTearMessage(ResvStateBlock *rsbEle, Ipv4Address PHOP);
    virtual void sendResvErrorMessage(ResvStateBlock *rsbEle, int errCode);

    virtual void setupHello();
    virtual void addHelloPeer(Ipv4Address peer);
    // True if the node owning peerInterface runs RSVP (used to auto-derive peers).
    virtual bool peerRunsRsvp(Ipv4Address peerInterface);
    virtual void startHello(Ipv4Address peer, simtime_t delay);
    virtual void removeHello(HelloState *h);

    virtual void recoveryEvent(Ipv4Address peer);

    virtual bool allocateResource(Ipv4Address OI, const SessionObj& session, double bandwidth);
    virtual void preempt(Ipv4Address OI, int priority, double bandwidth);
    virtual bool doCACCheck(const SessionObj& session, const SenderTspecObj& tspec, Ipv4Address OI);

    virtual void sendToIP(Packet *msg, Ipv4Address destAddr);

    virtual bool evalNextHopInterface(Ipv4Address destAddr, const EroVector& ERO, Ipv4Address& OI);

    virtual PathStateBlock *findPSB(const SessionObj& session, const SenderTemplateObj& sender);
    virtual ResvStateBlock *findRSB(const SessionObj& session, const SenderTemplateObj& sender, unsigned int& index);

    virtual PathStateBlock *findPsbById(int id);
    virtual ResvStateBlock *findRsbById(int id);

    std::vector<TrafficSession>::iterator findSession(const SessionObj& session);
    std::vector<TrafficPath>::iterator findPath(TrafficSession *session, const SenderTemplateObj& sender);

    virtual HelloState *findHello(Ipv4Address peer);

    virtual void print(const RsvpPathMsg *p);
    virtual void print(const RsvpResvMsg *r);

    virtual void readTrafficFromXML(const cXMLElement *traffic);
    virtual void readTrafficSessionFromXML(const cXMLElement *session);
    virtual EroVector readTrafficRouteFromXML(const cXMLElement *route);

    virtual void createPath(const SessionObj& session, const SenderTemplateObj& sender);

    virtual void pathProblem(PathStateBlock *psb);

    // C7 (make-before-break, RFC 3209 Section 4.6.4): signal a fresh-Lsp_Id replacement for
    // the (session, oldSender) path, leaving the old path/PSB completely untouched; used by
    // both pathProblem() (reactive: the old path failed) and considerReoptimization()
    // (proactive: a strictly better route was found). The actual cutover (classifier rebind
    // + old PathTear) happens later, in commitResv(), once the replacement's ingress label
    // is installed.
    virtual void triggerMakeBeforeBreak(const SessionObj& session, const SenderTemplateObj& oldSender);
    // Called from commitResv() the instant a make-before-break replacement's ingress label
    // is installed: rebinds the classifier from oldLspId to newSender/newInLabel (the actual
    // traffic cutover), then tears down and removes the old path/PSB.
    virtual void completeMakeBeforeBreakCutover(const SessionObj& session, const SenderTemplateObj& newSender, int oldLspId, int newInLabel);
    // C7 reoptimizeInterval: re-run CSPF for one established ingress path and, if it finds a
    // strictly cheaper route than the one currently in use, kick off make-before-break.
    virtual void considerReoptimization(const SessionObj& session, const SenderTemplateObj& sender);
    virtual void processREOPTIMIZE_TIMER(ReoptimizeTimerMsg *msg);

    virtual void addSession(const cXMLElement& node);
    virtual void delSession(const cXMLElement& node);

  protected:

    friend class RsvpClassifier;

    virtual int getInLabel(const SessionObj& session, const SenderTemplateObj& sender);

  public:
    RsvpTe();
    virtual ~RsvpTe();

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void handleMessageWhenDown(cMessage *msg) override;

    virtual void clear();
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    // IScriptable implementation
    virtual void processCommand(const cXMLElement& node) override;
};

bool operator==(const SessionObj& a, const SessionObj& b);
bool operator!=(const SessionObj& a, const SessionObj& b);

bool operator==(const FilterSpecObj& a, const FilterSpecObj& b);
bool operator!=(const FilterSpecObj& a, const FilterSpecObj& b);

bool operator==(const SenderTemplateObj& a, const SenderTemplateObj& b);
bool operator!=(const SenderTemplateObj& a, const SenderTemplateObj& b);

std::ostream& operator<<(std::ostream& os, const SessionObj& a);
std::ostream& operator<<(std::ostream& os, const SenderTemplateObj& a);
std::ostream& operator<<(std::ostream& os, const FlowSpecObj& a);

} // namespace inet

#endif


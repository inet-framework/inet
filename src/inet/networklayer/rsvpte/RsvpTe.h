//
// Copyright (C) 2005 Vojtech Janota
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_RSVPTE_H
#define __INET_RSVPTE_H

#include <vector>

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/scenario/IScriptable.h"
#include "inet/networklayer/rsvpte/IRsvpClassifier.h"
#include "inet/networklayer/rsvpte/IntServ_m.h"
#include "inet/networklayer/rsvpte/RsvpHelloMsg_m.h"
#include "inet/networklayer/rsvpte/RsvpPathMsg_m.h"
#include "inet/networklayer/rsvpte/RsvpResvMsg_m.h"
#include "inet/networklayer/rsvpte/SignallingMsg_m.h"
#include "inet/routing/base/RoutingProtocolBase.h"

namespace inet {

class RsvpClassifier;
class IIpv4RoutingTable;
class IInterfaceTable;
class Ted;
class LibTable;

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
    };

    struct TrafficSession {
        SessionObj sobj;

        std::vector<TrafficPath> paths;
    };

    std::vector<TrafficSession> traffic;

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

  protected:
    ModuleRefByPar<Ted> tedmod;
    ModuleRefByPar<IIpv4RoutingTable> rt;
    ModuleRefByPar<IInterfaceTable> ift;
    ModuleRefByPar<LibTable> lt;
    ModuleRefByPar<IRsvpClassifier> rpct;

    int maxPsbId = 0;
    int maxRsbId = 0;

    int maxSrcInstance = 0;

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


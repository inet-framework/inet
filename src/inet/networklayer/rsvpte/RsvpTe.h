//
// (C) 2005 Vojtech Janota
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//

#ifndef __INET_RSVP_H
#define __INET_RSVP_H

#include <vector>

#include "inet/common/INETDefs.h"
#include "inet/common/scenario/IScriptable.h"
#include "inet/networklayer/rsvpte/IntServ_m.h"
#include "inet/networklayer/rsvpte/IRsvpClassifier.h"
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

    struct traffic_path_t
    {
        SenderTemplateObj sender;
        SenderTspecObj tspec;

        EroVector ERO;
        simtime_t max_delay;

        int owner;
        bool permanent;
        int color;
    };

    struct traffic_session_t
    {
        SessionObj sobj;

        std::vector<traffic_path_t> paths;
    };

    std::vector<traffic_session_t> traffic;

    /**
     * Path State Block (PSB) structure
     */
    struct PathStateBlock
    {
        // SESSION object structure
        SessionObj Session_Object;

        // SENDER_TEMPLATE structure
        SenderTemplateObj Sender_Template_Object;

        // SENDER_TSPEC structure
        SenderTspecObj Sender_Tspec_Object;

        // Previous Hop Ipv4 address from PHOP object
        Ipv4Address Previous_Hop_Address;

        // Logical Interface Handle from PHOP object
        //Ipv4Address LIH;

        // List of outgoing Interfaces for this (sender, destination) single entry for unicast case
        Ipv4Address OutInterface;

        // this must be part of PSB to allow refreshing
        EroVector ERO;

        // PSB unique identifier
        int id;

        // XXX nam colors
        int color;

        // timer/timeout routines
        PsbTimerMsg *timerMsg;
        PsbTimeoutMsg *timeoutMsg;

        // handler module
        int handler;
    };

    typedef std::vector<PathStateBlock> PsbVector;

    /**
     * Reservation State Block (RSB) structure
     */
    struct ResvStateBlock
    {
        // SESSION object structure
        SessionObj Session_Object;

        // Next Hop Ipv4 address from PHOP object
        Ipv4Address Next_Hop_Address;

        // Outgoing Interface on which reservation is to be made or has been made
        Ipv4Address OI;

        // Flows description
        FlowDescriptorVector FlowDescriptor;

        // input labels we have currently installed in the database
        std::vector<int> inLabelVector;

        //we always use shared explicit
        //int style;

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
    struct HelloState
    {
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
    simtime_t retryInterval;

  protected:
    Ted *tedmod = nullptr;
    IIpv4RoutingTable *rt = nullptr;
    IInterfaceTable *ift = nullptr;
    LibTable *lt = nullptr;

    IRsvpClassifier *rpct = nullptr;

    int maxPsbId = 0;
    int maxRsbId = 0;

    int maxSrcInstance = 0;

    Ipv4Address routerId;

    PsbVector PSBList;
    RsbVector RSBList;
    HelloVector HelloList;

  protected:
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

    virtual PathStateBlock *createPSB(const Ptr<RsvpPathMsg>& msg);
    virtual PathStateBlock *createIngressPSB(const traffic_session_t& session, const traffic_path_t& path);
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

    virtual void setupHello();
    virtual void startHello(Ipv4Address peer, simtime_t delay);
    virtual void removeHello(HelloState *h);

    virtual void recoveryEvent(Ipv4Address peer);

    virtual bool allocateResource(Ipv4Address OI, const SessionObj& session, double bandwidth);
    virtual void preempt(Ipv4Address OI, int priority, double bandwidth);
    virtual bool doCACCheck(const SessionObj& session, const SenderTspecObj& tspec, Ipv4Address OI);
    virtual void announceLinkChange(int tedlinkindex);

    virtual void sendToIP(Packet *msg, Ipv4Address destAddr);

    virtual bool evalNextHopInterface(Ipv4Address destAddr, const EroVector& ERO, Ipv4Address& OI);

    virtual PathStateBlock *findPSB(const SessionObj& session, const SenderTemplateObj& sender);
    virtual ResvStateBlock *findRSB(const SessionObj& session, const SenderTemplateObj& sender, unsigned int& index);

    virtual PathStateBlock *findPsbById(int id);
    virtual ResvStateBlock *findRsbById(int id);

    std::vector<traffic_session_t>::iterator findSession(const SessionObj& session);
    std::vector<traffic_path_t>::iterator findPath(traffic_session_t *session, const SenderTemplateObj& sender);

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

#endif // ifndef __INET_RSVP_H


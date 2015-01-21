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
#include "inet/networklayer/rsvp_te/IntServ.h"
#include "RSVPPathMsg.h"
#include "RSVPResvMsg.h"
#include "RSVPHelloMsg.h"
#include "SignallingMsg_m.h"
#include "inet/networklayer/rsvp_te/IRSVPClassifier.h"
#include "inet/common/lifecycle/ILifecycle.h"

namespace inet {

class SimpleClassifier;
class IIPv4RoutingTable;
class IInterfaceTable;
class TED;
class LIBTable;

/**
 * TODO documentation
 */
class INET_API RSVP : public cSimpleModule, public IScriptable, public ILifecycle
{
  protected:

    struct traffic_path_t
    {
        SenderTemplateObj_t sender;
        SenderTspecObj_t tspec;

        EroVector ERO;
        simtime_t max_delay;

        int owner;
        bool permanent;
        int color;
    };

    struct traffic_session_t
    {
        SessionObj_t sobj;

        std::vector<traffic_path_t> paths;
    };

    std::vector<traffic_session_t> traffic;

    /**
     * Path State Block (PSB) structure
     */
    struct PathStateBlock_t
    {
        // SESSION object structure
        SessionObj_t Session_Object;

        // SENDER_TEMPLATE structure
        SenderTemplateObj_t Sender_Template_Object;

        // SENDER_TSPEC structure
        SenderTspecObj_t Sender_Tspec_Object;

        // Previous Hop IPv4 address from PHOP object
        IPv4Address Previous_Hop_Address;

        // Logical Interface Handle from PHOP object
        //IPv4Address LIH;

        // List of outgoing Interfaces for this (sender, destination) single entry for unicast case
        IPv4Address OutInterface;

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

    typedef std::vector<PathStateBlock_t> PSBVector;

    /**
     * Reservation State Block (RSB) structure
     */
    struct ResvStateBlock_t
    {
        // SESSION object structure
        SessionObj_t Session_Object;

        // Next Hop IPv4 address from PHOP object
        IPv4Address Next_Hop_Address;

        // Outgoing Interface on which reservation is to be made or has been made
        IPv4Address OI;

        // Flows description
        FlowDescriptorVector FlowDescriptor;

        // input labels we have currently installed in the database
        std::vector<int> inLabelVector;

        //we always use shared explicit
        //int style;

        // FLOWSPEC structure
        FlowSpecObj_t Flowspec_Object;

        // RSB unique identifier
        int id;

        // timer/timeout routines
        RsbRefreshTimerMsg *refreshTimerMsg;
        RsbCommitTimerMsg *commitTimerMsg;
        RsbTimeoutMsg *timeoutMsg;
    };

    typedef std::vector<ResvStateBlock_t> RSBVector;

    /**
     * RSVP Hello State structure
     */
    struct HelloState_t
    {
        IPv4Address peer;

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

    typedef std::vector<HelloState_t> HelloVector;

    simtime_t helloInterval;
    simtime_t helloTimeout;
    simtime_t retryInterval;

  protected:
    TED *tedmod = nullptr;
    IIPv4RoutingTable *rt = nullptr;
    IInterfaceTable *ift = nullptr;
    LIBTable *lt = nullptr;

    IRSVPClassifier *rpct = nullptr;

    int maxPsbId = 0;
    int maxRsbId = 0;

    int maxSrcInstance = 0;

    IPv4Address routerId;

    PSBVector PSBList;
    RSBVector RSBList;
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
    virtual void processRSVPMessage(RSVPMessage *msg);
    virtual void processHelloMsg(RSVPHelloMsg *msg);
    virtual void processPathMsg(RSVPPathMsg *msg);
    virtual void processResvMsg(RSVPResvMsg *msg);
    virtual void processPathTearMsg(RSVPPathTear *msg);
    virtual void processPathErrMsg(RSVPPathError *msg);

    virtual PathStateBlock_t *createPSB(RSVPPathMsg *msg);
    virtual PathStateBlock_t *createIngressPSB(const traffic_session_t& session, const traffic_path_t& path);
    virtual void removePSB(PathStateBlock_t *psb);
    virtual ResvStateBlock_t *createRSB(RSVPResvMsg *msg);
    virtual ResvStateBlock_t *createEgressRSB(PathStateBlock_t *psb);
    virtual void updateRSB(ResvStateBlock_t *rsb, RSVPResvMsg *msg);
    virtual void removeRSB(ResvStateBlock_t *rsb);
    virtual void removeRsbFilter(ResvStateBlock_t *rsb, unsigned int index);

    virtual void refreshPath(PathStateBlock_t *psbEle);
    virtual void refreshResv(ResvStateBlock_t *rsbEle);
    virtual void refreshResv(ResvStateBlock_t *rsbEle, IPv4Address PHOP);
    virtual void commitResv(ResvStateBlock_t *rsb);

    virtual void scheduleRefreshTimer(PathStateBlock_t *psbEle, simtime_t delay);
    virtual void scheduleTimeout(PathStateBlock_t *psbEle);
    virtual void scheduleRefreshTimer(ResvStateBlock_t *rsbEle, simtime_t delay);
    virtual void scheduleCommitTimer(ResvStateBlock_t *rsbEle);
    virtual void scheduleTimeout(ResvStateBlock_t *rsbEle);

    virtual void sendPathErrorMessage(PathStateBlock_t *psb, int errCode);
    virtual void sendPathErrorMessage(SessionObj_t session, SenderTemplateObj_t sender, SenderTspecObj_t tspec, IPv4Address nextHop, int errCode);
    virtual void sendPathTearMessage(IPv4Address peerIP, const SessionObj_t& session, const SenderTemplateObj_t& sender, IPv4Address LIH, IPv4Address NHOP, bool force);
    virtual void sendPathNotify(int handler, const SessionObj_t& session, const SenderTemplateObj_t& sender, int status, simtime_t delay);

    virtual void setupHello();
    virtual void startHello(IPv4Address peer, simtime_t delay);
    virtual void removeHello(HelloState_t *h);

    virtual void recoveryEvent(IPv4Address peer);

    virtual bool allocateResource(IPv4Address OI, const SessionObj_t& session, double bandwidth);
    virtual void preempt(IPv4Address OI, int priority, double bandwidth);
    virtual bool doCACCheck(const SessionObj_t& session, const SenderTspecObj_t& tspec, IPv4Address OI);
    virtual void announceLinkChange(int tedlinkindex);

    virtual void sendToIP(cMessage *msg, IPv4Address destAddr);

    virtual bool evalNextHopInterface(IPv4Address destAddr, const EroVector& ERO, IPv4Address& OI);

    virtual PathStateBlock_t *findPSB(const SessionObj_t& session, const SenderTemplateObj_t& sender);
    virtual ResvStateBlock_t *findRSB(const SessionObj_t& session, const SenderTemplateObj_t& sender, unsigned int& index);

    virtual PathStateBlock_t *findPsbById(int id);
    virtual ResvStateBlock_t *findRsbById(int id);

    std::vector<traffic_session_t>::iterator findSession(const SessionObj_t& session);
    std::vector<traffic_path_t>::iterator findPath(traffic_session_t *session, const SenderTemplateObj_t& sender);

    virtual HelloState_t *findHello(IPv4Address peer);

    virtual void print(RSVPPathMsg *p);
    virtual void print(RSVPResvMsg *r);

    virtual void readTrafficFromXML(const cXMLElement *traffic);
    virtual void readTrafficSessionFromXML(const cXMLElement *session);
    virtual EroVector readTrafficRouteFromXML(const cXMLElement *route);

    virtual void createPath(const SessionObj_t& session, const SenderTemplateObj_t& sender);

    virtual void pathProblem(PathStateBlock_t *psb);

    virtual void addSession(const cXMLElement& node);
    virtual void delSession(const cXMLElement& node);

  protected:

    friend class SimpleClassifier;

    virtual int getInLabel(const SessionObj_t& session, const SenderTemplateObj_t& sender);

  public:
    RSVP();
    virtual ~RSVP();

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;

    virtual void clear();
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;

    // IScriptable implementation
    virtual void processCommand(const cXMLElement& node) override;
};

bool operator==(const SessionObj_t& a, const SessionObj_t& b);
bool operator!=(const SessionObj_t& a, const SessionObj_t& b);

bool operator==(const FilterSpecObj_t& a, const FilterSpecObj_t& b);
bool operator!=(const FilterSpecObj_t& a, const FilterSpecObj_t& b);

bool operator==(const SenderTemplateObj_t& a, const SenderTemplateObj_t& b);
bool operator!=(const SenderTemplateObj_t& a, const SenderTemplateObj_t& b);

std::ostream& operator<<(std::ostream& os, const SessionObj_t& a);
std::ostream& operator<<(std::ostream& os, const SenderTemplateObj_t& a);
std::ostream& operator<<(std::ostream& os, const FlowSpecObj_t& a);

} // namespace inet

#endif // ifndef __INET_RSVP_H


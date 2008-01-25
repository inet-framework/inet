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

#ifndef __RSVP__H__
#define __RSVP__H__

#include <vector>
#include <omnetpp.h>

#include "IScriptable.h"
#include "IntServ.h"
#include "RSVPPathMsg.h"
#include "RSVPResvMsg.h"
#include "RSVPHelloMsg.h"
#include "SignallingMsg_m.h"
#include "IRSVPClassifier.h"
#include "NotificationBoard.h"

class SimpleClassifier;
class RoutingTable;
class InterfaceTable;
class TED;
class LIBTable;


/**
 * TODO documentation
 */
class INET_API RSVP : public cSimpleModule, public IScriptable
{
  private:

    struct traffic_path_t
    {
        SenderTemplateObj_t sender;
        SenderTspecObj_t tspec;

        EroVector ERO;
        double max_delay;

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

        // Previous Hop IP address from PHOP object
        IPAddress Previous_Hop_Address;

        // Logical Interface Handle from PHOP object
        //IPAddress LIH;

        // List of outgoing Interfaces for this (sender, destination) single entry for unicast case
        IPAddress OutInterface;

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

        // Next Hop IP address from PHOP object
        IPAddress Next_Hop_Address;

        // Outgoing Interface on which reservation is to be made or has been made
        IPAddress OI;

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
        IPAddress peer;

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

    double helloInterval;
    double helloTimeout;
    double retryInterval;

  private:
    TED *tedmod;
    RoutingTable *rt;
    InterfaceTable *ift;
    LIBTable *lt;
    NotificationBoard *nb;

    IRSVPClassifier *rpct;

    int maxPsbId;
    int maxRsbId;

    int maxSrcInstance;

    IPAddress routerId;

    PSBVector PSBList;
    RSBVector RSBList;
    HelloVector HelloList;

  protected:
    void processSignallingMessage(SignallingMsg *msg);
    void processPSB_TIMER(PsbTimerMsg *msg);
    void processPSB_TIMEOUT(PsbTimeoutMsg* msg);
    void processRSB_REFRESH_TIMER(RsbRefreshTimerMsg *msg);
    void processRSB_COMMIT_TIMER(RsbCommitTimerMsg *msg);
    void processRSB_TIMEOUT(RsbTimeoutMsg* msg);
    void processHELLO_TIMER(HelloTimerMsg* msg);
    void processHELLO_TIMEOUT(HelloTimeoutMsg* msg);
    void processPATH_NOTIFY(PathNotifyMsg* msg);
    void processRSVPMessage(RSVPMessage* msg);
    void processHelloMsg(RSVPHelloMsg* msg);
    void processPathMsg(RSVPPathMsg* msg);
    void processResvMsg(RSVPResvMsg* msg);
    void processPathTearMsg(RSVPPathTear* msg);
    void processPathErrMsg(RSVPPathError* msg);

    PathStateBlock_t* createPSB(RSVPPathMsg *msg);
    PathStateBlock_t* createIngressPSB(const traffic_session_t& session, const traffic_path_t& path);
    void removePSB(PathStateBlock_t *psb);
    ResvStateBlock_t* createRSB(RSVPResvMsg *msg);
    ResvStateBlock_t* createEgressRSB(PathStateBlock_t *psb);
    void updateRSB(ResvStateBlock_t* rsb, RSVPResvMsg *msg);
    void removeRSB(ResvStateBlock_t *rsb);
    void removeRsbFilter(ResvStateBlock_t *rsb, unsigned int index);

    void refreshPath(PathStateBlock_t *psbEle);
    void refreshResv(ResvStateBlock_t *rsbEle);
    void refreshResv(ResvStateBlock_t *rsbEle, IPAddress PHOP);
    void commitResv(ResvStateBlock_t *rsb);

    void scheduleRefreshTimer(PathStateBlock_t *psbEle, double delay);
    void scheduleTimeout(PathStateBlock_t *psbEle);
    void scheduleRefreshTimer(ResvStateBlock_t *rsbEle, double delay);
    void scheduleCommitTimer(ResvStateBlock_t *rsbEle);
    void scheduleTimeout(ResvStateBlock_t *rsbEle);

    void sendPathErrorMessage(PathStateBlock_t *psb, int errCode);
    void sendPathErrorMessage(SessionObj_t session, SenderTemplateObj_t sender, SenderTspecObj_t tspec, IPAddress nextHop, int errCode);
    void sendPathTearMessage(IPAddress peerIP, const SessionObj_t& session, const SenderTemplateObj_t& sender, IPAddress LIH, IPAddress NHOP, bool force);
    void sendPathNotify(int handler, const SessionObj_t& session, const SenderTemplateObj_t& sender, int status, double delay);

    void setupHello();
    void startHello(IPAddress peer, double delay);

    void recoveryEvent(IPAddress peer);

    bool allocateResource(IPAddress OI, const SessionObj_t& session, double bandwidth);
    void preempt(IPAddress OI, int priority, double bandwidth);
    bool doCACCheck(const SessionObj_t& session, const SenderTspecObj_t& tspec, IPAddress OI);
    void announceLinkChange(int tedlinkindex);

    void sendToIP(cMessage *msg, IPAddress destAddr);

    bool evalNextHopInterface(IPAddress destAddr, const EroVector& ERO, IPAddress& OI);

    PathStateBlock_t* findPSB(const SessionObj_t& session, const SenderTemplateObj_t& sender);
    ResvStateBlock_t* findRSB(const SessionObj_t& session, const SenderTemplateObj_t& sender, unsigned int& index);

    PathStateBlock_t* findPsbById(int id);
    ResvStateBlock_t* findRsbById(int id);

    std::vector<traffic_session_t>::iterator findSession(const SessionObj_t& session);
    std::vector<traffic_path_t>::iterator findPath(traffic_session_t *session, const SenderTemplateObj_t &sender);

    HelloState_t* findHello(IPAddress peer);

    void print(RSVPPathMsg *p);
    void print(RSVPResvMsg *r);

    void readTrafficFromXML(const cXMLElement *traffic);
    void readTrafficSessionFromXML(const cXMLElement *session);
    EroVector readTrafficRouteFromXML(const cXMLElement *route);

    void createPath(const SessionObj_t& session, const SenderTemplateObj_t& sender);

    void pathProblem(PathStateBlock_t *psb);

    void addSession(const cXMLElement& node);
    void delSession(const cXMLElement& node);

  private:

    friend class SimpleClassifier;

    int getInLabel(const SessionObj_t& session, const SenderTemplateObj_t& sender);

  public:
    RSVP();
    virtual ~RSVP();

  protected:
    virtual int numInitStages() const  {return 5;}
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);

    // IScriptable implementation
    virtual void processCommand(const cXMLElement& node);

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

#endif



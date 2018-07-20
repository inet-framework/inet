//
// Copyright (C) 2011 CoCo Communications
// Copyright (C) 2012 Opensim Ltd
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_IGMPV2_H
#define __INET_IGMPV2_H

#include "inet/common/INETDefs.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/ipv4/IgmpMessage_m.h"

#include <set>

namespace inet {

class IInterfaceTable;
class IIpv4RoutingTable;

class INET_API Igmpv2 : public cSimpleModule, public IProtocolRegistrationListener, public cListener
{
  protected:
    enum RouterState {
        IGMP_RS_INITIAL,
        IGMP_RS_QUERIER,
        IGMP_RS_NON_QUERIER,
    };

    enum RouterGroupState {
        IGMP_RGS_NO_MEMBERS_PRESENT,
        IGMP_RGS_MEMBERS_PRESENT,
        IGMP_RGS_V1_MEMBERS_PRESENT,
        IGMP_RGS_CHECKING_MEMBERSHIP,
    };

    enum HostGroupState {
        IGMP_HGS_NON_MEMBER,
        IGMP_HGS_DELAYING_MEMBER,
        IGMP_HGS_IDLE_MEMBER,
    };

    struct HostGroupData
    {
        Igmpv2 *owner;
        Ipv4Address groupAddr;
        HostGroupState state;
        bool flag;    // true when we were the last host to send a report for this group
        cMessage *timer;

        HostGroupData(Igmpv2 *owner, const Ipv4Address& group);
        virtual ~HostGroupData();
    };
    typedef std::map<Ipv4Address, HostGroupData *> GroupToHostDataMap;

    struct RouterGroupData
    {
        Igmpv2 *owner;
        Ipv4Address groupAddr;
        RouterGroupState state;
        cMessage *timer;
        cMessage *rexmtTimer;
        //cMessage *v1HostTimer;

        RouterGroupData(Igmpv2 *owner, const Ipv4Address& group);
        virtual ~RouterGroupData();
    };
    typedef std::map<Ipv4Address, RouterGroupData *> GroupToRouterDataMap;

    struct HostInterfaceData
    {
        Igmpv2 *owner;
        GroupToHostDataMap groups;

        HostInterfaceData(Igmpv2 *owner);
        virtual ~HostInterfaceData();
    };
    typedef std::map<int, HostInterfaceData *> InterfaceToHostDataMap;

    struct RouterInterfaceData
    {
        Igmpv2 *owner;
        GroupToRouterDataMap groups;
        RouterState igmpRouterState;
        cMessage *igmpQueryTimer;

        RouterInterfaceData(Igmpv2 *owner);
        virtual ~RouterInterfaceData();
    };
    typedef std::map<int, RouterInterfaceData *> InterfaceToRouterDataMap;

    // Timers
    enum IgmpTimerKind {
        IGMP_QUERY_TIMER,
        IGMP_HOSTGROUP_TIMER,
        IGMP_LEAVE_TIMER,
        IGMP_REXMT_TIMER
    };

    struct IgmpHostTimerContext
    {
        InterfaceEntry *ie;
        HostGroupData *hostGroup;
        IgmpHostTimerContext(InterfaceEntry *ie, HostGroupData *hostGroup) : ie(ie), hostGroup(hostGroup) {}
    };

    struct IgmpRouterTimerContext
    {
        InterfaceEntry *ie;
        RouterGroupData *routerGroup;
        IgmpRouterTimerContext(InterfaceEntry *ie, RouterGroupData *routerGroup) : ie(ie), routerGroup(routerGroup) {}
    };

  protected:
    IIpv4RoutingTable *rt;    // cached pointer
    IInterfaceTable *ift;    // cached pointer

    bool enabled;
    bool externalRouter;
    int robustness;    // RFC 2236: Section 8.1
    double queryInterval;    // RFC 2236: Section 8.2
    double queryResponseInterval;    // RFC 2236: Section 8.3
    double groupMembershipInterval;    // RFC 2236: Section 8.4
    double otherQuerierPresentInterval;    // RFC 2236: Section 8.5
    double startupQueryInterval;    // RFC 2236: Section 8.6
    int startupQueryCount;    // RFC 2236: Section 8.7
    double lastMemberQueryInterval;    // RFC 2236: Section 8.8
    int lastMemberQueryCount;    // RFC 2236: Section 8.9
    double unsolicitedReportInterval;    // RFC 2236: Section 8.10
    //double version1RouterPresentInterval;  // RFC 2236: Section 8.11

    // state variables per interface
    InterfaceToHostDataMap hostData;
    InterfaceToRouterDataMap routerData;

    // group counters
    int numGroups;
    int numHostGroups;
    int numRouterGroups;

    // message counters
    int numQueriesSent;
    int numQueriesRecv;
    int numGeneralQueriesSent;
    int numGeneralQueriesRecv;
    int numGroupSpecificQueriesSent;
    int numGroupSpecificQueriesRecv;
    int numReportsSent;
    int numReportsRecv;
    int numLeavesSent;
    int numLeavesRecv;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void handleRegisterService(const Protocol& protocol, cGate *out, ServicePrimitive servicePrimitive) override;
    virtual void handleRegisterProtocol(const Protocol& protocol, cGate *in, ServicePrimitive servicePrimitive) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
    virtual ~Igmpv2();

  protected:
    virtual HostInterfaceData *createHostInterfaceData();
    virtual RouterInterfaceData *createRouterInterfaceData();
    virtual HostGroupData *createHostGroupData(InterfaceEntry *ie, const Ipv4Address& group);
    virtual RouterGroupData *createRouterGroupData(InterfaceEntry *ie, const Ipv4Address& group);
    virtual HostInterfaceData *getHostInterfaceData(InterfaceEntry *ie);
    virtual RouterInterfaceData *getRouterInterfaceData(InterfaceEntry *ie);
    virtual HostGroupData *getHostGroupData(InterfaceEntry *ie, const Ipv4Address& group);
    virtual RouterGroupData *getRouterGroupData(InterfaceEntry *ie, const Ipv4Address& group);
    virtual void deleteHostInterfaceData(int interfaceId);
    virtual void deleteRouterInterfaceData(int interfaceId);
    virtual void deleteHostGroupData(InterfaceEntry *ie, const Ipv4Address& group);
    virtual void deleteRouterGroupData(InterfaceEntry *ie, const Ipv4Address& group);

    virtual void configureInterface(InterfaceEntry *ie);
    virtual void multicastGroupJoined(InterfaceEntry *ie, const Ipv4Address& groupAddr);
    virtual void multicastGroupLeft(InterfaceEntry *ie, const Ipv4Address& groupAddr);

    virtual void startTimer(cMessage *timer, double interval);
    virtual void startHostTimer(InterfaceEntry *ie, HostGroupData *group, double maxRespTime);

    virtual void sendQuery(InterfaceEntry *ie, const Ipv4Address& groupAddr, double maxRespTime);
    virtual void sendReport(InterfaceEntry *ie, HostGroupData *group);
    virtual void sendLeave(InterfaceEntry *ie, HostGroupData *group);
    virtual void sendToIP(Packet *msg, InterfaceEntry *ie, const Ipv4Address& dest);

    virtual void processQueryTimer(cMessage *msg);
    virtual void processHostGroupTimer(cMessage *msg);
    virtual void processLeaveTimer(cMessage *msg);
    virtual void processRexmtTimer(cMessage *msg);

    virtual void processIgmpMessage(Packet *packet, const Ptr<const IgmpMessage>& igmp);
    virtual void processQuery(InterfaceEntry *ie, Packet *packet);
    virtual void processGroupQuery(InterfaceEntry *ie, HostGroupData *group, simtime_t maxRespTime);
    //virtual void processV1Report(InterfaceEntry *ie, IgmpMessage *msg);
    virtual void processV2Report(InterfaceEntry *ie, Packet *packet);
    virtual void processLeave(InterfaceEntry *ie, Packet *packet);
};

}    // namespace inet

#endif    // ifndef __INET_IGMPV2_H


//
// Copyright (C) 2011 CoCo Communications
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IGMPV2_H
#define __INET_IGMPV2_H

#include <set>

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/ipv4/IgmpMessage_m.h"

namespace inet {

class IInterfaceTable;
class IIpv4RoutingTable;

class INET_API Igmpv2 : public cSimpleModule, public DefaultProtocolRegistrationListener, public cListener
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

    struct HostGroupData {
        Igmpv2 *owner;
        Ipv4Address groupAddr;
        HostGroupState state;
        bool flag; // true when we were the last host to send a report for this group
        cMessage *timer;

        HostGroupData(Igmpv2 *owner, const Ipv4Address& group);
        virtual ~HostGroupData();
    };
    typedef std::map<Ipv4Address, HostGroupData *> GroupToHostDataMap;

    struct RouterGroupData {
        Igmpv2 *owner;
        Ipv4Address groupAddr;
        RouterGroupState state;
        cMessage *timer;
        cMessage *rexmtTimer;
//        cMessage *v1HostTimer;

        RouterGroupData(Igmpv2 *owner, const Ipv4Address& group);
        virtual ~RouterGroupData();
    };
    typedef std::map<Ipv4Address, RouterGroupData *> GroupToRouterDataMap;

    struct HostInterfaceData {
        Igmpv2 *owner;
        GroupToHostDataMap groups;

        HostInterfaceData(Igmpv2 *owner);
        virtual ~HostInterfaceData();
        friend inline std::ostream& operator<<(std::ostream& out, const Igmpv2::HostInterfaceData& entry)
        {
            for (auto& g : entry.groups) {
                out << "(groupAddress: " << g.second->groupAddr << " ";
                out << "hostGroupState: " << Igmpv2::getHostGroupStateString(g.second->state) << " ";
                out << "groupTimer: " << g.second->timer->getArrivalTime() << " ";
                out << "lastHost: " << g.second->flag << ") ";
            }

            return out;
        }
    };

    struct RouterInterfaceData {
        Igmpv2 *owner;
        GroupToRouterDataMap groups;
        RouterState igmpRouterState;
        cMessage *igmpQueryTimer;

        RouterInterfaceData(Igmpv2 *owner);
        virtual ~RouterInterfaceData();
        friend inline std::ostream& operator<<(std::ostream& out, const Igmpv2::RouterInterfaceData& entry)
        {
            out << "routerState: " << Igmpv2::getRouterStateString(entry.igmpRouterState) << " ";
            out << "queryTimer: " << entry.igmpQueryTimer->getArrivalTime() << " ";
            if (entry.groups.empty())
                out << "(empty)";
            else {
                for (auto& g : entry.groups) {
                    out << "(groupAddress: " << g.second->groupAddr << " ";
                    out << "routerGroupState: " << Igmpv2::getRouterGroupStateString(g.second->state) << " ";
                    out << "timer: " << g.second->timer->getArrivalTime() << " ";
                    out << "rexmtTimer: " << g.second->rexmtTimer->getArrivalTime() << ") ";
                }
            }

            return out;
        }
    };

    struct IgmpHostTimerContext {
        NetworkInterface *ie;
        HostGroupData *hostGroup;
        IgmpHostTimerContext(NetworkInterface *ie, HostGroupData *hostGroup) : ie(ie), hostGroup(hostGroup) {}
    };

    struct IgmpRouterTimerContext {
        NetworkInterface *ie;
        RouterGroupData *routerGroup;
        IgmpRouterTimerContext(NetworkInterface *ie, RouterGroupData *routerGroup) : ie(ie), routerGroup(routerGroup) {}
    };

  protected:
    ModuleRefByPar<IIpv4RoutingTable> rt;
    ModuleRefByPar<IInterfaceTable> ift;

    bool enabled;
    bool externalRouter;
    int robustness; // RFC 2236: Section 8.1
    double queryInterval; // RFC 2236: Section 8.2
    double queryResponseInterval; // RFC 2236: Section 8.3
    double groupMembershipInterval; // RFC 2236: Section 8.4
    double otherQuerierPresentInterval; // RFC 2236: Section 8.5
    double startupQueryInterval; // RFC 2236: Section 8.6
    int startupQueryCount; // RFC 2236: Section 8.7
    double lastMemberQueryInterval; // RFC 2236: Section 8.8
    int lastMemberQueryCount; // RFC 2236: Section 8.9
    double unsolicitedReportInterval; // RFC 2236: Section 8.10
    // double version1RouterPresentInterval;  // RFC 2236: Section 8.11

    // group counters
    int numGroups = 0;
    int numHostGroups = 0;
    int numRouterGroups = 0;

    // message counters
    int numQueriesSent = 0;
    int numQueriesRecv = 0;
    int numGeneralQueriesSent = 0;
    int numGeneralQueriesRecv = 0;
    int numGroupSpecificQueriesSent = 0;
    int numGroupSpecificQueriesRecv = 0;
    int numReportsSent = 0;
    int numReportsRecv = 0;
    int numLeavesSent = 0;
    int numLeavesRecv = 0;

    // crcMode
    CrcMode crcMode = CRC_MODE_UNDEFINED;

    typedef std::map<int, HostInterfaceData *> InterfaceToHostDataMap;
    typedef std::map<int, RouterInterfaceData *> InterfaceToRouterDataMap;

    // state variables per interface
    InterfaceToHostDataMap hostData;
    InterfaceToRouterDataMap routerData;

  public:
    static const std::string getRouterStateString(RouterState rs);
    static const std::string getRouterGroupStateString(RouterGroupState rgs);
    static const std::string getHostGroupStateString(HostGroupState hgs);

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void handleRegisterService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override;
    virtual void handleRegisterProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
    virtual ~Igmpv2();

  protected:
    void addWatches();

    virtual void multicastGroupJoined(NetworkInterface *ie, const Ipv4Address& groupAddr);
    virtual void multicastGroupLeft(NetworkInterface *ie, const Ipv4Address& groupAddr);
    virtual void configureInterface(NetworkInterface *ie);

    virtual void deleteHostInterfaceData(int interfaceId);
    virtual void deleteRouterInterfaceData(int interfaceId);

    virtual void processQueryTimer(cMessage *msg);
    virtual void processHostGroupTimer(cMessage *msg);
    virtual void processLeaveTimer(cMessage *msg);
    virtual void processRexmtTimer(cMessage *msg);

    virtual void startTimer(cMessage *timer, double interval);
    virtual void startHostTimer(NetworkInterface *ie, HostGroupData *group, double maxRespTime);

    virtual void processIgmpMessage(Packet *packet);
    virtual void processQuery(NetworkInterface *ie, Packet *packet);
    virtual void processGroupQuery(NetworkInterface *ie, HostGroupData *group, simtime_t maxRespTime);
//    virtual void processV1Report(NetworkInterface *ie, IgmpMessage *msg);
    virtual void processV2Report(NetworkInterface *ie, Packet *packet);
    virtual void processLeave(NetworkInterface *ie, Packet *packet);

    virtual void sendQuery(NetworkInterface *ie, const Ipv4Address& groupAddr, double maxRespTime);
    virtual void sendReport(NetworkInterface *ie, HostGroupData *group);
    virtual void sendLeave(NetworkInterface *ie, HostGroupData *group);
    virtual void sendToIP(Packet *msg, NetworkInterface *ie, const Ipv4Address& dest);

    virtual RouterGroupData *createRouterGroupData(NetworkInterface *ie, const Ipv4Address& group);
    virtual HostGroupData *createHostGroupData(NetworkInterface *ie, const Ipv4Address& group);
    virtual RouterGroupData *getRouterGroupData(NetworkInterface *ie, const Ipv4Address& group);
    virtual HostGroupData *getHostGroupData(NetworkInterface *ie, const Ipv4Address& group);
    virtual void deleteRouterGroupData(NetworkInterface *ie, const Ipv4Address& group);
    virtual void deleteHostGroupData(NetworkInterface *ie, const Ipv4Address& group);

    virtual RouterInterfaceData *getRouterInterfaceData(NetworkInterface *ie);
    virtual RouterInterfaceData *createRouterInterfaceData();
    virtual HostInterfaceData *getHostInterfaceData(NetworkInterface *ie);
    virtual HostInterfaceData *createHostInterfaceData();

  public:
    static void insertCrc(CrcMode crcMode, const Ptr<IgmpMessage>& igmpMsg, Packet *payload);
    void insertCrc(const Ptr<IgmpMessage>& igmpMsg, Packet *payload) { insertCrc(crcMode, igmpMsg, payload); }
    bool verifyCrc(const Packet *packet);
};

} // namespace inet

#endif


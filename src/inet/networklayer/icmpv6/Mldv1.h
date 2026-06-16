//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_MLDV1_H
#define __INET_MLDV1_H

#include <map>

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/SimpleModule.h"
#include "inet/common/checksum/ChecksumMode_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/ipv6/Ipv6Address.h"

namespace inet {

class IInterfaceTable;
class Ipv6RoutingTable;

class INET_API Mldv1 : public OperationalBase, public cListener
{
  protected:
    // --- Host listener state machine (RFC 2710 §5) ---

    // RFC 2710 §5 host state names (Non-Listener / Delaying Listener / Idle Listener)
    enum HostGroupState {
        MLD_HGS_NON_LISTENER,
        MLD_HGS_DELAYING_LISTENER,
        MLD_HGS_IDLE_LISTENER,
    };

    // Timer kind constants (RFC 2710 router timer kinds added here)
    enum MldTimerKind {
        MLD_HOSTGROUP_TIMER = 1,
        MLD_QUERY_TIMER     = 2,    // periodic General Query timer on RouterInterfaceData
        MLD_LEAVE_TIMER     = 3,    // group membership timer on RouterGroupData (→ NO_LISTENERS)
        MLD_REXMT_TIMER     = 4,    // last-listener retransmit timer on RouterGroupData
    };

    struct HostGroupData {
        Mldv1 *owner;
        Ipv6Address groupAddr;
        HostGroupState state;
        bool flag; // true when we were the last listener to send a report for this group
        cMessage *timer;

        HostGroupData(Mldv1 *owner, const Ipv6Address& group);
        virtual ~HostGroupData();
    };
    typedef std::map<Ipv6Address, HostGroupData *> GroupToHostDataMap;

    struct HostInterfaceData {
        Mldv1 *owner;
        GroupToHostDataMap groups;

        HostInterfaceData(Mldv1 *owner);
        virtual ~HostInterfaceData();
    };

    typedef std::map<int, HostInterfaceData *> InterfaceToHostDataMap;

    struct MldHostTimerContext {
        NetworkInterface *ie;
        HostGroupData *hostGroup;
        MldHostTimerContext(NetworkInterface *ie, HostGroupData *hostGroup) : ie(ie), hostGroup(hostGroup) {}
    };

    // --- Router/querier state machine (RFC 2710 §6) ---

    // RFC 2710 §6 router group state names
    enum RouterGroupState {
        MLD_RGS_NO_LISTENERS_PRESENT,
        MLD_RGS_LISTENERS_PRESENT,
        MLD_RGS_CHECKING_LISTENERS,
    };

    struct RouterGroupData {
        Mldv1 *owner;
        Ipv6Address groupAddr;
        RouterGroupState state;
        cMessage *timer;        // group membership timer (→ leave/group-timer expiry → NO_LISTENERS)
        cMessage *rexmtTimer;   // last-listener retransmit timer (used in CHECKING_LISTENERS)

        RouterGroupData(Mldv1 *owner, const Ipv6Address& group);
        virtual ~RouterGroupData();
    };
    typedef std::map<Ipv6Address, RouterGroupData *> GroupToRouterDataMap;

    struct RouterInterfaceData {
        Mldv1 *owner;
        GroupToRouterDataMap groups;
        cMessage *mldQueryTimer;    // periodic General Query timer

        RouterInterfaceData(Mldv1 *owner);
        virtual ~RouterInterfaceData();
    };

    struct MldRouterTimerContext {
        NetworkInterface *ie;
        RouterGroupData *routerGroup;
        MldRouterTimerContext(NetworkInterface *ie, RouterGroupData *routerGroup) : ie(ie), routerGroup(routerGroup) {}
    };

    typedef std::map<int, RouterInterfaceData *> InterfaceToRouterDataMap;

  protected:
    ModuleRefByPar<IInterfaceTable> ift;
    ModuleRefByPar<Ipv6RoutingTable> rt;

    bool enabled = true;
    ChecksumMode checksumMode = CHECKSUM_MODE_UNDEFINED;

    // Host parameters
    double unsolicitedReportInterval = 10; // delay between a host's repeated Reports after a join

    // Router (querier) parameters
    double queryInterval = 125;             // interval between periodic General Queries
    double queryResponseInterval = 10;      // max delay a host may wait before answering a General Query
    double multicastListenerInterval = 260; // how long a group stays "has listeners" without a new Report
    double lastListenerQueryInterval = 1;   // interval between the group-specific Queries sent after a Done
    int lastListenerQueryCount = 2;         // number of group-specific Queries sent after a Done
    double startupQueryInterval = 31.25;    // interval between the initial General Queries at startup
    int startupQueryCount = 2;              // number of General Queries sent at startup

    // Group counters
    int numGroups = 0;
    int numHostGroups = 0;
    int numRouterGroups = 0;

    // Message counters
    int numReportsSent = 0;
    int numReportsRecv = 0;
    int numDonesSent = 0;
    int numDonesRecv = 0;
    int numQueriesSent = 0;
    int numQueriesRecv = 0;
    int numGeneralQueriesSent = 0;
    int numGeneralQueriesRecv = 0;
    int numGroupSpecificQueriesSent = 0;
    int numGroupSpecificQueriesRecv = 0;

    // Per-interface host state (keyed by NetworkInterface::getInterfaceId())
    InterfaceToHostDataMap hostData;

    // Per-interface router state (keyed by NetworkInterface::getInterfaceId())
    InterfaceToRouterDataMap routerData;

  protected:
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_NETWORK_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_NETWORK_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_NETWORK_LAYER; }
    virtual void handleStartOperation(LifecycleOperation *operation) override {}
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual ~Mldv1();

    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    // Signal handler (cListener)
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

    // Host join/leave handlers
    virtual void multicastGroupJoined(NetworkInterface *ie, const Ipv6Address& groupAddr);
    virtual void multicastGroupLeft(NetworkInterface *ie, const Ipv6Address& groupAddr);

    // Host timer handlers
    virtual void processHostGroupTimer(cMessage *msg);
    virtual void startTimer(cMessage *timer, double interval);
    virtual void startHostTimer(NetworkInterface *ie, HostGroupData *group, double maxRespTime);

    // Inbound message processing (Query + Report — bodies in Plan 03-02)
    virtual void processQuery(NetworkInterface *ie, Packet *packet);
    virtual void processGroupQuery(NetworkInterface *ie, HostGroupData *group, simtime_t maxRespTime);
    virtual void processReport(NetworkInterface *ie, Packet *packet);

    // Outbound send helpers
    virtual void sendReport(NetworkInterface *ie, HostGroupData *group);
    virtual void sendDone(NetworkInterface *ie, HostGroupData *group);

    // Host group-data CRUD
    virtual HostGroupData *createHostGroupData(NetworkInterface *ie, const Ipv6Address& group);
    virtual HostGroupData *getHostGroupData(NetworkInterface *ie, const Ipv6Address& group);
    virtual void deleteHostGroupData(NetworkInterface *ie, const Ipv6Address& group);

    // Host interface-data CRUD
    virtual HostInterfaceData *getHostInterfaceData(NetworkInterface *ie);
    virtual HostInterfaceData *createHostInterfaceData();
    virtual void deleteHostInterfaceData(int interfaceId);

    // MLD message dispatch and IP send helper
    virtual void processMldMessage(Packet *packet);
    virtual void sendToIPv6(Packet *msg, NetworkInterface *ie, const Ipv6Address& dest);

    // Router interface lifecycle
    virtual void configureInterface(NetworkInterface *ie);
    virtual void deleteRouterInterfaceData(int interfaceId);

    // Router timer handlers
    virtual void processQueryTimer(cMessage *msg);
    virtual void processLeaveTimer(cMessage *msg);
    virtual void processRexmtTimer(cMessage *msg);

    // Router outbound query
    virtual void sendQuery(NetworkInterface *ie, const Ipv6Address& groupAddr, double maxRespTime);

    // Router group-data CRUD
    virtual RouterGroupData *createRouterGroupData(NetworkInterface *ie, const Ipv6Address& group);
    virtual RouterGroupData *getRouterGroupData(NetworkInterface *ie, const Ipv6Address& group);
    virtual void deleteRouterGroupData(NetworkInterface *ie, const Ipv6Address& group);

    // Router interface-data CRUD
    virtual RouterInterfaceData *getRouterInterfaceData(NetworkInterface *ie);
    virtual RouterInterfaceData *createRouterInterfaceData();

    // Router Done handler
    virtual void processDone(NetworkInterface *ie, Packet *packet);
};

} // namespace inet

#endif // ifndef __INET_MLDV1_H

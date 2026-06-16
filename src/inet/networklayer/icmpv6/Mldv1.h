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

    // Timer kind constants (router timer kinds added in Phase 4)
    enum MldTimerKind {
        MLD_HOSTGROUP_TIMER = 1,
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

    // --- Phase 4: router-side declarations will be added here ---

  protected:
    ModuleRefByPar<IInterfaceTable> ift;
    ModuleRefByPar<Ipv6RoutingTable> rt;

    bool enabled = true;
    ChecksumMode checksumMode = CHECKSUM_MODE_UNDEFINED;
    bool sendTestMessage = false;
    bool sendTestQuery = false;
    bool sendTestLeave = false;
    Ipv6Address testMulticastGroup;  // UNSPECIFIED_ADDRESS when inactive
    cMessage *testTimer = nullptr;
    cMessage *testQueryTimer = nullptr;
    cMessage *testLeaveTimer = nullptr;

    // Host parameters
    double unsolicitedReportInterval = 10; // RFC 2710 §7.2 default 10s

    // Group counters
    int numGroups = 0;
    int numHostGroups = 0;

    // Message counters
    int numReportsSent = 0;
    int numReportsRecv = 0;
    int numDonesSent = 0;

    // Per-interface host state (keyed by NetworkInterface::getInterfaceId())
    InterfaceToHostDataMap hostData;

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
};

} // namespace inet

#endif // ifndef __INET_MLDV1_H

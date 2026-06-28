//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// This implementation is based on the IS-IS model of the ANSA project
// (https://ansa.omnetpp.org), Brno University of Technology, ported to the
// modern INET packet (Chunk) API.
//

#ifndef __INET_ISIS_H
#define __INET_ISIS_H

#include <vector>

#include "inet/common/ModuleRefByPar.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcSocket.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/routing/base/RoutingProtocolBase.h"
#include "inet/routing/isis/IsisCommon.h"
#include "inet/routing/isis/IsisMessages_m.h"

namespace inet {
namespace isis {

//
// State of an IS-IS adjacency (ISO/IEC 10589).
//
enum IsisAdjacencyState {
    ISIS_ADJ_DOWN,
    ISIS_ADJ_INITIALIZING,
    ISIS_ADJ_UP
};

//
// Per-interface IS-IS configuration and runtime state.
//
struct IsisInterface {
    int interfaceId = -1;
    bool broadcast = true;            // LAN circuit (true) vs. point-to-point (false)
    uint8_t circuitType = L1L2_TYPE;  // IsisCircuitType
    uint32_t metric = 10;
    uint8_t priority = 64;            // DIS election priority (LAN)
    simtime_t helloInterval;
    int holdMultiplier = 3;
    cMessage *helloTimer = nullptr;
};

//
// One IS-IS adjacency, i.e. a neighbour heard on a given interface at a given
// level.
//
struct IsisAdjacency {
    int interfaceId = -1;
    int level = 1;                    // 1 or 2
    SystemId neighbourSystemId;
    AreaId neighbourAreaId;
    MacAddress snpa;                  // neighbour's MAC (SNPA)
    IsisAdjacencyState state = ISIS_ADJ_DOWN;
    cMessage *holdTimer = nullptr;
};

//
// Implements the IS-IS routing protocol (ISO/IEC 10589 / RFC 1195). See the
// NED file for an overview. PDUs are exchanged over the link layer through an
// `Ieee8022LlcSocket`.
//
class INET_API Isis : public RoutingProtocolBase, public Ieee8022LlcSocket::ICallback
{
  protected:
    ModuleRefByPar<IInterfaceTable> ift;

    // This router's IS-IS identity, derived from the configured NET address.
    SystemId systemId;
    AreaId areaId;
    uint8_t isType = L1L2_TYPE;       // levels this IS participates in

    // PDUs ride directly on IEEE 802.2 LLC (SAP 0xFE), not on IP.
    Ieee8022LlcSocket llcSocket;

    std::vector<IsisInterface *> isisInterfaces;
    std::vector<IsisAdjacency *> adjacencies;

    simsignal_t adjacencyChangedSignal;

    // OSI multicast MACs for the All Level-1 / All Level-2 ISs.
    static const MacAddress ALL_L1_ISS;
    static const MacAddress ALL_L2_ISS;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    // OperationalBase lifecycle
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    // Ieee8022LlcSocket::ICallback
    virtual void socketDataArrived(Ieee8022LlcSocket *socket, Packet *packet) override;
    virtual void socketClosed(Ieee8022LlcSocket *socket) override;

    // configuration
    virtual void parseConfig();

    // Hello / adjacency
    virtual void sendLanHello(IsisInterface *isisIft, int level);
    virtual void processHello(Packet *packet);
    virtual void handleHelloTimer(IsisInterface *isisIft);
    virtual void handleHoldTimer(IsisAdjacency *adj);

    virtual IsisInterface *findInterface(int interfaceId);
    virtual IsisAdjacency *findAdjacency(int interfaceId, int level, const SystemId& sysId);
    virtual void setAdjacencyState(IsisAdjacency *adj, IsisAdjacencyState newState);

    virtual void clearState();

  public:
    Isis() {}
    virtual ~Isis();

    const SystemId& getSystemId() const { return systemId; }
    const AreaId& getAreaId() const { return areaId; }
};

} // namespace isis
} // namespace inet

#endif

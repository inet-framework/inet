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

#include <map>
#include <vector>

#include "inet/common/ModuleRefByPar.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcSocket.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/routing/base/RoutingProtocolBase.h"
#include "inet/routing/isis/IsisCommon.h"
#include "inet/routing/isis/IsisMessages_m.h"

namespace inet {

class Ipv4Route;

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
    int localCircuitId = 0;           // this router's circuit number (pseudonode id when DIS)
    simtime_t helloInterval;
    int holdMultiplier = 3;
    cMessage *helloTimer = nullptr;

    // Designated IS (DIS) election state for this LAN circuit (Level 1).
    bool isDis = false;               // is this router the DIS on this circuit?
    bool disValid = false;            // has a DIS been elected yet?
    PseudonodeId disPseudonodeId;     // the elected DIS's pseudonode id (LAN id)
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
    Ipv4Address neighbourIpAddress;   // neighbour's IP on this link (next hop for routes)
    int priority = 0;                 // neighbour's DIS priority (from its Hello)
    PseudonodeId lanId;               // the DIS pseudonode the neighbour advertises
    IsisAdjacencyState state = ISIS_ADJ_DOWN;
    cMessage *holdTimer = nullptr;
};

//
// One Link State PDU held in the link-state database (LSPDB).
//
struct IsisLsp {
    LspId lspId;
    uint32_t sequenceNumber = 0;
    Ptr<const IsisLspPacket> lsp;     // the stored LSP chunk
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

    ModuleRefByPar<IIpv4RoutingTable> rt;

    // Level-1 link-state database, keyed by LspId::toInt().
    std::map<uint64_t, IsisLsp *> lspDatabase;
    cMessage *regenerateLspTimer = nullptr;
    cMessage *spfTimer = nullptr;

    // IPv4 routes installed by IS-IS (Integrated IS-IS), for clean recomputation.
    std::vector<Ipv4Route *> isisRoutes;

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

    // PDU dispatch
    virtual void processPdu(Packet *packet);

    // Hello / adjacency
    virtual void sendLanHello(IsisInterface *isisIft, int level);
    virtual void processHello(Packet *packet);
    virtual void handleHelloTimer(IsisInterface *isisIft);
    virtual void handleHoldTimer(IsisAdjacency *adj);

    // DIS election (LAN)
    virtual void runDisElection(IsisInterface *isisIft);

    // Link-state PDUs (Level 1)
    virtual void scheduleLspRegeneration();
    virtual void originateLsp();
    virtual void originatePseudonodeLsp(IsisInterface *isisIft);
    virtual void installAndFloodOwnLsp(const Ptr<IsisLspPacket>& lsp);
    virtual void floodLsp(const Ptr<const IsisLspPacket>& lsp, int exceptInterfaceId);
    virtual void sendDatabaseToInterface(int interfaceId);
    virtual void processLsp(Packet *packet);
    virtual bool hasUpAdjacency(int interfaceId, int level);

    // Shortest-path computation and route installation (Integrated IS-IS)
    virtual void scheduleSpf();
    virtual void runSpf();
    virtual void removeIsisRoutes();
    virtual IsisLsp *lookupLsp(uint64_t systemId);
    virtual IsisAdjacency *findUpAdjacencyTo(uint64_t systemId);

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

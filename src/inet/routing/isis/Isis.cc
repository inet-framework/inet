//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// This implementation is based on the IS-IS model of the ANSA project
// (https://ansa.omnetpp.org), Brno University of Technology, ported to the
// modern INET packet (Chunk) API.
//

#include "inet/routing/isis/Isis.h"

#include <algorithm>
#include <cmath>
#include <set>

#include "inet/common/ModuleAccess.h"
#include "inet/common/Protocol.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/networklayer/contract/clns/ClnsAddress.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/ipv4/Ipv4Route.h"
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#include "inet/networklayer/ipv6/Ipv6Route.h"

namespace inet {
namespace isis {

Define_Module(Isis);

// IS-IS PDU LLC SAP (ISO/IEC 10589).
static const int ISIS_SAP = 0xFE;

// Self-message kinds.
enum IsisTimerKind {
    HELLO_TIMER = 1, HOLD_TIMER = 2, REGENERATE_LSP_TIMER = 3, SPF_TIMER = 4,
    REFRESH_TIMER = 5, AGE_TIMER = 6, CSNP_TIMER = 7
};

// Returns true if dest/mask is one of this router's directly-connected networks.
static bool isLocalNetwork(IInterfaceTable *ift, Ipv4Address dest, Ipv4Address mask)
{
    for (int i = 0; i < ift->getNumInterfaces(); i++) {
        auto *ie = ift->getInterface(i);
        if (ie->isLoopback())
            continue;
        auto ipv4Data = ie->findProtocolData<Ipv4InterfaceData>();
        if (ipv4Data == nullptr || ipv4Data->getIPAddress().isUnspecified())
            continue;
        if (ipv4Data->getNetmask() == mask && ipv4Data->getIPAddress().doAnd(mask) == dest)
            return true;
    }
    return false;
}

// Returns true if the IPv6 prefix is one of this router's directly-connected networks.
static bool isLocalNetwork6(IInterfaceTable *ift, Ipv6Address prefix, int prefixLength)
{
    for (int i = 0; i < ift->getNumInterfaces(); i++) {
        auto *ie = ift->getInterface(i);
        if (ie->isLoopback())
            continue;
        auto ipv6Data = ie->findProtocolData<Ipv6InterfaceData>();
        if (ipv6Data == nullptr)
            continue;
        for (int j = 0; j < ipv6Data->getNumAddresses(); j++) {
            Ipv6Address a = ipv6Data->getAddress(j);
            if (!a.isLinkLocal() && !a.isUnspecified() && a.getPrefix(prefixLength) == prefix)
                return true;
        }
    }
    return false;
}

const MacAddress Isis::ALL_L1_ISS("01:80:C2:00:00:14");
const MacAddress Isis::ALL_L2_ISS("01:80:C2:00:00:15");

Isis::~Isis()
{
    clearState();
}

void Isis::initialize(int stage)
{
    RoutingProtocolBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        ift.reference(this, "interfaceTableModule", true);
        const char *rtModule = par("routingTableModule").stringValue();
        if (rtModule && rtModule[0])
            rt.reference(this, "routingTableModule", true);
        const char *rt6Module = par("routingTableModule6").stringValue();
        if (rt6Module && rt6Module[0])
            rt6.reference(this, "routingTableModule6", true);
        adjacencyChangedSignal = registerSignal("adjacencyChanged");
        llcSocket.setOutputGate(gate("socketOut"));
        llcSocket.setCallback(this);
    }
}

void Isis::handleStartOperation(LifecycleOperation *operation)
{
    parseConfig();

    if (isisInterfaces.empty()) {
        EV_WARN << "IS-IS has no configured/enabled interfaces; staying idle\n";
        return;
    }

    // A single LLC socket receives IS-IS PDUs (DSAP/SSAP 0xFE) from every
    // interface; the egress interface is selected per packet with an
    // InterfaceReq tag.
    llcSocket.open(-1, ISIS_SAP, ISIS_SAP);

    for (auto *isisIft : isisInterfaces) {
        auto *ie = ift->getInterfaceById(isisIft->interfaceId);
        if (isType & L1_TYPE)
            ie->addMulticastMacAddress(ALL_L1_ISS);
        if (isType & L2_TYPE)
            ie->addMulticastMacAddress(ALL_L2_ISS);

        isisIft->helloTimer = new cMessage("helloTimer", HELLO_TIMER);
        isisIft->helloTimer->setContextPointer(isisIft);
        scheduleAfter(uniform(0, isisIft->helloInterval), isisIft->helloTimer);

        if (isisIft->broadcast) {
            isisIft->csnpTimer = new cMessage("csnpTimer", CSNP_TIMER);
            isisIft->csnpTimer->setContextPointer(isisIft);
            scheduleAfter(par("csnpInterval").doubleValue(), isisIft->csnpTimer);
        }
    }

    refreshTimer = new cMessage("refreshTimer", REFRESH_TIMER);
    scheduleAfter(par("lspRefreshInterval").doubleValue(), refreshTimer);
    ageTimer = new cMessage("ageTimer", AGE_TIMER);
    scheduleAfter(1.0, ageTimer);

    EV_INFO << "IS-IS started, system ID " << systemId << ", area " << areaId
            << ", on " << isisInterfaces.size() << " interface(s)\n";
}

void Isis::handleStopOperation(LifecycleOperation *operation)
{
    if (llcSocket.isOpen())
        llcSocket.close();
    removeIsisRoutes();
    clearState();
}

void Isis::handleCrashOperation(LifecycleOperation *operation)
{
    if (llcSocket.isOpen())
        llcSocket.destroy();
    clearState();
}

void Isis::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        switch (msg->getKind()) {
            case HELLO_TIMER:
                handleHelloTimer(static_cast<IsisInterface *>(msg->getContextPointer()));
                break;
            case HOLD_TIMER:
                handleHoldTimer(static_cast<IsisAdjacency *>(msg->getContextPointer()));
                break;
            case REGENERATE_LSP_TIMER:
                originateLsp();
                break;
            case SPF_TIMER:
                runSpf();
                break;
            case REFRESH_TIMER:
                originateLsp();
                scheduleAfter(par("lspRefreshInterval").doubleValue(), refreshTimer);
                break;
            case AGE_TIMER:
                ageLsps();
                scheduleAfter(1.0, ageTimer);
                break;
            case CSNP_TIMER: {
                auto *isisIft = static_cast<IsisInterface *>(msg->getContextPointer());
                if (isisIft->isDis)
                    sendCsnp(isisIft);
                scheduleAfter(par("csnpInterval").doubleValue(), isisIft->csnpTimer);
                break;
            }
            default:
                throw cRuntimeError("Unknown self message kind %d", msg->getKind());
        }
    }
    else if (llcSocket.belongsToSocket(msg))
        llcSocket.processMessage(msg);
    else
        throw cRuntimeError("Unknown message '%s'", msg->getName());
}

void Isis::socketDataArrived(Ieee8022LlcSocket *socket, Packet *packet)
{
    processPdu(packet);
    delete packet;
}

void Isis::processPdu(Packet *packet)
{
    const auto& pdu = packet->peekAtFront<IsisPdu>();
    switch (pdu->getPduType()) {
        case LAN_L1_HELLO:
        case LAN_L2_HELLO:
            processHello(packet);
            break;
        case L1_LSP:
        case L2_LSP:
            processLsp(packet);
            break;
        case L1_CSNP:
        case L2_CSNP:
            processCsnp(packet);
            break;
        case L1_PSNP:
        case L2_PSNP:
            processPsnp(packet);
            break;
        default:
            EV_WARN << "Ignoring unknown IS-IS PDU type " << (int)pdu->getPduType() << "\n";
            break;
    }
}

void Isis::socketClosed(Ieee8022LlcSocket *socket)
{
}

void Isis::parseConfig()
{
    cXMLElement *config = par("configData");
    if (config == nullptr)
        return;

    const char *net = config->getAttribute("net");
    if (net == nullptr || !net[0]) {
        EV_WARN << "IS-IS: no NET configured\n";
        return;
    }

    ClnsAddress address(net);
    systemId.setSystemId(address.getSystemId());
    areaId.setAreaId(address.getAreaId());

    const char *isTypeStr = config->getAttribute("isType");
    if (isTypeStr == nullptr || !strcmp(isTypeStr, "L1L2"))
        isType = L1L2_TYPE;
    else if (!strcmp(isTypeStr, "L1"))
        isType = L1_TYPE;
    else if (!strcmp(isTypeStr, "L2"))
        isType = L2_TYPE;
    else
        throw cRuntimeError("IS-IS: invalid isType '%s' (expected L1, L2 or L1L2)", isTypeStr);

    cXMLElementList interfaceElements = config->getChildrenByTagName("interface");
    if (!interfaceElements.empty()) {
        for (auto *element : interfaceElements) {
            const char *name = element->getAttribute("name");
            if (name == nullptr)
                throw cRuntimeError("IS-IS: <interface> without a 'name' attribute");
            auto *ie = ift->findInterfaceByName(name);
            if (ie == nullptr)
                throw cRuntimeError("IS-IS: no such interface '%s'", name);

            auto *isisIft = new IsisInterface();
            isisIft->interfaceId = ie->getInterfaceId();
            isisIft->localCircuitId = (int)isisInterfaces.size() + 1;
            const char *metric = element->getAttribute("metric");
            if (metric)
                isisIft->metric = atoi(metric);
            const char *priority = element->getAttribute("priority");
            if (priority)
                isisIft->priority = atoi(priority);
            const char *helloInterval = element->getAttribute("helloInterval");
            isisIft->helloInterval = helloInterval ? SimTime::parse(helloInterval) : SimTime(10, SIMTIME_S);
            const char *type = element->getAttribute("type");
            isisIft->broadcast = (type == nullptr || !strcmp(type, "broadcast"));
            isisIft->circuitType = isType;
            isisInterfaces.push_back(isisIft);
        }
    }
    else {
        // No explicit interface list: enable IS-IS on every non-loopback interface.
        for (int i = 0; i < ift->getNumInterfaces(); i++) {
            auto *ie = ift->getInterface(i);
            if (ie->isLoopback() || !ie->isBroadcast())
                continue;
            auto *isisIft = new IsisInterface();
            isisIft->interfaceId = ie->getInterfaceId();
            isisIft->localCircuitId = (int)isisInterfaces.size() + 1;
            isisIft->helloInterval = SimTime(10, SIMTIME_S);
            isisIft->circuitType = isType;
            isisInterfaces.push_back(isisIft);
        }
    }
}

void Isis::handleHelloTimer(IsisInterface *isisIft)
{
    if ((isType & L1_TYPE) && (isisIft->circuitType & L1_TYPE))
        sendLanHello(isisIft, 1);
    if ((isType & L2_TYPE) && (isisIft->circuitType & L2_TYPE))
        sendLanHello(isisIft, 2);
    scheduleAfter(isisIft->helloInterval, isisIft->helloTimer);
}

void Isis::sendLanHello(IsisInterface *isisIft, int level)
{
    auto *ie = ift->getInterfaceById(isisIft->interfaceId);

    auto hello = makeShared<IsisLanHelloPacket>();
    hello->setPduType(level == 1 ? LAN_L1_HELLO : LAN_L2_HELLO);
    hello->setCircuitType(isisIft->circuitType);
    hello->setSourceId(systemId);
    hello->setHoldTime((uint16_t)std::ceil((isisIft->helloInterval * isisIft->holdMultiplier).dbl()));
    hello->setPriority(isisIft->priority);
    hello->setLanId(isisIft->disValid ? isisIft->disPseudonodeId : PseudonodeId(systemId, 0));

    hello->appendAreaAddresses(areaId);
    if (rt)
        hello->appendProtocolsSupported(NLPID_IPV4);
    if (rt6)
        hello->appendProtocolsSupported(NLPID_IPV6);

    if (rt) {
        auto ipv4Data = ie->findProtocolData<Ipv4InterfaceData>();
        if (ipv4Data != nullptr && !ipv4Data->getIPAddress().isUnspecified())
            hello->appendIpInterfaceAddresses(ipv4Data->getIPAddress());
    }
    if (rt6) {
        // Advertise the interface's global IPv6 address as the IPv6 next hop
        // (directly resolvable via the on-link prefix, like the IPv4 next hop).
        auto ipv6Data = ie->findProtocolData<Ipv6InterfaceData>();
        if (ipv6Data != nullptr) {
            for (int j = 0; j < ipv6Data->getNumAddresses(); j++) {
                Ipv6Address a = ipv6Data->getAddress(j);
                if (!a.isLinkLocal() && !a.isUnspecified()) {
                    hello->appendIpv6InterfaceAddresses(a);
                    break;
                }
            }
        }
    }

    // IS Neighbours TLV: SNPAs of the neighbours already heard on this circuit.
    for (auto *adj : adjacencies)
        if (adj->interfaceId == isisIft->interfaceId && adj->level == level && adj->state != ISIS_ADJ_DOWN)
            hello->appendLanNeighbours(adj->snpa);

    // On-wire length (matches IsisLanHelloSerializer): header + fixed fields +
    // one count byte per TLV array followed by its entries.
    int len = 8 + 1 + 6 + 2 + 1 + 7
            + 1 + (int)hello->getAreaAddressesArraySize() * 3
            + 1 + (int)hello->getProtocolsSupportedArraySize()
            + 1 + (int)hello->getIpInterfaceAddressesArraySize() * 4
            + 1 + (int)hello->getIpv6InterfaceAddressesArraySize() * 16
            + 1 + (int)hello->getLanNeighboursArraySize() * 6;
    hello->setChunkLength(B(len));

    auto *packet = new Packet(level == 1 ? "IsisL1Hello" : "IsisL2Hello");
    packet->insertAtBack(hello);
    packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::isis);
    packet->addTag<InterfaceReq>()->setInterfaceId(isisIft->interfaceId);
    packet->addTag<MacAddressReq>()->setDestAddress(level == 1 ? ALL_L1_ISS : ALL_L2_ISS);

    EV_DETAIL << "Sending Level-" << level << " LAN Hello on interface "
              << ie->getInterfaceName() << "\n";
    llcSocket.send(packet);
}

void Isis::processHello(Packet *packet)
{
    int interfaceId = packet->getTag<InterfaceInd>()->getInterfaceId();
    MacAddress srcMac = packet->getTag<MacAddressInd>()->getSrcAddress();

    const auto& pdu = packet->peekAtFront<IsisPdu>();
    int level;
    switch (pdu->getPduType()) {
        case LAN_L1_HELLO: level = 1; break;
        case LAN_L2_HELLO: level = 2; break;
        default:
            EV_WARN << "Ignoring unsupported IS-IS PDU type " << (int)pdu->getPduType() << "\n";
            return;
    }

    auto *isisIft = findInterface(interfaceId);
    if (isisIft == nullptr)
        return; // not an IS-IS interface

    const auto& hello = packet->peekAtFront<IsisLanHelloPacket>();
    if (hello->getSourceId() == systemId)
        return; // our own Hello, ignore

    auto *adj = findAdjacency(interfaceId, level, hello->getSourceId());
    if (adj == nullptr) {
        adj = new IsisAdjacency();
        adj->interfaceId = interfaceId;
        adj->level = level;
        adj->neighbourSystemId = hello->getSourceId();
        adjacencies.push_back(adj);
        EV_INFO << "New Level-" << level << " neighbour " << hello->getSourceId()
                << " heard on interface " << ift->getInterfaceById(interfaceId)->getInterfaceName() << "\n";
    }
    adj->snpa = srcMac;
    adj->priority = hello->getPriority();
    adj->lanId = hello->getLanId();
    if (hello->getAreaAddressesArraySize() > 0)
        adj->neighbourAreaId = hello->getAreaAddresses(0);
    if (hello->getIpInterfaceAddressesArraySize() > 0)
        adj->neighbourIpAddress = hello->getIpInterfaceAddresses(0);
    if (hello->getIpv6InterfaceAddressesArraySize() > 0)
        adj->neighbourIpv6Address = hello->getIpv6InterfaceAddresses(0);

    // Two-way check: are we listed in the neighbour's IS Neighbours TLV?
    MacAddress myMac = ift->getInterfaceById(interfaceId)->getMacAddress();
    bool weAreListed = false;
    for (size_t k = 0; k < hello->getLanNeighboursArraySize(); k++)
        if (hello->getLanNeighbours(k) == myMac) {
            weAreListed = true;
            break;
        }
    setAdjacencyState(adj, weAreListed ? ISIS_ADJ_UP : ISIS_ADJ_INITIALIZING);

    // (Re)arm the hold timer.
    if (adj->holdTimer == nullptr) {
        adj->holdTimer = new cMessage("holdTimer", HOLD_TIMER);
        adj->holdTimer->setContextPointer(adj);
    }
    rescheduleAfter(hello->getHoldTime(), adj->holdTimer);

    runDisElection(isisIft);
}

void Isis::handleHoldTimer(IsisAdjacency *adj)
{
    EV_INFO << "Hold time expired for Level-" << adj->level << " neighbour "
            << adj->neighbourSystemId << "; tearing down adjacency\n";
    setAdjacencyState(adj, ISIS_ADJ_DOWN);
    adjacencies.erase(std::remove(adjacencies.begin(), adjacencies.end(), adj), adjacencies.end());
    int interfaceId = adj->interfaceId;
    cancelAndDelete(adj->holdTimer);
    delete adj;
    if (auto *isisIft = findInterface(interfaceId))
        runDisElection(isisIft);
}

void Isis::runDisElection(IsisInterface *isisIft)
{
    if (!isisIft->broadcast)
        return; // DIS election applies to LAN circuits only

    // Highest priority wins; ties are broken by the highest SNPA (MAC address).
    int bestPriority = isisIft->priority;
    MacAddress bestSnpa = ift->getInterfaceById(isisIft->interfaceId)->getMacAddress();
    IsisAdjacency *bestAdj = nullptr; // null => this router is the DIS

    for (auto *adj : adjacencies) {
        if (adj->interfaceId != isisIft->interfaceId || adj->level != 1 || adj->state != ISIS_ADJ_UP)
            continue;
        if (adj->priority > bestPriority || (adj->priority == bestPriority && adj->snpa > bestSnpa)) {
            bestPriority = adj->priority;
            bestSnpa = adj->snpa;
            bestAdj = adj;
        }
    }

    bool wasDis = isisIft->isDis;
    bool oldValid = isisIft->disValid;
    PseudonodeId oldDis = isisIft->disPseudonodeId;

    if (bestAdj == nullptr) {
        isisIft->isDis = true;
        isisIft->disPseudonodeId = PseudonodeId(systemId, isisIft->localCircuitId);
    }
    else {
        isisIft->isDis = false;
        isisIft->disPseudonodeId = bestAdj->lanId; // the DIS advertises its own pseudonode
    }
    isisIft->disValid = true;

    if (isisIft->isDis != wasDis || !oldValid || !(isisIft->disPseudonodeId == oldDis)) {
        EV_INFO << "DIS on " << ift->getInterfaceById(isisIft->interfaceId)->getInterfaceName()
                << " is " << (isisIft->isDis ? "this router" : "a neighbour")
                << ", pseudonode " << isisIft->disPseudonodeId << "\n";
        scheduleLspRegeneration();
    }
}

void Isis::scheduleLspRegeneration()
{
    if (regenerateLspTimer == nullptr)
        regenerateLspTimer = new cMessage("regenerateLspTimer", REGENERATE_LSP_TIMER);
    if (!regenerateLspTimer->isScheduled())
        scheduleAfter(0.1, regenerateLspTimer);
}

void Isis::installAndFloodOwnLsp(const Ptr<IsisLspPacket>& lsp)
{
    LspId lspId = lsp->getLspId();
    uint64_t key = lspId.toInt();
    auto it = lspDatabase.find(key);
    uint32_t seq = (it != lspDatabase.end()) ? it->second->sequenceNumber + 1 : 1;
    int lifetime = (int)par("lspMaxLifetime").doubleValue();
    lsp->setSequenceNumber(seq);
    lsp->setRemainingLifetime((uint16_t)lifetime);

    // On-wire length (matches IsisLspSerializer).
    int len = 8 + 2 + 8 + 4 + 2 + 1
            + 1 + (int)lsp->getAreaAddressesArraySize() * 3
            + 1 + (int)lsp->getProtocolsSupportedArraySize()
            + 1 + (int)lsp->getIsReachabilitiesArraySize() * 11
            + 1 + (int)lsp->getIpInternalReachabilitiesArraySize() * 12
            + 1 + (int)lsp->getIpExternalReachabilitiesArraySize() * 12
            + 1 + (int)lsp->getIpv6ReachabilitiesArraySize() * 21;
    lsp->setChunkLength(B(len));

    Ptr<const IsisLspPacket> constLsp = lsp;
    IsisLsp *&entry = lspDatabase[key];
    if (entry == nullptr)
        entry = new IsisLsp();
    entry->lspId = lspId;
    entry->sequenceNumber = seq;
    entry->expiryTime = simTime() + lifetime;
    entry->lsp = constLsp;

    EV_INFO << "Originated LSP " << lspId << " seq " << seq
            << " (" << lsp->getIsReachabilitiesArraySize() << " IS, "
            << lsp->getIpInternalReachabilitiesArraySize() << " IP reachabilities)\n";
    floodLsp(constLsp, -1);
}

void Isis::originateLsp()
{
    if (!(isType & L1_TYPE))
        return; // only Level-1 LSPs are generated so far

    // This router's own (non-pseudonode) LSP.
    auto lsp = makeShared<IsisLspPacket>();
    lsp->setPduType(L1_LSP);
    lsp->setLspId(LspId(PseudonodeId(systemId, 0)));
    lsp->setLspFlags(0x01); // Level-1 IS
    lsp->appendAreaAddresses(areaId);
    if (rt)
        lsp->appendProtocolsSupported(NLPID_IPV4);
    if (rt6)
        lsp->appendProtocolsSupported(NLPID_IPV6);

    // IS reachability: to the pseudonode of each LAN circuit with an elected DIS
    // (the pseudonode LSP in turn lists every IS on that LAN).
    for (auto *isisIft : isisInterfaces) {
        if (!isisIft->broadcast || !isisIft->disValid)
            continue;
        if (!hasUpAdjacency(isisIft->interfaceId, 1))
            continue;
        IsisIsReachability reach;
        reach.metric = isisIft->metric;
        reach.neighbourId = isisIft->disPseudonodeId;
        lsp->appendIsReachabilities(reach);
    }

    // IP reachability: every directly-connected prefix (Integrated IS-IS),
    // including subnets on non-IS-IS (passive) interfaces.
    if (rt) {
        for (int i = 0; i < ift->getNumInterfaces(); i++) {
            auto *ie = ift->getInterface(i);
            if (ie->isLoopback())
                continue;
            auto ipv4Data = ie->findProtocolData<Ipv4InterfaceData>();
            if (ipv4Data == nullptr || ipv4Data->getIPAddress().isUnspecified())
                continue;
            auto *isisIft = findInterface(ie->getInterfaceId());
            IsisIpReachability ipReach;
            ipReach.metric = isisIft ? isisIft->metric : 10;
            ipReach.address = ipv4Data->getIPAddress().doAnd(ipv4Data->getNetmask());
            ipReach.mask = ipv4Data->getNetmask();
            lsp->appendIpInternalReachabilities(ipReach);
        }
    }
    if (rt6) {
        for (int i = 0; i < ift->getNumInterfaces(); i++) {
            auto *ie = ift->getInterface(i);
            if (ie->isLoopback())
                continue;
            auto ipv6Data = ie->findProtocolData<Ipv6InterfaceData>();
            if (ipv6Data == nullptr)
                continue;
            auto *isisIft = findInterface(ie->getInterfaceId());
            for (int j = 0; j < ipv6Data->getNumAddresses(); j++) {
                Ipv6Address a = ipv6Data->getAddress(j);
                if (a.isLinkLocal() || a.isUnspecified())
                    continue;
                IsisIpv6Reachability r;
                r.metric = isisIft ? isisIft->metric : 10;
                r.address = a.getPrefix(64);
                r.prefixLength = 64;
                lsp->appendIpv6Reachabilities(r);
            }
        }
    }
    installAndFloodOwnLsp(lsp);

    // Pseudonode LSPs for the LAN circuits on which this router is the DIS.
    for (auto *isisIft : isisInterfaces)
        if (isisIft->broadcast && isisIft->isDis)
            originatePseudonodeLsp(isisIft);

    scheduleSpf();
}

void Isis::originatePseudonodeLsp(IsisInterface *isisIft)
{
    auto lsp = makeShared<IsisLspPacket>();
    lsp->setPduType(L1_LSP);
    lsp->setLspId(LspId(PseudonodeId(systemId, isisIft->localCircuitId)));
    lsp->setLspFlags(0x01);
    lsp->appendAreaAddresses(areaId);
    lsp->appendProtocolsSupported(NLPID_IPV4);

    // The pseudonode reaches the DIS itself and every IS on the LAN, metric 0.
    IsisIsReachability self;
    self.metric = 0;
    self.neighbourId = PseudonodeId(systemId, 0);
    lsp->appendIsReachabilities(self);
    for (auto *adj : adjacencies) {
        if (adj->interfaceId != isisIft->interfaceId || adj->level != 1 || adj->state != ISIS_ADJ_UP)
            continue;
        IsisIsReachability reach;
        reach.metric = 0;
        reach.neighbourId = PseudonodeId(adj->neighbourSystemId, 0);
        lsp->appendIsReachabilities(reach);
    }
    installAndFloodOwnLsp(lsp);
}

bool Isis::hasUpAdjacency(int interfaceId, int level)
{
    for (auto *adj : adjacencies)
        if (adj->interfaceId == interfaceId && adj->level == level && adj->state == ISIS_ADJ_UP)
            return true;
    return false;
}

void Isis::floodLsp(const Ptr<const IsisLspPacket>& lsp, int exceptInterfaceId)
{
    for (auto *isisIft : isisInterfaces) {
        if (isisIft->interfaceId == exceptInterfaceId)
            continue;
        if (!hasUpAdjacency(isisIft->interfaceId, 1))
            continue;
        auto *packet = new Packet("IsisL1Lsp");
        packet->insertAtBack(lsp);
        packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::isis);
        packet->addTag<InterfaceReq>()->setInterfaceId(isisIft->interfaceId);
        packet->addTag<MacAddressReq>()->setDestAddress(ALL_L1_ISS);
        llcSocket.send(packet);
    }
}

void Isis::sendLspOnInterface(const Ptr<const IsisLspPacket>& lsp, int interfaceId)
{
    auto *packet = new Packet("IsisL1Lsp");
    packet->insertAtBack(lsp);
    packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::isis);
    packet->addTag<InterfaceReq>()->setInterfaceId(interfaceId);
    packet->addTag<MacAddressReq>()->setDestAddress(ALL_L1_ISS);
    llcSocket.send(packet);
}

void Isis::sendDatabaseToInterface(int interfaceId)
{
    // Send every known LSP out one interface; used to synchronize the database
    // with a neighbour when an adjacency comes up (so a late-forming adjacency
    // still receives LSPs that were flooded before it existed).
    for (auto& kv : lspDatabase)
        sendLspOnInterface(kv.second->lsp, interfaceId);
}

void Isis::sendCsnp(IsisInterface *isisIft)
{
    // A Complete Sequence Numbers PDU summarizes the whole database. On a LAN it
    // is sent periodically by the DIS so the other ISs can detect missing or
    // stale LSPs.
    auto csnp = makeShared<IsisCsnpPacket>();
    csnp->setPduType(L1_CSNP);
    csnp->setSourceId(PseudonodeId(systemId, isisIft->localCircuitId));
    LspId start, end;
    end.setMax();
    csnp->setStartLspId(start);
    csnp->setEndLspId(end);
    for (auto& kv : lspDatabase) {
        IsisLspEntry e;
        e.remainingLifetime = (uint16_t)std::max(0.0, (kv.second->expiryTime - simTime()).dbl());
        e.lspId = kv.second->lspId;
        e.sequenceNumber = kv.second->sequenceNumber;
        e.checksum = 0;
        csnp->appendLspEntries(e);
    }
    csnp->setChunkLength(B(8 + 7 + 8 + 8 + 1 + (int)csnp->getLspEntriesArraySize() * 16));

    auto *packet = new Packet("IsisL1Csnp");
    packet->insertAtBack(csnp);
    packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::isis);
    packet->addTag<InterfaceReq>()->setInterfaceId(isisIft->interfaceId);
    packet->addTag<MacAddressReq>()->setDestAddress(ALL_L1_ISS);
    EV_DETAIL << "Sending CSNP on " << ift->getInterfaceById(isisIft->interfaceId)->getInterfaceName()
              << " (" << csnp->getLspEntriesArraySize() << " LSP entries)\n";
    llcSocket.send(packet);
}

void Isis::processCsnp(Packet *packet)
{
    int interfaceId = packet->getTag<InterfaceInd>()->getInterfaceId();
    const auto& csnp = packet->peekAtFront<IsisCsnpPacket>();
    if (csnp->getPduType() != L1_CSNP)
        return;

    std::set<uint64_t> reported;
    auto requests = makeShared<IsisPsnpPacket>();
    requests->setPduType(L1_PSNP);
    requests->setSourceId(PseudonodeId(systemId, 0));

    for (size_t i = 0; i < csnp->getLspEntriesArraySize(); i++) {
        const auto& e = csnp->getLspEntries(i);
        uint64_t key = e.lspId.toInt();
        reported.insert(key);
        auto it = lspDatabase.find(key);
        if (it == lspDatabase.end() || it->second->sequenceNumber < e.sequenceNumber) {
            // we lack it or have an older copy -> request it
            IsisLspEntry req;
            req.lspId = e.lspId;
            req.sequenceNumber = (it != lspDatabase.end()) ? it->second->sequenceNumber : 0;
            req.remainingLifetime = 0;
            req.checksum = 0;
            requests->appendLspEntries(req);
        }
        else if (it->second->sequenceNumber > e.sequenceNumber) {
            // our copy is newer -> send it back
            sendLspOnInterface(it->second->lsp, interfaceId);
        }
    }
    // LSPs we hold that the CSNP did not list -> the sender is missing them.
    for (auto& kv : lspDatabase)
        if (!reported.count(kv.first))
            sendLspOnInterface(kv.second->lsp, interfaceId);

    if (requests->getLspEntriesArraySize() > 0) {
        requests->setChunkLength(B(8 + 7 + 1 + (int)requests->getLspEntriesArraySize() * 16));
        auto *p = new Packet("IsisL1Psnp");
        p->insertAtBack(requests);
        p->addTag<PacketProtocolTag>()->setProtocol(&Protocol::isis);
        p->addTag<InterfaceReq>()->setInterfaceId(interfaceId);
        p->addTag<MacAddressReq>()->setDestAddress(ALL_L1_ISS);
        llcSocket.send(p);
    }
}

void Isis::processPsnp(Packet *packet)
{
    int interfaceId = packet->getTag<InterfaceInd>()->getInterfaceId();
    const auto& psnp = packet->peekAtFront<IsisPsnpPacket>();
    if (psnp->getPduType() != L1_PSNP)
        return;
    // Each entry requests an LSP newer than the reported sequence number; send
    // ours if we have a newer copy.
    for (size_t i = 0; i < psnp->getLspEntriesArraySize(); i++) {
        const auto& e = psnp->getLspEntries(i);
        auto it = lspDatabase.find(e.lspId.toInt());
        if (it != lspDatabase.end() && it->second->sequenceNumber > e.sequenceNumber)
            sendLspOnInterface(it->second->lsp, interfaceId);
    }
}

void Isis::processLsp(Packet *packet)
{
    int interfaceId = packet->getTag<InterfaceInd>()->getInterfaceId();
    const auto& lsp = packet->peekAtFront<IsisLspPacket>();
    if (lsp->getPduType() != L1_LSP)
        return; // only Level-1 so far

    LspId lspId = lsp->getLspId();
    uint64_t key = lspId.toInt();
    uint32_t seq = lsp->getSequenceNumber();
    auto it = lspDatabase.find(key);

    if (lsp->getRemainingLifetime() == 0) {
        // A purge: drop the LSP if we hold it and the purge is not stale.
        if (it != lspDatabase.end() && seq >= it->second->sequenceNumber) {
            EV_INFO << "Purging LSP " << lspId << " (received purge)\n";
            delete it->second;
            lspDatabase.erase(it);
            floodLsp(lsp, interfaceId); // forward the purge
            scheduleSpf();
        }
        return;
    }

    if (it == lspDatabase.end() || seq > it->second->sequenceNumber) {
        IsisLsp *entry = (it != lspDatabase.end()) ? it->second : (lspDatabase[key] = new IsisLsp());
        entry->lspId = lspId;
        entry->sequenceNumber = seq;
        entry->expiryTime = simTime() + lsp->getRemainingLifetime();
        entry->lsp = lsp;
        EV_INFO << "Installed LSP " << lspId << " seq " << seq << " from interface "
                << ift->getInterfaceById(interfaceId)->getInterfaceName()
                << " (" << lspDatabase.size() << " LSPs in database)\n";
        floodLsp(lsp, interfaceId); // forward to the other interfaces
        scheduleSpf();
    }
    else if (seq < it->second->sequenceNumber) {
        // Our copy is newer: send it back out the arrival interface.
        auto *p = new Packet("IsisL1Lsp");
        p->insertAtBack(it->second->lsp);
        p->addTag<PacketProtocolTag>()->setProtocol(&Protocol::isis);
        p->addTag<InterfaceReq>()->setInterfaceId(interfaceId);
        p->addTag<MacAddressReq>()->setDestAddress(ALL_L1_ISS);
        llcSocket.send(p);
    }
    else {
        // Same sequence number: still alive, refresh our copy's expiry.
        it->second->expiryTime = simTime() + lsp->getRemainingLifetime();
    }
}

void Isis::ageLsps()
{
    std::vector<IsisLsp *> expired;
    for (auto& kv : lspDatabase)
        if (simTime() >= kv.second->expiryTime)
            expired.push_back(kv.second);
    for (auto *entry : expired) {
        EV_INFO << "LSP " << entry->lspId << " expired; purging\n";
        purgeLsp(entry, -1);
    }
    if (!expired.empty())
        scheduleSpf();
}

void Isis::purgeLsp(IsisLsp *entry, int exceptInterfaceId)
{
    // Flood a zero-lifetime purge so neighbours also drop the LSP.
    auto purge = makeShared<IsisLspPacket>();
    purge->setPduType(L1_LSP);
    purge->setLspId(entry->lspId);
    purge->setSequenceNumber(entry->sequenceNumber);
    purge->setRemainingLifetime(0);
    // empty LSP: header + fixed fields + 6 empty TLV count bytes (matches serializer)
    purge->setChunkLength(B(8 + 2 + 8 + 4 + 2 + 1 + 6));
    floodLsp(purge, exceptInterfaceId);

    lspDatabase.erase(entry->lspId.toInt());
    delete entry;
}

IsisLsp *Isis::lookupLsp(uint64_t nodeKey)
{
    // nodeKey is a PseudonodeId::toInt(); the (fragment-0) LSP key is nodeKey << 8.
    auto it = lspDatabase.find(nodeKey << 8);
    return it != lspDatabase.end() ? it->second : nullptr;
}

IsisAdjacency *Isis::findUpAdjacencyTo(uint64_t sysId)
{
    for (auto *adj : adjacencies)
        if (adj->state == ISIS_ADJ_UP && adj->neighbourSystemId.getSystemId() == sysId)
            return adj;
    return nullptr;
}

void Isis::scheduleSpf()
{
    if (spfTimer == nullptr)
        spfTimer = new cMessage("spfTimer", SPF_TIMER);
    if (!spfTimer->isScheduled())
        scheduleAfter(0.2, spfTimer);
}

void Isis::removeIsisRoutes()
{
    if (rt)
        for (auto *route : isisRoutes)
            rt->deleteRoute(route);
    isisRoutes.clear();
    if (rt6)
        for (auto *route : isisRoutes6)
            rt6->deleteRoute(route);
    isisRoutes6.clear();
}

void Isis::runSpf()
{
    if (!rt && !rt6)
        return;

    // A node in the shortest-path computation.
    struct SpfNode {
        uint32_t distance = 0;
        int firstHopInterfaceId = -1;
        Ipv4Address nextHop;
        Ipv6Address nextHopV6;
    };

    // Nodes are keyed by PseudonodeId::toInt(), so both real ISs (circuit id 0)
    // and LAN pseudonodes (circuit id != 0) participate in the graph.
    uint64_t self = PseudonodeId(systemId, 0).toInt();
    std::map<uint64_t, SpfNode> paths;     // finalized shortest paths
    std::map<uint64_t, SpfNode> tentative;
    tentative[self] = SpfNode();

    // Dijkstra over the Level-1 link-state database.
    while (!tentative.empty()) {
        auto best = tentative.begin();
        for (auto it = tentative.begin(); it != tentative.end(); ++it)
            if (it->second.distance < best->second.distance)
                best = it;
        uint64_t u = best->first;
        SpfNode uNode = best->second;
        tentative.erase(best);
        paths[u] = uNode;

        IsisLsp *lspEntry = lookupLsp(u);
        if (lspEntry == nullptr)
            continue;
        const auto& lsp = lspEntry->lsp;
        for (size_t k = 0; k < lsp->getIsReachabilitiesArraySize(); k++) {
            const auto& reach = lsp->getIsReachabilities(k);
            uint64_t v = reach.neighbourId.toInt();
            if (paths.count(v))
                continue;
            SpfNode vNode;
            vNode.distance = uNode.distance + reach.metric;
            if (uNode.firstHopInterfaceId != -1) {
                // first hop already resolved earlier on the path
                vNode.firstHopInterfaceId = uNode.firstHopInterfaceId;
                vNode.nextHop = uNode.nextHop;
                vNode.nextHopV6 = uNode.nextHopV6;
            }
            else if ((v & 0xFF) == 0) {
                // v is a real IS one hop away (directly, or across a LAN
                // pseudonode we are on): the first hop is our adjacency to it
                auto *adj = findUpAdjacencyTo(v >> 8);
                if (adj == nullptr)
                    continue;
                vNode.firstHopInterfaceId = adj->interfaceId;
                vNode.nextHop = adj->neighbourIpAddress;
                vNode.nextHopV6 = adj->neighbourIpv6Address;
            }
            // else: v is a pseudonode reached from us -> first hop still unresolved
            auto it = tentative.find(v);
            if (it == tentative.end() || vNode.distance < it->second.distance)
                tentative[v] = vNode;
        }
    }

    // Install IP routes, shortest distance first so duplicates keep the best path.
    removeIsisRoutes();
    std::vector<std::pair<uint32_t, uint64_t>> ordered;
    for (auto& kv : paths)
        if (kv.first != self)
            ordered.push_back({ kv.second.distance, kv.first });
    std::sort(ordered.begin(), ordered.end());

    std::set<std::pair<uint32_t, uint32_t>> installed;
    std::set<std::string> installed6;
    for (auto& [dist, u] : ordered) {
        SpfNode& node = paths[u];
        if (node.firstHopInterfaceId < 0)
            continue;
        IsisLsp *lspEntry = lookupLsp(u);
        if (lspEntry == nullptr)
            continue;
        auto *outIe = ift->getInterfaceById(node.firstHopInterfaceId);
        const auto& lsp = lspEntry->lsp;

        if (rt && !node.nextHop.isUnspecified()) {
            for (size_t k = 0; k < lsp->getIpInternalReachabilitiesArraySize(); k++) {
                const auto& ipr = lsp->getIpInternalReachabilities(k);
                Ipv4Address dest = ipr.address;
                Ipv4Address mask = ipr.mask;
                if (dest.isUnspecified())
                    continue;
                auto key = std::make_pair(dest.getInt(), mask.getInt());
                if (installed.count(key) || isLocalNetwork(ift, dest, mask))
                    continue;
                installed.insert(key);

                auto *route = new Ipv4Route();
                route->setDestination(dest);
                route->setNetmask(mask);
                route->setGateway(node.nextHop);
                route->setInterface(outIe);
                route->setSourceType(IRoute::ISIS);
                route->setAdminDist(Ipv4Route::dISIS);
                route->setMetric(node.distance + ipr.metric);
                rt->addRoute(route);
                isisRoutes.push_back(route);

                EV_INFO << "Installed IS-IS route " << dest << "/" << mask.getNetmaskLength()
                        << " via " << node.nextHop << " dev " << outIe->getInterfaceName()
                        << " metric " << route->getMetric() << "\n";
            }
        }
        if (rt6 && !node.nextHopV6.isUnspecified()) {
            for (size_t k = 0; k < lsp->getIpv6ReachabilitiesArraySize(); k++) {
                const auto& ipr = lsp->getIpv6Reachabilities(k);
                Ipv6Address dest = ipr.address;
                int plen = ipr.prefixLength;
                if (dest.isUnspecified())
                    continue;
                std::string key = dest.str() + "/" + std::to_string(plen);
                if (installed6.count(key) || isLocalNetwork6(ift, dest, plen))
                    continue;
                installed6.insert(key);

                auto *route = new Ipv6Route(dest, plen, IRoute::ISIS);
                route->setNextHop(node.nextHopV6);
                route->setInterface(outIe);
                route->setAdminDist(Ipv6Route::dISIS);
                route->setMetric(node.distance + ipr.metric);
                rt6->addRoute(route);
                isisRoutes6.push_back(route);

                EV_INFO << "Installed IS-IS route " << dest << "/" << plen
                        << " via " << node.nextHopV6 << " dev " << outIe->getInterfaceName()
                        << " metric " << route->getMetric() << "\n";
            }
        }
    }
}

void Isis::setAdjacencyState(IsisAdjacency *adj, IsisAdjacencyState newState)
{
    IsisAdjacencyState oldState = adj->state;
    if (oldState == newState)
        return;
    adj->state = newState;
    const char *stateName = newState == ISIS_ADJ_UP ? "UP"
                          : newState == ISIS_ADJ_INITIALIZING ? "INITIALIZING" : "DOWN";
    EV_INFO << "Level-" << adj->level << " adjacency to " << adj->neighbourSystemId
            << " is now " << stateName << "\n";
    emit(adjacencyChangedSignal, (long)newState);

    // Entering or leaving the UP state changes our advertised reachability, so
    // (re)originate our LSP.
    if (newState == ISIS_ADJ_UP || oldState == ISIS_ADJ_UP)
        scheduleLspRegeneration();

    // Synchronize our whole database with a newly established neighbour.
    if (newState == ISIS_ADJ_UP)
        sendDatabaseToInterface(adj->interfaceId);
}

IsisInterface *Isis::findInterface(int interfaceId)
{
    for (auto *isisIft : isisInterfaces)
        if (isisIft->interfaceId == interfaceId)
            return isisIft;
    return nullptr;
}

IsisAdjacency *Isis::findAdjacency(int interfaceId, int level, const SystemId& sysId)
{
    for (auto *adj : adjacencies)
        if (adj->interfaceId == interfaceId && adj->level == level && adj->neighbourSystemId == sysId)
            return adj;
    return nullptr;
}

void Isis::clearState()
{
    for (auto *isisIft : isisInterfaces) {
        cancelAndDelete(isisIft->helloTimer);
        cancelAndDelete(isisIft->csnpTimer);
        delete isisIft;
    }
    isisInterfaces.clear();
    for (auto *adj : adjacencies) {
        cancelAndDelete(adj->holdTimer);
        delete adj;
    }
    adjacencies.clear();
    for (auto& kv : lspDatabase)
        delete kv.second;
    lspDatabase.clear();
    if (regenerateLspTimer != nullptr) {
        cancelAndDelete(regenerateLspTimer);
        regenerateLspTimer = nullptr;
    }
    if (refreshTimer != nullptr) {
        cancelAndDelete(refreshTimer);
        refreshTimer = nullptr;
    }
    if (ageTimer != nullptr) {
        cancelAndDelete(ageTimer);
        ageTimer = nullptr;
    }
    if (spfTimer != nullptr) {
        cancelAndDelete(spfTimer);
        spfTimer = nullptr;
    }
    isisRoutes.clear();
    isisRoutes6.clear();
}

} // namespace isis
} // namespace inet

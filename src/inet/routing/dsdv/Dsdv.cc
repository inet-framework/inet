// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/routing/dsdv/Dsdv.h"

#include <algorithm>
#include <memory>

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/routing/dsdv/DsdvProtocol.h"
#include "inet/transportlayer/common/L4PortTag_m.h"

namespace inet {
namespace dsdv {

Define_Module(Dsdv);

Dsdv::~Dsdv()
{
    cancelAndDelete(periodicTimer);
    cancelAndDelete(advertisementTimer);
    cancelAndDelete(expiryTimer);
}

void Dsdv::initialize(int stage)
{
    RoutingProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        interfaceTable.reference(this, "interfaceTableModule", true);
        routingTable.reference(this, "routingTableModule", true);
        controlPort = par("controlPort");
        period = par("fullDumpInterval");
        jitter = par("periodicJitter");
        if (controlPort < 1024 || controlPort > UINT16_MAX || period <= SIMTIME_ZERO || jitter < SIMTIME_ZERO ||
                par("neighborTimeout").doubleValue() <= 0 || par("routeTimeout").doubleValue() <= 0 ||
                par("settlingCap").doubleValue() <= 0)
            throw cRuntimeError("Invalid DSDV control port or timer parameters");
        periodicTimer = new cMessage("periodicTimer");
        advertisementTimer = new cMessage("advertisementTimer");
        expiryTimer = new cMessage("expiryTimer");
        // Incoming delivery wins ties; expiry precedes either form of sending.
        expiryTimer->setSchedulingPriority(10);
        advertisementTimer->setSchedulingPriority(20);
        periodicTimer->setSchedulingPriority(30);
        socket.setOutputGate(gate("socketOut"));
        socket.setCallback(this);
        WATCH(active);
        WATCH(startupDump);
        WATCH(capacity);
        WATCH_EXPR("selfAddress", active ? table.getSelf().current.destination : Ipv4Address::UNSPECIFIED_ADDRESS);
        WATCH_EXPR("selfSequence", active ? table.getSelf().current.sequence : uint64_t(0));
        WATCH_EXPR("numDestinations", table.getRouteEntries().size());
        WATCH_EXPR("numFiniteRoutes", std::count_if(table.getRouteEntries().begin(), table.getRouteEntries().end(),
                [this](const auto& item) {
                    return active && item.first != table.getSelf().current.destination && DsdvTable::isFinite(item.second.current);
                }));
        WATCH_EXPR("numNeighbors", table.getNeighbors().size());
        WATCH_EXPR("numPendingAdvertisements", table.getMustSend().size());
        WATCH_EXPR("routeEntries", table.getRouteEntries());
        WATCH_EXPR("neighbors", table.getNeighbors());
        WATCH_EXPR("fullDumpBaseline", table.getBaseline());
        WATCH_EXPR("mustSend", table.getMustSend());
        WATCH_EXPR("nextPeriodicDump", periodicTimer->isScheduled() ? periodicTimer->getArrivalTime() : SimTime::getMaxTime());
        WATCH_EXPR("nextAdvertisement", advertisementTimer->isScheduled() ? advertisementTimer->getArrivalTime() : SimTime::getMaxTime());
        WATCH_EXPR("nextExpiry", expiryTimer->isScheduled() ? expiryTimer->getArrivalTime() : SimTime::getMaxTime());
        auto group = ProtocolGroup::getUdpProtocolGroup();
        auto existing = group->findProtocol(controlPort);
        if (existing && existing != &dsdvProtocol)
            throw cRuntimeError("DSDV control port already belongs to another protocol");
        if (!existing)
            group->addProtocol(controlPort, &dsdvProtocol);
    }
    else if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        if (routingTable->getRouterIdAsGeneric().getType() != L3Address::IPv4)
            throw cRuntimeError("DSDV supports only an IPv4 routing table");
        routingInterface = interfaceTable->findInterfaceByName(par("interface"));
        if (!routingInterface || routingInterface->isLoopback() || !routingInterface->isBroadcast())
            throw cRuntimeError("DSDV requires one configured broadcast-capable ad hoc interface");
        for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
            auto interface = interfaceTable->getInterface(i);
            if (interface != routingInterface && !interface->isLoopback() && interface->isUp())
                throw cRuntimeError("DSDV supports only one active non-loopback interface");
        }
        getContainingNode(this)->subscribe(interfaceStateChangedSignal, this);
        getContainingNode(this)->subscribe(interfaceConfigChangedSignal, this);
        getContainingNode(this)->subscribe(interfaceIpv4ConfigChangedSignal, this);
        if (isUp() && interfaceUsable())
            start();
    }
}

bool Dsdv::interfaceUsable() const
{
    if (!routingInterface || !routingInterface->isUp() || !routingInterface->hasCarrier())
        return false;
    auto data = routingInterface->findProtocolData<Ipv4InterfaceData>();
    return data && DsdvTable::isHost(data->getIPAddress());
}

void Dsdv::start()
{
    if (active || !interfaceUsable())
        return;
    auto address = routingInterface->getProtocolData<Ipv4InterfaceData>()->getIPAddress();
    table.configure(address, par("settlingCap"), par("neighborTimeout"), par("routeTimeout"));
    int budget = std::min(par("payloadBudget").intValue(), int64_t(routingInterface->getMtu() - 28));
    if (budget < DsdvTable::HEADER_BYTES + DsdvTable::ENTRY_BYTES)
        throw cRuntimeError("DSDV payload budget/MTU cannot hold one entry plus IPv4/UDP overhead");
    capacity = std::min((budget - DsdvTable::HEADER_BYTES) / DsdvTable::ENTRY_BYTES, int(UINT16_MAX));
    table.start(simTime());
    socket.bind(controlPort);
    socket.setBroadcast(true);
    socket.setTimeToLive(1);
    active = true;
    startupDump = true;
    synchronizeRoutes();
    scheduleAt(simTime(), advertisementTimer);
    scheduleAt(simTime() + SimTime(uniform(0, period.dbl())), periodicTimer);
}

void Dsdv::removeOwnedRoutes()
{
    // Never retain borrowed route pointers across routing-table cleanup.
    for (int i = routingTable->getNumRoutes() - 1; i >= 0; i--) {
        auto route = routingTable->getRoute(i);
        if (route->getSource() == this) {
            routingTable->deleteRoute(route);
            emit(registerSignal("routeWithdrawn"), 1L);
        }
    }
}

void Dsdv::suspend(bool crash)
{
    cancelEvent(periodicTimer);
    cancelEvent(advertisementTimer);
    cancelEvent(expiryTimer);
    if (active) {
        active = false;
        table.suspend(simTime());
        if (crash)
            socket.destroy();
        else
            socket.close();
    }
    startupDump = false;
    removeOwnedRoutes();
}

void Dsdv::handleStartOperation(LifecycleOperation *)
{
    start();
}

void Dsdv::handleStopOperation(LifecycleOperation *)
{
    suspend(false);
}

void Dsdv::handleCrashOperation(LifecycleOperation *)
{
    suspend(true);
}

void Dsdv::scheduleTimer(cMessage *timer, simtime_t when)
{
    cancelEvent(timer);
    if (when != SimTime::getMaxTime())
        scheduleAt(when, timer);
}

void Dsdv::scheduleTimers()
{
    scheduleTimer(expiryTimer, table.nextExpiry());
    scheduleTimer(advertisementTimer, startupDump ? simTime() : table.nextAdvertisement(simTime()));
}

void Dsdv::handleMessageWhenUp(cMessage *message)
{
    if (!message->isSelfMessage()) {
        if (active && socket.belongsToSocket(message))
            socket.processMessage(message);
        else
            delete message;
        return;
    }
    if (!active)
        throw cRuntimeError("DSDV timer survived suspension");
    if (message == expiryTimer) {
        for (const auto& destination : table.expire(simTime())) {
            EV_INFO << "Expired DSDV route to " << destination << endl;
            emit(registerSignal("invalidationReason"), "expiry");
        }
        synchronizeRoutes();
    }
    else if (message == periodicTimer) {
        table.advanceSelf();
        sendAdvertisement(true);
        scheduleAt(simTime() + period + SimTime(uniform(0, jitter.dbl())), periodicTimer);
    }
    else if (message == advertisementTimer) {
        bool full = startupDump;
        startupDump = false;
        if (!full)
            emit(registerSignal("triggeredSend"), 1L);
        sendAdvertisement(full);
    }
    else
        throw cRuntimeError("Unknown DSDV timer");
    scheduleTimers();
}

void Dsdv::sendAdvertisement(bool full)
{
    auto advertisement = table.prepare(full, capacity, simTime());
    for (auto delay : advertisement.settlingDelays)
        emit(registerSignal("settlingDelay"), delay);
    for (const auto& entries : advertisement.datagrams) {
        auto header = makeShared<DsdvHeader>();
        header->setKind(advertisement.full ? 1 : 2);
        header->setSender(table.getSelf().current.destination);
        header->setSelfSequence(table.getSelf().current.sequence);
        header->setEntriesArraySize(entries.size());
        header->setChunkLength(B(DsdvTable::HEADER_BYTES + DsdvTable::ENTRY_BYTES * entries.size()));
        for (size_t i = 0; i < entries.size(); i++)
            header->setEntries(i, entries[i]);
        auto packet = new Packet("DSDV", header);
        packet->addTag<PacketProtocolTag>()->setProtocol(&dsdvProtocol);
        packet->addTag<InterfaceReq>()->setInterfaceId(routingInterface->getInterfaceId());
        packet->addTag<L3AddressReq>()->setSrcAddress(header->getSender());
        packet->addTag<FragmentationReq>()->setDontFragment(true);
        emit(registerSignal("controlBytes"), long(header->getChunkLength().get<B>()));
        emit(registerSignal("controlPacketSent"), 1L);
        socket.sendTo(packet, Ipv4Address::ALLONES_ADDRESS, controlPort);
    }
    if (!advertisement.datagrams.empty())
        emit(registerSignal(advertisement.full ? "fullDumpSent" : "incrementalSent"), 1L);
}

void Dsdv::synchronizeRoutes(const char *withdrawalReason)
{
    auto self = table.getSelf().current.destination;
    for (int i = routingTable->getNumRoutes() - 1; i >= 0; i--) {
        auto route = routingTable->getRoute(i);
        if (route->getSource() != this)
            continue;
        auto it = table.getRouteEntries().find(route->getDestinationAsGeneric().toIpv4());
        if (it == table.getRouteEntries().end() || !DsdvTable::isFinite(it->second.current)) {
            routingTable->deleteRoute(route);
            emit(registerSignal("routeWithdrawn"), 1L);
            if (withdrawalReason)
                emit(registerSignal("invalidationReason"), withdrawalReason);
        }
    }
    for (const auto& item : table.getRouteEntries()) {
        if (item.first == self || !DsdvTable::isFinite(item.second.current))
            continue;
        IRoute *owned = nullptr;
        for (int i = 0; i < routingTable->getNumRoutes(); i++) {
            auto route = routingTable->getRoute(i);
            if (route->getSource() == this && route->getDestinationAsGeneric() == item.first) {
                owned = route;
                break;
            }
        }
        bool changed = !owned || owned->getNextHopAsGeneric() != item.second.successor ||
                owned->getMetric() != int(item.second.current.metric);
        if (!owned) {
            owned = routingTable->createRoute();
            owned->setSource(this);
            owned->setSourceType(IRoute::MANET);
            owned->setDestination(item.first);
            owned->setPrefixLength(32);
            owned->setInterface(routingInterface);
            owned->setNextHop(item.second.successor);
            owned->setMetric(item.second.current.metric);
            routingTable->addRoute(owned);
        }
        else if (changed) {
            owned->setNextHop(item.second.successor);
            owned->setMetric(item.second.current.metric);
        }
        if (changed)
            emit(registerSignal("routeInstalled"), 1L);
    }
}

void Dsdv::socketDataArrived(UdpSocket *, Packet *packet)
{
    std::unique_ptr<Packet> owner(packet);
    auto incoming = packet->findTag<InterfaceInd>();
    auto address = packet->findTag<L3AddressInd>();
    auto port = packet->findTag<L4PortInd>();
    Ptr<const DsdvHeader> header;
    if (packet->getDataLength() >= B(DsdvTable::HEADER_BYTES)) {
        try {
            header = packet->peekAtFront<DsdvHeader>(packet->getDataLength(),
                    Chunk::PF_ALLOW_INCORRECT | Chunk::PF_ALLOW_INCOMPLETE);
        }
        catch (cRuntimeError&) {
            // Unrepresentable/truncated external bytes are a rejected datagram.
        }
    }
    if (!active || !incoming || incoming->getInterfaceId() != routingInterface->getInterfaceId() ||
            !address || !port || port->getSrcPort() != controlPort || !header ||
            !DsdvTable::isValidHeader(*header) || address->getSrcAddress() != header->getSender() ||
            header->getSender() == table.getSelf().current.destination) {
        emit(registerSignal("updateRejected"), 1L);
        return;
    }
    table.receive(*header, simTime());
    emit(registerSignal("updateAccepted"), 1L);
    synchronizeRoutes("remoteWithdrawal");
    scheduleTimers();
}

void Dsdv::socketErrorArrived(UdpSocket *, Indication *indication)
{
    delete indication;
}

void Dsdv::socketClosed(UdpSocket *)
{
}

void Dsdv::receiveSignal(cComponent *, simsignal_t, cObject *value, cObject *)
{
    Enter_Method_Silent();
    auto change = check_and_cast<NetworkInterfaceChangeDetails *>(value);
    if (!isUp())
        return;
    if (change->getNetworkInterface() != routingInterface) {
        auto interface = change->getNetworkInterface();
        if (!interface->isLoopback() && interface->isUp())
            throw cRuntimeError("DSDV supports only one active non-loopback interface");
        return;
    }
    if (interfaceUsable()) {
        if (active && routingInterface->getProtocolData<Ipv4InterfaceData>()->getIPAddress() !=
                table.getSelf().current.destination)
            throw cRuntimeError("DSDV does not support changing its IPv4 identity");
        start();
    }
    else if (active) {
        emit(registerSignal("invalidationReason"), "interfaceDown");
        suspend(false);
    }
}

} // namespace dsdv
} // namespace inet

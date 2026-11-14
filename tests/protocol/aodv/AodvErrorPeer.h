// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __AODV_ERROR_TEST_PEER_H
#define __AODV_ERROR_TEST_PEER_H
#include "AodvPeer.h"
#include "inet/common/ProtocolTag_m.h"

class AodvErrorPeer : public AodvPeer
{
    struct Error {
        L3Address target;
        bool noDelete;
        std::vector<UnreachableNode> nodes;
        simtime_t reception;
        int id;
    };
    struct Snapshot {
        uint32_t sequence;
        bool valid;
        bool active;
        simtime_t expiry;
        L3Address nextHop;
        NetworkInterface *interface;
        int metric;
    };
    std::string mode;
    Snapshot baseline;
    std::vector<Error> errors;
    std::vector<std::pair<L3Address, simtime_t>> arrivals;
    int delivered = 0;
    int drops = 0;
    int requests = 0;
    uint32_t nextId = 1;
    bool dataMode() const { return mode.find("Transit") == 0 || mode == "ForeignForward" || mode == "OwnedForward"; }
    bool prefixMode() const { return mode.find("Prefix") == 0; }
    IRoute *record(const L3Address& address, int prefix = 32, bool foreign = false) {
        for (int i = 0; i < routes->getNumRoutes(); i++) {
            auto route = routes->getRoute(i);
            if (route->getDestinationAsGeneric() == address && route->getPrefixLength() == prefix &&
                    route->getSource() == (foreign ? static_cast<cObject *>(this) : getModuleByPath("^.^.dut.aodv")))
                return route;
        }
        return nullptr;
    }
    IRoute *seed(const L3Address& address, int prefix, uint32_t sequence, bool active, bool foreign = false) {
        auto route = routes->createRoute();
        auto interfaces = check_and_cast<IInterfaceTable *>(getModuleByPath("^.^.dut.interfaceTable"));
        route->setDestination(address);
        route->setPrefixLength(prefix);
        route->setNextHop(peer);
        route->setInterface(interfaces->findInterfaceByName("wlan0"));
        route->setMetric(2);
        route->setSourceType(foreign ? IRoute::MANUAL : IRoute::AODV);
        route->setSource(foreign ? static_cast<cObject *>(this) : getModuleByPath("^.^.dut.aodv"));
        auto data = new AodvRouteData();
        data->setDestSeqNum(sequence);
        data->setHasValidDestNum(true);
        data->setIsActive(active);
        data->setLifeTime(5);
        route->setProtocolData(data);
        routes->addRoute(route);
        return route;
    }
    Snapshot routeSnapshot(IRoute *route) {
        ASSERT(route);
        auto data = check_and_cast<AodvRouteData *>(route->getProtocolData());
        return {data->getDestSeqNum(), data->hasValidDestNum(), data->isActive(), data->getLifeTime(),
            route->getNextHopAsGeneric(), route->getInterface(), route->getMetric()};
    }
    void unchanged(IRoute *route, const Snapshot& before) {
        auto after = routeSnapshot(route);
        ASSERT(after.sequence == before.sequence && after.valid == before.valid && after.active == before.active);
        ASSERT(after.expiry == before.expiry && after.nextHop == before.nextHop && after.interface == before.interface && after.metric == before.metric);
    }
    void ask(int output, const L3Address& target) {
        auto request = makeShared<Rreq>();
        request->setPacketType(RREQ);
        request->setChunkLength(B(24));
        request->setOriginatorAddr(Ipv4Address(output == 1 ? "10.0.0.3" : "10.0.0.4"));
        request->setOriginatorSeqNum(1);
        request->setRreqId(nextId++);
        request->setDestAddr(target);
        request->setUnknownSeqNumFlag(true);
        transmit(request, output);
    }
    void offer(const L3Address& target, uint32_t sequence) {
        auto reply = makeShared<Rrep>();
        reply->setPacketType(RREP);
        reply->setChunkLength(B(20));
        reply->setDestAddr(target);
        reply->setDestSeqNum(sequence);
        reply->setLifeTime(5);
        reply->setHopCount(1);
        reply->setOriginatorAddr(mode.find("NoPrecursor") == 0 ? dut : L3Address(Ipv4Address("10.0.0.3")));
        transmit(reply);
    }
    void transmitError(const std::vector<std::pair<L3Address, uint32_t>>& nodes, bool noDelete = false, int output = 0) {
        auto message = makeShared<Rerr>();
        message->setPacketType(RERR);
        message->setChunkLength(B(4 + 8 * nodes.size()));
        message->setNoDeleteFlag(noDelete);
        message->setUnreachableNodesArraySize(nodes.size());
        for (size_t i = 0; i < nodes.size(); i++) {
            UnreachableNode node;
            node.addr = nodes[i].first;
            node.seqNum = nodes[i].second;
            message->setUnreachableNodes(i, node);
        }
        transmit(message, output);
    }
    void checkSequence(const L3Address& target, uint32_t sequence, bool active) {
        AodvPeer::checkSequence(target, sequence, active, 2);
    }
    void checkRelay(uint32_t sequence, bool noDelete) {
        ASSERT(errors.size() == 1);
        const auto& message = errors.front();
        ASSERT(message.target == Ipv4Address("10.0.0.3") && message.noDelete == noDelete);
        ASSERT(message.nodes.size() == 1 && message.nodes.front().addr == destination && message.nodes.front().seqNum == sequence);
    }
    simtime_t lastArrival(const L3Address& target) {
        simtime_t time;
        for (const auto& arrival : arrivals)
            if (arrival.first == target)
                time = arrival.second;
        ASSERT(time > SIMTIME_ZERO);
        return time;
    }
    void prepareState() {
        if (prefixMode()) {
            seed(Ipv4Address("10.1.0.0"), 24, 100, true);
            if (mode != "PrefixUnique")
                seed(Ipv4Address("10.1.0.0"), 25, 500, mode != "PrefixInactiveWitness");
            seed(destination, 32, 20, true);
            return;
        }
        if (mode == "ForeignForward" || mode == "ForeignError")
            seed(destination, 32, 20, true, true);
        if (mode == "TransitAggregate")
            seed(Ipv4Address("10.1.0.0"), 24, 100, false);
        if (mode == "TransitBlockedHost" || mode == "TransitUnknownHostAggregate")
            seed(Ipv4Address("10.0.0.0"), 24, 100, true);
        auto route = record(destination);
        if (route && mode.find("Transit") == 0 && mode != "TransitAbsent") {
            auto data = check_and_cast<AodvRouteData *>(route->getProtocolData());
            data->setIsActive(false);
            data->setLifeTime(simTime() + SimTime(getModuleByPath("^.^.dut.aodv")->par("deletePeriod")));
            if (mode == "TransitKnown" || mode == "TransitBudget")
                data->setDestSeqNum(0xffffffffu);
            if (mode == "TransitUnknown" || mode == "TransitUnknownHostAggregate") {
                data->setHasValidDestNum(false);
                data->setDestSeqNum(999);
            }
            if (mode == "TransitBlockedHost")
                data->setDestSeqNum(5);
        }
        if (mode == "TransitBudgetCrossing") {
            for (auto target : {destination, L3Address(Ipv4Address("10.0.0.98"))}) {
                auto data = check_and_cast<AodvRouteData *>(record(target)->getProtocolData());
                data->setIsActive(false);
                data->setLifeTime(simTime() + SimTime(getModuleByPath("^.^.dut.aodv")->par("deletePeriod")));
            }
        }
        if (route)
            baseline = routeSnapshot(route);
        else if (mode == "ForeignForward" || mode == "ForeignError")
            baseline = routeSnapshot(record(destination, 32, true));
    }
    void finishCase() {
        ASSERT(requests == 0);
        if (mode == "NoDelete" || mode == "NoDeleteStale" || mode == "NoDeleteHalf" || mode == "NoDeleteInactive") {
            unchanged(record(destination), baseline);
            checkRelay(mode == "NoDelete" ? 20 : mode == "NoDeleteHalf" ? 0x80000014u : 19, true);
        }
        else if (mode == "StaleError" || mode == "HalfError" || mode == "OtherHop" || mode == "ForeignError" || mode == "NoPrecursorNotification") {
            unchanged(record(destination, 32, mode == "ForeignError"), baseline);
            ASSERT(errors.empty());
        }
        else if (mode == "OrdinaryError" || mode == "NoPrecursorError") {
            checkSequence(destination, 21, false);
            if (mode == "OrdinaryError") checkRelay(21, false);
            else ASSERT(errors.empty());
        }
        else if (prefixMode()) {
            auto prefix = routeSnapshot(record(Ipv4Address("10.1.0.0"), 24));
            if (mode == "PrefixUnique") ASSERT(!prefix.active && prefix.sequence == 600);
            else {
                ASSERT(prefix.active && prefix.sequence == 100 && prefix.expiry == SimTime(5));
                auto witness = routeSnapshot(record(Ipv4Address("10.1.0.0"), 25));
                ASSERT(witness.sequence == 500 && witness.expiry == SimTime(5));
                ASSERT(witness.active == (mode != "PrefixInactiveWitness"));
            }
            checkSequence(destination, 21, false);
            ASSERT(errors.empty());
        }
        else if (mode == "OwnedForward" || mode == "ForeignForward" || mode == "TransitUnknownHostAggregate") {
            ASSERT(delivered == 1 && drops == 0 && errors.empty());
            if (mode != "OwnedForward") unchanged(record(destination, 32, mode == "ForeignForward"), baseline);
        }
        else if (dataMode()) {
            int packets = mode == "TransitKnown" || mode == "TransitBudget" || mode == "TransitUnknown" || mode == "TransitBudgetCrossing" ? 2 : 1;
            ASSERT(delivered == 0 && int(arrivals.size()) == packets);
            if (mode == "TransitBudgetCrossing") {
                std::set<int> identifiers;
                for (const auto& message : errors) identifiers.insert(message.id);
                ASSERT(identifiers.size() == 2);
                ASSERT(errors.size() == 2);
                ASSERT(errors[0].reception > SimTime(0.9) && errors[1].reception > SimTime(1.12));
                checkSequence(destination, 21, false);
                checkSequence(Ipv4Address("10.0.0.98"), 31, false);
            }
            else {
                auto reported = mode == "TransitAggregate" ? L3Address(Ipv4Address("10.1.0.0")) : destination;
                uint32_t firstSequence = mode == "TransitAggregate" ? 101 : mode == "TransitBlockedHost" ? 6 : 0;
                if (mode == "TransitAbsent") ASSERT(!record(destination));
                else {
                    auto route = record(reported, mode == "TransitAggregate" ? 24 : 32);
                    auto data = check_and_cast<AodvRouteData *>(route->getProtocolData());
                    ASSERT(!data->isActive());
                    ASSERT(data->getLifeTime() == lastArrival(destination) + SimTime(getModuleByPath("^.^.dut.aodv")->par("deletePeriod")));
                    if (mode == "TransitUnknown") ASSERT(!data->hasValidDestNum() && data->getDestSeqNum() == 999);
                    else ASSERT(data->hasValidDestNum() && data->getDestSeqNum() == (mode == "TransitKnown" || mode == "TransitBudget" ? 1 : firstSequence));
                    if (mode == "TransitBlockedHost") {
                        auto aggregate = routeSnapshot(record(Ipv4Address("10.0.0.0"), 24));
                        ASSERT(aggregate.active && aggregate.sequence == 100);
                    }
                }
                int messages = mode == "TransitBudget" ? 1 : packets;
                ASSERT(int(errors.size()) == messages);
                for (int i = 0; i < messages; i++) {
                    ASSERT(errors[i].target == Ipv4Address("10.0.0.3"));
                    ASSERT(errors[i].nodes.size() == 1 && errors[i].nodes.front().addr == reported);
                    uint32_t expected = mode == "TransitKnown" ? i : firstSequence;
                    ASSERT(errors[i].nodes.front().seqNum == expected && !errors[i].noDelete);
                }

            }
        }
        else if (mode == "FanoutBudget") {
            checkSequence(destination, 21, false);
            checkSequence(Ipv4Address("10.0.0.98"), 31, false);
            ASSERT(errors.size() == 2);
            std::set<L3Address> targets;
            for (const auto& message : errors) {
                targets.insert(message.target);
                ASSERT(message.nodes.size() == 1 && message.nodes.front().addr == destination && message.nodes.front().seqNum == 21);
            }
            ASSERT(targets == std::set<L3Address>({Ipv4Address("10.0.0.3"), Ipv4Address("10.0.0.4")}));
        }
        else throw cRuntimeError("Unknown error scenario: %s", mode.c_str());
        pass(mode);
    }
  protected:
    void initialize(int stage) override {
        AodvPeer::initialize(stage);
        if (stage == INITSTAGE_APPLICATION_LAYER) {
            mode = par("scenario").stdstringValue();
            if (mode == "TransitAggregate") destination = Ipv4Address("10.1.0.7");
            dutIp = getModuleByPath("^.^.dut.ipv4.ip");
            dutIp->subscribe(packetReceivedFromLowerSignal, this);
            dutIp->subscribe(packetDroppedSignal, this);
        }
    }
    void handleMessage(cMessage *message) override {
        if (message != timer) { AodvPeer::handleMessage(message); return; }
        auto time = simTime();
        if (time == SimTime(0.1) && mode.find("NoPrecursor") != 0) ask(1, destination);
        if (time == SimTime(0.11) && !prefixMode() && mode != "ForeignForward" && mode != "ForeignError" && mode != "TransitAbsent" && mode != "TransitAggregate") offer(destination, 20);
        if (time == SimTime(0.115) && (mode == "FanoutBudget" || mode == "TransitBudgetCrossing")) offer(Ipv4Address("10.0.0.98"), 30);
        if (time == SimTime(0.12)) {
            prepareState();
            if (mode == "FanoutBudget") ask(2, destination);
        }
        if (time == SimTime(0.125) && mode == "FanoutBudget") ask(2, Ipv4Address("10.0.0.98"));
        if (time == SimTime(0.13) && !dataMode() && mode != "FanoutBudget") {
            if (prefixMode()) transmitError({{Ipv4Address("10.1.0.0"), 600}, {destination, 21}});
            else {
                bool notification = (mode.find("NoDelete") == 0 && mode != "NoDeleteInactive") || mode == "NoPrecursorNotification";
                uint32_t sequence = mode == "StaleError" || mode == "NoDeleteStale" ? 19 : mode == "HalfError" || mode == "NoDeleteHalf" ? 0x80000014u : notification ? 20 : 21;
                transmitError({{destination, sequence}}, notification, mode == "OtherHop" ? 1 : 0);
            }
        }
        if (time == SimTime(0.15)) {
            if (mode == "NoDeleteInactive") { baseline = routeSnapshot(record(destination)); errors.clear(); transmitError({{destination, 19}}, true); }
            if (mode == "FanoutBudget") transmitError({{destination, 21}});
        }
        if (time == SimTime(0.17) && mode == "FanoutBudget") transmitError({{Ipv4Address("10.0.0.98"), 31}});
        if ((time == SimTime(0.2) && !dataMode()) || (time == SimTime(0.3) && dataMode() && mode != "TransitBudgetCrossing") || time == SimTime(1.4)) {
            finishCase();
            return;
        }
        const double times[] = {0.1, 0.11, 0.115, 0.12, 0.125, 0.13, 0.15, 0.17, 0.2, 0.3, 1.4};
        scheduleAt(times[++step], timer);
    }
    void socketDataArrived(UdpSocket *, Packet *packet) override {
        auto control = packet->peekAtFront<AodvControlPacket>();
        if (auto message = dynamicPtrCast<const Rerr>(control)) {
            Error entry;
            entry.target = packet->getTag<L3AddressInd>()->getDestAddress();
            entry.noDelete = message->getNoDeleteFlag();
            entry.reception = simTime();
            entry.id = dynamicPtrCast<const Ipv4Header>(packet->getTag<NetworkProtocolInd>()->getNetworkProtocolHeader())->getIdentification();
            for (size_t i = 0; i < message->getUnreachableNodesArraySize(); i++) entry.nodes.push_back(message->getUnreachableNodes(i));
            ASSERT(packet->getTag<HopLimitInd>()->getHopLimit() == 1);
            errors.push_back(entry);
        }
        else if (auto request = dynamicPtrCast<const Rreq>(control)) {
            if (request->getOriginatorAddr() == dut) requests++;
        }
        else if (auto reply = dynamicPtrCast<const Rrep>(control)) {
            if (reply->getAckRequiredFlag()) {
                auto interfaces = check_and_cast<IInterfaceTable *>(getModuleByPath("^.interfaceTable"));
                for (int i = 0; i < getParentModule()->par("numWlanInterfaces").intValue(); i++)
                    if (interfaces->findInterfaceByName(("wlan" + std::to_string(i)).c_str())->getInterfaceId() == packet->getTag<InterfaceInd>()->getInterfaceId()) {
                        auto ack = makeShared<RrepAck>(); ack->setPacketType(RREPACK); ack->setChunkLength(B(2)); transmit(ack, i); break;
                    }
            }
        }
        delete packet;
    }
    void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *) override {
        Enter_Method("observeTransitError");
        auto packet = dynamic_cast<Packet *>(object);
        if (!packet || strncmp(packet->getName(), "queued-data-", 12)) return;
        if (source == dutIp && signal == packetReceivedFromLowerSignal)
            arrivals.emplace_back(packet->peekAtFront<Ipv4Header>()->getDestAddress(), simTime());
        else if (source == dutIp && signal == packetDroppedSignal) drops++;
        else if (source == getModuleByPath("^.ipv4.ip") && signal == packetReceivedFromLowerSignal) delivered++;
    }
};
Define_Module(AodvErrorPeer);
#endif

// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __AODV_LIVE_INTERFACE_TEST_PEER_H
#define __AODV_LIVE_INTERFACE_TEST_PEER_H

#include "AodvPeer.h"
#include "AodvRepairIpv4.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"
#include "inet/common/lifecycle/LifecycleController.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#include "inet/networklayer/ipv6/Ipv6Header_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/contract/IL3AddressType.h"

class AodvLiveInterfacePeer : public AodvPeer, public LifecycleController, public IDoneCallback
{
    std::string mode;
    cModuleType *removedType = nullptr;
    cGate *upperPrevious = nullptr, *upperNext = nullptr, *radioPrevious = nullptr;
    int removedId = -1;
    IRoute *manualRoute = nullptr;
    int dataCount = 0;
    int ownRepliesOnSecondary = 0;
    int staleOutputs = 0;
    int observedUdpOutputs = 0;
    int earlyData = 0;
    int earlyControl = 0;
    int recoveryAcks = 0;
    int errorsForTarget = 0;
    int siblingErrors = 0;
    std::set<long> errorIds;
    std::set<int> observedMacIds;
    bool secondaryUnavailable = false;
    std::vector<uint32_t> requestFloors;
    std::vector<uint32_t> secondaryReplySequences;
    bool operationPending = false;
    bool primaryCase = false;
    bool using6 = false;
    L3Address originalPrimary, originalSecondary;
    simtime_t permittedDataAt;
    const simtime_t times[10] = {0.1, 0.15, 0.2, 0.3, 0.45, 0.5, 0.65, 0.7, 0.75, 0.8};
    bool aggregateHistory() const { return mode == "DeletedHostBlocksAggregate" || mode == "DownHostBlocksAggregate" || mode == "ReaddressHostBlocksAggregate"; }
    void invoke() override { Enter_Method("interfaceOperationCompleted"); operationPending = false; }

    IInterfaceTable *dutInterfaces() {
        return check_and_cast<IInterfaceTable *>(getModuleByPath("^.^.dut.interfaceTable"));
    }
    NetworkInterface *dutInterface(int index) {
        return dutInterfaces()->findInterfaceByName(("wlan" + std::to_string(index)).c_str());
    }
    NetworkInterface *peerInterface(int index) {
        return check_and_cast<IInterfaceTable *>(getModuleByPath("^.interfaceTable"))->findInterfaceByName(
            ("wlan" + std::to_string(index)).c_str());
    }
    IRoute *findRoute(const L3Address& address) {
        for (int i = 0; i < routes->getNumRoutes(); ++i) {
            auto route = routes->getRoute(i);
            if (route->getSourceType() == IRoute::AODV && route->getDestinationAsGeneric() == address &&
                    route->getPrefixLength() == (using6 ? 128 : 32)) return route;
        }
        return nullptr;
    }
    void readdressPrimary(const char *v4, const char *v6) {
        if (!using6) dutInterface(0)->getProtocolDataForUpdate<Ipv4InterfaceData>()->setIPAddress(Ipv4Address(v4));
        else {
            auto data = dutInterface(0)->getProtocolDataForUpdate<Ipv6InterfaceData>();
            auto old = data->getPreferredAddress();
            data->removeAddress(old);
            data->assignAddress(Ipv6Address(v6), false, SIMTIME_ZERO, SIMTIME_ZERO);
        }
    }
    void operate(LifecycleOperation *operation) {
        LifecycleOperation::StringMap parameters;
        operation->initialize(getModuleByPath("^.^.dut.aodv"), parameters);
        ASSERT(!operationPending);
        operationPending = !initiateOperation(operation, this);
    }
    void sendControl(const Ptr<AodvControlPacket>& control, int output, bool multicast = false) {
        auto packet = new Packet("live-interface-control", control);
        auto local = peerInterface(output);
        auto remote = dutInterface(output);
        ASSERT(remote);
        auto address = multicast ? dut.getAddressType()->getBroadcastAddress() : remote->getNetworkAddress();
        packet->addTag<InterfaceReq>()->setInterfaceId(local->getInterfaceId());
        packet->addTag<L3AddressReq>()->setSrcAddress(local->getNetworkAddress());
        packet->addTag<NextHopAddressReq>()->setNextHopAddress(address);
        packet->addTag<HopLimitReq>()->setHopLimit(10);
        socket.sendTo(packet, address, 654);
    }
    void offer(uint32_t sequence, int link, simtime_t lifetime = 1.2, int prefix = 0, bool oldOriginator = false, L3Address target = L3Address()) {
        auto reply = makeShared<Rrep>();
        reply->setPacketType(using6 ? RREP_IPv6 : RREP); reply->setChunkLength(using6 ? B(44) : B(20));
        reply->setDestAddr(!target.isUnspecified() ? target : prefix ? L3Address(Ipv4Address("10.0.0.55")) : destination);
        reply->setPrefixSize(prefix); reply->setDestSeqNum(sequence);
        reply->setOriginatorAddr(oldOriginator ? dut : dutInterface(0)->getNetworkAddress());
        reply->setHopCount(mode == "Repair" || mode == "DeferredSiblingRepair" ? 2 : 1); reply->setLifeTime(lifetime);
        if (primaryCase) reply->setAckRequiredFlag(true);
        sendControl(reply, link);
    }
    void ask(bool own, int link, bool remote = false) {
        auto request = makeShared<Rreq>();
        request->setPacketType(using6 ? RREQ_IPv6 : RREQ); request->setChunkLength(using6 ? B(48) : B(24));
        request->setOriginatorAddr(peerInterface(link)->getNetworkAddress());
        request->setOriginatorSeqNum(10 + step); request->setRreqId(10 + step);
        request->setDestAddr(remote ? L3Address(Ipv4Address("198.51.100.77")) :
            own ? dutInterface(link)->getNetworkAddress() : destination);
        request->setUnknownSeqNumFlag(true); request->setDestOnlyFlag(own);
        if (mode == "Repair" || mode == "DeferredSiblingRepair") request->setHopCount(2);
        sendControl(request, link, using6 && mode == "Ipv6Membership" && step == 5);
    }
    void changeSecondary() {
        auto changed = dutInterface(1);
        removedId = changed->getInterfaceId();
        secondaryUnavailable = mode != "AddressChange" && mode != "ReaddressHostBlocksAggregate";
        if (mode == "AddressChange" || mode == "ReaddressHostBlocksAggregate")
            changed->getProtocolDataForUpdate<Ipv4InterfaceData>()->setIPAddress(Ipv4Address("10.1.0.10"));
        else if (mode == "DeletedHistoryStaleReply" || mode == "DeletedHostBlocksAggregate" ||
                mode == "ExpiredHistory" || mode == "DeleteDuringJitter")
            dutInterfaces()->deleteInterface(changed);
        else if (mode == "RecreateName") {
            removedType = changed->getModuleType();
            upperPrevious = changed->gate("upperLayerIn")->getPreviousGate();
            upperNext = changed->gate("upperLayerOut")->getNextGate();
            radioPrevious = changed->gate("radioIn")->getPreviousGate();
            removedId = changed->getInterfaceId();
            dutInterfaces()->deleteInterface(changed);
        }
        else
            changed->setState(NetworkInterface::DOWN);
    }
    void restoreSecondary() {
        if (removedType) {
            auto nic = check_and_cast<NetworkInterface *>(removedType->create("wlan", getModuleByPath("^.^.dut"), 1));
            nic->finalizeParameters(); nic->buildInside();
            upperPrevious->connectTo(nic->gate("upperLayerIn"));
            nic->gate("upperLayerOut")->connectTo(upperNext);
            radioPrevious->connectTo(nic->gate("radioIn"));
            nic->scheduleStart(simTime()); nic->callInitialize();
            auto mac = nic->getSubmodule("mac");
            observedMacIds.insert(mac->getId()); mac->subscribe(packetReceivedFromUpperSignal, this);
            auto data = nic->getProtocolDataForUpdate<Ipv4InterfaceData>();
            data->setNetmask(Ipv4Address("255.255.255.0")); data->setIPAddress(Ipv4Address("10.1.0.1"));
            ASSERT(nic->getInterfaceId() != removedId);
            removedType = nullptr;
        }
        else if (auto changed = dutInterface(1))
            changed->setState(NetworkInterface::UP);
        secondaryUnavailable = false;
    }
    void addManual() {
        manualRoute = routes->createRoute();
        manualRoute->setDestination(destination); manualRoute->setPrefixLength(32);
        manualRoute->setNextHop(peerInterface(0)->getNetworkAddress());
        manualRoute->setInterface(dutInterface(0)); manualRoute->setSourceType(IRoute::MANUAL);
        manualRoute->setSource(this); manualRoute->setMetric(0);
        routes->addRoute(manualRoute);
    }
    void repairData(L3Address target = L3Address()) {
        auto packet = new AodvTrackedPacket("repair-data", makeShared<ByteCountChunk>(B(8)));
        packet->addTag<InterfaceReq>()->setInterfaceId(peerInterface(0)->getInterfaceId());
        packet->addTag<L3AddressReq>()->setSrcAddress(peerInterface(0)->getNetworkAddress());
        packet->addTag<NextHopAddressReq>()->setNextHopAddress(dut);
        packet->addTag<HopLimitReq>()->setHopLimit(10);
        socket.sendTo(packet, target.isUnspecified() ? destination : target, 9999);
    }
    void runRepair() {
        if (step == 0) ask(true, 0);
        if (step == 1) {
            offer(20, 1);
            if (mode == "DeferredSiblingRepair") offer(20, 1, 1.2, 0, false, Ipv4Address("10.0.0.98"));
        }
        if (step == 2) {
            ASSERT(findRoute(destination));
            check_and_cast<inet::physicallayer::IRadio *>(getModuleByPath("^.wlan[1].radio"))->setRadioMode(inet::physicallayer::IRadio::RADIO_MODE_OFF);
        }
        if (step == 3) repairData();
        if (step == 4) {
            auto route = findRoute(destination);
            ASSERT(route && check_and_cast<AodvRouteData *>(route->getProtocolData())->isBeingRepaired());
            repairData();
            removedId = dutInterface(1)->getInterfaceId();
            secondaryUnavailable = true;
            if (mode == "DeferredSiblingRepair") {
                auto sibling = findRoute(Ipv4Address("10.0.0.98"));
                ASSERT(sibling && check_and_cast<AodvRouteData *>(sibling->getProtocolData())->isRepariable());
                dutInterface(1)->setState(NetworkInterface::DOWN);
                repairData(Ipv4Address("10.0.0.98"));
            }
            else dutInterfaces()->deleteInterface(dutInterface(1));
        }
        if (step == 5) {
            ASSERT(check_and_cast<AodvRepairIpv4 *>(getModuleByPath("^.^.dut.ipv4.ip"))->queuedCount() == 1);
            offer(22, 0);
        }
        if (step == 6) {
            auto route = findRoute(destination);
            ASSERT(route && route->getInterface() == dutInterface(0));
            auto state = check_and_cast<AodvRouteData *>(route->getProtocolData());
            ASSERT(state->isActive() && !state->isBeingRepaired());
            ASSERT(route->getInterface()->getInterfaceId() != removedId);
            std::cout << "repair observations data=" << dataCount << " originals=" << AodvDataCounters::created << "/" << AodvDataCounters::destroyed
                << " transit=" << AodvRepairTransitPacket::created << "/" << AodvRepairTransitPacket::destroyed
                << " queued=" << check_and_cast<AodvRepairIpv4 *>(getModuleByPath("^.^.dut.ipv4.ip"))->queuedCount() << "\n";
            ASSERT(dataCount == 1);
        }
        if (step == 7) {
            int expected = mode == "DeferredSiblingRepair" ? 3 : 2;
            ASSERT(staleOutputs == 0 && AodvDataCounters::created == expected && AodvDataCounters::destroyed == expected);
            ASSERT(observedUdpOutputs > 0);
            ASSERT(AodvRepairTransitPacket::created > 0 && AodvRepairTransitPacket::created == AodvRepairTransitPacket::destroyed);
            ASSERT(check_and_cast<AodvRepairIpv4 *>(getModuleByPath("^.^.dut.ipv4.ip"))->queuedCount() == 0);
            if (mode == "DeferredSiblingRepair") ASSERT(siblingErrors == 1);
            std::cout << "AODV LiveInterface " << mode << ": PASS\n";
            return;
        }
        static const double repairTimes[] = {0.1, 0.15, 0.2, 0.3, 0.32, 0.34, 0.4, 0.7};
        ++step; scheduleAt(repairTimes[step], timer);
    }
    void runDelayedReturn() {
        if (step == 0) ask(false, 1);
        if (step == 1) {
            auto reply = makeShared<Rrep>(); reply->setPacketType(RREP); reply->setChunkLength(B(20));
            reply->setDestAddr(destination); reply->setDestSeqNum(20); reply->setHopCount(1); reply->setLifeTime(1.2);
            reply->setOriginatorAddr(peerInterface(1)->getNetworkAddress()); sendControl(reply, 0);
        }
        if (step == 2) changeSecondary();
        if (step == 3) { restoreSecondary(); offer(22, 0); }
        if (step == 4) ask(false, 1);
        if (step == 5) {
            ASSERT(ownRepliesOnSecondary == 1 && secondaryReplySequences == std::vector<uint32_t>{22});
            ASSERT(staleOutputs == 0 && AodvDataCounters::created == 0 && AodvDataCounters::destroyed == 0);
            ASSERT(observedUdpOutputs > 0);
            std::cout << "AODV LiveInterface DownUpBeforeDelayedReply: PASS\n"; return;
        }
        static const double delayedTimes[] = {0.1, 0.15, 0.2, 0.25, 0.4, 0.65};
        ++step; scheduleAt(delayedTimes[step], timer);
    }
    void run() {
        if (mode == "Repair" || mode == "DeferredSiblingRepair") { runRepair(); return; }
        if (mode == "DownUpBeforeDelayedReply") { runDelayedReturn(); return; }
        if (step == 0) {
            if (mode != "QueuedDiscovery")
                offer(20, mode == "UnaffectedLink" ? 0 : 1, mode == "ExpiredHistory" ? 0.05 : 1.2);
            if (aggregateHistory()) offer(100, 0, 1.2, 24);
        }
        else if (step == 1) {
            if (mode == "PendingAck") ask(true, 1);
            else if (mode == "AddressChange" || mode == "DeleteDuringJitter") ask(false, 0, true);
            else if (mode != "ExpiredHistory" && mode != "QueuedDiscovery") ask(false, 0);
        }
        else if (step == 2) {
            if (primaryCase) {
                if (mode == "ReaddressIpv4" || mode == "ReaddressIpv6" || mode == "RepeatedChange" || mode == "OldDelayedReply") {
                    readdressPrimary("10.0.0.10", "2001:db8:1::10");
                    permittedDataAt = 0.6;
                }
                else {
                    dutInterface(0)->setState(NetworkInterface::DOWN);
                    permittedDataAt = SimTime::getMaxTime();
                    if (mode == "AllLinksDown") dutInterface(1)->setState(NetworkInterface::DOWN);
                }
            }
            else {
                changeSecondary();
                if (mode == "Lifecycle") operate(new ModuleStopOperation());
            }
        }
        else if (step == 3) {
            if (mode == "QueuedDiscovery") offer(22, 0);
            else if (mode != "Lifecycle")
                offer(mode == "ExpiredHistory" ? 20 : 19, 0, 1.2, 0, mode == "OldDelayedReply");
            if (aggregateHistory()) addManual();
        }
        else if (step == 4) {
            if (!primaryCase && mode != "QueuedDiscovery" && mode != "UnaffectedLink" && mode != "Lifecycle") {
                auto route = findRoute(destination);
                if (mode == "ExpiredHistory") {
                    ASSERT(route && check_and_cast<AodvRouteData *>(route->getProtocolData())->isActive());
                    ASSERT(check_and_cast<AodvRouteData *>(route->getProtocolData())->getDestSeqNum() == 20);
                }
                else if (route)
                    ASSERT(!check_and_cast<AodvRouteData *>(route->getProtocolData())->isActive());
            }
            if (manualRoute) { routes->deleteRoute(manualRoute); manualRoute = nullptr; }
            if (mode == "RepeatedChange") {
                readdressPrimary("10.0.0.11", "2001:db8:1::11");
                permittedDataAt = 0.85;
            }
            else if (primaryCase) {
                if (!dutInterface(0)->isUp()) permittedDataAt = 0.85;
                dutInterface(0)->setState(NetworkInterface::UP);
                if (mode == "AllLinksDown") restoreSecondary();
            }
            else {
                restoreSecondary();
                if (mode == "Lifecycle") operate(new ModuleStartOperation());
            }
        }
        else if (step == 5) {
            if (aggregateHistory()) ASSERT(dataCount == 2);
            offer(22, 0);
            if (primaryCase) ask(true, 0);
            if (mode == "PendingAck" || mode == "Ipv6Membership" || mode == "RecreateName") ask(true, 1);
        }
        else if (step == 6 && (mode == "PrimaryDown" || mode == "LearnDuringHold" || mode == "AllLinksDown")) {
            auto route = findRoute(destination);
            ASSERT(route && check_and_cast<AodvRouteData *>(route->getProtocolData())->isActive());
            ASSERT(dataCount == 0);
        }
        else if (step == 7 && (mode == "ReturnDuringHold" || mode == "Lifecycle")) {
            if (mode == "Lifecycle") { dutInterface(1)->setState(NetworkInterface::DOWN); operate(new ModuleCrashOperation()); }
            else { dutInterface(0)->setState(NetworkInterface::DOWN); permittedDataAt = SimTime::getMaxTime(); }
        }
        else if (step == 8 && (mode == "ReturnDuringHold" || mode == "Lifecycle")) {
            if (mode == "Lifecycle") { restoreSecondary(); operate(new ModuleStartOperation()); }
            else { dutInterface(0)->setState(NetworkInterface::UP); permittedDataAt = 1.15; }
        }
        else if (step == 9 && (mode == "ReturnDuringHold" || mode == "Lifecycle")) offer(23, 0);
        else if (step == 10) {
            int expected = mode == "PrimaryDown" || mode == "AllLinksDown" || mode == "LearnDuringHold" || mode == "RepeatedChange" ? 2 :
                mode == "ReturnDuringHold" || mode == "Lifecycle" ? 1 : 3;
            ASSERT(dataCount == expected);
            ASSERT(staleOutputs == 0);
            ASSERT(observedUdpOutputs > 0);
            ASSERT(earlyData == 0);
            ASSERT(earlyControl == 0);
            if (primaryCase) { ASSERT(recoveryAcks > 0); ASSERT(AodvDataCounters::errors == 0); }
            ASSERT(!operationPending);
            ASSERT(AodvDataCounters::created == 3 && AodvDataCounters::destroyed == 3);
            if (mode == "PendingAck") ASSERT(ownRepliesOnSecondary == 2);
            if (mode == "Ipv6Membership" || mode == "RecreateName") ASSERT(ownRepliesOnSecondary == 1);
            if (mode == "SecondaryDown" || mode == "RecreateName" || mode == "Ipv6Membership") ASSERT(errorsForTarget == 1);
            if (mode == "DeletedHistoryStaleReply" || aggregateHistory()) {
                ASSERT(std::find(requestFloors.begin(), requestFloors.end(), 21) != requestFloors.end());
                ASSERT(errorsForTarget == 1);
            }
            std::cout << "AODV LiveInterface " << mode << ": PASS\n";
            return;
        }
        ++step;
        scheduleAt(step < 10 ? times[step] : SimTime(1.3), timer);
    }
  protected:
    void initialize(int stage) override {
        if (stage != INITSTAGE_APPLICATION_LAYER) return;
        using6 = par("ipv6");
        if (using6) {
            dut = Ipv6Address("2001:db8:1::1"); peer = Ipv6Address("2001:db8:1::2");
            destination = Ipv6Address("2001:db8:9::99");
        }
        socket.setOutputGate(gate("socketOut")); socket.setCallback(this); socket.bind(654); socket.setBroadcast(true);
        routes = check_and_cast<IRoutingTable *>(getModuleByPath(using6 ? "^.^.dut.ipv6.routingTable" : "^.^.dut.ipv4.routingTable"));
        getModuleByPath(using6 ? "^.ipv6.ipv6" : "^.ipv4.ip")->subscribe(packetReceivedFromLowerSignal, this);
        if (using6) for (int i = 0; i < 2; ++i)
            socket.joinMulticastGroup(dut.getAddressType()->getBroadcastAddress(), peerInterface(i)->getInterfaceId());
        timer = new cMessage("live-interface-script"); scheduleAt(0.1, timer);
        mode = par("scenario").stdstringValue();
        primaryCase = par("primaryCase");
        originalPrimary = dutInterface(0)->getNetworkAddress(); originalSecondary = dutInterface(1)->getNetworkAddress();
        for (int i = 0; i < 2; ++i) {
            auto mac = dutInterface(i)->getSubmodule("mac");
            observedMacIds.insert(mac->getId()); mac->subscribe(packetReceivedFromUpperSignal, this);
        }
    }
    void handleMessage(cMessage *message) override {
        if (message == timer) run();
        else socket.processMessage(message);
    }
    void socketDataArrived(UdpSocket *, Packet *packet) override {
        auto control = packet->peekAtFront<AodvControlPacket>();
        if (auto request = dynamicPtrCast<const Rreq>(control)) {
            if (request->getDestAddr() == destination && !request->getUnknownSeqNumFlag()) requestFloors.push_back(request->getDestSeqNum());
        }
        else if (auto reply = dynamicPtrCast<const Rrep>(control)) {
            auto ingress = packet->getTag<InterfaceInd>()->getInterfaceId();
            bool secondary = ingress == peerInterface(1)->getInterfaceId();
            if (secondary) { ++ownRepliesOnSecondary; secondaryReplySequences.push_back(reply->getDestSeqNum()); }
            if (reply->getAckRequiredFlag() && !(mode == "PendingAck" && secondary)) {
                auto ack = makeShared<RrepAck>(); ack->setPacketType(using6 ? RREPACK_IPv6 : RREPACK);
                sendControl(ack, secondary ? 1 : 0);
            }
        }
        else if (auto error = dynamicPtrCast<const Rerr>(control)) {
            if (errorIds.insert(packet->getTreeId()).second)
                for (size_t i = 0; i < error->getUnreachableNodesArraySize(); ++i) {
                    if (error->getUnreachableNodes(i).addr == destination) ++errorsForTarget;
                    if (error->getUnreachableNodes(i).addr == L3Address(Ipv4Address("10.0.0.98"))) ++siblingErrors;
                }
        }
        else if (dynamicPtrCast<const RrepAck>(control) && primaryCase && simTime() >= 0.5 && simTime() < permittedDataAt)
            ++recoveryAcks;
        delete packet;
    }
    void receiveSignal(cComponent *source, simsignal_t, cObject *value, cObject *) override {
        Enter_Method("observeLiveInterfaceWire");
        auto packet = dynamic_cast<Packet *>(value);
        if (!packet) return;
        Ptr<const NetworkHeaderBase> header;
        if (using6) header = packet->peekAtFront<Ipv6Header>();
        else header = packet->peekAtFront<Ipv4Header>();
        bool data = !strncmp(packet->getName(), "queued-data-", 12) || !strcmp(packet->getName(), "repair-data");
        if (observedMacIds.count(source->getId()) && header->getProtocol() == &Protocol::udp) {
            ++observedUdpOutputs;
            if (secondaryUnavailable && packet->getTag<InterfaceReq>()->getInterfaceId() == removedId) ++staleOutputs;
            auto udp = packet->peekDataAt<UdpHeader>(header->getChunkLength());
            if (primaryCase && simTime() >= 0.2 && simTime() < permittedDataAt && udp->getDestPort() == 654) {
                auto control = packet->peekDataAt<AodvControlPacket>(header->getChunkLength() + udp->getChunkLength());
                if (dynamicPtrCast<const Rreq>(control) || dynamicPtrCast<const Rrep>(control)) ++earlyControl;
            }
            if (simTime() >= 0.2 && (mode == "ReaddressIpv4" || mode == "ReaddressIpv6" || mode == "RepeatedChange" || mode == "OldDelayedReply") &&
                    header->getSourceAddress() == originalPrimary) ++staleOutputs;
            if (simTime() >= 0.2 && (mode == "AddressChange" || mode == "ReaddressHostBlocksAggregate" || mode == "DeleteDuringJitter") &&
                    header->getSourceAddress() == originalSecondary) ++staleOutputs;
        }
        else if (data && (header->getDestinationAddress() == destination ||
                header->getDestinationAddress() == L3Address(Ipv4Address("10.0.0.88")))) {
            ++dataCount;
            if (primaryCase && simTime() < permittedDataAt) ++earlyData;
        }
    }
};
Define_Module(AodvLiveInterfacePeer);
#endif

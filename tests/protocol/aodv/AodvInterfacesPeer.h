// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __AODV_INTERFACES_TEST_PEER_H
#define __AODV_INTERFACES_TEST_PEER_H
#include "AodvPeer.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/lifecycle/LifecycleController.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/networklayer/ipv4/Ipv4.h"

class AodvInterfaceIpv4 : public Ipv4
{
  protected:
    void handleIncomingDatagram(Packet *packet) override {
        if (!strcmp(packet->getName(), "interface-missing-data"))
            packet->removeTagIfPresent<MacAddressInd>();
        Ipv4::handleIncomingDatagram(packet);
    }
};
Define_Module(AodvInterfaceIpv4);

class AodvInterfaceIngress : public SimpleModule
{
  protected:
    void handleMessage(cMessage *message) override {
        if (!strcmp(message->getName(), "interface-no-ingress"))
            check_and_cast<Packet *>(message)->removeTagIfPresent<InterfaceInd>();
        send(message, "out");
    }
};
Define_Module(AodvInterfaceIngress);

class AodvInterfacesPeer : public AodvPeer, public LifecycleController, public IDoneCallback
{
    struct Request { L3Address target, source, originator; uint32_t id, sequence; simtime_t time; };
    struct Reply { Ptr<const Rrep> header; L3Address source; int receiver; };
    std::string mode;
    std::vector<Request> requests;
    std::vector<Reply> replies;
    std::map<int, int> acknowledgments;
    std::vector<std::pair<L3Address, simtime_t>> dataArrivals;
    std::map<int, std::vector<simtime_t>> hellos;
    std::set<std::pair<L3Address, int>> broadcasts;
    std::set<L3Address> errorRecipients;
    UdpSocket observationSocket;
    int errors = 0;
    bool operationPending = false;
    void changeNode(LifecycleOperation *operation) {
        LifecycleOperation::StringMap parameters;
        operation->initialize(getModuleByPath("^.^.dut"), parameters);
        operationPending = !initiateOperation(operation, this);
    }
    void invoke() override { Enter_Method("interfaceOperationCompleted"); operationPending = false; }
    NetworkInterface *peerInterface(int index) {
        return check_and_cast<IInterfaceTable *>(getModuleByPath("^.interfaceTable"))->findInterfaceByName(("wlan" + std::to_string(index)).c_str());
    }
    NetworkInterface *dutInterface(int index) {
        return check_and_cast<IInterfaceTable *>(getModuleByPath("^.^.dut.interfaceTable"))->findInterfaceByName(("wlan" + std::to_string(index)).c_str());
    }
    L3Address dutAddress(int link) { return Ipv4Address((0x0a000001u) + (unsigned(link) << 16)); }
    int linkFor(int index) { return index == 3 ? 0 : index; }
    void sendControl(const Ptr<AodvControlPacket>& header, int output, const char *name = "scripted-aodv") {
        auto packet = new Packet(name, header);
        auto target = dutAddress(linkFor(output));
        packet->addTag<InterfaceReq>()->setInterfaceId(peerInterface(output)->getInterfaceId());
        packet->addTag<L3AddressReq>()->setSrcAddress(peerInterface(output)->getNetworkAddress());
        packet->addTag<NextHopAddressReq>()->setNextHopAddress(target);
        packet->addTag<HopLimitReq>()->setHopLimit(10);
        socket.sendTo(packet, target, 654);
    }
    void ask(const L3Address& target, uint32_t id, int output = 0, L3Address originator = L3Address(), bool known = false, bool destOnly = true) {
        auto request = makeShared<Rreq>(); request->setPacketType(RREQ); request->setChunkLength(B(24));
        request->setOriginatorAddr(originator.isUnspecified() ? peerInterface(output)->getNetworkAddress() : originator);
        request->setOriginatorSeqNum(id); request->setRreqId(id); request->setDestAddr(target);
        request->setDestSeqNum(known ? 25 : 0); request->setUnknownSeqNumFlag(!known); request->setDestOnlyFlag(destOnly);
        sendControl(request, output);
    }
    void offer(const L3Address& target, uint32_t sequence, int output) {
        auto reply = makeShared<Rrep>(); reply->setPacketType(RREP); reply->setChunkLength(B(20));
        reply->setDestAddr(target); reply->setDestSeqNum(sequence); reply->setLifeTime(1); reply->setHopCount(1);
        reply->setOriginatorAddr(dut); reply->setAckRequiredFlag(true);
        sendControl(reply, output);
    }
    void acknowledge(int output, const char *name = "scripted-interface-ack") {
        auto ack = makeShared<RrepAck>(); ack->setPacketType(RREPACK); ack->setChunkLength(B(2)); sendControl(ack, output, name);
    }
    void helloFrom(int output) {
        auto hello = makeShared<Rrep>(); hello->setPacketType(RREP); hello->setChunkLength(B(20));
        hello->setDestAddr(peerInterface(output)->getNetworkAddress()); hello->setDestSeqNum(10); hello->setLifeTime(1);
        sendControl(hello, output);
    }
    void transmitError(const L3Address& target, uint32_t sequence) {
        auto error = makeShared<Rerr>(); error->setPacketType(RERR); error->setChunkLength(B(12));
        error->setUnreachableNodesArraySize(1);
        UnreachableNode node; node.addr = target; node.seqNum = sequence; error->setUnreachableNodes(0, node);
        sendControl(error, 3);
    }
    int replyCount(const L3Address& target, int receiver) {
        int count = 0;
        for (const auto& reply : replies) count += reply.header->getDestAddr() == target && reply.receiver == receiver;
        return count;
    }
    void checkPath(const L3Address& target, uint32_t sequence, int output, bool active = true) {
        auto route = findRoute(target); ASSERT(route);
        auto state = check_and_cast<AodvRouteData *>(route->getProtocolData());
        ASSERT(state->hasValidDestNum() && state->getDestSeqNum() == sequence && state->isActive() == active);
        ASSERT(route->getInterface() == dutInterface(output));
    }
    void checkDiscovery() {
        std::map<L3Address, std::vector<Request>> grouped;
        for (const auto& request : requests) {
            ASSERT(request.originator == dut);
            grouped[request.target].push_back(request);
        }
        auto first = grouped[destination];
        ASSERT(first.size() == 2);
        ASSERT(first[0].id == 1 && first[1].id == 1 && first[0].sequence == 1 && first[1].sequence == 1);
        ASSERT(std::set<L3Address>({first[0].source, first[1].source}) == std::set<L3Address>({dutAddress(0), dutAddress(1)}));
        if (mode == "SecondaryScope") ASSERT(grouped.size() == 1 && dataArrivals.size() == 1);
        else if (mode != "Gateway") {
            auto second = grouped[Ipv4Address("10.0.0.98")];
            ASSERT(second.size() == 2 && second[0].id == 2 && second[1].id == 2);
            simtime_t window = mode == "DownStart" ? SimTime(1.15) : SimTime(1);
            ASSERT(second[0].time >= window && second[1].time >= window);
            ASSERT(dataArrivals.size() == 1);
        }
        else {
            ASSERT(grouped.size() == 1 && dataArrivals.size() == 2);
            ASSERT(dataArrivals[0].first == Ipv4Address("198.51.100.1") && dataArrivals[1].first == Ipv4Address("203.0.113.1"));
        }
        ASSERT(acknowledgments[1] == 1);
        int originals = mode == "SecondaryScope" ? 1 : 2;
        ASSERT(AodvDataCounters::created == originals && AodvDataCounters::destroyed == originals);
        if (mode == "DownStart") ASSERT(first.front().time < SimTime(0.25));
    }
    void finishCase() {
        if (mode == "Discovery" || mode == "DownStart" || mode == "Gateway" || mode == "SecondaryScope") checkDiscovery();
        else if (mode == "LocalAddresses") {
            ASSERT(replies.size() == 2 && requests.empty());
            ASSERT(replyCount(dutAddress(1), 0) == 1 && replyCount(dutAddress(0), 1) == 1);
            for (const auto& reply : replies) {
                ASSERT(reply.header->getDestSeqNum() == 25 && reply.header->getHopCount() == 0);
                ASSERT(reply.source == dutAddress(reply.receiver));
            }
        }
        else if (mode.find("Self") == 0) {
            ASSERT(requests.empty());
            ASSERT(!findRoute(mode == "SelfPrimary" ? dutAddress(0) : dutAddress(1)));
        }
        else if (mode == "Unselected") {
            ASSERT(replyCount(dut, 0) == 1 && acknowledgments[2] == 0 && acknowledgments[0] == 1);
            ASSERT(!findRoute(peerInterface(2)->getNetworkAddress()) && dataArrivals.size() == 1);
        }
        else if (mode == "ExternalGateway" || mode == "ExternalIngress") {
            ASSERT(dataArrivals.size() == 1 && requests.empty() && errors == 0);
        }
        else if (mode == "AckIsolation") {
            ASSERT(replyCount(dut, 0) == 2 && replyCount(dut, 1) == 1);
        }
        else if (mode == "Reconfigure") {
            ASSERT(replyCount(dut, 1) == 1 && replyCount(dut, 2) == 1);
            ASSERT(!findRoute(peerInterface(1)->getNetworkAddress()));
        }
        else if (mode == "OwnedReply") {
            ASSERT(replyCount(dut, 0) == 1 && replyCount(dut, 2) == 0);
            bool preserved = false;
            for (int i = 0; i < routes->getNumRoutes(); i++) {
                auto route = routes->getRoute(i);
                if (route->getSource() == this) {
                    ASSERT(route->getInterface() == dutInterface(2) && route->getMetric() == 0);
                    ASSERT(check_and_cast<AodvRouteData *>(route->getProtocolData())->getDestSeqNum() == 123);
                    preserved = true;
                }
            }
            ASSERT(preserved);
        }
        else if (mode == "Fanout") {
            ASSERT(errors == 2 && errorRecipients == std::set<L3Address>({peerInterface(0)->getNetworkAddress(), peerInterface(1)->getNetworkAddress()}));
            checkPath(destination, 21, 0, false); checkPath(Ipv4Address("10.0.0.98"), 31, 0, false);
        }
        else if (mode == "HelloSources") {
            ASSERT(hellos[0].size() >= 3 && hellos[1].size() >= 4);
            bool primaryAtSecond = false, secondaryAtSecond = false;
            for (auto time : hellos[0]) primaryAtSecond |= time > SimTime(0.2) && time < SimTime(0.21);
            for (auto time : hellos[1]) secondaryAtSecond |= time > SimTime(0.2) && time < SimTime(0.21);
            ASSERT(!primaryAtSecond && secondaryAtSecond);
        }
        else if (mode == "LivenessIsolation") {
            checkPath(peerInterface(0)->getNetworkAddress(), 10, 0);
            checkPath(destination, 21, 1, false);
            ASSERT(errors == 1 && errorRecipients == std::set<L3Address>({peerInterface(0)->getNetworkAddress()}));
        }
        else if (mode == "MissingIngress") { checkPath(destination, 20, 1); ASSERT(errors == 0); }
        else throw cRuntimeError("Unknown interface case %s", mode.c_str());
        pass("Interfaces" + mode);
    }
  protected:
    void initialize(int stage) override {
        AodvPeer::initialize(stage);
        if (stage == INITSTAGE_APPLICATION_LAYER) {
            mode = par("scenario").stdstringValue();
            if (mode == "ExternalGateway") {
                recipientIp = getModuleByPath("^.^.receiver.ipv4.ip");
                recipientIp->subscribe(packetReceivedFromLowerSignal, this);
            }
            if (mode == "SecondaryScope") destination = Ipv4Address("10.1.0.99");
            if (mode == "LivenessIsolation") {
                observationSocket.setOutputGate(gate("socketOut")); observationSocket.bind(23456);
            }
        }
    }
    void handleMessage(cMessage *message) override {
        if (message != timer) { AodvPeer::handleMessage(message); return; }
        auto time = simTime();
        if (time == SimTime(0.02)) {
            if (mode == "ExternalIngress") {
                auto route = routes->createRoute(); route->setDestination(peer); route->setPrefixLength(32);
                route->setNextHop(peer); route->setInterface(dutInterface(0));
                route->setMetric(1); route->setSourceType(IRoute::MANUAL); route->setSource(this);
                routes->addRoute(route);
            }
            if (mode == "OwnedReply") {
                auto route = routes->createRoute(); route->setDestination(peer); route->setPrefixLength(32);
                route->setNextHop(peerInterface(2)->getNetworkAddress()); route->setInterface(dutInterface(2));
                route->setMetric(0); route->setSourceType(IRoute::MANUAL); route->setSource(this);
                auto state = new AodvRouteData(); state->setDestSeqNum(123); state->setLifeTime(5);
                route->setProtocolData(state); routes->addRoute(route);
            }
            if (mode == "Fanout") offer(destination, 20, 3);
            if (mode == "HelloSources" || mode == "LivenessIsolation" || mode == "MissingIngress") helloFrom(0);
        }
        if (time == SimTime(0.03)) {
            if (mode == "Fanout") offer(Ipv4Address("10.0.0.98"), 30, 3);
            if (mode == "HelloSources" || mode == "LivenessIsolation" || mode == "MissingIngress") helloFrom(1);
        }
        if (time == SimTime(0.04) && (mode == "LivenessIsolation" || mode == "MissingIngress")) offer(destination, 20, 1);
        if (time == SimTime(0.08) && mode == "LivenessIsolation") ask(destination, 1, 0, L3Address(), false, false);
        if (time == SimTime(0.1)) {
            if (mode == "LocalAddresses") ask(dutAddress(1), 1, 0, peer, true);
            if (mode == "SelfPrimary") ask(destination, 1, 0, dutAddress(0));
            if (mode == "SelfSecondaryPrimaryIngress") ask(destination, 1, 0, dutAddress(1));
            if (mode == "SelfSecondarySecondaryIngress") ask(destination, 1, 1, dutAddress(1));
            if (mode == "Unselected") ask(dut, 7, 2, peer);
            if (mode == "AckIsolation") ask(dut, 1);
            if (mode == "Reconfigure") ask(dut, 1, 1);
            if (mode == "OwnedReply") ask(dut, 1);
            if (mode == "Fanout") ask(destination, 1, 0, L3Address(), false, false);
        }
        if (time == SimTime(0.11)) {
            if (mode == "Unselected") offer(destination, 99, 2);
            if (mode == "AckIsolation") ask(dut, 1, 1);
            if (mode == "Fanout") ask(destination, 1, 1, L3Address(), false, false);
        }
        if (time == SimTime(0.12)) {
            if (mode == "Fanout") ask(Ipv4Address("10.0.0.98"), 2, 0, L3Address(), false, false);
            if (mode == "HelloSources") { acknowledge(0); acknowledge(1); }
        }
        if (time == SimTime(0.13) && mode == "Fanout") ask(Ipv4Address("10.0.0.98"), 2, 1, L3Address(), false, false);
        if (time == SimTime(0.15)) {
            if (mode == "LocalAddresses") ask(dut, 2, 1, peer);
            if (mode == "Unselected") ask(dut, 7, 0, peer);
            if (mode == "Fanout") transmitError(destination, 21);
            if (mode == "MissingIngress") acknowledge(0, "interface-no-ingress");
            if (mode == "DownStart") {
                changeNode(new ModuleStartOperation());
            }
            if (mode == "LivenessIsolation") {
                auto packet = new Packet("interface-missing-data", makeShared<ByteCountChunk>(B(32)));
                packet->addTag<InterfaceReq>()->setInterfaceId(peerInterface(0)->getInterfaceId());
                packet->addTag<NextHopAddressReq>()->setNextHopAddress(dut);
                observationSocket.sendTo(packet, dut, 54321);
            }
        }
        if (time == SimTime(0.16) && mode == "Fanout") transmitError(Ipv4Address("10.0.0.98"), 31);
        if (time == SimTime(0.19) && mode == "Unselected") ASSERT(!findRoute(destination) && acknowledgments[2] == 0);
        if (time == SimTime(0.2)) {
            if (mode == "Unselected") offer(destination, 20, 0);
            if (mode == "AckIsolation") acknowledge(1);
            if (mode == "Reconfigure") changeNode(new ModuleStopOperation());
        }
        if (time == SimTime(0.21) && mode == "Reconfigure") {
            ASSERT(!operationPending);
            auto protocol = getModuleByPath("^.^.dut.aodv");
            if (protocol->hasPar("interfaces")) protocol->par("interfaces").setStringValue("wlan0 wlan2");
        }
        if ((time == SimTime(0.22) || time == SimTime(0.32) || time == SimTime(0.42)) && mode == "HelloSources") {
            acknowledge(0); acknowledge(1);
        }
        if (time == SimTime(0.25) && mode == "AckIsolation") ask(dut, 2);
        if (time == SimTime(0.25) && mode == "Reconfigure") { ASSERT(!operationPending); changeNode(new ModuleStartOperation()); }
        if (time == SimTime(0.26) && mode == "AckIsolation") ask(dut, 2, 1);
        if (time == SimTime(0.4) && mode == "Reconfigure") ask(dut, 4, 2);
        if (time == SimTime(0.42) && mode == "Reconfigure") ask(dut, 4, 1);
        if (time == SimTime(0.3) && mode == "Gateway") {
            bool found = false;
            for (int i = 0; i < routes->getNumRoutes(); i++) {
                auto route = routes->getRoute(i);
                if (route->getSource() == getModuleByPath("^.^.dut.aodv") && route->getPrefixLength() == 0) {
                    ASSERT(route->getInterface() == dutInterface(1) && route->getNextHopAsGeneric() == peerInterface(1)->getNetworkAddress());
                    ASSERT(check_and_cast<AodvRouteData *>(route->getProtocolData())->isActive()); found = true;
                }
            }
            ASSERT(found);
        }
        if (time == SimTime(0.3) && mode == "SecondaryScope")
            for (int i = 0; i < routes->getNumRoutes(); i++)
                ASSERT(routes->getRoute(i)->getSource() != getModuleByPath("^.^.dut.aodv") || routes->getRoute(i)->getPrefixLength() != 0);
        if (time == SimTime(1.4) || (time == SimTime(0.5) && mode != "Discovery" && mode != "DownStart" && mode != "Gateway")) { finishCase(); return; }
        const double times[] = {0.01,0.02,0.03,0.04,0.08,0.1,0.11,0.12,0.13,0.14,0.15,0.16,0.18,0.19,0.2,0.21,0.22,0.25,0.26,0.3,0.32,0.4,0.42,0.5,1,1.2,1.4};
        scheduleAt(times[++step], timer);
    }
    void socketDataArrived(UdpSocket *, Packet *packet) override {
        auto source = packet->getTag<L3AddressInd>()->getSrcAddress();
        auto target = packet->getTag<L3AddressInd>()->getDestAddress();
        auto interfaceId = packet->getTag<InterfaceInd>()->getInterfaceId();
        int receiver = -1;
        for (int i = 0; i < 4; i++) if (peerInterface(i)->getInterfaceId() == interfaceId) receiver = i;
        ASSERT(receiver >= 0);
        if (target.isBroadcast()) {
            auto network = dynamicPtrCast<const Ipv4Header>(packet->getTag<NetworkProtocolInd>()->getNetworkProtocolHeader());
            if (!broadcasts.emplace(source, network->getIdentification()).second) { delete packet; return; }
        }
        auto control = packet->peekAtFront<AodvControlPacket>();
        if (auto request = dynamicPtrCast<const Rreq>(control)) {
            requests.push_back({request->getDestAddr(), source, request->getOriginatorAddr(), request->getRreqId(), request->getOriginatorSeqNum(), simTime()});
            if ((mode == "Discovery" || mode == "DownStart" || mode == "Gateway" || mode == "SecondaryScope") && request->getDestAddr() == destination && source == dutAddress(1))
                offer(destination, 20, 1);
        }
        else if (auto reply = dynamicPtrCast<const Rrep>(control)) {
            if (reply->getOriginatorAddr().isUnspecified()) {
                int link = source == dutAddress(0) ? 0 : source == dutAddress(1) ? 1 : 2;
                ASSERT(reply->getDestAddr() == source && reply->getHopCount() == 0 && reply->getDestSeqNum() == 0);
                hellos[link].push_back(simTime());
            }
            else {
                replies.push_back({reply, source, receiver});
                if (reply->getAckRequiredFlag() && (mode != "AckIsolation" || receiver == 0)) acknowledge(receiver);
            }
        }
        else if (dynamicPtrCast<const RrepAck>(control)) {
            ASSERT(packet->getTag<HopLimitInd>()->getHopLimit() == 1);
            acknowledgments[receiver]++;
        }
        else if (auto error = dynamicPtrCast<const Rerr>(control)) {
            errors++; errorRecipients.insert(target);
            if (mode == "Fanout" || mode == "LivenessIsolation") {
                ASSERT(!error->getNoDeleteFlag() && error->getUnreachableNodesArraySize() == 1);
                ASSERT(error->getUnreachableNodes(0).addr == destination && error->getUnreachableNodes(0).seqNum == 21);
                ASSERT(source == dutAddress(linkFor(receiver)));
            }
        }
        delete packet;
    }
    void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *) override {
        Enter_Method("observeInterfaceData");
        auto packet = dynamic_cast<Packet *>(object);
        bool recipient = mode == "ExternalGateway" && source == recipientIp;
        if (!packet || (!recipient && source != getModuleByPath("^.ipv4.ip")) || signal != packetReceivedFromLowerSignal ||
                strncmp(packet->getName(), "queued-data-", 12)) return;
        auto destination = packet->peekAtFront<Ipv4Header>()->getDestAddress();
        dataArrivals.emplace_back(destination, simTime());
        if (mode == "ExternalGateway" || mode == "ExternalIngress")
            ASSERT(packet->getTag<MacAddressInd>()->getSrcAddress() == dutInterface(mode == "ExternalGateway" ? 2 : 0)->getMacAddress());
        if (mode == "Discovery" || mode == "DownStart" || mode == "Gateway" || mode == "SecondaryScope")
            ASSERT(packet->getTag<InterfaceInd>()->getInterfaceId() == peerInterface(1)->getInterfaceId());
    }
};
Define_Module(AodvInterfacesPeer);
#endif

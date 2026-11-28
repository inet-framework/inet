// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __AODV_HELLO_TEST_PEER_H
#define __AODV_HELLO_TEST_PEER_H
#include "AodvPeer.h"
#include "inet/networklayer/ipv4/Ipv4.h"
#include "inet/linklayer/common/MacAddressTag_m.h"

// Test adapter owns incoming packets before IP invokes any netfilter hook.
class AodvObservationIpv4 : public Ipv4
{
    void strip(Packet *packet) {
        std::string mode = par("observationMode").stdstringValue();
        bool data = !strcmp(packet->getName(), "hello-observation-data");
        bool hello = !strcmp(packet->getName(), "scripted-hello");
        if ((data && (mode == "MissingData" || mode == "MissingTie")) ||
                (hello && mode == "MissingInitial" && simTime() < SimTime(0.2)))
            packet->removeTagIfPresent<MacAddressInd>();
    }
  protected:
    void handleIncomingDatagram(Packet *packet) override {
        simtime_t delivery = par("observationDelivery");
        if (!strcmp(packet->getName(), "hello-observation-data") && delivery > simTime()) {
            scheduleAt(delivery, packet);
            return;
        }
        strip(packet);
        Ipv4::handleIncomingDatagram(packet);
    }
    void handleMessageWhenUp(cMessage *message) override {
        if (message->isSelfMessage()) {
            auto packet = check_and_cast<Packet *>(message);
            strip(packet);
            Ipv4::handleIncomingDatagram(packet);
        }
        else Ipv4::handleMessageWhenUp(message);
    }
};
Define_Module(AodvObservationIpv4);

class AodvHelloIngressFilter : public SimpleModule
{
  protected:
    void handleMessage(cMessage *message) override {
        if (!strcmp(message->getName(), "hello-metadata-failure")) {
            auto packet = check_and_cast<Packet *>(message);
            if (par("mode").stdstringValue() == "MissingInterface")
                packet->removeTagIfPresent<InterfaceInd>();
            else
                packet->addTagIfAbsent<InterfaceInd>()->setInterfaceId(INT_MAX);
        }
        send(message, "out");
    }
};
Define_Module(AodvHelloIngressFilter);

class AodvHelloPeer : public AodvPeer
{
    UdpSocket dataSocket;
    std::string mode;
    bool ethernet = false;
    int errors = 0;
    int validationReplies = 0;
    simtime_t calibrationIngress;
    simtime_t pathIngress;
    simtime_t dataIngress;
    simtime_t deadline;
    bool realChain() const { return hasPar("realChain") && par("realChain").boolValue(); }
    NetworkInterface *peerInterface(int index) {
        auto interfaces = check_and_cast<IInterfaceTable *>(getModuleByPath("^.interfaceTable"));
        return interfaces->findInterfaceByName(((ethernet ? "eth" : "wlan") + std::to_string(index)).c_str());
    }
    void control(const Ptr<AodvControlPacket>& header, const char *name, int index = 0, L3Address source = L3Address()) {
        auto output = peerInterface(index);
        auto packet = new Packet(name, header);
        packet->addTag<InterfaceReq>()->setInterfaceId(output->getInterfaceId());
        auto addresses = packet->addTag<L3AddressReq>();
        addresses->setSrcAddress(source.isUnspecified() ? output->getNetworkAddress() : source);
        addresses->setNonLocalSrcAddress(true); // scripted one-hop sender identity
        packet->addTag<NextHopAddressReq>()->setNextHopAddress(dut);
        packet->addTag<HopLimitReq>()->setHopLimit(10);
        socket.sendTo(packet, dut, controlPort);
    }
    void helloFrom(const L3Address& sender, uint32_t sequence, int index = 0, simtime_t lifetime = 5) {
        auto hello = makeShared<Rrep>();
        hello->setPacketType(RREP);
        hello->setChunkLength(B(20));
        hello->setDestAddr(sender);
        hello->setDestSeqNum(sequence);
        hello->setLifeTime(lifetime);
        control(hello, "scripted-hello", index, sender);
    }
    void offer(bool calibration = false) {
        auto reply = makeShared<Rrep>();
        reply->setPacketType(RREP);
        reply->setChunkLength(B(20));
        reply->setOriginatorAddr(dut);
        reply->setDestAddr(calibration ? L3Address(Ipv4Address("10.0.0.97")) : destination);
        reply->setDestSeqNum(20);
        reply->setLifeTime(5);
        reply->setHopCount(1);
        control(reply, calibration ? "hello-calibration" : "hello-path");
    }
    void data(int index = 0) {
        auto packet = new Packet("hello-observation-data", makeShared<ByteCountChunk>(B(32)));
        packet->addTag<InterfaceReq>()->setInterfaceId(peerInterface(index)->getInterfaceId());
        auto addresses = packet->addTag<L3AddressReq>();
        addresses->setSrcAddress(Ipv4Address("10.0.0.50"));
        addresses->setNonLocalSrcAddress(true); // replay the network header forwarded by N
        packet->addTag<NextHopAddressReq>()->setNextHopAddress(dut);
        dataSocket.sendTo(packet, dut, 54321);
    }
    bool tie() const { return mode.find("First") != std::string::npos || mode.find("TimeoutBefore") == 0; }
    bool early() const { return mode == "RefreshFirst" || mode == "MissingFirst"; }
    bool disabled() const {
        return mode == "MissingData" || mode == "MissingInitial" || mode == "Rebinding" ||
            mode == "Ambiguous" || mode == "NoArm" || mode == "ZeroAlone" || mode == "Disabled" || mode == "MissingFirst" ||
            mode == "MissingInterface" || mode == "UnknownInterface";
    }
    bool refreshed() const { return mode == "Data" || mode == "Source" || mode == "OtherControl" || mode == "StaleHello" || mode == "RefreshFirst" || mode == "Validation"; }
    void checkDestination(bool active) {
        auto route = findRoute(destination);
        ASSERT(route);
        auto state = check_and_cast<AodvRouteData *>(route->getProtocolData());
        ASSERT(state->isActive() == active && state->hasValidDestNum());
        ASSERT(state->getDestSeqNum() == (active ? 20u : 21u));
    }
  protected:
    void initialize(int stage) override {
        AodvPeer::initialize(stage);
        if (stage == INITSTAGE_APPLICATION_LAYER) {
            mode = par("scenario").stdstringValue();
            ethernet = par("ethernet");
            dataSocket.setOutputGate(gate("socketOut"));
            dataSocket.bind(23456);
            dutIp = getModuleByPath("^.^.dut.ipv4.ip");
            dutIp->subscribe(packetReceivedFromLowerSignal, this);
            if (realChain()) {
                auto table = check_and_cast<IRoutingTable *>(getModuleByPath("^.ipv4.routingTable"));
                auto route = table->createRoute();
                route->setDestination(dut);
                route->setPrefixLength(32);
                route->setNextHop(dut);
                route->setInterface(peerInterface(0));
                route->setMetric(1);
                route->setSourceType(IRoute::MANUAL);
                route->setSource(this);
                table->addRoute(route);
            }
        }
    }
    void handleMessage(cMessage *message) override {
        if (message != timer) { AodvPeer::handleMessage(message); return; }
        auto time = simTime();
        if (time == SimTime(0.01) && tie()) offer(true);
        if (time == SimTime(0.02) && tie()) {
            ASSERT(calibrationIngress > SimTime(0.01));
            deadline = SimTime(0.11) + (calibrationIngress - SimTime(0.01)) + SimTime(0.2);
            getModuleByPath("^.^.dut.ipv4.ip")->par("observationDelivery").setDoubleValue(deadline.dbl());
        }
        if (time == SimTime(0.03) && early()) data();
        if (time == SimTime(0.08) && mode == "Source" && !realChain()) helloFrom(Ipv4Address("10.0.0.50"), 100, 1);
        if (time == SimTime(0.1) && mode != "NoArm" && mode != "ZeroAlone") helloFrom(peer, 10);
        if (time == SimTime(0.11)) offer();
        if (time == SimTime(0.12) && !ethernet && mode != "NoPrecursor") {
            auto interfaces = check_and_cast<IInterfaceTable *>(getModuleByPath("^.^.dut.interfaceTable"));
            check_and_cast<AodvRouteData *>(findRoute(destination)->getProtocolData())->addPrecursor(Ipv4Address("10.0.0.3"), interfaces->findInterfaceByName("wlan0")->getInterfaceId());
        }
        if (time == SimTime(0.25)) {
            if (mode == "Data" || (mode == "Source" && !realChain()) || mode == "NoArm" || mode == "MissingData") data();
            if (mode == "UnknownSender") data(1);
            if (tie() && !early()) data();
            if (mode == "OtherControl") {
                auto ack = makeShared<RrepAck>(); ack->setPacketType(RREPACK); ack->setChunkLength(B(2)); control(ack, "scripted-ack");
            }
            if (mode == "MissingInterface" || mode == "UnknownInterface") {
                auto ack = makeShared<RrepAck>(); ack->setPacketType(RREPACK); ack->setChunkLength(B(2)); control(ack, "hello-metadata-failure");
            }
            if (mode == "StaleHello") helloFrom(peer, 9);
            if (mode == "ZeroAlone") helloFrom(peer, 10, 0, 0);
            if (mode == "Rebinding") helloFrom(peer, 10, 1);
            if (mode == "Ambiguous") helloFrom(Ipv4Address("10.0.0.50"), 100);
        }
        if (mode == "Validation" && time == SimTime(0.52)) {
            auto ack = makeShared<RrepAck>(); ack->setPacketType(RREPACK); ack->setChunkLength(B(2)); control(ack, "late-validation-ack");
        }
        if (mode == "Validation" && (time == SimTime(0.25) || time == SimTime(0.55))) {
            auto request = makeShared<Rreq>();
            request->setPacketType(RREQ); request->setChunkLength(B(24));
            request->setOriginatorAddr(peer); request->setOriginatorSeqNum(1);
            request->setRreqId(time == SimTime(0.25) ? 1 : 2);
            request->setDestAddr(dut); request->setUnknownSeqNumFlag(true);
            control(request, "hello-validation-request");
        }
        if (time == SimTime(0.3) && (mode == "MissingData" || mode == "MissingInitial" || mode == "MissingInterface" || mode == "UnknownInterface")) helloFrom(peer, 10);
        if (time == SimTime(0.35)) {
            if (tie()) ASSERT(dataIngress == deadline && pathIngress + SimTime(0.2) == deadline);
            checkDestination(disabled() || refreshed());
            if (mode == "Source") {
                auto state = check_and_cast<AodvRouteData *>(findRoute(Ipv4Address("10.0.0.50"))->getProtocolData());
                ASSERT(!state->isActive() && state->getDestSeqNum() == 101);
            }
            if (mode == "StaleHello") {
                auto state = check_and_cast<AodvRouteData *>(findRoute(peer)->getProtocolData());
                ASSERT(state->getDestSeqNum() == 10);
            }
        }
        if (time == SimTime(0.6)) {
            if (mode == "Validation") ASSERT(validationReplies == 1);
            checkDestination(disabled());
            ASSERT(errors == ((!disabled() && !ethernet && mode != "NoPrecursor") ? 1 : 0));
            pass("HelloLoss" + mode);
            return;
        }
        const double times[] = {0.01, 0.02, 0.03, 0.08, 0.1, 0.11, 0.12, 0.2, 0.25, 0.3, 0.35, 0.52, 0.55, 0.6};
        scheduleAt(times[++step], timer);
    }
    void socketDataArrived(UdpSocket *, Packet *packet) override {
        if (auto error = dynamicPtrCast<const Rerr>(packet->peekAtFront<AodvControlPacket>())) {
            errors++;
            ASSERT(!error->getNoDeleteFlag() && error->getUnreachableNodesArraySize() == 1);
            ASSERT(error->getUnreachableNodes(0).addr == destination && error->getUnreachableNodes(0).seqNum == 21);
            ASSERT(packet->getTag<L3AddressInd>()->getDestAddress() == Ipv4Address("10.0.0.3"));
        }
        else if (auto reply = dynamicPtrCast<const Rrep>(packet->peekAtFront<AodvControlPacket>())) {
            if (reply->getDestAddr() == dut && reply->getAckRequiredFlag()) validationReplies++;
        }
        delete packet;
    }
    void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *) override {
        Enter_Method("observeHelloIngress");
        if (source != dutIp || signal != packetReceivedFromLowerSignal) return;
        auto packet = dynamic_cast<Packet *>(object);
        if (!packet) return;
        if (!strcmp(packet->getName(), "hello-calibration")) calibrationIngress = simTime();
        if (!strcmp(packet->getName(), "hello-path")) pathIngress = simTime();
        if (!strcmp(packet->getName(), "hello-observation-data")) {
            dataIngress = simTime();
            ASSERT(packet->peekAtFront<Ipv4Header>()->getSrcAddress() == Ipv4Address("10.0.0.50"));
            if (realChain())
                ASSERT(packet->getTag<MacAddressInd>()->getSrcAddress() == peerInterface(0)->getMacAddress());
        }
    }
};
Define_Module(AodvHelloPeer);

// A real source sends data over Ethernet to N; N forwards it over its ad hoc link.
class AodvForwardedSource : public AodvPeer
{
    UdpSocket dataSocket;
  protected:
    void initialize(int stage) override {
        AodvPeer::initialize(stage);
        if (stage == INITSTAGE_APPLICATION_LAYER) {
            peer = Ipv4Address("10.0.0.50");
            dataSocket.setOutputGate(gate("socketOut"));
            dataSocket.bind(23456);
        }
    }
    void handleMessage(cMessage *message) override {
        if (message != timer) { AodvPeer::handleMessage(message); return; }
        if (step++ == 0) {
            hello(100, 5);
            scheduleAt(0.25, timer);
        }
        else {
            auto interfaces = check_and_cast<IInterfaceTable *>(getModuleByPath("^.interfaceTable"));
            auto packet = new Packet("hello-observation-data", makeShared<ByteCountChunk>(B(32)));
            packet->addTag<InterfaceReq>()->setInterfaceId(interfaces->findInterfaceByName("eth0")->getInterfaceId());
            packet->addTag<NextHopAddressReq>()->setNextHopAddress(Ipv4Address("10.9.0.1"));
            packet->addTag<L3AddressReq>()->setSrcAddress(peer);
            dataSocket.sendTo(packet, dut, 54321);
        }
    }
};
Define_Module(AodvForwardedSource);
#endif

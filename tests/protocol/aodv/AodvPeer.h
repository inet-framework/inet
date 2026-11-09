// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __AODV_TEST_PEER_H
#define __AODV_TEST_PEER_H

#include "inet/common/SimpleModule.h"
#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/NextHopAddressTag_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/IRoutingTable.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/routing/aodv/AodvControlPackets_m.h"
#include "inet/routing/aodv/AodvRouteData.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

using namespace inet;
using namespace inet::aodv;

// A scripted wire peer. It never invokes Aodv handlers or reproduces route selection.
class AodvPeer : public SimpleModule, public UdpSocket::ICallback, public cListener
{
  protected:
    UdpSocket socket;
    cMessage *timer = nullptr;
    int step = 0;
    int replies = 0;
    uint32_t replySequence = 0;
    int forwardedRequests = 0;
    uint32_t forwardedSequence = 0;
    bool forwardedUnknown = false;
    int emittedData = 0;
    IRoutingTable *routes = nullptr;
    L3Address peer = Ipv4Address("10.0.0.2");
    L3Address destination = Ipv4Address("10.0.0.99");
    L3Address dut = Ipv4Address("10.0.0.1");

    int numInitStages() const override { return NUM_INIT_STAGES; }
    void initialize(int stage) override {
        if (stage != INITSTAGE_APPLICATION_LAYER)
            return;
        socket.setOutputGate(gate("socketOut"));
        socket.setCallback(this);
        socket.bind(654);
        socket.setBroadcast(true);
        routes = check_and_cast<IRoutingTable *>(getModuleByPath("^.^.dut.ipv4.routingTable"));
        getModuleByPath("^.ipv4.ip")->subscribe(packetReceivedFromLowerSignal, this);
        timer = new cMessage("script");
        scheduleAt(0.1, timer);
    }
    void transmit(const Ptr<AodvControlPacket>& control) {
        auto packet = new Packet("scripted-aodv", control);
        auto interfaces = check_and_cast<IInterfaceTable *>(getModuleByPath("^.interfaceTable"));
        packet->addTag<InterfaceReq>()->setInterfaceId(interfaces->findInterfaceByName("wlan0")->getInterfaceId());
        packet->addTag<HopLimitReq>()->setHopLimit(10);
        packet->addTag<NextHopAddressReq>()->setNextHopAddress(dut);
        socket.sendTo(packet, dut, 654);
    }
    void request(uint32_t sequence, uint32_t originatorSequence = 1, bool unknown = false) {
        auto request = makeShared<Rreq>();
        request->setPacketType(RREQ);
        request->setChunkLength(B(24));
        request->setOriginatorAddr(peer);
        request->setOriginatorSeqNum(originatorSequence);
        request->setRreqId(step + 1);
        request->setDestAddr(dut);
        request->setDestSeqNum(sequence);
        request->setUnknownSeqNumFlag(unknown);
        transmit(request);
    }
    void reply(uint32_t sequence, simtime_t lifetime = 5, unsigned int hops = 0) {
        auto reply = makeShared<Rrep>();
        reply->setPacketType(RREP);
        reply->setChunkLength(B(20));
        reply->setOriginatorAddr(dut);
        reply->setDestAddr(destination);
        reply->setDestSeqNum(sequence);
        reply->setLifeTime(lifetime);
        reply->setHopCount(hops);
        transmit(reply);
    }
    void requestFloor(const L3Address& target, bool unknown, uint32_t sequence) {
        auto request = makeShared<Rreq>();
        request->setPacketType(RREQ);
        request->setChunkLength(B(24));
        request->setOriginatorAddr(Ipv4Address("10.0.0.50"));
        request->setOriginatorSeqNum(1);
        request->setRreqId(step + 1);
        request->setDestAddr(target);
        request->setDestSeqNum(sequence);
        request->setUnknownSeqNumFlag(unknown);
        request->setDestOnlyFlag(true);
        transmit(request);
    }
    void hello(uint32_t sequence, simtime_t lifetime) {
        auto message = makeShared<Rrep>();
        message->setPacketType(RREP);
        message->setChunkLength(B(20));
        message->setDestAddr(peer);
        message->setDestSeqNum(sequence);
        message->setLifeTime(lifetime);
        transmit(message);
    }
    void sendError(uint32_t sequence) {
        auto message = makeShared<Rerr>();
        message->setPacketType(RERR);
        message->setChunkLength(B(12));
        message->setUnreachableNodesArraySize(1);
        UnreachableNode node;
        node.addr = destination;
        node.seqNum = sequence;
        message->setUnreachableNodes(0, node);
        transmit(message);
    }
    void checkFloor(bool unknown, uint32_t sequence) {
        ASSERT(forwardedRequests > 0);
        ASSERT(forwardedUnknown == unknown && forwardedSequence == sequence);
        forwardedRequests = 0;
    }
    IRoute *findRoute(const L3Address& address) {
        for (int i = 0; i < routes->getNumRoutes(); i++) {
            auto route = routes->getRoute(i);
            if (route->getSourceType() == IRoute::AODV && route->getDestinationAsGeneric() == address &&
                    route->getPrefixLength() == 32)
                return route;
        }
        return nullptr;
    }
    void handleMessage(cMessage *message) override {
        if (message != timer) {
            socket.processMessage(message);
            return;
        }
        const std::string scenario = par("scenario").stdstringValue();
        if (scenario != "Control") {
            runScenario(scenario);
            return;
        }
        if (step == 0) {
            if (scenario == "Control")
                request(0, 1, true);
            else
                throw cRuntimeError("Unknown AODV scenario");
            step++;
            scheduleAfter(0.05, timer);
        }
        else {
            ASSERT(replies == 1);
            ASSERT(replySequence == 0);
            auto reverse = findRoute(peer);
            ASSERT(reverse && reverse->getNextHopAsGeneric() == peer && reverse->getMetric() == 1);
            auto data = check_and_cast<AodvRouteData *>(reverse->getProtocolData());
            ASSERT(data->isActive() && data->hasValidDestNum() && data->getDestSeqNum() == 1);
            std::cout << "AODV " << scenario << ": PASS\n";
        }
    }
    void checkSequence(const L3Address& address, uint32_t expected, bool active = true, unsigned int hops = 1) {
        auto route = findRoute(address);
        ASSERT(route);
        auto data = check_and_cast<AodvRouteData *>(route->getProtocolData());
        ASSERT(data->hasValidDestNum() && data->getDestSeqNum() == expected);
        ASSERT(data->isActive() == active);
        ASSERT(route->getNextHopAsGeneric() == peer && unsigned(route->getMetric()) == hops);
    }
    void runScenario(const std::string& scenario) {
        if (scenario == "SequenceFreshness") {
            const uint32_t input[] = {0xfffffffeu, 0xffffffffu, 0, 0xffffffffu,
                0x7fffffffu, 0x80000000u, 0, 0x7fffffffu, 0x80000000u};
            const uint32_t expected[] = {0xfffffffeu, 0xffffffffu, 0, 0,
                0x7fffffffu, 0x80000000u, 0x80000000u, 0x80000000u, 0x80000000u};
            if (step > 0)
                checkSequence(destination, expected[step - 1]);
            if (step < 9)
                reply(input[step]);
            else
                return pass(scenario);
        }
        else if (scenario == "ReverseRoute") {
            const uint32_t input[] = {0xfffffffeu, 0, 0xffffffffu, 0x7fffffffu, 0x80000000u, 0x7fffffffu};
            const uint32_t expected[] = {0xfffffffeu, 0, 0, 0x7fffffffu, 0x80000000u, 0x80000000u};
            if (step == 0)
                reply(13);
            else {
                if (step == 1) {
                    auto neighbor = findRoute(peer);
                    ASSERT(neighbor && !check_and_cast<AodvRouteData *>(neighbor->getProtocolData())->hasValidDestNum());
                }
                else
                    checkSequence(peer, expected[step - 2]);
                if (step <= 6)
                    request(0, input[step - 1], step == 2 || step == 3);
                else
                    return pass(scenario);
            }
        }
        else if (scenario == "DestinationSequence") {
            const uint32_t input[] = {1, 5, 4, 5, 6, 0x80000006u, 0};
            const uint32_t expected[] = {1, 5, 5, 5, 6, 6, 6};
            if (step > 0) {
                ASSERT(replies == step);
                ASSERT(replySequence == expected[step - 1]);
            }
            if (step < 7)
                request(input[step], 1, step == 6);
            else
                return pass(scenario);
        }
        else if (scenario == "RequestFloors") {
            switch (step) {
                case 0: requestFloor(destination, true, 0xffffffffu); break;
                case 1: checkFloor(true, 0xffffffffu); requestFloor(destination, false, 5); break;
                case 2: checkFloor(false, 5); requestFloor(peer, true, 0xfffffffeu); break;
                case 3: checkFloor(true, 0xfffffffeu); requestFloor(peer, false, 5); break;
                case 4: checkFloor(false, 5); reply(0xfffffffeu); break;
                case 5: checkSequence(destination, 0xfffffffeu); requestFloor(destination, true, 0); break;
                case 6: checkFloor(false, 0xfffffffeu); requestFloor(destination, false, 0); break;
                case 7: checkFloor(false, 0); requestFloor(destination, false, 0xfffffffdu); break;
                case 8: checkFloor(false, 0xfffffffeu); requestFloor(destination, false, 0x7ffffffeu); break;
                case 9: checkFloor(false, 0x7ffffffeu); requestFloor(destination, false, 0xfffffffeu); break;
                case 10: checkFloor(false, 0xfffffffeu); checkSequence(destination, 0xfffffffeu); reply(0); break;
                case 11: checkSequence(destination, 0); requestFloor(destination, false, 0xfffffffeu); break;
                default: checkFloor(false, 0); checkSequence(destination, 0); return pass(scenario);
            }
        }
        else if (scenario == "ReplyAcceptance") {
            switch (step) {
                case 0: reply(20, 0.02); break;
                case 1: checkSequence(destination, 20, false); break;
                case 2:
                    checkSequence(destination, 20, false);
                    ASSERT(emittedData == 0 && forwardedRequests == 1);
                    reply(20, 5, 2);
                    break;
                default:
                    checkSequence(destination, 20, true, 3);
                    ASSERT(emittedData == 1 && forwardedRequests == 1);
                    return pass(scenario);
            }
        }
        else if (scenario == "HelloAcceptance") {
            switch (step) {
                case 0: hello(100, 0); break;
                case 1: ASSERT(!findRoute(peer)); hello(0xfffffffeu, 5); break;
                case 2: checkSequence(peer, 0xfffffffeu); hello(0, 5); break;
                case 3: checkSequence(peer, 0); hello(0xffffffffu, 5); break;
                case 4: checkSequence(peer, 0); hello(0x80000000u, 5); break;
                case 5: checkSequence(peer, 0); hello(0, 5); break;
                default: checkSequence(peer, 0); return pass(scenario);
            }
        }
        else if (scenario == "ErrorFreshness") {
            switch (step) {
                case 0: reply(0xfffffffeu); break;
                case 1: checkSequence(destination, 0xfffffffeu); sendError(0xffffffffu); break;
                case 2: checkSequence(destination, 0xffffffffu, false); reply(0); break;
                case 3: checkSequence(destination, 0); sendError(0xffffffffu); break;
                case 4: checkSequence(destination, 0); sendError(0x80000000u); break;
                case 5: checkSequence(destination, 0); sendError(0); break;
                default: checkSequence(destination, 0, false); return pass(scenario);
            }
        }
        else if (scenario == "NeighborSequence") {
            destination = peer;
            if (step == 0)
                reply(20, 0.02);
            else if (step == 1) {
                checkSequence(peer, 20, false);
                reply(19);
            }
            else if (step == 2) {
                checkSequence(peer, 20, false);
                reply(20);
            }
            else {
                checkSequence(peer, 20);
                return pass(scenario);
            }
        }
        else
            throw cRuntimeError("Unknown AODV scenario: %s", scenario.c_str());
        step++;
        scheduleAfter(0.05, timer);
    }
    void pass(const std::string& scenario) { std::cout << "AODV " << scenario << ": PASS\n"; }
    void socketDataArrived(UdpSocket *, Packet *packet) override {
        auto control = packet->peekAtFront<AodvControlPacket>();
        if (auto reply = dynamicPtrCast<const Rrep>(control)) {
            replies++;
            replySequence = reply->getDestSeqNum();
            if (reply->getAckRequiredFlag()) {
                auto ack = makeShared<RrepAck>();
                ack->setPacketType(RREPACK);
                transmit(ack);
            }
        }
        else if (auto request = dynamicPtrCast<const Rreq>(control)) {
            forwardedRequests++;
            forwardedUnknown = request->getUnknownSeqNumFlag();
            forwardedSequence = request->getDestSeqNum();
            if (par("scenario").stdstringValue() == "ReplyAcceptance")
                this->reply(19);
        }
        delete packet;
    }
    void socketErrorArrived(UdpSocket *, Indication *indication) override { delete indication; }
    void socketClosed(UdpSocket *) override {}
    void receiveSignal(cComponent *, simsignal_t, cObject *value, cObject *) override {
        Enter_Method("observeData");
        auto packet = dynamic_cast<Packet *>(value);
        if (packet && packet->peekAtFront<Ipv4Header>()->getDestAddress() == destination.toIpv4())
            emittedData++;
    }
    void finish() override {
        if (par("scenario").stdstringValue() == "ReplyAcceptance")
            ASSERT(emittedData == 1 && forwardedRequests == 1);
    }
    using cListener::finish;
  public:
    ~AodvPeer() { cancelAndDelete(timer); }
};
Define_Module(AodvPeer);

class AodvDataSource : public SimpleModule
{
    cMessage *timer = nullptr;
    UdpSocket socket;
  protected:
    int numInitStages() const override { return NUM_INIT_STAGES; }
    void initialize(int stage) override {
        if (stage == INITSTAGE_APPLICATION_LAYER) {
            socket.setOutputGate(gate("socketOut"));
            socket.bind(12345);
            timer = new cMessage("data");
            scheduleAt(0.16, timer);
        }
    }
    void handleMessage(cMessage *message) override {
        if (message != timer) {
            delete message;
            return;
        }
        auto packet = new Packet("queued-data", makeShared<ByteCountChunk>(B(10)));
        socket.sendTo(packet, Ipv4Address("10.0.0.99"), 12345);
    }
  public:
    ~AodvDataSource() { cancelAndDelete(timer); }
};
Define_Module(AodvDataSource);
#endif

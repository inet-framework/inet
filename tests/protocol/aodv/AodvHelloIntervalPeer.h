// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __AODV_HELLO_INTERVAL_TEST_PEER_H
#define __AODV_HELLO_INTERVAL_TEST_PEER_H

#include "AodvPeer.h"
#include "inet/common/lifecycle/LifecycleController.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/routing/aodv/AodvControlPacketsSerializer.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include <vector>

class AodvHelloIntervalWire : public AodvControlPacketsSerializer
{
  public:
    using AodvControlPacketsSerializer::serializeFields;
};

// All changes reach AODV via UDP/IP or public lifecycle operations. Public
// route inspection observes results; no AODV method or private timer is called.
class AodvHelloIntervalPeer : public AodvPeer, public LifecycleController, public IDoneCallback
{
    UdpSocket dataSocket;
    std::string mode;
    int preservedRequests = 0;
    int preservedReplies = 0;
    std::vector<simtime_t> firstHelloTimes;
    std::vector<simtime_t> secondHelloTimes;
    cModule *firstUdp = nullptr;
    cModule *secondUdp = nullptr;
    bool operationComplete = true;

    L3Address address(int index) const { return Ipv4Address(index ? "10.0.0.3" : "10.0.0.2"); }
    NetworkInterface *output(int index) {
        auto table = check_and_cast<IInterfaceTable *>(getModuleByPath("^.interfaceTable"));
        return table->findInterfaceByName(("wlan" + std::to_string(index)).c_str());
    }
    void control(const Ptr<AodvControlPacket>& header, int index = 0,
            L3Address target = L3Address()) {
        Ptr<const Chunk> body = header;
        if (mode == "Malformed" || mode == "UnknownExtensions" || mode == "Forwarding") {
            AodvHelloIntervalWire serializer;
            MemoryOutputStream stream;
            serializer.serializeFields(stream, header);
            body = makeShared<BytesChunk>(stream.getData());
        }
        auto packet = new Packet("hello-interval-control", body);
        packet->addTag<InterfaceReq>()->setInterfaceId(output(index)->getInterfaceId());
        packet->addTag<L3AddressReq>()->setSrcAddress(address(index));
        packet->addTag<NextHopAddressReq>()->setNextHopAddress(target.isUnspecified() ? dut : target);
        packet->addTag<HopLimitReq>()->setHopLimit(10);
        socket.sendTo(packet, target.isUnspecified() ? dut : target, controlPort);
    }
    template<typename T>
    void extensions(const Ptr<T>& header, const std::vector<uint8_t>& bytes, B baseLength) {
        header->setExtensionsArraySize(bytes.size());
        for (size_t i = 0; i < bytes.size(); ++i)
            header->setExtensions(i, bytes[i]);
        header->setChunkLength(baseLength + B(bytes.size()));
    }
    std::vector<uint8_t> intervalBytes(uint32_t milliseconds) {
        return {1, 4, uint8_t(milliseconds >> 24), uint8_t(milliseconds >> 16),
            uint8_t(milliseconds >> 8), uint8_t(milliseconds)};
    }
    void helloFrom(int index, const std::vector<uint8_t>& bytes, uint32_t sequence = 10,
            L3Address target = L3Address()) {
        auto header = makeShared<Rrep>();
        header->setPacketType(RREP);
        header->setDestAddr(address(index));
        header->setDestSeqNum(sequence);
        header->setLifeTime(5);
        extensions(header, bytes, B(20));
        control(header, index, target);
    }
    void path(int index = 0, L3Address target = L3Address()) {
        auto header = makeShared<Rrep>();
        header->setPacketType(RREP);
        header->setChunkLength(B(20));
        header->setOriginatorAddr(target.isUnspecified() ? dut : target);
        header->setDestAddr(index ? Ipv4Address("10.0.0.98") : Ipv4Address("10.0.0.99"));
        header->setDestSeqNum(20);
        header->setHopCount(1);
        header->setLifeTime(5);
        control(header, index, target);
    }
    void checkPath(int index, bool active) {
        auto route = findRoute(index ? L3Address(Ipv4Address("10.0.0.98")) : destination);
        ASSERT(route);
        auto data = check_and_cast<AodvRouteData *>(route->getProtocolData());
        ASSERT(data->isActive() == active);
        ASSERT(data->getDestSeqNum() == (active ? 20u : 21u));
    }
    void sendData() {
        auto packet = new Packet("hello-interval-data", makeShared<ByteCountChunk>(B(16)));
        packet->addTag<InterfaceReq>()->setInterfaceId(output(0)->getInterfaceId());
        packet->addTag<L3AddressReq>()->setSrcAddress(address(0));
        packet->addTag<NextHopAddressReq>()->setNextHopAddress(dut);
        dataSocket.sendTo(packet, dut, 54321);
    }
    void requestFrom(int index, const std::vector<uint8_t>& bytes) {
        auto header = makeShared<Rreq>();
        header->setPacketType(RREQ);
        header->setOriginatorAddr(Ipv4Address("10.0.0.50"));
        header->setOriginatorSeqNum(10);
        header->setRreqId(900);
        header->setDestAddr(Ipv4Address("10.0.0.77"));
        header->setUnknownSeqNumFlag(true);
        header->setDestOnlyFlag(true);
        extensions(header, bytes, B(24));
        control(header, index);
    }
    void operate(LifecycleOperation *operation) {
        LifecycleOperation::StringMap parameters;
        operation->initialize(getModuleByPath("^.^.dut.aodv"), parameters);
        ASSERT(parameters.empty());
        operationComplete = false;
        if (initiateOperation(operation, this)) operationComplete = true;
    }
    void checkNativeHello(const Ptr<const Rrep>& header, bool second) {
        uint32_t milliseconds = second ? 2 : 1;
        ASSERT(header->getExtensionsArraySize() == 6);
        auto bytes = intervalBytes(milliseconds);
        for (size_t i = 0; i < bytes.size(); ++i)
            ASSERT(header->getExtensions(i) == bytes[i]);
        ASSERT(header->getLifeTime() >= SimTime(2 * milliseconds, SIMTIME_MS));
        AodvHelloIntervalWire serializer;
        MemoryOutputStream stream;
        serializer.serializeFields(stream, header);
        const auto& encoded = stream.getData();
        ASSERT(encoded.size() == 26 && encoded[0] == 2);
        // Independent wire oracle: the extension follows the 20-byte IPv4 RREP.
        ASSERT(encoded[20] == 1 && encoded[21] == 4);
        ASSERT(encoded[22] == 0 && encoded[23] == 0 && encoded[24] == 0 && encoded[25] == milliseconds);
        (second ? secondHelloTimes : firstHelloTimes).push_back(simTime());
    }
  protected:
    void initialize(int stage) override {
        AodvPeer::initialize(stage);
        if (stage != INITSTAGE_APPLICATION_LAYER)
            return;
        mode = par("scenario").stdstringValue();
        dataSocket.setOutputGate(gate("socketOut"));
        dataSocket.setCallback(this);
        dataSocket.bind(23456);
        if (mode == "Rounding") {
            firstUdp = getModuleByPath("^.^.dut.udp");
            secondUdp = getModuleByPath("^.^.dut2.udp");
            firstUdp->subscribe(packetSentToLowerSignal, this);
            secondUdp->subscribe(packetSentToLowerSignal, this);
        }
    }
    void handleMessage(cMessage *message) override {
        if (message != timer) {
            if (dataSocket.belongsToSocket(message)) dataSocket.processMessage(message);
            else AodvPeer::handleMessage(message);
            return;
        }
        simtime_t now = simTime();
        if (mode == "Rounding") {
            if (now == SimTime(0.01)) {
                path();
                path(0, Ipv4Address("10.0.0.4"));
            }
            if (now == SimTime(0.08)) {
                ASSERT(firstHelloTimes.size() > 20 && secondHelloTimes.size() > 20);
                for (size_t i = 1; i < firstHelloTimes.size(); ++i)
                    ASSERT(firstHelloTimes[i] - firstHelloTimes[i - 1] == SimTime(0.00075));
                for (size_t i = 1; i < secondHelloTimes.size(); ++i)
                    ASSERT(secondHelloTimes[i] - secondHelloTimes[i - 1] == SimTime(0.0015));
                pass("HelloIntervalExtension" + mode);
                return;
            }
        }
        else if (mode == "Bounds") {
            if (now == SimTime(0.1)) helloFrom(0, intervalBytes(UINT32_MAX), 50);
            if (now == SimTime(0.11)) helloFrom(0, intervalBytes(UINT32_MAX), 50, Ipv4Address("10.0.0.4"));
            if (now == SimTime(0.3)) {
                auto route = findRoute(peer);
                ASSERT(route && check_and_cast<AodvRouteData *>(route->getProtocolData())->getDestSeqNum() == 50);
                auto table = check_and_cast<IRoutingTable *>(getModuleByPath("^.^.dut2.ipv4.routingTable"));
                for (int i = 0; i < table->getNumRoutes(); ++i)
                    ASSERT(table->getRoute(i)->getSourceType() != IRoute::AODV ||
                        table->getRoute(i)->getDestinationAsGeneric() != peer);
                pass("HelloIntervalExtension" + mode);
                return;
            }
        }
        else if (mode == "Forwarding") {
            if (now == SimTime(0.05)) requestFrom(1, {});
            if (now == SimTime(0.1)) {
                auto header = makeShared<Rrep>();
                header->setPacketType(RREP);
                header->setOriginatorAddr(Ipv4Address("10.0.0.50"));
                header->setDestAddr(destination);
                header->setDestSeqNum(20);
                header->setLifeTime(5);
                extensions(header, {55, 2, 7, 8}, B(20));
                control(header);
            }
            if (now == SimTime(0.3)) {
                ASSERT(preservedReplies == 1);
                pass("HelloIntervalExtension" + mode);
                return;
            }
        }
        else {
            if (now == SimTime(0.1)) {
                auto bytes = mode == "LegacyFallback" ? std::vector<uint8_t>{} :
                    intervalBytes(mode == "DataRefresh" || mode == "Restart" ? 300 : 100);
                if (mode == "UnknownExtensions") bytes.insert(bytes.begin(), {55, 3, 1, 2, 3});
                helloFrom(0, bytes);
            }
            if (now == SimTime(0.11)) path();
            if (now == SimTime(0.12) && mode == "DifferentIntervals") helloFrom(1, intervalBytes(300));
            if (now == SimTime(0.13) && mode == "DifferentIntervals") path(1);
            if (now == SimTime(0.2) && mode == "Restart") {
                operate(new ModuleStopOperation());
            }
            if (now == SimTime(0.24) && mode == "Restart") {
                ASSERT(operationComplete);
                operate(new ModuleStartOperation());
            }
            if (now == SimTime(0.2) && mode == "UnknownExtensions") requestFrom(0, {55, 3, 1, 2, 3});
            if (mode == "Malformed") {
                if (now == SimTime(0.24)) helloFrom(0, {1, 4, 0, 0}, 999);
                if (now == SimTime(0.25)) helloFrom(0, {1, 4, 0, 0, 0, 0}, 999);
                if (now == SimTime(0.26)) helloFrom(0, {128, 0}, 999);
                if (now == SimTime(0.27)) helloFrom(0, {0, 0}, 999);
                if (now == SimTime(0.28)) helloFrom(0, {1, 4, 0, 0, 0, 100, 1, 4, 0, 0, 0, 100}, 999);
            }
            if (now == SimTime(0.25) && mode == "Restart") {
                ASSERT(operationComplete);
                helloFrom(0, {});
            }
            if (now == SimTime(0.26) && mode == "Restart") path();
            if (now == SimTime(0.3) && mode == "Malformed") {
                auto route = findRoute(peer);
                ASSERT(route && check_and_cast<AodvRouteData *>(route->getProtocolData())->getDestSeqNum() == 10);
            }
            if (now == SimTime(0.4) && (mode == "DifferentIntervals" || mode == "LegacyFallback" || mode == "Malformed")) {
                checkPath(0, false);
                if (mode == "DifferentIntervals") checkPath(1, true);
            }
            if (now == SimTime(0.45) && mode == "UnknownExtensions") {
                checkPath(0, false);
                ASSERT(preservedRequests > 0);
            }
            if (now == SimTime(0.5) && mode == "DataRefresh") sendData();
            if (now == SimTime(0.55) && mode == "Restart") checkPath(0, false);
            if (now == SimTime(0.8)) {
                if (mode == "DifferentIntervals") checkPath(1, false);
                if (mode == "DataRefresh") checkPath(0, true);
            }
            if (now == SimTime(1.2)) {
                if (mode == "Restart") ASSERT(findRoute(destination) == nullptr);
                else checkPath(0, false);
                pass("HelloIntervalExtension" + mode);
                return;
            }
        }
        const double times[] = {0.01, 0.05, 0.08, 0.1, 0.11, 0.12, 0.13, 0.2,
            0.24, 0.25, 0.26, 0.27, 0.28, 0.3, 0.4, 0.45, 0.5, 0.55, 0.8, 1.2};
        scheduleAt(times[++step], timer);
    }
    void socketDataArrived(UdpSocket *, Packet *packet) override {
        auto header = packet->peekAtFront<AodvControlPacket>();
        if (auto request = dynamicPtrCast<const Rreq>(header)) {
            if (mode == "UnknownExtensions" && request->getOriginatorAddr() == Ipv4Address("10.0.0.50")) {
                ASSERT(request->getExtensionsArraySize() == 5);
                const uint8_t expected[] = {55, 3, 1, 2, 3};
                for (size_t i = 0; i < 5; ++i) ASSERT(request->getExtensions(i) == expected[i]);
                ++preservedRequests;
            }
        }
        else if (auto reply = dynamicPtrCast<const Rrep>(header)) {
            if (mode == "Forwarding" && reply->getOriginatorAddr() == Ipv4Address("10.0.0.50") &&
                    reply->getDestAddr() == destination) {
                ASSERT(reply->getExtensionsArraySize() == 4 && reply->getHopCount() == 1);
                const uint8_t expected[] = {55, 2, 7, 8};
                for (size_t i = 0; i < 4; ++i) ASSERT(reply->getExtensions(i) == expected[i]);
                ASSERT(packet->getTag<L3AddressInd>()->getDestAddress() == address(1));
                ++preservedReplies;
            }
        }
        delete packet;
    }
    void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *) override {
        if (mode != "Rounding" || signal != packetSentToLowerSignal ||
                (source != firstUdp && source != secondUdp)) return;
        auto packet = dynamic_cast<Packet *>(object);
        if (!packet) return;
        auto header = packet->peekDataAt<AodvControlPacket>(B(8));
        auto reply = dynamicPtrCast<const Rrep>(header);
        if (reply && reply->getOriginatorAddr().isUnspecified())
            checkNativeHello(reply, source == secondUdp);
    }
  public:
    void invoke() override { operationComplete = true; }
};
Define_Module(AodvHelloIntervalPeer);

#endif

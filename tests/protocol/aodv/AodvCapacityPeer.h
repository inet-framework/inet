// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __AODV_ERROR_CAPACITY_TEST_PEER_H
#define __AODV_ERROR_CAPACITY_TEST_PEER_H
#include "AodvPeer.h"
#include "inet/routing/aodv/AodvControlPacketsSerializer.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"

class AodvErrorWireSerializer : public AodvControlPacketsSerializer
{
  public:
    using AodvControlPacketsSerializer::serializeFields;
    using AodvControlPacketsSerializer::deserializeFields;
};

class AodvCapacityPeer : public AodvPeer
{
    int count = 0;
    int failures = 0;
    std::map<L3Address, std::map<L3Address, uint32_t>> received;
    std::map<L3Address, int> messages;
    L3Address target(int i) { return Ipv4Address(0x0a020001u + i); }
    void seedRecords() {
        auto interfaces = check_and_cast<IInterfaceTable *>(getModuleByPath("^.^.dut.interfaceTable"));
        for (int i = 0; i < count; i++) {
            auto route = routes->createRoute();
            route->setDestination(target(i));
            route->setPrefixLength(32);
            route->setNextHop(peer);
            route->setInterface(interfaces->findInterfaceByName("wlan0"));
            route->setMetric(2);
            route->setSourceType(IRoute::AODV);
            route->setSource(getModuleByPath("^.^.dut.aodv"));
            auto data = new AodvRouteData();
            data->setLifeTime(5);
            data->setDestSeqNum(1000 + i);
            data->addPrecursor(Ipv4Address("10.0.0.3"), interfaces->findInterfaceByName("wlan0")->getInterfaceId());
            data->addPrecursor(Ipv4Address("10.1.0.4"), interfaces->findInterfaceByName("wlan1")->getInterfaceId());
            route->setProtocolData(data);
            routes->addRoute(route);
        }
    }
  protected:
    void initialize(int stage) override {
        AodvPeer::initialize(stage);
        if (stage == INITSTAGE_APPLICATION_LAYER) {
            count = par("count");
            getModuleByPath("^.^.dut")->subscribe(linkBrokenSignal, this);
        }
    }
    void handleMessage(cMessage *message) override {
        if (message != timer) { AodvPeer::handleMessage(message); return; }
        if (step == 0) { request(0, 1, true); step++; scheduleAt(0.12, timer); return; }
        if (step == 1) { seedRecords(); step++; scheduleAt(0.15, timer); return; }
        if (step == 2) {
            check_and_cast<inet::physicallayer::IRadio *>(getModuleByPath("^.wlan[0].radio"))->setRadioMode(inet::physicallayer::IRadio::RADIO_MODE_OFF);
            step++; scheduleAt(0.8, timer); return;
        }
        ASSERT(failures == 1);
        bool limited = par("limited");
        int expected = limited ? 255 : count;
        ASSERT(received.size() == 2);
        for (auto address : {L3Address(Ipv4Address("10.0.0.3")), L3Address(Ipv4Address("10.1.0.4"))}) {
            ASSERT(int(received[address].size()) == expected);
            ASSERT(messages[address] == (limited ? 1 : (count + 254) / 255));
        }
        ASSERT(received[Ipv4Address("10.0.0.3")] == received[Ipv4Address("10.1.0.4")]);
        for (int i = 0; i < count; i++) {
            auto route = findRoute(target(i));
            ASSERT(route);
            auto data = check_and_cast<AodvRouteData *>(route->getProtocolData());
            ASSERT(!data->isActive() && data->hasValidDestNum() && data->getDestSeqNum() == uint32_t(1001 + i));
        }
        pass(par("scenario").stdstringValue());
    }
    void socketDataArrived(UdpSocket *, Packet *packet) override {
        auto control = packet->peekAtFront<AodvControlPacket>();
        if (auto error = dynamicPtrCast<const Rerr>(control)) {
            AodvErrorWireSerializer serializer;
            MemoryOutputStream stream;
            serializer.serializeFields(stream, error);
            auto bytes = stream.getData();
            size_t size = error->getUnreachableNodesArraySize();
            ASSERT(size > 0 && size <= 255 && bytes.size() == 4 + 8 * size && bytes[3] == size);
            MemoryInputStream input(bytes);
            auto decoded = dynamicPtrCast<const Rerr>(serializer.deserializeFields(input, typeid(Rerr)));
            ASSERT(decoded && decoded->getUnreachableNodesArraySize() == size);
            auto recipient = packet->getTag<L3AddressInd>()->getDestAddress();
            messages[recipient]++;
            for (size_t i = 0; i < size; i++) {
                const auto& node = decoded->getUnreachableNodes(i);
                uint32_t index = node.addr.toIpv4().getInt() - 0x0a020001u;
                ASSERT(index < unsigned(count) && node.seqNum == 1001 + index);
                ASSERT(received[recipient].emplace(node.addr, node.seqNum).second);
            }
            ASSERT(!error->getNoDeleteFlag() && packet->getTag<HopLimitInd>()->getHopLimit() == 1);
        }
        else if (auto reply = dynamicPtrCast<const Rrep>(control)) {
            if (reply->getAckRequiredFlag()) {
                auto ack = makeShared<RrepAck>(); ack->setPacketType(RREPACK); ack->setChunkLength(B(2)); transmit(ack);
            }
        }
        delete packet;
    }
    void receiveSignal(cComponent *, simsignal_t signal, cObject *object, cObject *) override {
        Enter_Method("observeCapacityFailure");
        if (signal == linkBrokenSignal) {
            auto packet = check_and_cast<Packet *>(object);
            ASSERT(!strncmp(packet->getName(), "queued-data-", 12));
            failures++;
        }
    }
};
Define_Module(AodvCapacityPeer);
#endif

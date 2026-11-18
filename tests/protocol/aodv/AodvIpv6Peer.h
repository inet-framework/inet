// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __AODV_IPV6_TEST_PEER_H
#define __AODV_IPV6_TEST_PEER_H
#include "AodvPeer.h"
#include "inet/networklayer/ipv6/Ipv6Header_m.h"
#include "inet/routing/aodv/AodvControlPacketsSerializer.h"
class AodvIpv6Wire : public AodvControlPacketsSerializer
{
  public:
    using AodvControlPacketsSerializer::serializeFields;
    using AodvControlPacketsSerializer::deserializeFields;
};
class AodvIpv6Peer : public AodvPeer
{
    int acknowledgments = 0;
    int requests = 0;
    void offer(uint32_t sequence) {
        auto h = makeShared<Rrep>(); h->setPacketType(RREP_IPv6); h->setChunkLength(B(44));
        h->setOriginatorAddr(dut); h->setDestAddr(destination); h->setDestSeqNum(sequence);
        h->setHopCount(1); h->setLifeTime(2); h->setAckRequiredFlag(true); transmit(h);
    }
    IRoute *hostRoute() {
        for (int i = 0; i < routes->getNumRoutes(); i++) {
            auto r = routes->getRoute(i);
            if (r->getSourceType() == IRoute::AODV && r->getPrefixLength() == 128 && r->getDestinationAsGeneric() == destination) return r;
        }
        return nullptr;
    }
  protected:
    void initialize(int stage) override {
        if (stage != INITSTAGE_APPLICATION_LAYER) return;
        peer = Ipv6Address("2001:db8:1::2"); dut = Ipv6Address("2001:db8:1::1"); destination = Ipv6Address("2001:db8:8::99");
        socket.setOutputGate(gate("socketOut")); socket.setCallback(this); socket.bind(654); socket.setBroadcast(true);
        socket.joinMulticastGroup(Ipv6Address("ff01::1"), check_and_cast<IInterfaceTable *>(getModuleByPath("^.interfaceTable"))->findInterfaceByName("wlan0")->getInterfaceId());
        routes = check_and_cast<IRoutingTable *>(getModuleByPath("^.^.dut.ipv6.routingTable"));
        getModuleByPath("^.ipv6.ipv6")->subscribe(packetReceivedFromLowerSignal, this);
        timer = new cMessage("ipv6-script"); scheduleAt(0.3, timer);
    }
    void handleMessage(cMessage *m) override {
        if (m != timer) { socket.processMessage(m); return; }
        if (step == 0) { ASSERT(hostRoute()); offer(0xfffffffdu); scheduleAt(0.4, timer); }
        if (step == 1) { ASSERT(check_and_cast<AodvRouteData *>(hostRoute()->getProtocolData())->getDestSeqNum() == 0xfffffffeu); offer(0); scheduleAt(0.5, timer); }
        if (step == 2) { ASSERT(check_and_cast<AodvRouteData *>(hostRoute()->getProtocolData())->getDestSeqNum() == 0); offer(0x80000000u); scheduleAt(0.8, timer); }
        if (step == 3) {
            ASSERT(hostRoute() && check_and_cast<AodvRouteData *>(hostRoute()->getProtocolData())->isActive());
            ASSERT(check_and_cast<AodvRouteData *>(hostRoute()->getProtocolData())->getDestSeqNum() == 0);
            ASSERT(emittedData == 1 && requests == 1 && acknowledgments == 4);
            std::cout << "AODV IPv6 Basic: PASS\n";
        }
        step++;
    }
    void socketDataArrived(UdpSocket *, Packet *p) override {
        auto h = p->peekAtFront<AodvControlPacket>();
        AodvIpv6Wire serializer; MemoryOutputStream output; serializer.serializeFields(output, h);
        MemoryInputStream input(output.getData());
        if (auto q = dynamicPtrCast<const Rreq>(h)) {
            ASSERT(q->getPacketType() == RREQ_IPv6 && q->getOriginatorAddr() == dut && q->getDestAddr() == destination);
            auto decoded = dynamicPtrCast<const Rreq>(serializer.deserializeFields(input, typeid(Rreq)));
            ASSERT(output.getData().size() == 48 && decoded && decoded->getOriginatorAddr() == dut && decoded->getDestAddr() == destination);
            requests++; offer(0xfffffffeu);
        }
        if (dynamicPtrCast<const RrepAck>(h)) {
            auto decoded = dynamicPtrCast<const RrepAck>(serializer.deserializeFields(input, typeid(RrepAck)));
            ASSERT(h->getPacketType() == RREPACK_IPv6 && output.getData().size() == 2);
            ASSERT(decoded && decoded->getPacketType() == RREPACK_IPv6);
            acknowledgments++;
        }
        delete p;
    }
    void receiveSignal(cComponent *, simsignal_t, cObject *v, cObject *) override {
        Enter_Method("observeNativeIpv6Data"); auto p = dynamic_cast<Packet *>(v); if (!p) return;
        auto ip = p->peekAtFront<Ipv6Header>();
        if (ip->getDestAddress() == destination.toIpv6()) { ASSERT(ip->getSrcAddress() == dut.toIpv6()); emittedData++; }
    }
    void finish() override {}
};
Define_Module(AodvIpv6Peer);
#endif

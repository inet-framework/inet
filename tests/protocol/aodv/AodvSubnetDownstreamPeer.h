// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __AODV_SUBNET_DOWNSTREAM_PEER_H
#define __AODV_SUBNET_DOWNSTREAM_PEER_H
#include "AodvPeer.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
class AodvSubnetDownstreamPeer : public AodvPeer
{
    IRoute *prefix(IRoutingTable *table) {
        for (int i = 0; i < table->getNumRoutes(); i++) {
            auto route = table->getRoute(i);
            if (route->getSourceType() == IRoute::AODV && route->getPrefixLength() == 24 &&
                    route->getDestinationAsGeneric() == L3Address(Ipv4Address("10.8.0.0"))) return route;
        }
        return nullptr;
    }
  protected:
    void initialize(int stage) override {
        if (stage != INITSTAGE_APPLICATION_LAYER) return;
        socket.setOutputGate(gate("socketOut")); socket.setCallback(this); socket.bind(654); socket.setBroadcast(true);
        routes = check_and_cast<IRoutingTable *>(getModuleByPath("^.^.dut.ipv4.routingTable"));
        getModuleByPath("^.ipv4.ip")->subscribe(packetReceivedFromLowerSignal, this);
        timer = new cMessage("downstream"); scheduleAt(0.05, timer);
    }
    void handleMessage(cMessage *message) override {
        if (message != timer) { socket.processMessage(message); return; }
        if (step++ == 0) {
            auto h = makeShared<Rrep>(); h->setPacketType(RREP); h->setChunkLength(B(20));
            h->setOriginatorAddr(dut); h->setDestAddr(Ipv4Address("10.8.0.99")); h->setPrefixSize(24);
            h->setDestSeqNum(100); h->setHopCount(1); h->setLifeTime(2); transmit(h);
            scheduleAt(0.4, timer);
        }
        else {
            auto downstream = check_and_cast<IRoutingTable *>(getModuleByPath("^.^.requester.ipv4.routingTable"));
            auto a = prefix(routes); auto b = prefix(downstream); ASSERT(a && b);
            ASSERT(check_and_cast<AodvRouteData *>(a->getProtocolData())->getDestSeqNum() == 100);
            ASSERT(check_and_cast<AodvRouteData *>(b->getProtocolData())->getDestSeqNum() == 100);
            ASSERT(a->getMetric() == 2 && b->getMetric() == 3 && b->getNextHopAsGeneric() == L3Address(Ipv4Address("10.1.0.1")));
            auto output = check_and_cast<IInterfaceTable *>(getModuleByPath("^.^.dut.interfaceTable"))->findInterfaceByName("wlan1");
            ASSERT(check_and_cast<AodvRouteData *>(a->getProtocolData())->getPrecursorList().count({Ipv4Address("10.1.0.2"), output->getInterfaceId()}));
            ASSERT(emittedData == 2 && forwardedRequests == 0);
            std::cout << "AODV Subnet Downstream: PASS\n";
        }
    }
    void socketDataArrived(UdpSocket *, Packet *packet) override {
        if (dynamicPtrCast<const Rreq>(packet->peekAtFront<AodvControlPacket>())) forwardedRequests++;
        delete packet;
    }
    void receiveSignal(cComponent *, simsignal_t, cObject *value, cObject *) override {
        Enter_Method("observeDownstreamData"); auto packet = dynamic_cast<Packet *>(value); if (!packet) return;
        auto ip = packet->peekAtFront<Ipv4Header>();
        if (ip->getDestAddress() == Ipv4Address("10.8.0.99") || ip->getDestAddress() == Ipv4Address("10.8.0.100")) {
            ASSERT(ip->getSrcAddress() == Ipv4Address("10.1.0.2"));
            auto output = check_and_cast<IInterfaceTable *>(getModuleByPath("^.interfaceTable"))->findInterfaceByName("wlan0");
            ASSERT(packet->getTag<MacAddressInd>()->getDestAddress() == output->getMacAddress());
            emittedData++;
        }
    }
    void finish() override {}
};
Define_Module(AodvSubnetDownstreamPeer);
#endif

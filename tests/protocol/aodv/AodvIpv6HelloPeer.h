// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __AODV_IPV6_HELLO_PEER_H
#define __AODV_IPV6_HELLO_PEER_H
#include "AodvPeer.h"
#include "inet/networklayer/ipv6/Ipv6Header_m.h"
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
inline void addIpv6FixtureRoute(cModule *node, cObject *owner, const L3Address& target, const L3Address& next, const char *name) {
    auto table = check_and_cast<IRoutingTable *>(node->getSubmodule("ipv6")->getSubmodule("routingTable"));
    auto iface = check_and_cast<IInterfaceTable *>(node->getSubmodule("interfaceTable"))->findInterfaceByName(name);
    ASSERT(iface);
    auto route = table->createRoute(); route->setSourceType(IRoute::MANUAL); route->setSource(owner);
    route->setDestination(target); route->setPrefixLength(128); route->setNextHop(next); route->setMetric(1); route->setInterface(iface);
    table->addRoute(route);
}
class AodvIpv6ForwardedSource : public SimpleModule
{
    UdpSocket output;
    cMessage *emitTimer = nullptr;
    int sent = 0;
  protected:
    int numInitStages() const override { return NUM_INIT_STAGES; }
    void initialize(int stage) override {
        if (stage != INITSTAGE_APPLICATION_LAYER) return;
        auto target = L3Address(Ipv6Address("2001:db8:1::1"));
        addIpv6FixtureRoute(getParentModule(), this, target, Ipv6Address("2001:db8:2::2"), "eth0");
        output.setOutputGate(gate("socketOut")); output.bind(12345);
        emitTimer = new cMessage("ipv6-source-data"); scheduleAt(0.18, emitTimer);
    }
    void handleMessage(cMessage *message) override {
        if (message != emitTimer) { delete message; return; }
        output.sendTo(new Packet("ipv6-forwarded-source-data", makeShared<ByteCountChunk>(B(8))), Ipv6Address("2001:db8:1::1"), 9999);
        if (++sent < 5) scheduleAfter(0.1, emitTimer);
    }
  public:
    ~AodvIpv6ForwardedSource() { cancelAndDelete(emitTimer); }
};
Define_Module(AodvIpv6ForwardedSource);

class AodvIpv6HelloPeer : public AodvPeer
{
    cModule *dutIp = nullptr;
    simtime_t lastData, lastNeighbor;
    int forwarded = 0, requests = 0, errors = 0;
    IRoute *owned(const L3Address& target) {
        for (int i = 0; i < routes->getNumRoutes(); i++) {
            auto route = routes->getRoute(i);
            if (route->getSourceType() == IRoute::AODV && route->getPrefixLength() == 128 && route->getDestinationAsGeneric() == target) return route;
        }
        return nullptr;
    }
    AodvRouteData *state(const L3Address& target) { auto route = owned(target); ASSERT(route); return check_and_cast<AodvRouteData *>(route->getProtocolData()); }
  protected:
    void initialize(int stage) override {
        if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
            auto interfaces = check_and_cast<IInterfaceTable *>(getModuleByPath("^.interfaceTable"));
            for (int i = 0; i < interfaces->getNumInterfaces(); i++)
                if (interfaces->getInterface(i)->findProtocolData<Ipv6InterfaceData>())
                    interfaces->getInterface(i)->getProtocolDataForUpdate<Ipv6InterfaceData>()->setAdvSendAdvertisements(false);
        }
        if (stage != INITSTAGE_APPLICATION_LAYER) return;
        peer = Ipv6Address("2001:db8:1::2"); dut = Ipv6Address("2001:db8:1::1"); destination = Ipv6Address("2001:db8:8::99");
        addIpv6FixtureRoute(getParentModule(), this, dut, dut, "wlan0");
        socket.setOutputGate(gate("socketOut")); socket.setCallback(this); socket.bind(654); socket.setBroadcast(true);
        auto iface = check_and_cast<IInterfaceTable *>(getModuleByPath("^.interfaceTable"))->findInterfaceByName("wlan0");
        socket.joinMulticastGroup(Ipv6Address("ff01::1"), iface->getInterfaceId());
        routes = check_and_cast<IRoutingTable *>(getModuleByPath("^.^.dut.ipv6.routingTable"));
        dutIp = getModuleByPath("^.^.dut.ipv6.ipv6"); dutIp->subscribe(packetReceivedFromLowerSignal, this);
        timer = new cMessage("ipv6-hello-observation"); scheduleAt(0.05, timer);
    }
    void handleMessage(cMessage *message) override {
        if (message != timer) { socket.processMessage(message); return; }
        if (step == 0) {
            auto hello = makeShared<Rrep>(); hello->setPacketType(RREP_IPv6); hello->setChunkLength(B(44));
            hello->setDestAddr(peer); hello->setDestSeqNum(20); hello->setLifeTime(1); transmit(hello);
            scheduleAt(0.06, timer);
        }
        if (step == 1) {
            auto reply = makeShared<Rrep>(); reply->setPacketType(RREP_IPv6); reply->setChunkLength(B(44));
            reply->setOriginatorAddr(dut); reply->setDestAddr(destination); reply->setDestSeqNum(100); reply->setHopCount(1); reply->setLifeTime(2); transmit(reply);
            scheduleAt(0.7, timer);
        }
        if (step == 2) {
            ASSERT(forwarded == 5 && lastData >= SimTime(0.58) && lastData < SimTime(0.6));
            ASSERT(lastNeighbor >= lastData && lastNeighbor < SimTime(0.7));
            ASSERT(state(peer)->isActive() && state(destination)->isActive());
            ASSERT(!owned(Ipv6Address("2001:db8:2::3")));
            scheduleAt(0.95, timer);
        }
        if (step == 3) {
            ASSERT(simTime() > lastNeighbor + SimTime(0.2));
            ASSERT(!state(peer)->isActive() && state(peer)->getDestSeqNum() == 21);
            ASSERT(!state(destination)->isActive() && state(destination)->getDestSeqNum() == 101);
            ASSERT(requests == 0 && errors == 0);
            std::cout << "AODV IPv6 HelloForwarding: PASS\n";
        }
        step++;
    }
    void socketDataArrived(UdpSocket *, Packet *packet) override {
        auto h = packet->peekAtFront<AodvControlPacket>(); requests += dynamicPtrCast<const Rreq>(h) != nullptr; errors += dynamicPtrCast<const Rerr>(h) != nullptr; delete packet;
    }
    void receiveSignal(cComponent *source, simsignal_t, cObject *object, cObject *) override {
        Enter_Method("observeNativeIpv6Forwarding"); if (source != dutIp) return;
        auto p = dynamic_cast<Packet *>(object); if (!p) return;
        auto mac = p->findTag<MacAddressInd>(); ASSERT(mac);
        auto iface = check_and_cast<IInterfaceTable *>(getModuleByPath("^.interfaceTable"))->findInterfaceByName("wlan0");
        if (mac->getSrcAddress() != iface->getMacAddress()) return;
        lastNeighbor = simTime();
        auto ip = p->peekAtFront<Ipv6Header>();
        if (ip->getProtocolId() == IP_PROT_UDP && ip->getSrcAddress() == Ipv6Address("2001:db8:2::3")) {
            ASSERT(ip->getDestAddress() == dut.toIpv6() && p->peekDataAt<UdpHeader>(ip->getChunkLength())->getDestPort() == 9999);
            forwarded++; lastData = simTime();
        }
    }
    void finish() override {}
};
Define_Module(AodvIpv6HelloPeer);
#endif

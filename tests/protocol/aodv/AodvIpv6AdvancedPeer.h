// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __AODV_IPV6_ADVANCED_PEER_H
#define __AODV_IPV6_ADVANCED_PEER_H
#include "AodvSubnetPeer.h"
#include "inet/common/lifecycle/LifecycleController.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/networklayer/ipv6/Ipv6Header_m.h"
class AodvIpv6AdvancedPeer : public AodvPeer, public LifecycleController, public IDoneCallback
{
    struct Request { L3Address target; simtime_t time; uint32_t id; };
    std::string mode;
    std::vector<Request> requests;
    std::vector<uint32_t> selfReplies;
    std::set<long> delivered;
    std::vector<simtime_t> arrivals;
    int acknowledgments = 0;
    bool operationPending = false;
    L3Address second = Ipv6Address("2001:db8:8::98");
    bool incoming() const { return mode == "Incoming" || mode == "IncomingRestart"; }
    bool restart() const { return mode == "IncomingRestart" || mode == "Restart" || mode == "CrashReady" || mode == "StopQueued"; }
    bool prequeued() const { return mode == "CrashReady" || mode == "StopQueued"; }
    void invoke() override { Enter_Method("ipv6OperationCompleted"); operationPending = false; }
    void operate(LifecycleOperation *operation) {
        LifecycleOperation::StringMap params; operation->initialize(getModuleByPath("^.^.dut"), params);
        operationPending = !initiateOperation(operation, this);
    }
    void query(unsigned id, bool known = false) {
        auto h = makeShared<Rreq>(); h->setPacketType(RREQ_IPv6); h->setChunkLength(B(48));
        h->setOriginatorAddr(peer); h->setOriginatorSeqNum(id); h->setRreqId(id); h->setDestAddr(dut);
        h->setUnknownSeqNumFlag(!known); h->setDestSeqNum(known ? 50 : 0);
        transmit(h, 0, Ipv6Address("ff01::1"));
    }
    void offer(const L3Address& target, uint32_t seq) {
        auto h = makeShared<Rrep>(); h->setPacketType(RREP_IPv6); h->setChunkLength(B(44));
        h->setOriginatorAddr(dut); h->setDestAddr(target); h->setDestSeqNum(seq);
        h->setHopCount(1); h->setLifeTime(2); h->setAckRequiredFlag(true); transmit(h);
    }
    void emitData(const L3Address& target) {
        auto p = new AodvTrackedPacket("ipv6-compatibility-data", makeShared<ByteCountChunk>(B(8)));
        check_and_cast<AodvSubnetSource *>(getModuleByPath("^.^.dut.app[0]"))->emitData(p, target);
    }
    void run() {
        if (step == 0) {
            if (incoming()) query(1);
            else { emitData(destination); if (mode == "RateLimit" || mode == "DeferredReply" || prequeued()) emitData(second); }
        }
        if (step == 1 && restart()) operate(mode == "StopQueued" ? static_cast<LifecycleOperation *>(new ModuleStopOperation()) : new ModuleCrashOperation());
        if (step == 2 && restart()) { ASSERT(!operationPending); operate(new ModuleStartOperation()); }
        if (step == 3 && restart()) { if (incoming()) query(2, true); else emitData(second); }
        if (step == 4 && restart() && !incoming()) offer(second, 30);
        if (step == 5 && restart()) ASSERT(incoming() ? selfReplies.size() == 1 : delivered.size() == 1);
        if (step == 6 && mode == "IncomingRestart") query(3);
        if (step == 7 && mode == "DeferredReply") offer(second, 30);
        if (step == 8) {
            ASSERT(!operationPending);
            if (incoming()) {
                ASSERT(requests.empty() && delivered.empty() && acknowledgments == 0);
                ASSERT(selfReplies.size() == (restart() ? 2u : 1u) && selfReplies[0] == 0);
                if (restart()) ASSERT(selfReplies[1] == 50);
            }
            else {
                ASSERT(requests.size() == (mode == "RateLimit" ? 2u : 1u));
                ASSERT(requests[0].target == destination && requests[0].id == 1 && requests[0].time < SimTime(0.1));
                ASSERT(delivered.size() == 2 && acknowledgments == 2);
                if (mode == "RateLimit") ASSERT(requests[1].target == second && requests[1].id == 2 && requests[1].time >= SimTime(1));
                if (restart()) ASSERT(arrivals[1] >= SimTime(0.23) && arrivals[1] < SimTime(0.3));
                if (mode == "DeferredReply") ASSERT(arrivals[1] >= SimTime(0.4) && arrivals[1] < SimTime(0.5));
                ASSERT(AodvDataCounters::created == (prequeued() ? 3 : 2));
            }
            ASSERT(AodvDataCounters::created == AodvDataCounters::destroyed);
            std::cout << "AODV IPv6 " << mode << ": PASS\n"; return;
        }
        static const double times[] = {0.05, 0.12, 0.13, 0.17, 0.18, 0.22, 0.27, 0.4, 1.2};
        step++; scheduleAt(times[step], timer);
    }
  protected:
    void initialize(int stage) override {
        if (stage != INITSTAGE_APPLICATION_LAYER) return;
        mode = par("scenario").stdstringValue(); peer = Ipv6Address("2001:db8:1::2"); dut = Ipv6Address("2001:db8:1::1"); destination = Ipv6Address("2001:db8:8::99");
        socket.setOutputGate(gate("socketOut")); socket.setCallback(this); socket.bind(654); socket.setBroadcast(true);
        auto iface = check_and_cast<IInterfaceTable *>(getModuleByPath("^.interfaceTable"))->findInterfaceByName("wlan0");
        socket.joinMulticastGroup(Ipv6Address("ff01::1"), iface->getInterfaceId());
        routes = check_and_cast<IRoutingTable *>(getModuleByPath("^.^.dut.ipv6.routingTable"));
        getModuleByPath("^.ipv6.ipv6")->subscribe(packetReceivedFromLowerSignal, this);
        timer = new cMessage("ipv6-compatibility"); scheduleAt(0.05, timer);
    }
    void handleMessage(cMessage *m) override { if (m == timer) run(); else socket.processMessage(m); }
    void socketDataArrived(UdpSocket *, Packet *p) override {
        auto h = p->peekAtFront<AodvControlPacket>();
        if (auto q = dynamicPtrCast<const Rreq>(h)) {
            ASSERT(q->getPacketType() == RREQ_IPv6 && q->getOriginatorAddr() == dut);
            requests.push_back({q->getDestAddr(), simTime(), q->getRreqId()});
            if (q->getDestAddr() == destination) offer(destination, 20);
            if (q->getDestAddr() == second) offer(second, 30);
        }
        if (auto r = dynamicPtrCast<const Rrep>(h)) {
            ASSERT(r->getPacketType() == RREP_IPv6 && r->getDestAddr() == dut && r->getOriginatorAddr() == peer);
            selfReplies.push_back(r->getDestSeqNum());
            ASSERT(p->getTag<L3AddressInd>()->getSrcAddress() == dut);
            if (r->getAckRequiredFlag()) { auto a = makeShared<RrepAck>(); a->setPacketType(RREPACK_IPv6); a->setChunkLength(B(2)); transmit(a); }
        }
        if (dynamicPtrCast<const RrepAck>(h)) { ASSERT(h->getPacketType() == RREPACK_IPv6); acknowledgments++; }
        delete p;
    }
    void receiveSignal(cComponent *, simsignal_t, cObject *object, cObject *) override {
        Enter_Method("observeIpv6Compatibility"); auto p = dynamic_cast<Packet *>(object); if (!p) return;
        auto ip = p->peekAtFront<Ipv6Header>();
        if (ip->getProtocolId() == IP_PROT_UDP && (L3Address(ip->getDestAddress()) == destination || L3Address(ip->getDestAddress()) == second)) {
            auto udp = p->peekDataAt<UdpHeader>(ip->getChunkLength()); if (udp->getDestPort() != 9999) return;
            ASSERT(L3Address(ip->getSrcAddress()) == dut);
            if (delivered.insert(p->getTreeId()).second) arrivals.push_back(simTime());
        }
    }
    void finish() override {}
};
Define_Module(AodvIpv6AdvancedPeer);
#endif

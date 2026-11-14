// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __AODV_SUBNET_TEST_PEER_H
#define __AODV_SUBNET_TEST_PEER_H
#include "AodvPeer.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/networklayer/ipv4/IcmpHeader.h"
#include "inet/common/lifecycle/LifecycleController.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"
class AodvSubnetSource : public SimpleModule
{
    UdpSocket output;
  protected:
    int numInitStages() const override { return NUM_INIT_STAGES; }
    void initialize(int stage) override {
        if (stage == INITSTAGE_APPLICATION_LAYER) { output.setOutputGate(gate("socketOut")); output.bind(12345); }
    }
    void handleMessage(cMessage *message) override { delete message; }
  public:
    void emitData(Packet *packet, const L3Address& destination) {
        Enter_Method("emitData"); take(packet); output.sendTo(packet, destination, 9999);
    }
};
Define_Module(AodvSubnetSource);
class AodvSubnetPeer : public AodvPeer, public LifecycleController
{
    std::string mode;
    std::vector<Ptr<const Rreq>> requests;
    std::vector<Ptr<const Rrep>> answers;
    std::vector<Ptr<const Rerr>> errors;
    std::set<int> dataIds;
    L3Address member = Ipv4Address("10.8.0.99");
    L3Address other = Ipv4Address("10.8.0.100");
    IRoute *route(const L3Address& target, int prefix) {
        for (int i = 0; i < routes->getNumRoutes(); i++) {
            auto r = routes->getRoute(i);
            if (r->getSourceType() == IRoute::AODV && r->getPrefixLength() == prefix &&
                    r->getDestinationAsGeneric() == target.getPrefix(prefix)) return r;
        }
        return nullptr;
    }
    AodvRouteData *state(const L3Address& target, int prefix) {
        auto r = route(target, prefix); ASSERT(r); return check_and_cast<AodvRouteData *>(r->getProtocolData());
    }
    void offer(const L3Address& target, int prefix, uint32_t seq, simtime_t lifetime = 2, int output = 0) {
        auto h = makeShared<Rrep>(); h->setPacketType(RREP); h->setChunkLength(B(20));
        h->setDestAddr(target); h->setPrefixSize(prefix == 32 ? 0 : prefix); h->setDestSeqNum(seq);
        h->setOriginatorAddr(mode == "Forwarded" ? L3Address(Ipv4Address("10.0.0.3")) : dut); h->setLifeTime(lifetime); h->setHopCount(1);
        transmit(h, output);
    }
    void ask(bool unknown, bool only, uint32_t floor = 5) {
        auto h = makeShared<Rreq>(); h->setPacketType(RREQ); h->setChunkLength(B(24));
        h->setOriginatorAddr(Ipv4Address("10.0.0.3")); h->setOriginatorSeqNum(step + 1); h->setRreqId(step + 1);
        h->setDestAddr(member); h->setDestSeqNum(floor); h->setUnknownSeqNumFlag(unknown); h->setDestOnlyFlag(only);
        transmit(h, 1);
    }
    void transmitError(bool n, bool unrelated = false) {
        auto h = makeShared<Rerr>(); h->setPacketType(RERR); h->setChunkLength(B(unrelated ? 20 : 12));
        h->setNoDeleteFlag(n); h->setUnreachableNodesArraySize(unrelated ? 2 : 1);
        UnreachableNode entry; entry.addr = member.getPrefix(24); entry.seqNum = 101; h->setUnreachableNodes(0, entry);
        if (unrelated) { entry.addr = Ipv4Address("10.9.0.99"); entry.seqNum = 31; h->setUnreachableNodes(1, entry); }
        transmit(h);
    }
    void data(const L3Address& target, bool local = true) {
        auto p = new AodvTrackedPacket("subnet-data", makeShared<ByteCountChunk>(B(8)));
        if (local) {
            check_and_cast<AodvSubnetSource *>(getModuleByPath("^.^.dut.app[0]"))->emitData(p, target);
        }
        else {
            p->addTag<InterfaceReq>()->setInterfaceId(check_and_cast<IInterfaceTable *>(getModuleByPath("^.interfaceTable"))->findInterfaceByName("wlan1")->getInterfaceId());
            p->addTag<NextHopAddressReq>()->setNextHopAddress(dut);
            socket.sendTo(p, target, 9999);
        }
    }
    void unknownHost() {
        auto r = routes->createRoute(); r->setSourceType(IRoute::AODV);
        r->setSource(getModuleByPath("^.^.dut.aodv")); r->setDestination(member); r->setPrefixLength(32);
        r->setNextHop(peer); r->setMetric(2);
        r->setInterface(check_and_cast<IInterfaceTable *>(getModuleByPath("^.^.dut.interfaceTable"))->findInterfaceByName("wlan0"));
        auto d = new AodvRouteData(); d->setHasValidDestNum(false); d->setIsActive(false); d->setLifeTime(2); r->setProtocolData(d);
        routes->addRoute(r);
    }
    void memberUnreachable() {
        auto packet = new Packet("member-host-unreachable");
        auto udp = makeShared<UdpHeader>(); udp->setSrcPort(12345); udp->setDestPort(9999); udp->setTotalLengthField(B(8));
        packet->insertAtFront(udp);
        auto quoted = makeShared<Ipv4Header>(); quoted->setSrcAddress(dut.toIpv4()); quoted->setDestAddress(member.toIpv4());
        quoted->setProtocolId(IP_PROT_UDP); quoted->setTotalLengthField(B(28)); packet->insertAtFront(quoted);
        auto icmp = makeShared<IcmpHeader>(); icmp->setType(ICMP_DESTINATION_UNREACHABLE); icmp->setCode(ICMP_DU_HOST_UNREACHABLE);
        icmp->setChecksumMode(CHECKSUM_DECLARED_CORRECT); packet->insertAtFront(icmp);
        packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::icmpv4);
        packet->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);
        auto addresses = packet->addTag<L3AddressReq>(); addresses->setSrcAddress(peer); addresses->setDestAddress(dut);
        packet->addTag<InterfaceReq>()->setInterfaceId(check_and_cast<IInterfaceTable *>(getModuleByPath("^.interfaceTable"))->findInterfaceByName("wlan0")->getInterfaceId());
        packet->addTag<NextHopAddressReq>()->setNextHopAddress(dut);
        send(packet, "socketOut");
    }
    void run() {
        if (mode == "HelloBlock") { runHelloBlock(); return; }
        if (mode == "RecoveryAuthority") { runRecovery(); return; }
        if (mode == "Sustained") { runSustained(); return; }
        if (mode == "Forwarded") {
            if (step == 0) ask(true, true);
            if (step == 1) offer(member, 24, 100);
            if (step == 2) { ASSERT(!answers.empty() && answers[0]->getPrefixSize() == 24 && answers[0]->getHopCount() == 2); data(other); }
            if (step == 3) { ASSERT(dataIds.size() == 1); std::cout << "AODV Subnet Forwarded: PASS\n"; return; }
            step++; scheduleAfter(0.08, timer); return;
        }
        if (step == 0) {
            if (mode == "Alias") offer(peer, 31, 100);
            else if (mode == "KnownBlock" || mode == "RetainedHost") offer(member, 32, 10, 0.02);
            else offer(member, 24, mode == "Wrap" ? 0xfffffffeu : mode == "CountersSwap" ? 5 : 100,
                    mode == "RetainedFloor" ? 0.06 : 2);
        }
        if (step == 1) {
            if (mode == "Alias") {
                ASSERT(route(peer, 31) && route(peer, 32));
                ASSERT(state(peer, 31)->getDestSeqNum() == 100 && !state(peer, 32)->hasValidDestNum());
            }
            else if (mode == "KnownBlock" || mode == "RetainedHost") offer(member, 24, 100);
            else {
                ASSERT(route(member, 24) && !route(member, 32));
                if (mode == "Basic") { data(member); data(other); }
                if (mode == "Counters" || mode == "CountersSwap") offer(member, 32, mode == "CountersSwap" ? 100 : 5);
                if (mode == "Wrap") offer(member, 24, 0);
                if (mode == "UnknownMask") { unknownHost(); data(member); }
                if (mode == "Cached" || mode == "RelayError" || mode == "RelayN") ask(true, false);
                if (mode == "KnownRequest") ask(false, false);
                if (mode == "OnlyRequest") ask(true, true);
                if (mode == "RetainedFloor") data(member);
                if (mode == "Ambiguous" || mode == "AmbiguousN" || mode == "InactiveAmbiguous" || mode == "MixedError")
                    offer(member, 25, 200, mode == "InactiveAmbiguous" ? 0.02 : 2);
                if (mode == "MixedError") offer(Ipv4Address("10.9.0.99"), 32, 30);
                if (mode == "UniqueError") transmitError(false);
                if (mode == "OutgoingError") transmitError(false);
                if (mode == "IcmpMember") memberUnreachable();
                if (mode == "Manual") {
                    auto r = routes->createRoute(); r->setDestination(member); r->setPrefixLength(32);
                    r->setSourceType(IRoute::MANUAL); r->setSource(this); r->setMetric(0); r->setNextHop(Ipv4Address("10.0.0.3"));
                    r->setInterface(check_and_cast<IInterfaceTable *>(getModuleByPath("^.^.dut.interfaceTable"))->findInterfaceByName("wlan0"));
                    routes->addRoute(r); data(member);
                }
            }
        }
        if (step == 2) {
            if (mode == "Basic" || mode == "UnknownMask") ASSERT(dataIds.size() == (mode == "Basic" ? 2u : 1u));
            if (mode == "Counters" || mode == "CountersSwap") {
                ASSERT(state(member, 24)->getDestSeqNum() == (mode == "CountersSwap" ? 5u : 100u));
                ASSERT(state(member, 32)->getDestSeqNum() == (mode == "CountersSwap" ? 100u : 5u)); data(member);
            }
            if (mode == "Wrap") { ASSERT(state(member, 24)->getDestSeqNum() == 0); offer(member, 24, 0x80000000u); }
            if (mode == "KnownBlock" || mode == "RetainedHost") { data(member); data(other); }
            if (mode == "Cached") { ASSERT(answers.size() == 1); ASSERT(answers[0]->getPrefixSize() == 24 && answers[0]->getDestAddr() == member && answers[0]->getDestSeqNum() == 100); ASSERT(state(member, 24)->getPrecursorList().size() == 1); }
            if (mode == "RelayError" || mode == "RelayN") { ASSERT(answers.size() == 1 && answers[0]->getPrefixSize() == 24); transmitError(mode == "RelayN"); }
            if (mode == "KnownRequest" || mode == "OnlyRequest") {
                ASSERT(answers.empty() && !requests.empty()); auto q = requests.back();
                ASSERT(q->getUnknownSeqNumFlag() == (mode == "OnlyRequest"));
                ASSERT(q->getDestSeqNum() == 5 && q->getDestOnlyFlag() == (mode == "OnlyRequest"));
            }
            if (mode == "RetainedFloor") {
                ASSERT(!requests.empty() && requests[0]->getUnknownSeqNumFlag() && requests[0]->getDestOnlyFlag() && requests[0]->getDestSeqNum() == 0);
                offer(member, 24, 99); // cannot replace retained prefix knowledge
            }
            if (mode == "Ambiguous" || mode == "AmbiguousN" || mode == "InactiveAmbiguous") transmitError(mode == "AmbiguousN");
            if (mode == "MixedError") transmitError(false, true);
            if (mode == "UniqueError") ASSERT(!state(member, 24)->isActive() && state(member, 24)->getDestSeqNum() == 101);
            if (mode == "OutgoingError") data(other, false);
            if (mode == "IcmpMember") { ASSERT(state(member, 24)->isActive()); data(member); }
            if (mode == "Manual") ASSERT(dataIds.size() == 1 && state(member, 24)->getDestSeqNum() == 100);
        }
        if (step == 3) {
            if (mode == "KnownBlock" || mode == "RetainedHost") {
                ASSERT(dataIds.size() == 1); ASSERT(!requests.empty() && !requests[0]->getUnknownSeqNumFlag() && requests[0]->getDestSeqNum() == 10);
                ASSERT(!state(member, 32)->isActive()); offer(member, 32, 10);
            }
            if (mode == "RetainedFloor") { ASSERT(state(member, 24)->getDestSeqNum() == 100 && !state(member, 24)->isActive()); offer(member, 32, 5); }
            if (mode == "Wrap") ASSERT(state(member, 24)->getDestSeqNum() == 0);
            if (mode == "IcmpMember") { ASSERT(state(member, 24)->isActive() && state(member, 24)->getDestSeqNum() == 100); ASSERT(dataIds.size() == 1 && errors.empty()); }
            if (mode == "RelayError" || mode == "RelayN") {
                ASSERT(state(member, 24)->isActive() == (mode == "RelayN"));
                ASSERT(state(member, 24)->getDestSeqNum() == (mode == "RelayN" ? 100u : 101u));
                ASSERT(!errors.empty() && errors[0]->getNoDeleteFlag() == (mode == "RelayN"));
                ASSERT(errors[0]->getUnreachableNodes(0).addr == member.getPrefix(24) && errors[0]->getUnreachableNodes(0).seqNum == 101);
            }
            if (mode == "Ambiguous" || mode == "AmbiguousN" || mode == "InactiveAmbiguous") {
                ASSERT(state(member, 24)->isActive() && state(member, 24)->getDestSeqNum() == 100);
                ASSERT(state(member, 25)->getDestSeqNum() == 200);
                ASSERT(state(member, 25)->isActive() == (mode != "InactiveAmbiguous"));
            }
            if (mode == "MixedError") {
                ASSERT(state(member, 24)->isActive() && state(member, 24)->getDestSeqNum() == 100);
                ASSERT(state(member, 25)->isActive() && state(member, 25)->getDestSeqNum() == 200);
                ASSERT(!state(Ipv4Address("10.9.0.99"), 32)->isActive() && state(Ipv4Address("10.9.0.99"), 32)->getDestSeqNum() == 31);
            }
            if (mode == "OutgoingError") {
                ASSERT(!errors.empty() && errors[0]->getUnreachableNodesArraySize() == 1);
                ASSERT(errors[0]->getUnreachableNodes(0).addr == member.getPrefix(24));
                ASSERT(errors[0]->getUnreachableNodes(0).seqNum == 102);
            }
        }
        if (step == 4) {
            if (mode == "KnownBlock" || mode == "RetainedHost") { ASSERT(dataIds.size() == 2); data(member); }
            if (mode == "RetainedFloor") { ASSERT(dataIds.size() == 1); ASSERT(state(member, 32)->getDestSeqNum() == 5); }
            if (mode == "Counters" || mode == "CountersSwap") ASSERT(dataIds.size() == 1);
        }
        if (step == 5) {
            if (mode == "KnownBlock" || mode == "RetainedHost") ASSERT(dataIds.size() == 3);
            std::cout << "AODV Subnet " << mode << ": PASS\n"; return;
        }
        step++; scheduleAfter(0.08, timer);
    }
    void runRecovery() {
        if (step == 0) {
            for (auto operation : {static_cast<LifecycleOperation *>(new ModuleCrashOperation()), static_cast<LifecycleOperation *>(new ModuleStartOperation())}) {
                LifecycleOperation::StringMap parameters; operation->initialize(getModuleByPath("^.^.dut"), parameters); ASSERT(initiateOperation(operation));
            }
        }
        if (step == 1) { unknownHost(); state(member, 32)->setLifeTime(0.2); hello(10, 1); data(member); }
        if (step == 2) offer(member, 24, 100, 0.02);
        if (step == 3) data(Ipv4Address("10.9.0.99"), false); // extend hold without touching the aggregate
        if (step == 4) { ASSERT(state(member, 24)->getDestSeqNum() == 100); offer(member, 24, 99); }
        if (step == 5) { ASSERT(!state(member, 24)->isActive() && state(member, 24)->getDestSeqNum() == 100); offer(member, 32, 5); }
        if (step == 6) ASSERT(dataIds.empty());
        if (step == 7) { ASSERT(dataIds.size() == 1 && requests.empty()); ASSERT(state(member, 32)->getDestSeqNum() == 5); }
        if (step == 8) { std::cout << "AODV Subnet RecoveryAuthority: PASS\n"; return; }
        static const double times[] = {0.05, 0.13, 0.21, 0.29, 0.55, 0.57, 0.58, 0.61, 0.65};
        step++; scheduleAt(times[step], timer);
    }
    void runSustained() {
        if (step == 0) offer(member, 32, 10, 0.02);
        if (step == 1) offer(member, 24, 100);
        if (step >= 2 && step <= 41) { ASSERT(state(member, 32)->getDestSeqNum() == 10); data(member); }
        if (step == 42) {
            ASSERT(dataIds.empty() && !route(member, 32) && requests.size() >= 5);
            ASSERT(state(member, 24)->isActive()); data(member);
        }
        if (step == 43) { ASSERT(dataIds.size() == 1); ASSERT(AodvDataCounters::created == AodvDataCounters::destroyed); std::cout << "AODV Subnet Sustained: PASS\n"; return; }
        step++; scheduleAt(step == 1 ? 0.09 : step <= 41 ? 0.13 + 0.01 * (step - 2) : step == 42 ? 0.75 : 0.83, timer);
    }
    void runHelloBlock() {
        if (step == 0) hello(10, 2);
        if (step == 1) {
            ASSERT(state(member, 32)->isActive());
            check_and_cast<inet::physicallayer::IRadio *>(getModuleByPath("^.wlan[0].radio"))->setRadioMode(inet::physicallayer::IRadio::RADIO_MODE_OFF);
            data(member); // the failed MAC-owned original must never be queued for retransmission
        }
        if (step == 2) { ASSERT(!state(member, 32)->isActive() && state(member, 32)->getDestSeqNum() == 11); offer(member, 24, 1000, 2, 1); }
        if (step == 3) { ASSERT(!state(member, 32)->isActive() && state(member, 24)->isActive()); data(member); data(other); }
        if (step == 4) { ASSERT(dataIds.size() == 1); ASSERT(!requests.empty() && requests[0]->getDestSeqNum() == 11); offer(member, 32, 11, 2, 1); }
        if (step == 5) {
            ASSERT(dataIds.size() == 2 && state(member, 32)->isActive());
            ASSERT(AodvDataCounters::created == 3 && AodvDataCounters::destroyed == 3);
            std::cout << "AODV Subnet HelloBlock: PASS\n"; return;
        }
        step++; scheduleAfter(0.08, timer);
    }
  protected:
    void initialize(int stage) override {
        if (stage != INITSTAGE_APPLICATION_LAYER) return;
        mode = par("scenario").stdstringValue(); socket.setOutputGate(gate("socketOut")); socket.setCallback(this); socket.bind(654); socket.setBroadcast(true);
        if (mode == "HelloBlock") { member = peer; other = Ipv4Address("10.0.0.50"); }
        routes = check_and_cast<IRoutingTable *>(getModuleByPath("^.^.dut.ipv4.routingTable"));
        getModuleByPath("^.ipv4.ip")->subscribe(packetReceivedFromLowerSignal, this);
        timer = new cMessage("subnet-script"); scheduleAt(0.05, timer);
    }
    void handleMessage(cMessage *m) override { if (m == timer) run(); else socket.processMessage(m); }
    void socketDataArrived(UdpSocket *, Packet *p) override {
        auto h = p->peekAtFront<AodvControlPacket>();
        if (auto q = dynamicPtrCast<const Rreq>(h)) requests.push_back(q);
        if (auto r = dynamicPtrCast<const Rrep>(h)) {
            answers.push_back(r);
            if (r->getAckRequiredFlag()) { auto ack = makeShared<RrepAck>(); ack->setPacketType(RREPACK); ack->setChunkLength(B(2)); transmit(ack, 1); }
        }
        if (auto e = dynamicPtrCast<const Rerr>(h)) errors.push_back(e);
        delete p;
    }
    void receiveSignal(cComponent *, simsignal_t, cObject *v, cObject *) override {
        Enter_Method("observeSubnetWire"); auto p = dynamic_cast<Packet *>(v); if (!p) return;
        auto ip = p->peekAtFront<Ipv4Header>();
        if (ip->getDestAddress() == member.toIpv4() || ip->getDestAddress() == other.toIpv4()) {
            auto output = check_and_cast<IInterfaceTable *>(getModuleByPath("^.interfaceTable"))->findInterfaceByName(mode == "Manual" || mode == "HelloBlock" ? "wlan1" : "wlan0");
            ASSERT(p->getTag<MacAddressInd>()->getDestAddress() == output->getMacAddress());
            ASSERT(ip->getSrcAddress() == dut.toIpv4()); dataIds.insert(ip->getIdentification());
        }
    }
    void finish() override {}
};
Define_Module(AodvSubnetPeer);
#endif

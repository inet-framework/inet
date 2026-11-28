// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __AODV_REPAIR_TEST_PEER_H
#define __AODV_REPAIR_TEST_PEER_H
#include "AodvSubnetPeer.h"
#include "AodvRepairIngress.h"
#include "AodvRepairIpv4.h"
#include "inet/networklayer/ipv6/Ipv6Header_m.h"
#include "inet/networklayer/contract/INetfilter.h"
#include "inet/routing/aodv/AodvControlPacketsSerializer.h"
class AodvRepairWire : public AodvControlPacketsSerializer
{
  public:
    using AodvControlPacketsSerializer::serializeFields;
    using AodvControlPacketsSerializer::deserializeFields;
};
class AodvRepairSource : public SimpleModule, public UdpSocket::ICallback
{
    UdpSocket output;
  protected:
    int numInitStages() const override { return NUM_INIT_STAGES; }
    void initialize(int stage) override { if (stage == INITSTAGE_APPLICATION_LAYER) { output.setOutputGate(gate("socketOut")); output.setCallback(this); output.bind(654); } }
    void handleMessage(cMessage *message) override { output.processMessage(message); }
    void socketDataArrived(UdpSocket *, Packet *packet) override { if (dynamicPtrCast<const Rerr>(packet->peekAtFront<AodvControlPacket>())) errorCount++; delete packet; }
    void socketErrorArrived(UdpSocket *, Indication *indication) override { delete indication; }
    void socketClosed(UdpSocket *) override {}
  public:
    int errorCount = 0;
    void emitPacket(Packet *packet, const L3Address& target, int port) {
        Enter_Method("emitPacket"); take(packet);
        auto interface = check_and_cast<IInterfaceTable *>(getModuleByPath("^.interfaceTable"))->findInterfaceByName("wlan0");
        packet->addTag<InterfaceReq>()->setInterfaceId(interface->getInterfaceId());
        packet->addTag<NextHopAddressReq>()->setNextHopAddress(Ipv4Address("10.0.0.1"));
        packet->addTag<HopLimitReq>()->setHopLimit(10); output.sendTo(packet, target, port);
    }
};
Define_Module(AodvRepairSource);
class AodvRepairPeer : public AodvPeer, public LifecycleController, public IDoneCallback, public NetfilterBase::HookBase
{
    std::string mode;
    UdpSocket dataSink;
    bool operationPending = false;
    bool using6 = false;
    simtime_t tieBoundary;
    bool tie() const { return mode == "ReplyFirst" || mode == "TimeoutFirst"; }
    INetfilter *sourceIp = nullptr;
    std::map<const Packet *, int> sourceOutputs;
    INetfilter::IHook::Result datagramPreRoutingHook(Packet *) override { return INetfilter::IHook::ACCEPT; }
    INetfilter::IHook::Result datagramForwardHook(Packet *) override { return INetfilter::IHook::ACCEPT; }
    INetfilter::IHook::Result datagramPostRoutingHook(Packet *) override { return INetfilter::IHook::ACCEPT; }
    INetfilter::IHook::Result datagramLocalInHook(Packet *) override { return INetfilter::IHook::ACCEPT; }
    INetfilter::IHook::Result datagramLocalOutHook(Packet *packet) override {
        Enter_Method("sourceLocalOut");
        auto output = sourceOutputs.find(packet);
        if (output != sourceOutputs.end()) { packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(output->second); sourceOutputs.erase(output); }
        return INetfilter::IHook::ACCEPT;
    }
    void invoke() override { Enter_Method("repairOperationCompleted"); operationPending = false; }
    int prefixLength = 32;
    std::vector<Ptr<const Rreq>> requests;
    std::set<std::pair<L3Address, uint32_t>> requestIds;
    std::vector<Ptr<const Rerr>> errors;
    std::set<long> errorIds;
    std::set<long> dataIds;
    L3Address member = Ipv4Address("10.8.0.99"), other = Ipv4Address("10.8.0.100");
    bool aggregate() const { return mode.find("Aggregate") == 0 || mode == "IndependentHost" || mode == "EmptySuccess" || mode == "EmptyFailure" || mode == "OriginatingFirstAggregate" || mode == "DeferredAggregate"; }
    bool unknown() const { return mode.find("UnknownHost") == 0; }
    bool declined() const { return mode == "Off" || mode == "ExplicitOff" || mode == "Distance" || mode == "SourceMissing" || mode == "TtlTooLarge" || mode == "ZeroMaximum"; }
    bool starved() const { return mode == "RateStarved" || mode == "JitterStarved" || mode == "RateStop" || mode == "JitterStop"; }
    bool lifecycle() const { return mode == "Crash" || mode == "Restart" || mode == "Shutdown" || mode == "RateStop" || mode == "JitterStop"; }
    bool succeeds() const { return mode == "HostSuccess" || mode == "Longer" || mode == "AggregateSuccess" || mode == "EmptySuccess" || mode == "UnknownHostSuccess" || mode == "HelloSuccess" || mode == "NeighborSuccess" || mode == "OutputConstraint" || tie(); }
    void checkDisposal() {
        ASSERT(AodvDataCounters::created == AodvDataCounters::destroyed);
        if (!using6) {
            ASSERT(AodvRepairTransitPacket::created > 0 && AodvRepairTransitPacket::created == AodvRepairTransitPacket::destroyed);
            ASSERT(check_and_cast<AodvRepairIpv4 *>(getModuleByPath("^.^.dut.ipv4.ip"))->queuedCount() == 0);
        }
    }
    L3Address peerAddress(int suffix) const {
        return using6 ? L3Address(Ipv6Address(("2001:db8:1::" + std::to_string(suffix)).c_str())) : L3Address(Ipv4Address(0x0a000000u + suffix));
    }
    IRoute *owned(const L3Address& address, int prefix) {
        for (int i = 0; i < routes->getNumRoutes(); i++) {
            auto r = routes->getRoute(i);
            if (r->getSource() == getModuleByPath("^.^.dut.aodv") && r->getPrefixLength() == prefix && r->getDestinationAsGeneric() == address.getPrefix(prefix)) return r;
        }
        return nullptr;
    }
    AodvRouteData *state() { auto r = owned(member, prefixLength); ASSERT(r); return check_and_cast<AodvRouteData *>(r->getProtocolData()); }
    void control(const Ptr<AodvControlPacket>& h, int output) {
        if (!using6) { transmit(h, output); return; }
        auto packet = new Packet("scripted-aodv", h);
        emitIpv6(packet, dut, 654, output);
    }
    void emitIpv6(Packet *packet, const L3Address& target, int port, int output) {
        auto iface = check_and_cast<IInterfaceTable *>(getModuleByPath("^.interfaceTable"))->findInterfaceByName(("wlan" + std::to_string(output)).c_str());
        packet->addTag<L3AddressReq>()->setSrcAddress(iface->getNetworkAddress());
        packet->addTag<NextHopAddressReq>()->setNextHopAddress(dut);
        packet->addTag<HopLimitReq>()->setHopLimit(10);
        sourceOutputs[packet] = iface->getInterfaceId();
        socket.sendTo(packet, target, port);
    }
    void sourceRoute() {
        auto q = makeShared<Rreq>(); q->setPacketType(using6 ? RREQ_IPv6 : RREQ); q->setChunkLength(B(using6 ? 48 : 24));
        q->setOriginatorAddr(peerAddress(3)); q->setOriginatorSeqNum(10); q->setRreqId(50);
        q->setDestAddr(dut); q->setUnknownSeqNumFlag(true); q->setHopCount(2);
        if (mode == "HelloSuccess" || mode == "NeighborSuccess")
            check_and_cast<AodvRepairSource *>(getModuleByPath("^.^.source.app[0]"))->emitPacket(new Packet("repair-source-request", q), dut, 654);
        else control(q, 1);
    }
    void offer(int prefix, uint32_t seq, unsigned metric, int output, L3Address target = L3Address()) {
        auto r = makeShared<Rrep>(); r->setPacketType(using6 ? RREP_IPv6 : RREP); r->setChunkLength(B(using6 ? 44 : 20)); r->setDestAddr(target.isUnspecified() ? member : target);
        r->setPrefixSize(prefix == (using6 ? 128 : 32) ? 0 : prefix); r->setDestSeqNum(seq); r->setHopCount(metric - 1);
        r->setAckRequiredFlag(using6); // prime native ND before the subsequent real MAC loss
        r->setOriginatorAddr(dut); r->setLifeTime(2);
        if (using6) {
            AodvRepairWire serializer; MemoryOutputStream stream; serializer.serializeFields(stream, r);
            MemoryInputStream input(stream.getData()); auto decoded = dynamicPtrCast<const Rrep>(serializer.deserializeFields(input, typeid(Rrep)));
            ASSERT(stream.getData().size() == 44 && decoded && decoded->getDestAddr() == member && decoded->getDestSeqNum() == seq);
        }
        control(r, output);
    }
    void data(const L3Address& target, bool local) {
        auto packet = new AodvTrackedPacket("repair-data", makeShared<ByteCountChunk>(B(8)));
        if (local) {
            if (mode == "OutputConstraint") packet->addTag<NextHopAddressReq>()->setNextHopAddress(peer);
            check_and_cast<AodvSubnetSource *>(getModuleByPath("^.^.dut.app[0]"))->emitData(packet, target);
        }
        else {
            if (using6) { emitIpv6(packet, target, 9999, 1); return; }
            if (mode == "HelloSuccess" || mode == "NeighborSuccess") {
                check_and_cast<AodvRepairSource *>(getModuleByPath("^.^.source.app[0]"))->emitPacket(packet, target, 9999);
                return;
            }
            packet->addTag<InterfaceReq>()->setInterfaceId(check_and_cast<IInterfaceTable *>(getModuleByPath("^.interfaceTable"))->findInterfaceByName("wlan1")->getInterfaceId());
            packet->addTag<NextHopAddressReq>()->setNextHopAddress(dut); socket.sendTo(packet, target, 9999);
        }
    }
    void stop(bool restart) {
        auto operation = mode == "Shutdown" ? static_cast<LifecycleOperation *>(new ModuleStopOperation()) : new ModuleCrashOperation();
        LifecycleOperation::StringMap params; operation->initialize(getModuleByPath("^.^.dut"), params);
        operationPending = !initiateOperation(operation, this);
        if (restart) { auto operation = new ModuleStartOperation(); LifecycleOperation::StringMap params; operation->initialize(getModuleByPath("^.^.dut"), params); ASSERT(initiateOperation(operation)); }
    }
    void run() {
        if (mode.find("OriginatingFirst") == 0) { runOriginatingFirst(); return; }
        if (mode.find("Deferred") == 0) { runDeferred(); return; }
        if (mode == "SecondFailure") { runSecondFailure(); return; }
        if (step == 0) { if (mode == "RateStarved" || mode == "RateStop") data(Ipv4Address("10.9.0.99"), true); }
        if (step == 1) sourceRoute();
        if (step == 2) offer(prefixLength, 20, mode == "Distance" ? 4 : 3, 0);
        if (step == 3) {
            ASSERT(state()->isActive());
            if (unknown()) state()->setHasValidDestNum(false);
            if (mode == "SourceMissing") { auto r = owned(Ipv4Address("10.0.0.3"), 32); ASSERT(r); routes->deleteRoute(r); }
            check_and_cast<inet::physicallayer::IRadio *>(getModuleByPath("^.wlan[0].radio"))->setRadioMode(inet::physicallayer::IRadio::RADIO_MODE_OFF);
        }
        if (step == 4) data(member, false);
        if (step == 5) {
            ASSERT(!state()->isActive() && state()->getDestSeqNum() == (unknown() ? 20u : 21u));
            ASSERT(state()->isBeingRepaired() == !declined());
            if (!declined()) { ASSERT(errors.empty()); data(member, false); }
        }
        if (step == 6 && !declined()) data(member, true);
        if (step == 7 && aggregate() && mode != "EmptySuccess" && mode != "EmptyFailure") data(other, false);
        if (step == 8) {
            if (mode == "HostSuccess" || mode == "AggregateSuccess" || mode == "OutputConstraint") offer(prefixLength, 21, 2, 2);
            if (mode == "Longer") offer(prefixLength, 21, 4, 2);
            if (mode == "Stale") offer(prefixLength, 20, 2, 2);
            if (mode == "IndependentHost" || mode == "EmptySuccess" || mode == "EmptyFailure") offer(32, 5, 2, 2);
            if (unknown()) offer(24, 100, 2, 2);
            if (mode == "HelloSuccess") {
                auto h = makeShared<Rrep>(); h->setPacketType(RREP); h->setChunkLength(B(20)); h->setDestAddr(member); h->setDestSeqNum(21); h->setLifeTime(2); control(h, 2);
            }
            if (mode == "NeighborSuccess") sendRecoveryRequest(false);
            if (lifecycle()) stop(mode == "Restart");
        }
        if (step == 9 && (mode == "EmptySuccess" || mode == "EmptyFailure")) { ASSERT(state()->isBeingRepaired() && dataIds.size() == 2); if (mode == "EmptySuccess") offer(prefixLength, 21, 4, 2); }
        if (step == 9 && unknown()) { ASSERT(state()->isBeingRepaired() && dataIds.size() == 2); if (mode == "UnknownHostSuccess") sendRecoveryRequest(true); }
        if (step == 10) {
            if (succeeds()) {
                ASSERT(state()->isActive() && !state()->isBeingRepaired() && !state()->isRepariable());
                ASSERT(dataIds.size() == (mode == "AggregateSuccess" ? 3u : mode == "OutputConstraint" ? 1u : mode == "TimeoutFirst" ? 0u : 2u));
            }
            if (mode == "IndependentHost" || mode == "EmptyFailure") {
                ASSERT(dataIds.size() == 2 && owned(member, 32));
                ASSERT(check_and_cast<AodvRouteData *>(owned(member, 32)->getProtocolData())->getDestSeqNum() == 5);
            }
            if (!starved() && !lifecycle() && !succeeds())
                ASSERT(!state()->isActive() && !state()->isBeingRepaired() && !state()->isRepariable() && state()->getDestSeqNum() == (unknown() ? 20u : 21u));
            if (lifecycle()) ASSERT(!operationPending && !owned(member, prefixLength));
        }
        if (step == 11 && starved() && !lifecycle()) { ASSERT(!state()->isActive() && !state()->isBeingRepaired() && !state()->isRepariable() && state()->getDestSeqNum() == 21); if (mode == "RateStarved") stop(false); }
        if (step == 12) {
            int repairRequests = 0;
            for (const auto& q : requests) if (q->getDestAddr() == member) {
                repairRequests++; ASSERT(q->getHopCount() == 0);
                ASSERT(q->getUnknownSeqNumFlag() == (aggregate() || unknown()));
                ASSERT(q->getDestSeqNum() == (aggregate() || unknown() ? 0u : 21u));
                ASSERT(q->getDestOnlyFlag() == aggregate());
            }
            ASSERT(repairRequests == (declined() || starved() ? 0 : 1));
            if (succeeds()) {
                ASSERT((state()->isActive() || mode == "UnknownHostSuccess" || mode == "NeighborSuccess") && !state()->isBeingRepaired() && !state()->isRepariable());
                ASSERT(state()->getDestSeqNum() == (mode == "UnknownHostSuccess" ? 7u : 21u));
            }
            bool noError = (succeeds() && mode != "Longer" && mode != "EmptySuccess" && mode != "TimeoutFirst") || lifecycle();
            ASSERT(errors.empty() == noError);
            if (!noError) {
                ASSERT(errors.size() == 1 && errors[0]->getUnreachableNodesArraySize() == 1);
                ASSERT(errors[0]->getNoDeleteFlag() == (mode == "Longer" || mode == "EmptySuccess"));
                ASSERT(errors[0]->getUnreachableNodes(0).addr == member.getPrefix(prefixLength));
                ASSERT(errors[0]->getUnreachableNodes(0).seqNum == (unknown() ? 0u : 21u));
            }
            ASSERT(AodvDataCounters::created == AodvDataCounters::destroyed);
            checkDisposal();
            if (tie()) { auto ingress = check_and_cast<AodvRepairIngress *>(getModuleByPath("^.^.dut.ingress")); ASSERT(ingress->scheduled == 1 && ingress->delivery == tieBoundary); }
            if (unknown()) ASSERT(owned(member, 24) && check_and_cast<AodvRouteData *>(owned(member, 24)->getProtocolData())->isActive());
            if (mode == "HelloSuccess" || mode == "NeighborSuccess") ASSERT(check_and_cast<AodvRepairSource *>(getModuleByPath("^.^.source.app[0]"))->errorCount == 0);
            std::cout << "AODV LocalRepair " << mode << ": PASS\n"; return;
        }
        static const double times[] = {0.02, 0.05, 0.09, 0.13, 0.17, 0.21, 0.22, 0.23, 0.25, 0.28, 0.36, 0.65, 0.9};
        step++; scheduleAt(step == 12 && mode == "JitterStarved" ? 1.25 : times[step], timer);
    }
    void sendRecoveryRequest(bool ownsPrefix) {
        auto q = makeShared<Rreq>(); q->setPacketType(RREQ); q->setChunkLength(B(24));
        q->setOriginatorAddr(ownsPrefix ? member : L3Address(Ipv4Address("10.0.0.50")));
        q->setOriginatorSeqNum(7); q->setRreqId(77); q->setDestAddr(dut); q->setUnknownSeqNumFlag(true); q->setHopCount(1); control(q, 2);
    }
    void runOriginatingFirst() {
        L3Address second = Ipv4Address("10.9.0.99");
        if (step == 1) sourceRoute();
        if (step == 2) { offer(prefixLength, 20, 3, 0); offer(prefixLength, 30, 3, 0, second); }
        if (step == 3) check_and_cast<inet::physicallayer::IRadio *>(getModuleByPath("^.wlan[0].radio"))->setRadioMode(inet::physicallayer::IRadio::RADIO_MODE_OFF);
        if (step == 4) data(member, false);
        if (step == 5) {
            ASSERT(state()->isBeingRepaired());
            ASSERT(check_and_cast<AodvRouteData *>(owned(second, prefixLength)->getProtocolData())->isRepariable());
            data(second, true);
        }
        if (step == 6) data(aggregate() ? L3Address(Ipv4Address("10.9.0.100")) : second, false);
        if (step == 8) offer(prefixLength, 32, 2, 2, second);
        if (step == 9) { ASSERT(dataIds.size() == 1 && state()->isBeingRepaired()); offer(prefixLength, 21, 4, 2); }
        if (step == 10) {
            ASSERT(state()->isActive() && !state()->isBeingRepaired());
            auto r = owned(second, prefixLength); ASSERT(r);
            ASSERT(check_and_cast<AodvRouteData *>(r->getProtocolData())->isActive() && !check_and_cast<AodvRouteData *>(r->getProtocolData())->isBeingRepaired());
            ASSERT(errors.size() == 2 && requests.size() == 2 && dataIds.size() == 1);
            ASSERT(!errors[0]->getNoDeleteFlag() && errors[0]->getUnreachableNodes(0).addr == second.getPrefix(prefixLength) && errors[0]->getUnreachableNodes(0).seqNum == 32);
            ASSERT(errors[1]->getNoDeleteFlag() && errors[1]->getUnreachableNodes(0).addr == member.getPrefix(prefixLength));
        }
        if (step == 12) { ASSERT(errors.size() == 2 && AodvDataCounters::created == 3 && AodvDataCounters::destroyed == 3); checkDisposal(); std::cout << "AODV LocalRepair " << mode << ": PASS\n"; return; }
        static const double times[] = {0.02, 0.05, 0.09, 0.13, 0.17, 0.21, 0.22, 0.23, 0.25, 0.28, 0.36, 0.65, 0.9};
        step++; scheduleAt(times[step], timer);
    }
    void runSecondFailure() {
        if (step == 1) sourceRoute();
        if (step == 2) offer(32, 20, 3, 0);
        if (step == 3) check_and_cast<inet::physicallayer::IRadio *>(getModuleByPath("^.wlan[0].radio"))->setRadioMode(inet::physicallayer::IRadio::RADIO_MODE_OFF);
        if (step == 4 || step == 5) data(member, false);
        if (step == 6) data(member, true);
        if (step == 8) offer(32, 21, 2, 2);
        if (step == 9) {
            ASSERT(state()->isActive() && !state()->isBeingRepaired() && dataIds.size() == 2);
            check_and_cast<inet::physicallayer::IRadio *>(getModuleByPath("^.wlan[0].radio"))->setRadioMode(inet::physicallayer::IRadio::RADIO_MODE_RECEIVER);
            check_and_cast<inet::physicallayer::IRadio *>(getModuleByPath("^.wlan[2].radio"))->setRadioMode(inet::physicallayer::IRadio::RADIO_MODE_OFF);
            data(member, false);
        }
        if (step == 10) { ASSERT(state()->isBeingRepaired() && state()->getDestSeqNum() == 22); data(member, false); }
        if (step == 11) data(member, true);
        if (step == 12) offer(32, 22, 3, 0);
        if (step == 13) {
            ASSERT(state()->isActive() && !state()->isBeingRepaired() && !state()->isRepariable() && state()->getDestSeqNum() == 22);
            ASSERT(dataIds.size() == 4 && requests.size() == 2 && errors.size() == 1);
            ASSERT(requests[0]->getDestSeqNum() == 21 && requests[1]->getDestSeqNum() == 22);
            ASSERT(errors[0]->getNoDeleteFlag() && errors[0]->getUnreachableNodes(0).seqNum == 22);
            ASSERT(AodvDataCounters::created == 6 && AodvDataCounters::destroyed == 6);
            checkDisposal();
            std::cout << "AODV LocalRepair SecondFailure: PASS\n"; return;
        }
        static const double times[] = {0.02, 0.05, 0.09, 0.13, 0.17, 0.21, 0.22, 0.23, 0.25, 0.36, 0.4, 0.42, 0.45, 0.9};
        step++; scheduleAt(times[step], timer);
    }
    void runDeferred() {
        L3Address second = Ipv4Address("10.9.0.99"); bool deadline = mode == "DeferredDeadline";
        if (step == 1) sourceRoute();
        if (step == 2) { offer(prefixLength, 20, 3, 0); offer(prefixLength, 30, 3, 0, second); }
        if (step == 3) check_and_cast<inet::physicallayer::IRadio *>(getModuleByPath("^.wlan[0].radio"))->setRadioMode(inet::physicallayer::IRadio::RADIO_MODE_OFF);
        if (step == 4) data(member, false);
        if (step == 5) {
            ASSERT(state()->isBeingRepaired() && requests.size() == 1);
            auto data = check_and_cast<AodvRouteData *>(owned(second, prefixLength)->getProtocolData());
            ASSERT(data->isRepariable() && !data->isBeingRepaired() && data->getDestSeqNum() == 31);
            if (!deadline) this->data(second, false);
        }
        if (step == 6 && !deadline) data(second, true);
        if (step == 8) offer(prefixLength, 21, 2, 2);
        if (step == 9 && !deadline) {
            ASSERT(!state()->isBeingRepaired() && check_and_cast<AodvRouteData *>(owned(second, prefixLength)->getProtocolData())->isBeingRepaired());
            offer(prefixLength, 31, 2, 2, second);
        }
        if (step == 10) {
            ASSERT(state()->isActive() && !state()->isBeingRepaired());
            auto data = check_and_cast<AodvRouteData *>(owned(second, prefixLength)->getProtocolData());
            ASSERT(data->isRepariable() == deadline && !data->isBeingRepaired() && data->isActive() == !deadline);
        }
        if (step == 11 && deadline) {
            ASSERT(!check_and_cast<AodvRouteData *>(owned(second, prefixLength)->getProtocolData())->isRepariable()); data(second, false);
        }
        if (step == 12) {
            ASSERT(state()->isActive() && !state()->isBeingRepaired());
            ASSERT(requests.size() == (deadline ? 1u : 2u) && dataIds.size() == (deadline ? 0u : 2u));
            if (deadline) { ASSERT(errors.size() == 1 && !errors[0]->getNoDeleteFlag()); ASSERT(errors[0]->getUnreachableNodes(0).addr == second && errors[0]->getUnreachableNodes(0).seqNum == 32); }
            else { ASSERT(errors.empty()); ASSERT(requests[1]->getDestAddr() == second && requests[1]->getUnknownSeqNumFlag() == aggregate() && requests[1]->getDestSeqNum() == (aggregate() ? 0u : 31u)); }
            checkDisposal(); std::cout << "AODV LocalRepair " << mode << ": PASS\n"; return;
        }
        static const double times[] = {0.02, 0.05, 0.09, 0.13, 0.17, 0.21, 0.22, 0.23, 0.25, 0.28, 0.36, 0.65, 0.9};
        step++; scheduleAt(step == 11 && deadline ? 0.75 : times[step], timer);
    }
  protected:
    void initialize(int stage) override {
        if (stage != INITSTAGE_APPLICATION_LAYER) return;
        mode = par("scenario").stdstringValue(); prefixLength = aggregate() ? 24 : 32;
        using6 = hasPar("ipv6") && par("ipv6").boolValue();
        if (using6) { prefixLength = 128; dut = peerAddress(1); peer = peerAddress(2); member = Ipv6Address("2001:db8:8::99"); other = Ipv6Address("2001:db8:8::100"); }
        if (mode == "HelloSuccess" || mode == "NeighborSuccess") member = Ipv4Address("10.0.0.4");
        socket.setOutputGate(gate("socketOut")); socket.setCallback(this); socket.bind(654); socket.setBroadcast(true);
        dataSink.setOutputGate(gate("socketOut")); dataSink.setCallback(this); dataSink.bind(9999);
        if (using6) {
            auto interfaces = check_and_cast<IInterfaceTable *>(getModuleByPath("^.interfaceTable"));
            for (int i = 0; i < 3; i++) socket.joinMulticastGroup(Ipv6Address("ff01::1"), interfaces->findInterfaceByName(("wlan" + std::to_string(i)).c_str())->getInterfaceId());
            sourceIp = check_and_cast<INetfilter *>(getModuleByPath("^.ipv6.ipv6")); sourceIp->registerHook(0, this);
            scheduleAt(0.11, new cMessage("prime-ipv6-neighbor"));
        }
        routes = check_and_cast<IRoutingTable *>(getModuleByPath(using6 ? "^.^.dut.ipv6.routingTable" : "^.^.dut.ipv4.routingTable"));
        getModuleByPath(using6 ? "^.ipv6.ipv6" : "^.ipv4.ip")->subscribe(packetReceivedFromLowerSignal, this);
        if (tie()) { getModuleByPath("^.^.dut")->subscribe(linkBrokenSignal, this); scheduleAt(0.14, new cMessage("prepare-tie-reply")); }
        timer = new cMessage("repair-script"); scheduleAt(0.02, timer);
    }
    void handleMessage(cMessage *message) override {
        if (message->isSelfMessage() && message != timer) {
            if (!strcmp(message->getName(), "prime-ipv6-neighbor")) data(peer, true);
            else if (!strcmp(message->getName(), "prepare-tie-reply")) offer(prefixLength, 21, 2, 2);
            else if (!strcmp(message->getName(), "release-tie-reply")) check_and_cast<AodvRepairIngress *>(getModuleByPath("^.^.dut.ingress"))->releaseAt(tieBoundary);
            else throw cRuntimeError("Unknown repair fixture timer");
            delete message;
        }
        else if (message == timer) run(); else if (dataSink.belongsToSocket(message)) dataSink.processMessage(message); else socket.processMessage(message);
    }
    void socketDataArrived(UdpSocket *receiver, Packet *packet) override {
        if (receiver == &dataSink) { delete packet; return; }
        auto h = packet->peekAtFront<AodvControlPacket>();
        if (auto q = dynamicPtrCast<const Rreq>(h)) {
            if (requestIds.insert({q->getOriginatorAddr(), q->getRreqId()}).second) requests.push_back(q);
            if (q->getDestAddr() == member) ASSERT(packet->getTag<HopLimitInd>()->getHopLimit() == (mode == "SecondFailure" && requests.size() > 1 ? 4 : 5));
        }
        if (auto e = dynamicPtrCast<const Rerr>(h)) {
            if (!errorIds.insert(packet->getTreeId()).second) { delete packet; return; }
            if (using6) {
                AodvRepairWire serializer; MemoryOutputStream stream; serializer.serializeFields(stream, e);
                MemoryInputStream input(stream.getData()); auto decoded = dynamicPtrCast<const Rerr>(serializer.deserializeFields(input, typeid(Rerr)));
                ASSERT(e->getPacketType() == RERR_IPv6 && stream.getData().size() == 24 && decoded && decoded->getUnreachableNodes(0).addr == member);
            }
            errors.push_back(e);
        }
        if (auto r = dynamicPtrCast<const Rrep>(h)) if (r->getAckRequiredFlag()) { auto a = makeShared<RrepAck>(); a->setPacketType(using6 ? RREPACK_IPv6 : RREPACK); a->setChunkLength(B(2)); transmit(a, 1); }
        delete packet;
    }
    void receiveSignal(cComponent *, simsignal_t signal, cObject *object, cObject *) override {
        Enter_Method("observeRepairedData"); auto p = dynamic_cast<Packet *>(object); if (!p) return;
        if (signal == linkBrokenSignal) {
            tieBoundary = simTime() + SimTime(0.14);
            if (mode == "ReplyFirst") check_and_cast<AodvRepairIngress *>(getModuleByPath("^.^.dut.ingress"))->releaseAt(tieBoundary);
            else scheduleAt(simTime() + SimTime(1, SIMTIME_NS), new cMessage("release-tie-reply"));
            return;
        }
        L3Address destination, source; b length; int protocol;
        if (using6) { auto ip = p->peekAtFront<Ipv6Header>(); destination = ip->getDestAddress(); source = ip->getSrcAddress(); length = ip->getChunkLength(); protocol = ip->getProtocolId(); }
        else { auto ip = p->peekAtFront<Ipv4Header>(); destination = ip->getDestAddress(); source = ip->getSrcAddress(); length = ip->getChunkLength(); protocol = ip->getProtocolId(); }
        if (protocol != IP_PROT_UDP || p->peekDataAt<UdpHeader>(length)->getDestPort() != 9999) return;
        if (destination == member || destination == other || ((mode.find("OriginatingFirst") == 0 || mode.find("Deferred") == 0) && destination == L3Address(Ipv4Address("10.9.0.99")))) {
            auto alternate = check_and_cast<IInterfaceTable *>(getModuleByPath("^.interfaceTable"))->findInterfaceByName(mode == "SecondFailure" && simTime() > SimTime(0.45) ? "wlan0" : "wlan2");
            auto mac = p->getTag<MacAddressInd>()->getDestAddress();
            ASSERT(mac == alternate->getMacAddress() || (using6 && source == dut && mac.isBroadcast()));
            dataIds.insert(p->getTreeId());
        }
    }
    void finish() override { if (sourceIp) { sourceIp->unregisterHook(this); sourceIp = nullptr; } }
};
Define_Module(AodvRepairPeer);
#endif

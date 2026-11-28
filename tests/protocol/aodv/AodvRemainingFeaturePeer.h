// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __AODV_REMAINING_FEATURE_PEER_H
#define __AODV_REMAINING_FEATURE_PEER_H

#include "AodvPeer.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/lifecycle/LifecycleController.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/networklayer/common/Icmpv4ErrorTag_m.h"
#include "inet/networklayer/common/Icmpv6ErrorTag_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/ipv6/Ipv6Header_m.h"
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/icmpv6/Icmpv6Header_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"
#include "inet/routing/aodv/AodvControlPacketsSerializer.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/common/packet/chunk/BytesChunk.h"

class AodvFeatureWire : public AodvControlPacketsSerializer
{
  public:
    using AodvControlPacketsSerializer::serializeFields;
};

// Public interface-table setup before AODV's actual start validation.
class AodvFeatureSetup : public SimpleModule
{
  protected:
    int numInitStages() const override { return NUM_INIT_STAGES; }
    void initialize(int stage) override {
        if (stage != INITSTAGE_ROUTER_ID_ASSIGNMENT) return;
        std::string mode = par("mode").stdstringValue();
        auto table = check_and_cast<IInterfaceTable *>(getModuleByPath("^.dut.interfaceTable"));
        auto primary = table->findInterfaceByName("wlan0");
        auto secondary = table->findInterfaceByName("wlan1");
        if (mode == "MixedFamilies") {
            auto data = secondary->addProtocolData<Ipv4InterfaceData>();
            data->setIPAddress(Ipv4Address("10.1.0.1")); data->setNetmask(Ipv4Address("255.255.255.0"));
        }
        if (mode == "Unspecified") { delete primary->removeProtocolData<Ipv6InterfaceData>(); primary->addProtocolData<Ipv6InterfaceData>(); }
        if (mode == "Tentative") {
            delete primary->removeProtocolData<Ipv6InterfaceData>();
            primary->addProtocolData<Ipv6InterfaceData>()->assignAddress(Ipv6Address("2001:db8:1::1"), true, 0, 0);
        }
        if (mode == "DuplicateAddresses") {
            delete secondary->removeProtocolData<Ipv6InterfaceData>();
            secondary->addProtocolData<Ipv6InterfaceData>()->assignAddress(primary->getNetworkAddress().toIpv6(), false, 0, 0);
        }
        if (mode == "LinkLocalOnly") {
            delete primary->removeProtocolData<Ipv6InterfaceData>(); delete secondary->removeProtocolData<Ipv6InterfaceData>();
            primary->addProtocolData<Ipv6InterfaceData>()->assignAddress(Ipv6Address("fe80::1"), false, 0, 0);
            secondary->addProtocolData<Ipv6InterfaceData>()->assignAddress(Ipv6Address("fe80::2"), false, 0, 0);
        }
    }
    void handleMessage(cMessage *message) override { delete message; }
};
Define_Module(AodvFeatureSetup);

class AodvFeatureSource : public SimpleModule
{
    UdpSocket socket;
  protected:
    int numInitStages() const override { return NUM_INIT_STAGES; }
    void initialize(int stage) override {
        if (stage == INITSTAGE_APPLICATION_LAYER) resetSocket();
    }
    void handleMessage(cMessage *message) override {
        if (auto error = dynamic_cast<Indication *>(message)) {
            const Packet *quoted = nullptr;
            if (auto tag = error->findTag<Icmpv4ErrorInd>()) {
                ASSERT(tag->getType() == ICMP_DESTINATION_UNREACHABLE && tag->getCode() == ICMP_DU_HOST_UNREACHABLE);
                quoted = tag->getOriginalPacket();
            }
            if (auto tag = error->findTag<Icmpv6ErrorInd>()) {
                ASSERT(tag->getType() == ICMPv6_DESTINATION_UNREACHABLE && tag->getCode() == (memberErrorExpected ? ADDRESS_UNREACHABLE : NO_ROUTE_TO_DEST));
                quoted = tag->getOriginalPacket();
            }
            ASSERT(quoted);
            auto address = quoted->getTag<L3AddressInd>()->getDestAddress();
            auto ports = quoted->getTag<L4PortInd>();
            ASSERT(ports->getSrcPort() == 12345 && ports->getDestPort() == 9999);
            errors.push_back(address);
        }
        delete message;
    }
  public:
    std::vector<L3Address> errors;
    bool memberErrorExpected = false;
    void resetSocket() {
        Enter_Method("resetFeatureSocket");
        socket = UdpSocket(); socket.setOutputGate(gate("socketOut")); socket.bind(12345);
    }
    void emitData(const L3Address& target) {
        Enter_Method("emitFeatureData");
        socket.sendTo(new AodvTrackedPacket("remaining-feature-data", makeShared<ByteCountChunk>(B(8))), target, 9999);
    }
};
Define_Module(AodvFeatureSource);

class AodvRemainingFeaturePeer : public SimpleModule, public UdpSocket::ICallback,
        public cListener, public LifecycleController, public IDoneCallback
{
    struct Request { L3Address target, originator, source; uint32_t id, sequence; int input; simtime_t time; };
    struct Reply { Ptr<const Rrep> header; L3Address source; int input; };
    UdpSocket socket;
    cMessage *timer = nullptr;
    IRoutingTable *routes = nullptr;
    AodvFeatureSource *source = nullptr;
    IInterfaceTable *peerInterfaces = nullptr;
    IInterfaceTable *dutInterfaces = nullptr;
    std::string family, mode;
    bool using6 = true, twoLinks = true, pending = false;
    int step = 0;
    std::vector<Request> requests;
    std::vector<Reply> replies;
    std::vector<std::pair<Ptr<const Rerr>, int>> errors;
    std::map<int, int> acknowledgments;
    std::set<long> delivered;
    std::vector<L3Address> deliveredTargets;
    std::set<int> deliveredInputs;
    int unexpectedIcmp = 0;
    L3Address dut, member, other;

    int numInitStages() const override { return NUM_INIT_STAGES; }
    NetworkInterface *peerInterface(int index) { return peerInterfaces->findInterfaceByName(("wlan" + std::to_string(index)).c_str()); }
    NetworkInterface *dutInterface(int index) { return dutInterfaces->findInterfaceByName(("wlan" + std::to_string(twoLinks ? index : 0)).c_str()); }
    L3Address dutAddress(int output) { return dutInterface(output)->getNetworkAddress(); }
    int inputIndex(Packet *packet) {
        int id = packet->getTag<InterfaceInd>()->getInterfaceId();
        for (int i = 0; i < 2; ++i) if (peerInterface(i)->getInterfaceId() == id) return i;
        throw cRuntimeError("Unexpected peer input interface");
    }
    IRoute *route(const L3Address& target, int prefix) {
        for (int i = 0; i < routes->getNumRoutes(); ++i) {
            auto r = routes->getRoute(i);
            if (r->getSourceType() == IRoute::AODV && r->getDestinationAsGeneric() == target.getPrefix(prefix) && r->getPrefixLength() == prefix) return r;
        }
        return nullptr;
    }
    AodvRouteData *state(const L3Address& target, int prefix) {
        auto r = route(target, prefix); ASSERT(r); return check_and_cast<AodvRouteData *>(r->getProtocolData());
    }
    void transmit(const Ptr<AodvControlPacket>& header, int output, bool multicast = false) {
        auto p = new Packet("remaining-feature-control");
        if (family == "Subnet") {
            AodvFeatureWire serializer; MemoryOutputStream bytes;
            serializer.serializeFields(bytes, header);
            if (auto reply = dynamicPtrCast<const Rrep>(header)) {
                ASSERT(bytes.getData().size() == 44);
                ASSERT(bytes.getData()[0] == RREP_IPv6 && (bytes.getData()[2] & 0x7f) == reply->getPrefixSize());
            }
            p->insertAtBack(makeShared<BytesChunk>(bytes.getData()));
        }
        else p->insertAtBack(header);
        auto target = multicast ? L3Address(Ipv6Address("ff01::1")) : dutAddress(output);
        p->addTag<InterfaceReq>()->setInterfaceId(peerInterface(output)->getInterfaceId());
        p->addTag<L3AddressReq>()->setSrcAddress(peerInterface(output)->getNetworkAddress());
        p->addTag<NextHopAddressReq>()->setNextHopAddress(target);
        p->addTag<HopLimitReq>()->setHopLimit(10);
        socket.sendTo(p, target, 654);
    }
    void offer(const L3Address& target, unsigned int prefix, uint32_t sequence, int output,
            simtime_t lifetime = 2, bool ack = false, L3Address originator = L3Address()) {
        auto h = makeShared<Rrep>(); h->setPacketType(using6 ? RREP_IPv6 : RREP); h->setChunkLength(using6 ? B(44) : B(20));
        h->setOriginatorAddr(originator.isUnspecified() ? dut : originator); h->setDestAddr(target); h->setDestSeqNum(sequence);
        h->setPrefixSize(prefix == (using6 ? 128u : 32u) ? 0 : prefix); h->setLifeTime(lifetime); h->setHopCount(1); h->setAckRequiredFlag(ack);
        transmit(h, output);
    }
    void ask(const L3Address& target, int output, bool destinationOnly = false, bool multicast = false) {
        auto h = makeShared<Rreq>(); h->setPacketType(using6 ? RREQ_IPv6 : RREQ); h->setChunkLength(using6 ? B(48) : B(24));
        h->setOriginatorAddr(peerInterface(output)->getNetworkAddress()); h->setOriginatorSeqNum(step + 1); h->setRreqId(step + 1);
        h->setDestAddr(target); h->setUnknownSeqNumFlag(true); h->setDestOnlyFlag(destinationOnly); transmit(h, output, multicast);
    }
    void hello(int output) {
        auto h = makeShared<Rrep>(); h->setPacketType(RREP_IPv6); h->setChunkLength(B(44));
        h->setDestAddr(peerInterface(output)->getNetworkAddress()); h->setDestSeqNum(7); h->setLifeTime(0.4);
        transmit(h, output);
    }
    void sendError(const L3Address& target, uint32_t sequence, int output) {
        auto h = makeShared<Rerr>(); h->setPacketType(using6 ? RERR_IPv6 : RERR); h->setChunkLength(using6 ? B(24) : B(12));
        h->setUnreachableNodesArraySize(1); UnreachableNode node; node.addr = target; node.seqNum = sequence; h->setUnreachableNodes(0, node);
        transmit(h, output);
    }
    void transitData() {
        auto p = new AodvTrackedPacket("remaining-feature-transit", makeShared<ByteCountChunk>(B(8)));
        p->addTag<InterfaceReq>()->setInterfaceId(peerInterface(1)->getInterfaceId());
        p->addTag<L3AddressReq>()->setSrcAddress(peerInterface(1)->getNetworkAddress());
        p->addTag<NextHopAddressReq>()->setNextHopAddress(dutAddress(1));
        socket.sendTo(p, member, 9999);
    }
    void memberError() {
        auto p = new Packet("remaining-member-unreachable");
        auto udp = makeShared<UdpHeader>(); udp->setSrcPort(12345); udp->setDestPort(9999); udp->setTotalLengthField(B(8)); p->insertAtFront(udp);
        auto quoted = makeShared<Ipv6Header>(); quoted->setSrcAddress(dut.toIpv6()); quoted->setDestAddress(member.toIpv6());
        quoted->setProtocolId(IP_PROT_UDP); quoted->setPayloadLength(B(8)); p->insertAtFront(quoted);
        auto icmp = makeShared<Icmpv6DestUnreachableMsg>(); icmp->setType(ICMPv6_DESTINATION_UNREACHABLE); icmp->setCode(ADDRESS_UNREACHABLE);
        icmp->setChecksumMode(CHECKSUM_DECLARED_CORRECT); p->insertAtFront(icmp);
        p->addTag<PacketProtocolTag>()->setProtocol(&Protocol::icmpv6); p->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ipv6);
        auto addresses = p->addTag<L3AddressReq>(); addresses->setSrcAddress(peerInterface(0)->getNetworkAddress()); addresses->setDestAddress(dut);
        p->addTag<InterfaceReq>()->setInterfaceId(peerInterface(0)->getInterfaceId()); p->addTag<NextHopAddressReq>()->setNextHopAddress(dut);
        send(p, "socketOut");
    }
    void operate(LifecycleOperation *operation) {
        LifecycleOperation::StringMap params; operation->initialize(getModuleByPath("^.^.dut"), params);
        pending = !initiateOperation(operation, this);
    }
    void invoke() override { Enter_Method("featureOperationCompleted"); pending = false; }
    void finalCheck() {
        ASSERT(!pending);
        if (family == "Discovery") {
            if (mode == "Recovered") { ASSERT(source->errors.empty() && delivered.size() == 1); }
            else if (mode == "StopCrash") { ASSERT(source->errors.empty() && delivered.empty()); }
            else if (mode == "RepairFailure") {
                std::cout << "RepairFailure: local errors=" << source->errors.size() << ", RERR=" << errors.size() << ", peer ICMP=" << unexpectedIcmp << "\n";
                ASSERT(source->errors.empty() && !errors.empty() && unexpectedIcmp == 0);
            }
            else {
                ASSERT(source->errors.size() == 2 && delivered.empty());
                ASSERT(std::multiset<L3Address>(source->errors.begin(), source->errors.end()) == std::multiset<L3Address>({member, other}));
                ASSERT(requests.size() == (mode == "SharedOwner" ? 1u : 2u));
            }
        }
        else if (family == "Interfaces") {
            if (mode == "IngressReply") {
                ASSERT(replies.size() == 1 && replies[0].input == 1 && replies[0].source == dutAddress(1));
                ASSERT(replies[0].header->getDestAddr() == dutAddress(1));
            }
            else if (mode == "AckIsolation") { ASSERT(acknowledgments[0] == 1 && acknowledgments[1] == 1); }
            else if (mode == "ErrorPrecursors") {
                ASSERT(!errors.empty() && errors[0].second == 1 && !state(member, 128)->isActive());
                ASSERT(errors[0].first->getUnreachableNodes(0).addr == member);
            }
            else if (mode == "HelloIsolation") {
                std::cout << "HelloIsolation: primary dependent=" << state(member, 128)->isActive() << ", secondary dependent=" << state(other, 128)->isActive() << "\n";
                ASSERT(!state(member, 128)->isActive() && state(other, 128)->isActive());
            }
            else if (mode == "RestartMembership" || mode == "StartAfterDownInit") {
                ASSERT(replies.size() == (mode == "RestartMembership" ? 4u : 2u));
                ASSERT(std::count_if(replies.begin(), replies.end(), [](const Reply& r) { return r.input == 0; }) == int(replies.size() / 2));
            }
            else {
                size_t targets = mode == "RateBudget" ? 2 : 1;
                ASSERT(requests.size() == 2 * targets && delivered.size() == targets && source->errors.empty());
                for (size_t t = 0; t < targets; ++t) {
                    auto target = t == 0 ? member : other;
                    std::vector<Request> pair; for (const auto& r : requests) if (r.target == target) pair.push_back(r);
                    ASSERT(pair.size() == 2 && pair[0].id == pair[1].id && pair[0].sequence == pair[1].sequence);
                    ASSERT(pair[0].originator == dut && pair[1].originator == dut && pair[0].input != pair[1].input);
                    if (t == 1) ASSERT(pair[0].time >= SimTime(1) && pair[1].time >= SimTime(1));
                }
                ASSERT(route(member, 128)->getInterface() == dutInterface(1));
            }
        }
        else {
            int prefix = mode == "Prefix127" ? 127 : mode == "Host128" ? 128 : 64;
            ASSERT(route(member, prefix));
            if (mode == "IndependentCounters") { ASSERT(state(member, 128)->getDestSeqNum() == 5 && state(member, 64)->getDestSeqNum() == 100 && delivered.size() == 2 && deliveredInputs == std::set<int>({0, 1})); }
            else if (mode == "InactiveHostBlock") { ASSERT(delivered.size() == 2 && state(member, 128)->getDestSeqNum() == 6 && !requests.empty()); }
            else if (mode == "CachedReply" || mode == "RelayPrefix") {
                ASSERT(!replies.empty() && replies.back().input == 1 && replies.back().header->getPrefixSize() == 64);
                ASSERT(replies.back().header->getDestAddr() == member && replies.back().header->getDestSeqNum() == 100);
                if (mode == "RelayPrefix") ASSERT(replies.back().header->getHopCount() == 2);
            }
            else if (mode == "ErrorAmbiguity") { ASSERT(state(member, 64)->getDestSeqNum() == 100 && state(member, 65)->getDestSeqNum() == 200 && state(member, 64)->isActive() && state(member, 65)->isActive()); }
            else if (mode == "Repair") { ASSERT(!requests.empty() && delivered.size() >= 1 && state(member, 64)->isActive() && route(member, 64)->getInterface() == dutInterface(1)); }
            else if (mode == "IcmpMember") { ASSERT(delivered.size() == 2 && state(member, 64)->isActive() && state(member, 64)->getDestSeqNum() == 100); ASSERT(source->errors.size() == 1 && source->errors[0] == member); }
            else {
                ASSERT(delivered.size() == (mode == "Host128" ? 1u : 2u) && requests.empty() && source->errors.empty());
                if (mode == "TwoLinks") ASSERT(deliveredInputs == std::set<int>({0, 1}) && state(member, 64)->getDestSeqNum() == 101 && route(member, 64)->getInterface() == dutInterface(1));
            }
        }
        ASSERT(AodvDataCounters::created == AodvDataCounters::destroyed);
        std::cout << "AODV Remaining " << family << " " << mode << ": PASS\n";
    }
    void run() {
        if (family == "Discovery") {
            if (mode == "RepairFailure") {
                if (step == 0) offer(member, 128, 20, 0, 2, true);
                if (step == 1) {
                    ASSERT(acknowledgments[0] == 1);
                    ask(Ipv6Address("2001:db8:9::1"), 1, true);
                }
                if (step == 2) {
                    check_and_cast<physicallayer::IRadio *>(getModuleByPath("^.wlan[0].radio"))->setRadioMode(physicallayer::IRadio::RADIO_MODE_OFF);
                    transitData();
                }
                if (step == 3) transitData();
            }
            else if (mode == "RecoveryThenExhaustion") {
                if (step == 0) operate(new ModuleCrashOperation());
                if (step == 1) { ASSERT(!pending); operate(new ModuleStartOperation()); source->resetSocket(); source->emitData(member); source->emitData(other); }
            }
            else {
                if (step == 0) { source->emitData(member); if (mode != "Recovered") source->emitData(other); }
                if (mode == "StopCrash" && step == 1) operate(new ModuleCrashOperation());
            }
        }
        else if (family == "Interfaces") {
            if (mode == "IngressReply" && step == 0) ask(dutAddress(1), 1);
            else if (mode == "AckIsolation" && step == 0) { offer(member, 128, 20, 0, 2, true); offer(other, 128, 30, 1, 2, true); }
            else if (mode == "ErrorPrecursors") { if (step == 0) offer(member, 128, 20, 0); if (step == 1) ask(member, 1); if (step == 2) sendError(member, 21, 0); }
            else if (mode == "HelloIsolation") {
                if (step == 0) { hello(0); hello(1); offer(member, 128, 20, 0); offer(other, 128, 30, 1); }
                else {
                    // Router discovery is native traffic and also refreshes a
                    // live neighbor. Silence this physical link completely.
                    if (step == 1) {
                        for (int input = 0; input < 2; ++input) {
                            auto neighbor = state(peerInterface(input)->getNetworkAddress(), 128);
                            ASSERT(neighbor->isActive() && neighbor->hasValidDestNum() && neighbor->getDestSeqNum() == 7);
                        }
                        peerInterface(0)->setState(NetworkInterface::DOWN);
                    }
                    hello(1);
                }
            }
            else if (mode == "RestartMembership") {
                if (step == 0 || step == 4) { ask(dutAddress(0), 0, false, true); ask(dutAddress(1), 1, false, true); }
                if (step == 1) operate(new ModuleCrashOperation());
                if (step == 2) { ASSERT(!pending); operate(new ModuleStartOperation()); }
            }
            else if (mode == "StartAfterDownInit") { if (step == 0) operate(new ModuleStartOperation()); if (step == 2) { ask(dutAddress(0), 0, false, true); ask(dutAddress(1), 1, false, true); } }
            else if (step == 0 && (mode == "DiscoveryFanout" || mode == "RateBudget")) { source->emitData(member); if (mode == "RateBudget") source->emitData(other); }
        }
        else {
            int prefix = mode == "Prefix127" ? 127 : mode == "Host128" ? 128 : 64;
            if (step == 0) {
                if (mode == "RelayPrefix") ask(Ipv6Address("2001:db8:9::1"), 1, true);
                else {
                    offer(member, prefix, 100, mode == "Repair" || mode == "ErrorAmbiguity" || mode == "CachedReply" || mode == "TwoLinks" ? 0 : 1);
                    if (mode == "IndependentCounters") offer(member, 128, 5, 0);
                    if (mode == "InactiveHostBlock") offer(member, 128, 5, 0, 0.02);
                    if (mode == "ErrorAmbiguity") offer(member, 65, 200, 0);
                    if (mode == "Repair") ask(Ipv6Address("2001:db8:9::1"), 1, true);
                }
            }
            if (step == 1) {
                if (mode == "CachedReply") ask(member, 1);
                else if (mode == "RelayPrefix") offer(member, 64, 100, 0, 2, false, peerInterface(1)->getNetworkAddress());
                else if (mode == "ErrorAmbiguity") sendError(member.getPrefix(64), 250, 0);
                else if (mode == "Repair") { check_and_cast<physicallayer::IRadio *>(getModuleByPath("^.wlan[0].radio"))->setRadioMode(physicallayer::IRadio::RADIO_MODE_OFF); transitData(); }
                else source->emitData(member);
            }
            if (step == 2) {
                if (mode == "IcmpMember") memberError();
                else if (mode == "Repair") transitData();
                else if (mode != "Host128" && mode != "CachedReply" && mode != "RelayPrefix" && mode != "ErrorAmbiguity" && mode != "TwoLinks") source->emitData(other);
                if (mode == "InactiveHostBlock") {
                    ASSERT(delivered.empty());
                    ASSERT(std::any_of(requests.begin(), requests.end(), [&](const Request& r) { return r.target == member; }));
                    offer(member, 128, 6, 1);
                }
                if (mode == "TwoLinks") offer(member, 64, 101, 1);
            }
            if (step == 3) {
                if (mode == "IcmpMember") source->emitData(other);
                if (mode == "Repair") offer(member, 64, 101, 1);
                if (mode == "TwoLinks") source->emitData(other);
            }
        }
        static const double times[] = {0.05, 0.15, 0.25, 0.35, 0.45};
        ++step;
        if (step < 5) scheduleAt(times[step], timer);
        else scheduleAt(mode == "HelloIsolation" ? 0.6 : 1.3, timer);
    }
  protected:
    void initialize(int stage) override {
        if (stage != INITSTAGE_APPLICATION_LAYER) return;
        family = par("family").stdstringValue(); mode = par("scenario").stdstringValue(); using6 = par("usingIpv6"); twoLinks = par("twoLinks");
        peerInterfaces = check_and_cast<IInterfaceTable *>(getModuleByPath("^.interfaceTable"));
        dutInterfaces = check_and_cast<IInterfaceTable *>(getModuleByPath("^.^.dut.interfaceTable"));
        dut = using6 ? L3Address(Ipv6Address("2001:db8:1::1")) : L3Address(Ipv4Address("10.0.0.1"));
        member = using6 ? L3Address(Ipv6Address("2001:db8:8::99")) : L3Address(Ipv4Address(mode == "SharedOwner" ? "192.0.2.1" : "10.0.0.99"));
        other = using6 ? L3Address(Ipv6Address("2001:db8:8::100")) : L3Address(Ipv4Address(mode == "SharedOwner" ? "198.51.100.1" : "10.0.0.98"));
        if (mode == "Prefix127") { member = Ipv6Address("2001:db8:8::100"); other = Ipv6Address("2001:db8:8::101"); }
        routes = check_and_cast<IRoutingTable *>(getModuleByPath(using6 ? "^.^.dut.ipv6.routingTable" : "^.^.dut.ipv4.routingTable"));
        source = check_and_cast<AodvFeatureSource *>(getModuleByPath("^.^.dut.app[0]"));
        source->memberErrorExpected = family == "Subnet" && mode == "IcmpMember";
        socket.setOutputGate(gate("socketOut")); socket.setCallback(this); socket.bind(654); socket.setBroadcast(true);
        if (using6) for (int i = 0; i < 2; ++i) socket.joinMulticastGroup(Ipv6Address("ff01::1"), peerInterface(i)->getInterfaceId());
        getModuleByPath(using6 ? "^.ipv6.ipv6" : "^.ipv4.ip")->subscribe(packetReceivedFromLowerSignal, this);
        timer = new cMessage("remaining-feature-script"); scheduleAt(0.05, timer);
    }
    void handleMessage(cMessage *message) override {
        if (message != timer) { socket.processMessage(message); return; }
        if (step == 5) finalCheck(); else run();
    }
    void socketDataArrived(UdpSocket *, Packet *packet) override {
        auto h = packet->peekAtFront<AodvControlPacket>(); int input = inputIndex(packet);
        if (auto q = dynamicPtrCast<const Rreq>(h)) {
            auto from = packet->getTag<L3AddressInd>()->getSrcAddress();
            bool duplicateObservation = family == "Discovery" && std::any_of(requests.begin(), requests.end(), [&](const Request& r) { return r.target == q->getDestAddr() && r.source == from && r.id == q->getRreqId(); });
            if (!duplicateObservation) requests.push_back({q->getDestAddr(), q->getOriginatorAddr(), from, q->getRreqId(), q->getOriginatorSeqNum(), input, simTime()});
            if ((family == "Interfaces" && (mode == "DiscoveryFanout" || mode == "RateBudget")) || (family == "Discovery" && mode == "Recovered")) {
                if (input == (twoLinks ? 1 : 0)) offer(q->getDestAddr(), using6 ? 128 : 32, 20, twoLinks ? 1 : 0);
            }
        }
        else if (auto r = dynamicPtrCast<const Rrep>(h)) {
            if (!r->getOriginatorAddr().isUnspecified()) {
                replies.push_back({r, packet->getTag<L3AddressInd>()->getSrcAddress(), input});
                if (r->getAckRequiredFlag()) { auto ack = makeShared<RrepAck>(); ack->setPacketType(using6 ? RREPACK_IPv6 : RREPACK); ack->setChunkLength(B(2)); transmit(ack, input); }
            }
        }
        else if (auto e = dynamicPtrCast<const Rerr>(h)) errors.emplace_back(e, input);
        else if (dynamicPtrCast<const RrepAck>(h)) acknowledgments[input]++;
        delete packet;
    }
    void socketErrorArrived(UdpSocket *, Indication *indication) override { delete indication; }
    void socketClosed(UdpSocket *) override {}
    void receiveSignal(cComponent *, simsignal_t, cObject *object, cObject *) override {
        Enter_Method("observeRemainingFeature");
        auto p = dynamic_cast<Packet *>(object); if (!p) return;
        Ptr<const NetworkHeaderBase> ip = using6 ? staticPtrCast<const NetworkHeaderBase>(p->peekAtFront<Ipv6Header>()) : staticPtrCast<const NetworkHeaderBase>(p->peekAtFront<Ipv4Header>());
        if (ip->getDestinationAddress() != member && ip->getDestinationAddress() != other) {
            if (using6 && dynamicPtrCast<const Ipv6Header>(ip)->getProtocolId() == IP_PROT_IPv6_ICMP &&
                    p->peekDataAt<Icmpv6Header>(ip->getChunkLength())->getType() < 128) {
                unexpectedIcmp++;
                std::cout << "Observed peer ICMPv6 at " << simTime() << " from " << ip->getSourceAddress() << " to " << ip->getDestinationAddress() << " type=" << p->peekDataAt<Icmpv6Header>(ip->getChunkLength())->getType() << "\n";
            }
            return;
        }
        auto udp = p->peekDataAt<UdpHeader>(ip->getChunkLength()); if (udp->getDestPort() != 9999) return;
        if (family == "Subnet") {
            auto mac = p->getTag<MacAddressInd>();
            ASSERT(mac->getDestAddress() == peerInterface(inputIndex(p))->getMacAddress());
        }
        if (delivered.insert(p->getTreeId()).second) { deliveredTargets.push_back(ip->getDestinationAddress()); deliveredInputs.insert(inputIndex(p)); }
    }
  public:
    ~AodvRemainingFeaturePeer() { cancelAndDelete(timer); }
};
Define_Module(AodvRemainingFeaturePeer);
#endif

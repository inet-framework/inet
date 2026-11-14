// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __AODV_SUBNET_AUTHORITY_TEST_PEER_H
#define __AODV_SUBNET_AUTHORITY_TEST_PEER_H

#include "AodvPeer.h"
#include "inet/common/Simsignals.h"
#include "inet/common/lifecycle/LifecycleController.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/ipv6/Ipv6RoutingTable.h"
#include "inet/networklayer/ipv6/Ipv6Route.h"
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#include "inet/networklayer/icmpv6/Icmpv6Header_m.h"
#include "inet/networklayer/common/Icmpv4ErrorTag_m.h"
#include "inet/networklayer/common/Icmpv6ErrorTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"

// A wire requester and observer. Only native AODV instances elect or advertise.
class AodvSubnetAuthorityPeer : public SimpleModule, public UdpSocket::ICallback,
        public cListener, public LifecycleController
{
    UdpSocket controlSocket;
    UdpSocket dataSocket;
    cMessage *timer = nullptr;
    std::string mode;
    int step = 0;
    uint32_t requestId = 0;
    bool ipv6 = false;
    bool election = false;
    bool external = false;
    int deliveries = 0;
    int indications = 0;
    int echoes = 0;
    bool primaryDown = false;
    int controlAfterPrimaryDown = 0;
    int unavailableExportArrivals = 0;
    struct Answer {
        L3Address target;
        L3Address source;
        unsigned int prefix;
        uint32_t sequence;
    };
    std::vector<Answer> answers;
    std::vector<Answer> authoritativeSends;
    std::vector<UnreachableNode> errors;
    IRoute *installedExternalRoute = nullptr;
    struct DelayedRequest : cMessage {
        Ptr<Rreq> request;
        L3Address destination;
        DelayedRequest(const Ptr<Rreq>& request, const L3Address& destination) :
            cMessage("second-candidate-probe"), request(request), destination(destination) {}
    };
    std::set<DelayedRequest *> delayedRequests;
    struct DelayedData : cMessage {
        L3Address destination;
        bool viaHigh;
        DelayedData(const L3Address& destination, bool viaHigh) :
            cMessage("spaced-member-data"), destination(destination), viaHigh(viaHigh) {}
    };
    std::set<DelayedData *> delayedData;

    L3Address address(const char *v4, const char *v6) const {
        return ipv6 ? L3Address(Ipv6Address(v6)) : L3Address(Ipv4Address(v4));
    }
    L3Address low() const { return address("10.0.0.1", "2001:db8:1::1"); }
    L3Address high() const { return address("10.0.0.4", "2001:db8:1::4"); }
    L3Address requester() const { return address("10.0.0.2", "2001:db8:1::2"); }
    L3Address member() const {
        return external ? address("198.51.100.99", "2001:db8:9::99") : address("192.0.2.99", "2001:db8:8::99");
    }
    L3Address other() const {
        if (mode == "TwoPrefixes" || mode == "OverlappingPrefixes")
            return external ? address("198.51.100.129", "2001:db8:9::81") : address("192.0.2.129", "2001:db8:8::81");
        return external ? address("198.51.100.100", "2001:db8:9::100") : address("192.0.2.100", "2001:db8:8::100");
    }
    L3Address missing() const { return address("192.0.2.101", "2001:db8:8::101"); }
    int prefixLength() const { return ipv6 ? 64 : 24; }
    bool missingMemberCase() const {
        return mode == "MemberMissing" || mode == "MemberReturns" || mode == "Ipv6MissingMember";
    }
    NetworkInterface *iface(const char *node, const char *name) const {
        auto path = std::string("^.^.") + node + ".interfaceTable";
        return check_and_cast<IInterfaceTable *>(getModuleByPath(path.c_str()))->findInterfaceByName(name);
    }
    IRoutingTable *table(const char *node) const {
        auto path = std::string("^.^.") + node + (ipv6 ? ".ipv6.routingTable" : ".ipv4.routingTable");
        return check_and_cast<IRoutingTable *>(getModuleByPath(path.c_str()));
    }
    void operate(const char *node, LifecycleOperation *operation) {
        auto path = std::string("^.^.") + node + ".aodv";
        LifecycleOperation::StringMap parameters;
        operation->initialize(getModuleByPath(path.c_str()), parameters);
        ASSERT(parameters.empty());
        initiateOperation(operation); // UDP closure may complete asynchronously.
    }
    void transmitRequest(const Ptr<Rreq>& request, const L3Address& destination) {
        auto packet = new Packet("authority-wire-request", request);
        auto output = iface("peer", "wlan0");
        packet->addTag<InterfaceReq>()->setInterfaceId(output->getInterfaceId());
        packet->addTag<L3AddressReq>()->setSrcAddress(requester());
        packet->addTag<HopLimitReq>()->setHopLimit(4);
        packet->addTag<NextHopAddressReq>()->setNextHopAddress(destination);
        controlSocket.sendTo(packet, destination, 654);
    }
    void ask(const L3Address& target, bool knownHost = false, bool floor = false, uint32_t sequence = 0) {
        auto request = makeShared<Rreq>();
        request->setPacketType(ipv6 ? RREQ_IPv6 : RREQ);
        request->setOriginatorAddr(requester());
        request->setOriginatorSeqNum(++requestId);
        request->setRreqId(requestId);
        request->setDestAddr(target);
        request->setUnknownSeqNumFlag(!knownHost);
        request->setDestOnlyFlag(true);
        request->setDestSeqNum(knownHost ? sequence : 0);
        request->setHopCount(0);
        if (floor) {
            request->setExtensionsArraySize(7);
            request->setExtensions(0, 253);
            request->setExtensions(1, 5);
            request->setExtensions(2, prefixLength());
            for (unsigned int i = 0; i < 4; ++i)
                request->setExtensions(i + 3, sequence >> (24 - 8 * i));
        }
        request->setChunkLength((ipv6 ? B(48) : B(24)) + B(floor ? 7 : 0));
        transmitRequest(request, low());
        if (election) {
            auto second = makeShared<Rreq>(*request);
            second->setRreqId(++requestId);
            second->setOriginatorSeqNum(requestId);
            auto delayed = new DelayedRequest(second, high());
            delayedRequests.insert(delayed);
            scheduleAt(simTime() + 0.02, delayed);
        }
    }
    void data(const L3Address& destination, bool viaHigh = false, bool delayed = false) {
        if (delayed) {
            auto message = new DelayedData(destination, viaHigh);
            delayedData.insert(message);
            scheduleAt(simTime() + 0.02, message);
            return;
        }
        auto packet = new Packet("authority-member-data", makeShared<ByteCountChunk>(B(16)));
        packet->addTag<InterfaceReq>()->setInterfaceId(iface("peer", "wlan0")->getInterfaceId());
        packet->addTag<NextHopAddressReq>()->setNextHopAddress(mode == "PrimaryDownUnavailableExport" ?
                L3Address(Ipv4Address("10.0.1.1")) : viaHigh ? high() : low());
        dataSocket.sendTo(packet, destination, 9999);
    }
    Answer last(const L3Address& target) const {
        for (auto i = answers.rbegin(); i != answers.rend(); ++i)
            if (i->target == target)
                return *i;
        throw cRuntimeError("No authoritative answer for %s in %s", target.str().c_str(), mode.c_str());
    }
    int answerCount(const L3Address& target) const {
        return std::count_if(answers.begin(), answers.end(), [&](const Answer& a) { return a.target == target; });
    }
    void checkReply(const L3Address& target, bool viaHigh = false) const {
        auto result = last(target);
        ASSERT(result.prefix == unsigned(prefixLength()));
        if (!election)
            ASSERT(result.source == (viaHigh ? high() : low()));
        else {
            ASSERT(std::any_of(authoritativeSends.begin(), authoritativeSends.end(), [&](const Answer& a) {
                return a.target == target && a.source == (viaHigh ? high() : low());
            }));
        }
    }
    void addRoute(const char *node, const L3Address& destination, int prefix,
            const L3Address& gateway, const char *output) {
        auto routes = table(node);
        auto route = routes->createRoute();
        route->setSourceType(IRoute::MANUAL);
        route->setDestination(destination);
        route->setPrefixLength(prefix);
        route->setNextHop(gateway);
        route->setInterface(iface(node, output));
        route->setMetric(1);
        routes->addRoute(route);
        if (!strcmp(node, "dut"))
            installedExternalRoute = route;
    }
    void installExternalRoutes() {
        if (mode == "InPlaceUpdate")
            check_and_cast<Ipv6RoutingTable *>(table("dut"))->addOrUpdateOwnAdvPrefix(member().toIpv6().getPrefix(64), 64,
                    iface("dut", "eth0")->getInterfaceId(), 2);
        else if (mode != "LateRoute" && mode != "NoRecursiveExport")
            addRoute("dut", member().getPrefix(prefixLength()), prefixLength(),
                    address("192.0.2.254", "2001:db8:8::fe"), "eth0");
        if (mode == "ExternalIpv6Expiry")
            check_and_cast<Ipv6Route *>(installedExternalRoute)->setExpiryTime(0.4);
        if (mode == "TwoGateways")
            addRoute("dut2", member().getPrefix(prefixLength()), prefixLength(),
                    address("192.0.2.254", "2001:db8:8::fe"), "eth0");
        if (mode == "NoRecursiveExport") {
            auto route = table("dut")->createRoute();
            route->setSourceType(IRoute::AODV);
            route->setSource(getModuleByPath("^.^.dut.aodv"));
            route->setDestination(member().getPrefix(prefixLength()));
            route->setPrefixLength(prefixLength());
            route->setInterface(iface("dut", "eth0"));
            route->setNextHop(address("192.0.2.254", "2001:db8:8::fe"));
            route->setMetric(1);
            auto state = new AodvRouteData();
            state->setLifeTime(2);
            route->setProtocolData(state);
            table("dut")->addRoute(route);
            installedExternalRoute = route;
        }
        addRoute("gateway", requester().getPrefix(prefixLength()), prefixLength(),
                address("192.0.2.1", "2001:db8:8::1"), "eth0");
        addRoute("member", L3Address(ipv6 ? L3Address(Ipv6Address::UNSPECIFIED_ADDRESS) : L3Address(Ipv4Address::UNSPECIFIED_ADDRESS)),
                0, address("198.51.100.1", "2001:db8:9::1"), "eth0");
        addRoute("member2", L3Address(ipv6 ? L3Address(Ipv6Address::UNSPECIFIED_ADDRESS) : L3Address(Ipv4Address::UNSPECIFIED_ADDRESS)),
                0, address("198.51.100.1", "2001:db8:9::1"), "eth0");
    }
    void complete() {
        std::cout << "AODV Subnet Feature " << mode << ": PASS\n";
        cancelEvent(timer);
    }

  protected:
    int numInitStages() const override { return NUM_INIT_STAGES; }
    void initialize(int stage) override {
        if (stage != INITSTAGE_APPLICATION_LAYER)
            return;
        mode = par("scenario").stdstringValue();
        ipv6 = par("ipv6").boolValue();
        election = par("election").boolValue();
        external = par("external").boolValue();
        controlSocket.setOutputGate(gate("socketOut"));
        controlSocket.setCallback(this);
        controlSocket.bind(654);
        controlSocket.setBroadcast(true);
        dataSocket.setOutputGate(gate("socketOut"));
        dataSocket.setCallback(this);
        dataSocket.bind(4000);
        if (ipv6)
            controlSocket.joinMulticastGroup(Ipv6Address("ff02::1"), iface("peer", "wlan0")->getInterfaceId());
        auto deliverySignal = mode == "ReturnPath" ? packetSentSignal : packetReceivedSignal;
        getModuleByPath("^.^.member.app[0]")->subscribe(deliverySignal, this);
        getModuleByPath("^.^.member2.app[0]")->subscribe(deliverySignal, this);
        getModuleByPath("^.^.dut.udp")->subscribe(packetSentToLowerSignal, this);
        getModuleByPath("^.^.dut2.udp")->subscribe(packetSentToLowerSignal, this);
        if (mode == "PrimaryDownUnavailableExport")
            getModuleByPath("^.^.dut.ipv4.ip")->subscribe(packetReceivedFromLowerSignal, this);
        timer = new cMessage("authority-script");
        scheduleAt(0.01, timer);
    }
    void handleMessage(cMessage *message) override {
        if (auto delayed = dynamic_cast<DelayedData *>(message)) {
            delayedData.erase(delayed);
            data(delayed->destination, delayed->viaHigh);
            delete delayed;
            return;
        }
        if (auto delayed = dynamic_cast<DelayedRequest *>(message)) {
            delayedRequests.erase(delayed);
            transmitRequest(delayed->request, delayed->destination);
            delete delayed;
            return;
        }
        if (message != timer) {
            if (controlSocket.belongsToSocket(message))
                controlSocket.processMessage(message);
            else
                dataSocket.processMessage(message);
            return;
        }
        if (step++ == 0) {
            if (mode == "Ipv6MissingMember")
                iface("dut", "eth0")->getProtocolDataForUpdate<Ipv6InterfaceData>()->_setRetransTimer(0.03);
            if (ipv6) {
                auto unspecified = L3Address(Ipv6Address::UNSPECIFIED_ADDRESS);
                auto attached = L3Address(Ipv6Address("2001:db8:8::"));
                addRoute("dut", attached, 64, unspecified, "eth0");
                addRoute("dut2", attached, 64, unspecified, "eth0");
                if (external)
                    addRoute("gateway", member().getPrefix(64), 64, unspecified, "eth1");
            }
            if (external)
                installExternalRoutes();
            if (mode == "LateLowerCandidate")
                iface("dut", "eth0")->setState(NetworkInterface::DOWN);
            scheduleAt(0.15, timer);
            return;
        }
        if (election) {
            electionStep();
            return;
        }
        if (external) {
            externalStep();
            return;
        }
        authorityStep();
    }
    void authorityStep() {
        if (step == 2) {
            if (mode == "CounterDomains")
                ask(low(), true, false, 100);
            else
                ask(missingMemberCase() ? missing() : member(),
                        mode == "KnownHostFloor", mode == "MarkedFloor", mode == "KnownHostFloor" ? 100 : 17);
            scheduleAt(0.25, timer);
        }
        else if (step == 3) {
            if (mode == "KnownHostFloor") {
                ASSERT(answerCount(member()) == 0);
                complete();
            }
            else if (mode == "CounterDomains") {
                ASSERT(last(low()).sequence == 100);
                ask(member());
                scheduleAt(0.35, timer);
            }
            else if (mode == "WithdrawLink") {
                checkReply(member());
                iface("dut", "eth0")->setState(NetworkInterface::DOWN);
                scheduleAt(0.4, timer);
            }
            else if (mode == "Restart" || mode == "CrashRestart") {
                checkReply(member());
                operate("dut", mode == "CrashRestart" ? static_cast<LifecycleOperation *>(new ModuleCrashOperation()) : new ModuleStopOperation());
                scheduleAt(0.35, timer);
            }
            else if (missingMemberCase()) {
                checkReply(missing());
                data(missing());
                scheduleAt(0.55, timer);
            }
            else {
                checkReply(member());
                if (mode == "MarkedFloor")
                    ASSERT(last(member()).sequence == 17);
                data(member());
                data(other(), false, true);
                scheduleAt(0.5, timer);
            }
        }
        else if (step == 4) {
            if (mode == "CounterDomains") {
                checkReply(member());
                ASSERT(last(member()).sequence != 100);
                complete();
            }
            else if (mode == "WithdrawLink") {
                auto prefix = member().getPrefix(prefixLength());
                ASSERT(std::any_of(errors.begin(), errors.end(), [&](const UnreachableNode& e) {
                    return e.addr == prefix && e.seqNum > last(member()).sequence;
                }));
                answers.clear();
                ask(member());
                scheduleAt(0.5, timer);
            }
            else if (mode == "Restart" || mode == "CrashRestart") {
                operate("dut", new ModuleStartOperation());
                ask(member(), false, true, mode == "CrashRestart" ? 0xfffffffeu : 29);
                answers.clear();
                scheduleAt(0.45, timer);
            }
            else if (missingMemberCase()) {
                ASSERT(indications == 1 && deliveries == 0);
                if (mode != "MemberReturns") {
                    ask(member());
                    data(member(), false, true);
                    scheduleAt(0.8, timer);
                }
                else {
                    ASSERT(!ipv6);
                    iface("member2", "eth0")->getProtocolDataForUpdate<Ipv4InterfaceData>()->setIPAddress(missing().toIpv4());
                    data(missing());
                    scheduleAt(0.85, timer);
                }
            }
            else {
                ASSERT(deliveries == 2 && indications == 0);
                complete();
            }
        }
        else if (step == 5) {
            if (mode == "Restart" || mode == "CrashRestart") {
                ASSERT(answerCount(member()) == 0);
                scheduleAt(0.8, timer);
            }
            else if (mode == "WithdrawLink") {
                ASSERT(answerCount(member()) == 0);
                iface("dut", "eth0")->setState(NetworkInterface::UP);
                iface("dut", "eth0")->setCarrier(true);
                scheduleAt(0.65, timer);
            }
            else {
                ASSERT(deliveries == 1 && indications == 1);
                if (mode != "MemberReturns") {
                    checkReply(member());
                    ASSERT(last(member()).sequence == last(missing()).sequence);
                }
                complete();
            }
        }
        else if (step == 6) {
            ask(member(), false, true, mode == "CrashRestart" ? 0xfffffffeu : 29);
            scheduleAt(1.0, timer);
        }
        else {
            checkReply(member());
            if (mode == "CrashRestart")
                ASSERT(last(member()).sequence == 0xfffffffeu);
            else
                ASSERT(last(member()).sequence >= 29);
            complete();
        }
    }
    void electionStep() {
        if (step == 2) {
            ask(member(), false, mode == "CounterWrap", mode == "CounterWrap" ? 0xfffffffeu : 0);
            scheduleAt(0.25, timer);
        }
        else if (step == 3) {
            checkReply(member(), mode == "LateLowerCandidate");
            if (mode == "LowestAddress") {
                ASSERT(std::none_of(authoritativeSends.begin(), authoritativeSends.end(), [&](const Answer& a) {
                    return a.target == member() && a.source == high();
                }));
                data(member());
                scheduleAt(0.5, timer);
            }
            else if (mode == "TwoPrefixes") {
                ask(other());
                scheduleAt(0.4, timer);
            }
            else if (mode == "CounterWrap") {
                ASSERT(last(member()).sequence == 0xfffffffeu);
                ask(member(), false, true, 0);
                scheduleAt(0.4, timer);
            }
            else if (mode == "LateLowerCandidate") {
                iface("dut", "eth0")->setState(NetworkInterface::UP);
                iface("dut", "eth0")->setCarrier(true);
                scheduleAt(0.42, timer);
            }
            else if (mode == "LostCoordination" || mode == "PartitionHeal") {
                iface("dut", "eth0")->setState(NetworkInterface::DOWN);
                scheduleAt(0.42, timer);
            }
            else {
                operate("dut", mode == "LeaderCrash" ? static_cast<LifecycleOperation *>(new ModuleCrashOperation()) : new ModuleStopOperation());
                scheduleAt(0.42, timer);
            }
        }
        else if (step == 4) {
            if (mode == "LowestAddress") {
                ASSERT(deliveries == 1);
                complete();
            }
            else if (mode == "TwoPrefixes") {
                ASSERT(std::any_of(answers.begin(), answers.end(), [&](const Answer& a) {
                    return a.target == other() && a.prefix == 25 && a.source == high();
                }));
                data(member());
                data(other(), true, true);
                scheduleAt(0.6, timer);
            }
            else if (mode == "CounterWrap") {
                ASSERT(last(member()).sequence == 0);
                ask(member(), false, true, 5);
                scheduleAt(0.55, timer);
            }
            else {
                answers.clear();
                authoritativeSends.clear();
                ask(member());
                scheduleAt(0.55, timer);
            }
        }
        else if (step == 5) {
            if (mode == "TwoPrefixes") {
                ASSERT(deliveries == 2);
                complete();
            }
            else if (mode == "CounterWrap") {
                ASSERT(last(member()).sequence == 5);
                operate("dut", new ModuleCrashOperation());
                scheduleAt(0.72, timer);
            }
            else {
                checkReply(member(), mode != "LateLowerCandidate");
                if (mode == "Rejoin") {
                    operate("dut", new ModuleStartOperation());
                    scheduleAt(1.05, timer);
                }
                else if (mode == "PartitionHeal") {
                    iface("dut", "eth0")->setState(NetworkInterface::UP);
                    iface("dut", "eth0")->setCarrier(true);
                    scheduleAt(0.8, timer);
                }
                else {
                    data(member(), mode != "LateLowerCandidate");
                    scheduleAt(0.8, timer);
                }
            }
        }
        else if (step == 6) {
            if (mode == "CounterWrap") {
                answers.clear();
                authoritativeSends.clear();
                ask(member());
                scheduleAt(0.9, timer);
            }
            else if (mode == "Rejoin" || mode == "PartitionHeal") {
                answers.clear();
                authoritativeSends.clear();
                ask(member());
                scheduleAt(1.1, timer);
            }
            else {
                ASSERT(deliveries == 1);
                complete();
            }
        }
        else {
            checkReply(member(), mode == "CounterWrap");
            if (mode == "CounterWrap")
                ASSERT(last(member()).sequence == 5);
            complete();
        }
    }
    void externalStep() {
        if (step == 2) {
            ask(member());
            scheduleAt(0.25, timer);
        }
        else if (step == 3) {
            if (mode == "LateRoute" || mode == "NoRecursiveExport") {
                ASSERT(answerCount(member()) == 0);
                if (mode == "NoRecursiveExport") {
                    table("dut")->deleteRoute(installedExternalRoute);
                    installedExternalRoute = nullptr;
                    addRoute("dut", member().getPrefix(prefixLength()), prefixLength(), requester(), "wlan0");
                    ask(member());
                    scheduleAt(0.4, timer);
                    return;
                }
                addRoute("dut", member().getPrefix(prefixLength()), prefixLength(),
                        address("192.0.2.254", "2001:db8:8::fe"), "eth0");
                scheduleAt(0.4, timer);
            }
            else {
                checkReply(member());
                if (mode == "ExternalRouteWithdraw") {
                    table("dut")->deleteRoute(installedExternalRoute);
                    installedExternalRoute = nullptr;
                    scheduleAt(0.4, timer);
                }
                else if (mode == "PrimaryDownUnavailableExport") {
                    table("dut")->deleteRoute(installedExternalRoute);
                    installedExternalRoute = nullptr;
                    primaryDown = true;
                    iface("dut", "wlan0")->setState(NetworkInterface::DOWN);
                    scheduleAt(0.4, timer);
                }
                else if (mode == "ExternalIpv6Expiry")
                    scheduleAt(0.55, timer);
                else if (mode == "CoveringReplacement") {
                    auto previous = installedExternalRoute;
                    addRoute("dut", member().getPrefix(prefixLength()), prefixLength(),
                            address("192.0.2.254", "2001:db8:8::fe"), "eth0");
                    table("dut")->deleteRoute(previous);
                    scheduleAt(0.4, timer);
                }
                else if (mode == "InPlaceUpdate") {
                    check_and_cast<Ipv6RoutingTable *>(table("dut"))->addOrUpdateOwnAdvPrefix(member().toIpv6().getPrefix(64), 64,
                            iface("dut", "eth0")->getInterfaceId(), 3);
                    scheduleAt(0.4, timer);
                }
                else if (mode == "TwoGateways") {
                    operate("dut", new ModuleCrashOperation());
                    scheduleAt(0.43, timer);
                }
                else if (mode == "OverlappingPrefixes") {
                    ask(other());
                    scheduleAt(0.4, timer);
                }
                else {
                    data(member());
                    data(other(), false, true);
                    scheduleAt(0.5, timer);
                }
            }
        }
        else if (step == 4) {
            if (mode == "PrimaryDownUnavailableExport") {
                ASSERT(iface("dut", "wlan1")->isUp());
                data(member());
                scheduleAt(0.65, timer);
            }
            else if (mode == "NoRecursiveExport") {
                ASSERT(answerCount(member()) == 0);
                complete();
            }
            else if (mode == "TwoGateways") {
                answers.clear();
                ask(member());
                scheduleAt(0.6, timer);
            }
            else if (mode == "OverlappingPrefixes") {
                ASSERT(last(other()).prefix == 25);
                data(member());
                data(other(), false, true);
                scheduleAt(0.65, timer);
            }
            else if (mode == "ExternalRouteWithdraw" || mode == "ExternalIpv6Expiry") {
                ASSERT(std::any_of(errors.begin(), errors.end(), [&](const UnreachableNode& e) {
                    return e.addr == member().getPrefix(prefixLength());
                }));
                answers.clear();
                ask(member());
                if (mode == "ExternalIpv6Expiry")
                    data(member(), false, true);
                scheduleAt(mode == "ExternalIpv6Expiry" ? 0.75 : 0.5, timer);
            }
            else if (mode == "LateRoute" || mode == "CoveringReplacement" || mode == "InPlaceUpdate") {
                ASSERT(errors.empty());
                ask(member());
                scheduleAt(0.55, timer);
            }
            else {
                ASSERT(deliveries == 2 && indications == 0);
                if (mode == "ReturnPath")
                    ASSERT(echoes == 2);
                complete();
            }
        }
        else {
            if (mode == "PrimaryDownUnavailableExport") {
                ASSERT(unavailableExportArrivals == 1 && deliveries == 0);
                ASSERT(controlAfterPrimaryDown == 0);
                complete();
                return;
            }
            if (mode == "TwoGateways") {
                if (step == 5) {
                    checkReply(member(), true);
                    data(member(), true);
                    scheduleAt(0.8, timer);
                }
                else {
                    ASSERT(deliveries == 1 && indications == 0);
                    complete();
                }
                return;
            }
            if (mode == "ExternalRouteWithdraw" || mode == "ExternalIpv6Expiry") {
                ASSERT(answerCount(member()) == 0);
                ASSERT(deliveries == 0);
            }
            else if (mode == "OverlappingPrefixes")
                ASSERT(deliveries == 2);
            else
                checkReply(member());
            complete();
        }
    }
    void socketDataArrived(UdpSocket *socket, Packet *packet) override {
        if (socket == &controlSocket) {
            auto control = packet->peekAtFront<AodvControlPacket>();
            if (auto reply = dynamicPtrCast<const Rrep>(control)) {
                if (reply->getAckRequiredFlag()) {
                    auto ack = makeShared<RrepAck>();
                    ack->setPacketType(ipv6 ? RREPACK_IPv6 : RREPACK);
                    ack->setChunkLength(B(2));
                    auto response = new Packet("authority-validation-ack", ack);
                    auto previous = packet->getTag<L3AddressInd>()->getSrcAddress();
                    response->addTag<InterfaceReq>()->setInterfaceId(iface("peer", "wlan0")->getInterfaceId());
                    response->addTag<L3AddressReq>()->setSrcAddress(requester());
                    response->addTag<NextHopAddressReq>()->setNextHopAddress(previous);
                    response->addTag<HopLimitReq>()->setHopLimit(1);
                    controlSocket.sendTo(response, previous, 654);
                }
                if (!reply->getOriginatorAddr().isUnspecified())
                    answers.push_back({reply->getDestAddr(), packet->getTag<L3AddressInd>()->getSrcAddress(), reply->getPrefixSize(), reply->getDestSeqNum()});
            }
            if (auto error = dynamicPtrCast<const Rerr>(control))
                for (size_t i = 0; i < error->getUnreachableNodesArraySize(); ++i)
                    errors.push_back(error->getUnreachableNodes(i));
        }
        else
            echoes++;
        delete packet;
    }
    void socketErrorArrived(UdpSocket *socket, Indication *indication) override {
        // A direct probe of a stopped candidate may receive UDP Port Unreachable.
        // It is control evidence, not the member-data error under test.
        if (socket == &dataSocket) {
            indications++;
            if (missingMemberCase()) {
                const Packet *original;
                if (ipv6) {
                    auto error = indication->getTag<Icmpv6ErrorInd>();
                    ASSERT(error->getType() == ICMPv6_DESTINATION_UNREACHABLE && error->getCode() == ADDRESS_UNREACHABLE);
                    original = error->getOriginalPacket();
                }
                else {
                    auto error = indication->getTag<Icmpv4ErrorInd>();
                    ASSERT(error->getType() == ICMP_DESTINATION_UNREACHABLE && error->getCode() == ICMP_DU_HOST_UNREACHABLE);
                    original = error->getOriginalPacket();
                }
                ASSERT(original->getTag<L3AddressInd>()->getDestAddress() == missing());
                auto ports = original->getTag<L4PortInd>();
                ASSERT(ports->getSrcPort() == 4000 && ports->getDestPort() == 9999);
            }
        }
        delete indication;
    }
    void socketClosed(UdpSocket *) override {}
    void receiveSignal(cComponent *source, simsignal_t signal, cObject *value, cObject *) override {
        Enter_Method("observeMemberDelivery");
        auto packet = dynamic_cast<Packet *>(value);
        if (!packet)
            return;
        if (signal == packetReceivedFromLowerSignal) {
            if (!strcmp(packet->getName(), "authority-member-data")) {
                ASSERT(packet->getTag<InterfaceInd>()->getInterfaceId() == iface("dut", "wlan1")->getInterfaceId());
                unavailableExportArrivals++;
            }
            return;
        }
        if (signal == packetSentToLowerSignal) {
            auto udp = packet->peekAtFront<UdpHeader>();
            if (udp->getDestinationPort() != 654)
                return;
            if (primaryDown && source == getModuleByPath("^.^.dut.udp"))
                controlAfterPrimaryDown++;
            auto control = packet->peekDataAt<AodvControlPacket>(udp->getChunkLength());
            if (auto reply = dynamicPtrCast<const Rrep>(control))
                if (reply->getHopCount() == 0 && !reply->getOriginatorAddr().isUnspecified())
                    authoritativeSends.push_back({reply->getDestAddr(),
                        source == getModuleByPath("^.^.dut.udp") ? low() : high(),
                        reply->getPrefixSize(), reply->getDestSeqNum()});
            return;
        }
        if (!strcmp(packet->getName(), "authority-member-data"))
            deliveries++;
    }
    void finish() override {}
  public:
    using cListener::finish;
    ~AodvSubnetAuthorityPeer() override {
        cancelAndDelete(timer);
        for (auto delayed : delayedRequests)
            cancelAndDelete(delayed);
        for (auto delayed : delayedData)
            cancelAndDelete(delayed);
    }
};
Define_Module(AodvSubnetAuthorityPeer);

#endif

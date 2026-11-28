// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __AODV_SUBNET_RECOVERY_PATH_TEST_PEER_H
#define __AODV_SUBNET_RECOVERY_PATH_TEST_PEER_H

#include "AodvPeer.h"
#include "inet/networklayer/contract/INetfilter.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"

// Fault injection drops RERR output only. Every discovery, failure transition and
// sequence increment belongs to the real source/relay/authority AODV instances.
class AodvSubnetRecoveryPathPeer : public SimpleModule, public UdpSocket::ICallback,
        public NetfilterBase::HookBase, public cListener
{
    UdpSocket dataSocket;
    INetfilter *relayIp = nullptr;
    IRoutingTable *relayRoutes = nullptr;
    cMessage *timer = nullptr;
    std::string mode;
    int step = 0;
    int deliveries = 0;
    int secondMemberDeliveries = 0;
    int suppressedErrors = 0;
    int actualLinkFailures = 0;
    uint32_t preFailureFloor = 0;
    uint32_t advancedFloor = 0;
    uint32_t forwardedFloor = 0;
    bool sourceWithoutFloor = false;
    bool relayDiscoveryObserved = false;
    int applicationErrors = 0;
    int echoReplies = 0;
    L3Address member = Ipv4Address("192.0.2.99");
    IRoute *aggregate() const {
        for (int i = 0; i < relayRoutes->getNumRoutes(); ++i) {
            auto route = relayRoutes->getRoute(i);
            if (route->getSourceType() == IRoute::AODV && route->getPrefixLength() == 24 &&
                    route->getDestinationAsGeneric() == member.getPrefix(24))
                return route;
        }
        return nullptr;
    }
    void sendData(bool secondMember = false) {
        dataSocket.sendTo(new Packet("subnet-recovery-data", makeShared<ByteCountChunk>(B(16))),
                secondMember ? L3Address(Ipv4Address(mode == "ExternalReturn" ? "198.51.100.100" : "192.0.2.100")) : member, 9999);
    }
    void addNativeRoute(const char *node, const char *destination, int prefix, const char *nextHop, const char *output) {
        auto path = std::string("^.^.") + node;
        auto routes = check_and_cast<IRoutingTable *>(getModuleByPath((path + ".ipv4.routingTable").c_str()));
        auto interfaces = check_and_cast<IInterfaceTable *>(getModuleByPath((path + ".interfaceTable").c_str()));
        auto route = routes->createRoute();
        route->setSourceType(IRoute::MANUAL);
        route->setDestination(Ipv4Address(destination));
        route->setPrefixLength(prefix);
        route->setNextHop(Ipv4Address(nextHop));
        route->setInterface(interfaces->findInterfaceByName(output));
        route->setMetric(1);
        routes->addRoute(route);
    }
    void radio(bool up) {
        auto radio = check_and_cast<inet::physicallayer::IRadio *>(getModuleByPath("^.^.authority.wlan[0].radio"));
        radio->setRadioMode(up ? inet::physicallayer::IRadio::RADIO_MODE_RECEIVER : inet::physicallayer::IRadio::RADIO_MODE_OFF);
    }
    bool marker(const Ptr<const Rreq>& request, uint32_t& sequence) const {
        for (size_t offset = 0; offset + 2 <= request->getExtensionsArraySize();) {
            unsigned int type = request->getExtensions(offset++);
            unsigned int size = request->getExtensions(offset++);
            ASSERT(offset + size <= request->getExtensionsArraySize());
            if (type == 253) {
                ASSERT(size == 5 && request->getExtensions(offset) == 24);
                sequence = 0;
                for (unsigned int i = 1; i < 5; ++i)
                    sequence = (sequence << 8) | request->getExtensions(offset + i);
                return true;
            }
            offset += size;
        }
        return false;
    }
    void seedRelayHistory() {
        auto route = relayRoutes->createRoute();
        route->setSource(getModuleByPath("^.^.relay.aodv"));
        route->setSourceType(IRoute::AODV);
        route->setDestination(member.getPrefix(24));
        route->setPrefixLength(24);
        route->setInterface(check_and_cast<IInterfaceTable *>(getModuleByPath("^.^.relay.interfaceTable"))->findInterfaceByName("wlan0"));
        route->setNextHop(Ipv4Address("10.0.0.3"));
        route->setMetric(1);
        auto state = new AodvRouteData();
        state->setIsActive(false);
        state->setDestSeqNum(17);
        state->setLifeTime(2);
        route->setProtocolData(state);
        relayRoutes->addRoute(route);
    }
  protected:
    int numInitStages() const override { return NUM_INIT_STAGES; }
    void initialize(int stage) override {
        if (stage != INITSTAGE_APPLICATION_LAYER)
            return;
        mode = par("scenario").stdstringValue();
        if (mode == "ExternalReturn")
            member = Ipv4Address("198.51.100.99");
        dataSocket.setOutputGate(gate("socketOut"));
        dataSocket.setCallback(this);
        dataSocket.bind(4000);
        relayIp = check_and_cast<INetfilter *>(getModuleByPath("^.^.relay.ipv4.ip"));
        relayRoutes = check_and_cast<IRoutingTable *>(getModuleByPath("^.^.relay.ipv4.routingTable"));
        relayIp->registerHook(-100, this);
        auto deliverySignal = mode == "ExternalReturn" ? packetSentSignal : packetReceivedSignal;
        getModuleByPath("^.^.member.app[0]")->subscribe(deliverySignal, this);
        getModuleByPath("^.^.member2.app[0]")->subscribe(deliverySignal, this);
        getModuleByPath("^.^.relay")->subscribe(linkBrokenSignal, this);
        getModuleByPath("^.udp")->subscribe(packetSentToLowerSignal, this);
        getModuleByPath("^.^.relay.udp")->subscribe(packetSentToLowerSignal, this);
        timer = new cMessage("subnet-native-recovery");
        scheduleAt(0.1, timer);
    }
    void handleMessage(cMessage *message) override {
        if (message != timer) {
            dataSocket.processMessage(message);
            return;
        }
        if (mode == "UnknownOriginatorThroughRetainedRelay") {
            if (step++ == 0) {
                seedRelayHistory();
                sendData();
                scheduleAt(0.5, timer);
            }
            else {
                ASSERT(sourceWithoutFloor && forwardedFloor == 17);
                ASSERT(deliveries == 1 && applicationErrors == 0);
                std::cout << "AODV Subnet Native Recovery " << mode << ": PASS\n";
            }
            return;
        }
        if (mode == "ExternalReturn") {
            if (step == 0) {
                addNativeRoute("authority", "198.51.100.0", 24, "192.0.2.254", "eth0");
                addNativeRoute("gateway", "10.1.0.0", 24, "192.0.2.1", "eth0");
                addNativeRoute("member", "0.0.0.0", 0, "198.51.100.1", "eth0");
                addNativeRoute("member2", "0.0.0.0", 0, "198.51.100.1", "eth0");
                scheduleAt(0.2, timer);
            }
            else if (step == 1 || step == 2) {
                if (step == 2) {
                    auto routes = check_and_cast<IRoutingTable *>(getModuleByPath("^.ipv4.routingTable"));
                    bool nativeAggregate = false;
                    for (int i = 0; i < routes->getNumRoutes(); ++i) {
                        auto route = routes->getRoute(i);
                        if (route->getSourceType() == IRoute::AODV && route->getPrefixLength() == 24 &&
                                route->getDestinationAsGeneric() == member.getPrefix(24))
                            nativeAggregate = check_and_cast<AodvRouteData *>(route->getProtocolData())->isActive();
                    }
                    ASSERT(nativeAggregate && sourceWithoutFloor && relayDiscoveryObserved);
                }
                sendData(step == 2);
                scheduleAt(step == 1 ? 0.3 : 0.5, timer);
            }
            else {
                ASSERT(deliveries == 2 && secondMemberDeliveries == 1);
                ASSERT(echoReplies == 2 && applicationErrors == 0);
                std::cout << "AODV Subnet Native Recovery ExternalReturn: PASS\n";
            }
            step++;
            return;
        }
        static const double times[] = {0.1, 0.2, 0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7, 0.75, 0.82, 1.2};
        double time = times[step];
        if (time == 0.2 || time == 0.3 || (time >= 0.4 && time <= 0.6) || time == 0.75 || time == 0.82)
            sendData(time == 0.3);
        if (time == 0.35) {
            ASSERT(deliveries == 2 && secondMemberDeliveries == 1);
            auto route = aggregate();
            ASSERT(route);
            auto state = check_and_cast<AodvRouteData *>(route->getProtocolData());
            ASSERT(state->isActive() && state->hasValidDestNum());
            preFailureFloor = state->getDestSeqNum();
            radio(false);
        }
        if (time == 0.65) {
            auto route = aggregate();
            ASSERT(route);
            auto state = check_and_cast<AodvRouteData *>(route->getProtocolData());
            ASSERT(!state->isActive() && state->hasValidDestNum());
            advancedFloor = state->getDestSeqNum();
            uint32_t advances = advancedFloor - preFailureFloor;
            ASSERT(actualLinkFailures >= 1 && suppressedErrors >= 3);
            ASSERT(advances >= 3 && advances < 0x80000000u);
        }
        if (time == 0.7)
            radio(true);
        if (time == 1.2) {
            ASSERT(deliveries == 3 && applicationErrors == 0);
            ASSERT(forwardedFloor > advancedFloor);
            auto route = aggregate();
            ASSERT(route && check_and_cast<AodvRouteData *>(route->getProtocolData())->isActive());
            ASSERT(check_and_cast<AodvRouteData *>(route->getProtocolData())->getDestSeqNum() >= forwardedFloor);
            std::cout << "AODV Subnet Native Recovery " << mode << ": PASS\n";
            return;
        }
        scheduleAt(times[++step], timer);
    }
    Result datagramLocalOutHook(Packet *packet) override {
        if (mode != "RelearnAfterFailure" || simTime() < 0.35 || simTime() >= 0.7)
            return ACCEPT;
        auto ip = packet->peekAtFront<Ipv4Header>();
        if (ip->getProtocolId() != IP_PROT_UDP)
            return ACCEPT;
        auto udp = packet->peekDataAt<UdpHeader>(ip->getHeaderLength());
        if (udp->getDestinationPort() != 654)
            return ACCEPT;
        auto control = packet->peekDataAt<AodvControlPacket>(ip->getHeaderLength() + udp->getChunkLength());
        if (dynamicPtrCast<const Rerr>(control)) {
            suppressedErrors++;
            return DROP;
        }
        return ACCEPT;
    }
    Result datagramPreRoutingHook(Packet *) override { return ACCEPT; }
    Result datagramForwardHook(Packet *) override { return ACCEPT; }
    Result datagramPostRoutingHook(Packet *) override { return ACCEPT; }
    Result datagramLocalInHook(Packet *) override { return ACCEPT; }
    void socketDataArrived(UdpSocket *, Packet *packet) override {
        ASSERT(mode == "ExternalReturn");
        auto source = packet->getTag<L3AddressInd>()->getSrcAddress();
        ASSERT(source == member || source == L3Address(Ipv4Address("198.51.100.100")));
        echoReplies++;
        delete packet;
    }
    void socketErrorArrived(UdpSocket *, Indication *indication) override { applicationErrors++; delete indication; }
    void socketClosed(UdpSocket *) override {}
    void receiveSignal(cComponent *source, simsignal_t signal, cObject *value, cObject *) override {
        Enter_Method("observeNativeSubnetRecovery");
        auto packet = dynamic_cast<Packet *>(value);
        if (!packet)
            return;
        if ((signal == packetReceivedSignal || signal == packetSentSignal) && !strcmp(packet->getName(), "subnet-recovery-data")) {
            deliveries++;
            if (source == getModuleByPath("^.^.member2.app[0]"))
                secondMemberDeliveries++;
            return;
        }
        if (signal == linkBrokenSignal && !strcmp(packet->getName(), "subnet-recovery-data")) {
            actualLinkFailures++;
            return;
        }
        if (signal == packetSentToLowerSignal) {
            auto udp = packet->peekAtFront<UdpHeader>();
            if (udp->getDestinationPort() != 654)
                return;
            auto control = packet->peekDataAt<AodvControlPacket>(udp->getChunkLength());
            if (auto request = dynamicPtrCast<const Rreq>(control)) {
                uint32_t sequence = 0;
                bool found = marker(request, sequence);
                if (source == getModuleByPath("^.udp") && !found)
                    sourceWithoutFloor = true;
                if (source == getModuleByPath("^.^.relay.udp") && request->getDestAddr() == member)
                    relayDiscoveryObserved = true;
                if (source == getModuleByPath("^.^.relay.udp") && found) {
                    ASSERT(request->getUnknownSeqNumFlag() && request->getDestOnlyFlag() && request->getDestSeqNum() == 0);
                    forwardedFloor = std::max(forwardedFloor, sequence);
                }
            }
        }
    }
  public:
    using cListener::finish;
    ~AodvSubnetRecoveryPathPeer() override { cancelAndDelete(timer); }
};
Define_Module(AodvSubnetRecoveryPathPeer);
#endif

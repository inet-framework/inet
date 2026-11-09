// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __AODV_DISCOVERY_TEST_PEER_H
#define __AODV_DISCOVERY_TEST_PEER_H

#include "AodvPeer.h"
#include "inet/linklayer/common/MacAddressTag_m.h"

class AodvDiscoveryPeer : public AodvPeer
{
    std::vector<L3Address> requestedTargets;
    std::vector<std::string> dataNames;
    cComponent *peerIp = nullptr;
    MacAddress expectedMac;
    int wrongOutputs = 0;
    int receivedErrors = 0;
  protected:
    void initialize(int stage) override {
        AodvPeer::initialize(stage);
        if (stage == INITSTAGE_APPLICATION_LAYER) {
            peerIp = getModuleByPath("^.ipv4.ip");
            auto interfaces = check_and_cast<IInterfaceTable *>(getModuleByPath("^.interfaceTable"));
            expectedMac = interfaces->findInterfaceByName("wlan0")->getMacAddress();
            getParentModule()->getParentModule()->subscribe(packetSentToLowerSignal, this);
        }
    }
    void assertComplete(int packets, int requests) {
        ASSERT(int(dataNames.size()) == packets);
        ASSERT(int(requestedTargets.size()) == requests);
        ASSERT(AodvDataCounters::created == AodvDataCounters::destroyed);
        ASSERT(AodvDataCounters::errors == 0);
    }
    void handleMessage(cMessage *message) override {
        if (message != timer) {
            AodvPeer::handleMessage(message);
            return;
        }
        std::string scenario = par("scenario").stdstringValue();
        if (scenario == "DeferredReply" && step == 0) {
            destination = Ipv4Address("10.0.0.98");
            reply(1, 5, 1);
            step++;
            scheduleAt(1.2, timer);
            return;
        }
        if (scenario == "ReplyDuringJitter" && step == 0) {
            hello(1, 5);
            step++;
            scheduleAt(0.5, timer);
            return;
        }
        if (scenario == "IndependentRoute" && step == 0) {
            auto interfaces = check_and_cast<IInterfaceTable *>(getModuleByPath("^.^.dut.interfaceTable"));
            auto route = routes->createRoute();
            route->setDestination(destination);
            route->setPrefixLength(32);
            route->setNextHop(peer);
            route->setInterface(interfaces->findInterfaceByName("wlan0"));
            route->setSourceType(IRoute::MANUAL);
            route->setSource(this);
            route->setMetric(0);
            routes->addRoute(route);
            step++;
            scheduleAt(0.5, timer);
            return;
        }
        if (scenario == "NeighborCompletion" && step < 2) {
            reply(step == 0 ? 20 : 19, step == 0 ? 0.02 : 5);
            step++;
            scheduleAt(step == 1 ? 0.17 : 0.22, timer);
            return;
        }
        if (scenario == "RateLimit")
            assertComplete(2, 2);
        else if (scenario == "GatewayQueues" || scenario == "DeferredReply")
            assertComplete(2, 1);
        else if (scenario == "DiscoveryExhaustion")
            assertComplete(0, 2);
        else if (scenario == "GatewayFailure")
            assertComplete(0, 1);
        else if (scenario == "ReplyDuringJitter" || scenario == "IndependentRoute")
            assertComplete(1, 0);
        else if (scenario == "NeighborCompletion") {
            checkSequence(destination, 20, false);
            ASSERT(dataNames.size() == 1 && dataNames.front() == "queued-data-0");
            ASSERT(std::find(requestedTargets.begin(), requestedTargets.end(), peer) == requestedTargets.end());
        }
        else if (scenario == "OutputConstraints") {
            ASSERT(wrongOutputs == 0);
            assertComplete(2, 1);
            int omitted = par("incompatibleIndex");
            std::vector<std::string> expected;
            for (int i = 0; i < 3; i++)
                if (i != omitted)
                    expected.push_back("queued-data-" + std::to_string(i));
            ASSERT(dataNames == expected);
        }
        else if (scenario == "TransitAdmission") {
            ASSERT(requestedTargets.empty() && receivedErrors == 1);
            ASSERT(dataNames.empty());
        }
        else
            throw cRuntimeError("Unknown discovery scenario: %s", scenario.c_str());
        pass(scenario);
    }
    void socketDataArrived(UdpSocket *socket, Packet *packet) override {
        auto header = packet->peekAtFront<AodvControlPacket>();
        std::string scenario = par("scenario").stdstringValue();
        if (auto request = dynamicPtrCast<const Rreq>(header)) {
            requestedTargets.push_back(request->getDestAddr());
            if (scenario == "RateLimit" || scenario == "GatewayQueues" || scenario == "OutputConstraints" ||
                    (scenario == "DeferredReply" && request->getDestAddr() == Ipv4Address("10.0.0.99"))) {
                destination = request->getDestAddr();
                reply(1, 5, request->getDestAddr() == peer ? 0 : 1);
            }
        }
        else if (dynamicPtrCast<const Rerr>(header))
            receivedErrors++;
        AodvPeer::socketDataArrived(socket, packet);
    }
    void receiveSignal(cComponent *source, simsignal_t signal, cObject *value, cObject *) override {
        Enter_Method("observeDiscovery");
        auto packet = dynamic_cast<Packet *>(value);
        if (!packet || strncmp(packet->getName(), "queued-data-", 12))
            return;
        if (source == peerIp && signal == packetReceivedFromLowerSignal)
            dataNames.push_back(packet->getName());
        else if (signal == packetSentToLowerSignal &&
                source->getFullPath().find(".dut.wlan[") != std::string::npos) {
            auto mac = packet->findTag<MacAddressReq>();
            if (source->getFullPath().find(".dut.wlan[0].mac") == std::string::npos ||
                    !mac || mac->getDestAddress() != expectedMac)
                wrongOutputs++;
        }
    }
    void finish() override {
        auto scenario = par("scenario").stdstringValue();
        if (scenario == "NeighborCompletion") {
            ASSERT(dataNames.size() == 1);
            ASSERT(std::find(requestedTargets.begin(), requestedTargets.end(), peer) == requestedTargets.end());
        }
    }
    using AodvPeer::finish;
};
Define_Module(AodvDiscoveryPeer);
#endif

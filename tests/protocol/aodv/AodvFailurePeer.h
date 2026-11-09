// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __AODV_FAILURE_TEST_PEER_H
#define __AODV_FAILURE_TEST_PEER_H
#include "AodvPeer.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"
#include "inet/queueing/queue/PacketQueue.h"
#ifdef INET_WITH_IEEE80211
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#endif

class AodvTagStrippingQueue : public inet::queueing::PacketQueue
{
  public:
    void pushPacket(Packet *packet, const cGate *gate) override {
        Enter_Method("pushPacket");
        if (!strncmp(packet->getName(), "queued-data-", 12))
            packet->removeTagIfPresent<InterfaceReq>();
        PacketQueue::pushPacket(packet, gate);
    }
};
Define_Module(AodvTagStrippingQueue);

class AodvFailurePeer : public AodvPeer
{
    int failures = 0;
    int delivered = 0;
    bool retainedOutput = false;
    bool immediateReplacement = false;
    bool replacedOnWire = false;
    MacAddress attemptedMac;
    void offer(const L3Address& target, uint32_t sequence, int output = 0, bool claimNeighbor = false) {
        auto reply = makeShared<Rrep>();
        reply->setPacketType(RREP);
        reply->setChunkLength(B(20));
        reply->setOriginatorAddr(dut);
        reply->setDestAddr(target);
        reply->setDestSeqNum(sequence);
        reply->setLifeTime(5);
        reply->setHopCount(1);
        if (!claimNeighbor)
            transmit(reply, output);
        else {
            auto interfaces = check_and_cast<IInterfaceTable *>(getModuleByPath("^.interfaceTable"));
            auto selected = interfaces->findInterfaceByName("wlan1");
            auto packet = new Packet("scripted-binding-change", reply);
            packet->addTag<InterfaceReq>()->setInterfaceId(selected->getInterfaceId());
            packet->addTag<L3AddressReq>()->setSrcAddress(peer);
            packet->addTag<NextHopAddressReq>()->setNextHopAddress(dut);
            packet->addTag<HopLimitReq>()->setHopLimit(10);
            socket.sendTo(packet, dut, 654);
        }
    }
    void seed(const L3Address& target, uint32_t sequence) {
        auto route = routes->createRoute();
        auto interfaces = check_and_cast<IInterfaceTable *>(getModuleByPath("^.^.dut.interfaceTable"));
        route->setDestination(target);
        route->setPrefixLength(32);
        route->setNextHop(peer);
        route->setInterface(interfaces->findInterfaceByName("wlan0"));
        route->setMetric(2);
        route->setSourceType(IRoute::AODV);
        route->setSource(getModuleByPath("^.^.dut.aodv"));
        auto data = new AodvRouteData();
        data->setIsActive(true);
        data->setHasValidDestNum(true);
        data->setDestSeqNum(sequence);
        data->setLifeTime(5);
        route->setProtocolData(data);
        routes->addRoute(route);
    }
    void next(simtime_t time) { step++; scheduleAt(time, timer); }
  protected:
    void initialize(int stage) override {
        AodvPeer::initialize(stage);
        if (stage == INITSTAGE_APPLICATION_LAYER) {
            auto interfaces = check_and_cast<IInterfaceTable *>(getModuleByPath("^.interfaceTable"));
            attemptedMac = interfaces->findInterfaceByName("wlan0")->getMacAddress();
            getModuleByPath("^.^.dut")->subscribe(linkBrokenSignal, this);
            immediateReplacement = hasPar("immediateReplacement") && par("immediateReplacement").boolValue();
            if (immediateReplacement)
                getModuleByPath("^.^.dut")->subscribe(packetSentToLowerSignal, this);
        }
    }
    void handleMessage(cMessage *message) override {
        if (message != timer) { AodvPeer::handleMessage(message); return; }
        std::string scenario = par("scenario").stdstringValue();
        if (simTime() == SimTime(0.1)) {
            if (scenario == "NoBinding") {
                seed(destination, 20);
                seed(Ipv4Address("10.0.0.98"), 30);
            }
            else
                offer(destination, 20);
            next(0.11);
            return;
        }
        if (simTime() == SimTime(0.11)) {
            if (scenario != "NoBinding")
                offer(Ipv4Address("10.0.0.98"), 30);
            next(scenario == "Ambiguous" || scenario == "Rebinding" || immediateReplacement ? 0.12 : 0.15);
            return;
        }
        if (simTime() == SimTime(0.12)) {
            offer(Ipv4Address("10.0.0.97"), 40, 1, scenario == "Rebinding");
            next(0.15);
            return;
        }
        if (simTime() == SimTime(0.15)) {
            auto radio = check_and_cast<inet::physicallayer::IRadio *>(getModuleByPath("^.wlan[0].radio"));
            radio->setRadioMode(inet::physicallayer::IRadio::RADIO_MODE_OFF);
            if (scenario == "Ambiguous")
                check_and_cast<inet::physicallayer::IRadio *>(getModuleByPath("^.wlan[1].radio"))->setRadioMode(inet::physicallayer::IRadio::RADIO_MODE_OFF);
            next(0.18);
            return;
        }
        if (simTime() == SimTime(0.18)) {
            if (scenario == "Ambiguous") {
                auto route = findRoute(destination);
                route->setNextHop(Ipv4Address("10.0.0.3"));
                auto data = check_and_cast<AodvRouteData *>(route->getProtocolData());
                data->setDestSeqNum(21);
                data->setLifeTime(5);
            }
            else if (!immediateReplacement)
                offer(destination, 21, 1);
            next(scenario == "Ambiguous" ? SimTime(0.28) : (hasPar("checkTime") ? SimTime(par("checkTime")) : SimTime(0.45)));
            return;
        }
        if (simTime() == SimTime(0.28)) {
            check_and_cast<inet::physicallayer::IRadio *>(getModuleByPath("^.wlan[1].radio"))->setRadioMode(inet::physicallayer::IRadio::RADIO_MODE_TRANSCEIVER);
            next(0.45);
            return;
        }
        ASSERT(failures == 1);
        if (immediateReplacement)
            ASSERT(replacedOnWire);
        ASSERT(retainedOutput == (scenario != "MissingOutput"));
        auto replacement = findRoute(destination);
        auto data = check_and_cast<AodvRouteData *>(replacement->getProtocolData());
        ASSERT(data->isActive() && data->getDestSeqNum() == 21);
        ASSERT(replacement->getNextHopAsGeneric() == Ipv4Address("10.0.0.3"));
        auto dependent = findRoute(Ipv4Address("10.0.0.98"));
        auto dependentData = check_and_cast<AodvRouteData *>(dependent->getProtocolData());
        bool attributed = scenario == "Replacement" || scenario == "ExpiredNeighbor";
        ASSERT(dependentData->isActive() != attributed);
        ASSERT(dependentData->getDestSeqNum() == (attributed ? 31 : 30));
        if (scenario == "ExpiredNeighbor")
            ASSERT(!findRoute(peer) && !findRoute(Ipv4Address("10.0.0.3")));
        ASSERT(delivered == 1);
        ASSERT(AodvDataCounters::created == 2 && AodvDataCounters::destroyed == 2);
        pass(scenario);
    }
    void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *) override {
        Enter_Method("observeFailedAttempt");
        auto packet = dynamic_cast<Packet *>(object);
        if (!packet || strncmp(packet->getName(), "queued-data-", 12))
            return;
        if (signal == packetSentToLowerSignal && !replacedOnWire &&
                source->getFullPath().find(".dut.wlan[0].mac") != std::string::npos) {
            auto route = findRoute(destination);
            route->setNextHop(Ipv4Address("10.0.0.3"));
            auto data = check_and_cast<AodvRouteData *>(route->getProtocolData());
            data->setDestSeqNum(21);
            data->setLifeTime(5);
            replacedOnWire = true;
        }
        else if (signal == linkBrokenSignal) {
            failures++;
#ifdef INET_WITH_IEEE80211
            if (immediateReplacement) {
                ASSERT(source->getFullPath().find(".mac.dcf") != std::string::npos);
                auto header = packet->peekAtFront<inet::ieee80211::Ieee80211DataHeader>();
                ASSERT(!header->getToDS() && !header->getFromDS() && header->getRetry());
                ASSERT(header->getReceiverAddress() == attemptedMac);
            }
#endif
            auto output = packet->findTag<InterfaceReq>();
            auto mac = packet->findTag<MacAddressReq>();
            retainedOutput = output && mac;
            ASSERT(mac && mac->getDestAddress() == attemptedMac);
            if (output) {
                auto interfaces = check_and_cast<IInterfaceTable *>(getModuleByPath("^.^.dut.interfaceTable"));
                ASSERT(output->getInterfaceId() == interfaces->findInterfaceByName("wlan0")->getInterfaceId());
            }
        }
        else if (signal == packetReceivedFromLowerSignal)
            delivered++;
    }
};
Define_Module(AodvFailurePeer);
#endif

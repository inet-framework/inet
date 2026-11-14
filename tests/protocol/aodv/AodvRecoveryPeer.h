// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __AODV_RECOVERY_TEST_PEER_H
#define __AODV_RECOVERY_TEST_PEER_H
#include "AodvPeer.h"
#include "inet/common/lifecycle/LifecycleController.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/networklayer/ipv4/Ipv4.h"
#include "inet/linklayer/common/MacAddressTag_m.h"

// The real owner finishes reinjection before a test callback can restart the node.
class AodvRecoveryIpv4 : public Ipv4
{
    bool notified = false;
  protected:
    void handleIncomingDatagram(Packet *packet) override {
        if (hasPar("stripObservation") && par("stripObservation").boolValue() && simTime() < SimTime(0.1) &&
                !strcmp(packet->getName(), "hello-observation-data"))
            packet->removeTagIfPresent<MacAddressInd>();
        simtime_t delivery = hasPar("transitDelivery") ? par("transitDelivery").doubleValue() : 0;
        if (delivery > simTime() && packet->peekAtFront<Ipv4Header>()->getDestAddress() == Ipv4Address("10.0.0.77"))
            scheduleAt(delivery, packet);
        else
            Ipv4::handleIncomingDatagram(packet);
    }
    void handleMessageWhenUp(cMessage *message) override {
        if (message->isSelfMessage())
            Ipv4::handleIncomingDatagram(check_and_cast<Packet *>(message));
        else
            Ipv4::handleMessageWhenUp(message);
    }
  public:
    void reinjectQueuedDatagram(const Packet *packet) override {
        Enter_Method("reinjectQueuedDatagram");
        Ipv4::reinjectQueuedDatagram(packet);
        if (par("reenter").boolValue() && !notified) {
            notified = true;
            emit(registerSignal("recoveryReinjected"), 1L);
        }
    }
};
Define_Module(AodvRecoveryIpv4);

class AodvRecoveryPeer : public AodvPeer, public LifecycleController
{
    std::string mode;
    simtime_t restartTime;
    std::vector<simtime_t> times;
    std::vector<simtime_t> requests;
    std::vector<simtime_t> deliveries;
    int acknowledgments = 0;
    int selfReplies = 0;
    int cachedReplies = 0;
    int relayedReplies = 0;
    int errors = 0;
    int forbiddenAdvertisements = 0;
    uint32_t restoredSequence = 0;
    simtime_t transitIngress;
    bool reentered = false;
    UdpSocket observationSocket;
    bool zero() const { return mode == "ZeroRestart" || mode == "ZeroForward" || mode == "DoubleRestart"; }
    bool tie() const { return mode == "ExtensionFirst" || mode == "WakeFirst"; }
    bool queueCase() const { return mode.find("Queue") == 0; }
    bool helloReset() const { return mode == "ObservationReset" || mode == "BindingReset"; }
    bool validationReset() const { return mode == "AckReset" || mode == "BlacklistReset"; }
    bool noData() const { return mode == "Cleanup" || mode == "Foreign" || mode.find("Delayed") == 0 || helloReset() || validationReset(); }
    void helloFrom(int output = 0) {
        auto hello = makeShared<Rrep>(); hello->setPacketType(RREP); hello->setChunkLength(B(20));
        hello->setDestAddr(peer); hello->setDestSeqNum(10); hello->setLifeTime(1);
        auto packet = new Packet("recovery-hello", hello);
        auto interfaces = check_and_cast<IInterfaceTable *>(getModuleByPath("^.interfaceTable"));
        packet->addTag<InterfaceReq>()->setInterfaceId(interfaces->findInterfaceByName(output ? "wlan1" : "wlan0")->getInterfaceId());
        packet->addTag<L3AddressReq>()->setSrcAddress(peer);
        packet->addTag<NextHopAddressReq>()->setNextHopAddress(dut);
        packet->addTag<HopLimitReq>()->setHopLimit(1);
        socket.sendTo(packet, dut, 654);
    }
    void operate(LifecycleOperation *operation, bool whole = false) {
        auto root = getModuleByPath(whole ? "^.^.dut" : "^.^.dut.aodv");
        LifecycleOperation::StringMap parameters;
        operation->initialize(root, parameters);
        ASSERT(parameters.empty());
        ASSERT(initiateOperation(operation));
    }
    void offer(const L3Address& target, uint32_t sequence, bool relay = false, bool ack = true) {
        auto reply = makeShared<Rrep>();
        reply->setPacketType(RREP); reply->setChunkLength(B(20));
        reply->setDestAddr(target); reply->setDestSeqNum(sequence);
        reply->setOriginatorAddr(relay ? L3Address(Ipv4Address("10.0.0.3")) : dut);
        reply->setLifeTime(1); reply->setHopCount(1); reply->setAckRequiredFlag(ack);
        transmit(reply);
    }
    void ask(const L3Address& target, uint32_t id, int output = 0, bool known = false) {
        auto request = makeShared<Rreq>();
        request->setPacketType(RREQ); request->setChunkLength(B(24));
        request->setOriginatorAddr(output ? L3Address(Ipv4Address("10.0.0.3")) : peer);
        request->setOriginatorSeqNum(id); request->setRreqId(id);
        request->setDestAddr(target); request->setDestSeqNum(known ? 100 : 0);
        request->setUnknownSeqNumFlag(!known); request->setDestOnlyFlag(target != destination);
        transmit(request, output);
    }
    void seedForeign() {
        auto route = routes->createRoute();
        auto interfaces = check_and_cast<IInterfaceTable *>(getModuleByPath("^.^.dut.interfaceTable"));
        route->setDestination(Ipv4Address("10.0.0.88")); route->setPrefixLength(32);
        route->setNextHop(peer); route->setInterface(interfaces->findInterfaceByName("wlan0"));
        route->setMetric(2); route->setSourceType(IRoute::MANUAL); route->setSource(this);
        auto state = new AodvRouteData(); state->setDestSeqNum(123); state->setLifeTime(5);
        route->setProtocolData(state); routes->addRoute(route);
    }
    int ownedRoutes() {
        int count = 0;
        for (int i = 0; i < routes->getNumRoutes(); i++)
            count += routes->getRoute(i)->getSource() == getModuleByPath("^.^.dut.aodv");
        return count;
    }
    bool learned() const { return mode != "NoReply" && !queueCase() && !noData(); }
    void checkEarly() {
        if (validationReset()) ASSERT(selfReplies == 0);
        if (mode == "ForwardReply" || mode == "ZeroForward") ASSERT(relayedReplies == 0);
        if (mode == "Advertisements") ASSERT(selfReplies == 0 && forbiddenAdvertisements == 0);
        if (mode == "CachedAdvertisement") ASSERT(cachedReplies == 0);
        if (mode == "DownInit") {
            ASSERT(!requests.empty() && requests.front() < restartTime + SimTime(0.1));
            ASSERT(deliveries.size() == 1);
            return;
        }
        if (!queueCase() && !noData()) {
            ASSERT(deliveries.empty());
            ASSERT(requests.empty());
            if (learned()) {
                ASSERT(acknowledgments == 1);
                auto state = check_and_cast<AodvRouteData *>(findRoute(destination)->getProtocolData());
                ASSERT(state->isActive() && state->getDestSeqNum() == 20);
            }
        }
    }
    void finishCase() {
        ASSERT(!forbiddenAdvertisements);
        if (mode == "Cleanup" || mode == "Foreign") {
            ASSERT(ownedRoutes() == 0);
            if (mode == "Foreign") {
                bool found = false;
                for (int i = 0; i < routes->getNumRoutes(); i++) {
                    auto route = routes->getRoute(i);
                    if (route->getSource() == this) {
                        auto state = check_and_cast<AodvRouteData *>(route->getProtocolData());
                        ASSERT(state->isActive() && state->getDestSeqNum() == 123 && state->getLifeTime() == SimTime(5));
                        found = true;
                    }
                }
                ASSERT(found);
            }
        }
        else if (mode.find("Delayed") == 0) {
            ASSERT(requests.empty() && errors == 0);
        }
        else if (queueCase()) {
            ASSERT(AodvDataCounters::created > 0 && AodvDataCounters::created == AodvDataCounters::destroyed);
            ASSERT(deliveries.empty());
            for (auto time : requests) ASSERT(time < SimTime(0.1));
        }
        else if (helloReset()) ASSERT(errors == 1 && selfReplies == 1 && acknowledgments == 1);
        else if (validationReset()) ASSERT(selfReplies == 1);
        else if (mode == "Reentrant") {
            ASSERT(reentered && deliveries.size() <= 1);
            ASSERT(AodvDataCounters::created == 3 && AodvDataCounters::destroyed == 3);
        }
        else if (mode == "NoReply") {
            ASSERT(!requests.empty() && requests.front() >= restartTime + SimTime(0.3));
            ASSERT(deliveries.empty() && AodvDataCounters::created == AodvDataCounters::destroyed);
        }
        else {
            ASSERT(deliveries.size() == 1);
            if (mode != "DownInit") ASSERT(requests.empty());
            simtime_t earliest = mode == "Extend" || mode == "ExtendLearned" || mode == "ExtensionFirst" ? transitIngress + SimTime(0.3) : restartTime + SimTime(0.3);
            if (mode == "DoubleRestart") earliest += SimTime(0.02);
            if (mode != "DownInit") ASSERT(deliveries.front() >= earliest && deliveries.front() < earliest + SimTime(0.01));
            if (mode == "Extend" || mode == "ExtendLearned" || tie()) ASSERT(errors == 1);
            if (mode == "ExtendLearned") {
                auto state = check_and_cast<AodvRouteData *>(findRoute(destination)->getProtocolData());
                ASSERT(state->isActive() && state->getDestSeqNum() == 22);
            }
            if (tie()) ASSERT(transitIngress == SimTime(0.4));
            if (mode == "Advertisements") ASSERT(selfReplies == 1 && restoredSequence == 100);
            if (mode == "CachedAdvertisement") ASSERT(cachedReplies == 1);
            if (mode == "ForwardReply" || mode == "ZeroForward") ASSERT(relayedReplies == 1);
        }
        pass("RebootRecovery" + mode);
    }
  protected:
    void initialize(int stage) override {
        AodvPeer::initialize(stage);
        if (stage == INITSTAGE_APPLICATION_LAYER) {
            mode = par("scenario").stdstringValue();
            restartTime = zero() ? SIMTIME_ZERO : tie() ? SimTime(0.1) : SimTime(0.15);
            dutIp = getModuleByPath("^.^.dut.ipv4.ip");
            dutIp->subscribe(packetReceivedFromLowerSignal, this);
            dutIp->subscribe(registerSignal("recoveryReinjected"), this);
            if (mode == "ObservationReset") {
                observationSocket.setOutputGate(gate("socketOut"));
                observationSocket.bind(23456);
            }
            for (double time : {0.0, 0.02, 0.04, 0.1, 0.125, 0.15})
                times.push_back(SimTime(time));
            for (double offset : {0.08, 0.1, 0.11, 0.12, 0.13, 0.15, 0.2, 0.25, 0.3, 0.34, 0.35, 0.36, 0.4, 0.6, 0.8})
                times.push_back(restartTime + SimTime(offset));
            std::sort(times.begin(), times.end());
            times.erase(std::unique(times.begin(), times.end()), times.end());
        }
    }
    void handleMessage(cMessage *message) override {
        if (message != timer) { AodvPeer::handleMessage(message); return; }
        auto time = simTime();
        if (time == SIMTIME_ZERO && zero()) {
            operate(mode == "DoubleRestart" ? static_cast<LifecycleOperation *>(new ModuleStopOperation()) : new ModuleCrashOperation());
            operate(new ModuleStartOperation());
        }
        if (time == SimTime(0.02) && mode == "DoubleRestart") { operate(new ModuleStopOperation()); operate(new ModuleStartOperation()); }
        if (time == SimTime(0.02) && !zero() && mode != "DownInit") {
            if (helloReset()) helloFrom();
            else if (validationReset()) ask(dut, 2);
            else if (mode.find("Delayed") == 0) ask(destination, 1);
            else offer(queueCase() ? L3Address(Ipv4Address("10.0.0.97")) : destination, 80, false, false);
            if (mode == "Foreign") seedForeign();
        }
        if (time == SimTime(0.04) && mode == "BindingReset") helloFrom(1);
        if (time == SimTime(0.04) && mode == "ObservationReset") {
            auto packet = new Packet("hello-observation-data", makeShared<ByteCountChunk>(B(32)));
            auto interfaces = check_and_cast<IInterfaceTable *>(getModuleByPath("^.interfaceTable"));
            packet->addTag<InterfaceReq>()->setInterfaceId(interfaces->findInterfaceByName("wlan0")->getInterfaceId());
            packet->addTag<NextHopAddressReq>()->setNextHopAddress(dut);
            observationSocket.sendTo(packet, dut, 54321);
        }
        if (time == SimTime(0.1) && !zero() && mode != "DownInit")
            operate(mode == "Crash" || mode == "QueueCrash" ? static_cast<LifecycleOperation *>(new ModuleCrashOperation()) : new ModuleStopOperation(), mode == "QueueCrash");
        if (time == SimTime(0.1) && tie()) operate(new ModuleStartOperation());
        if (time == SimTime(0.125) && (mode == "Cleanup" || mode == "Foreign" || queueCase())) {
            if (queueCase() && mode != "QueueRecovery") ASSERT(AodvDataCounters::created == AodvDataCounters::destroyed);
            ASSERT(ownedRoutes() == 0);
        }
        if (time == SimTime(0.15) && !zero() && !tie()) operate(new ModuleStartOperation(), mode == "DownInit" || mode == "QueueCrash");
        if (time == restartTime + SimTime(0.08) && (mode == "ForwardReply" || mode == "ZeroForward" || mode == "ExtendLearned"))
            ask(Ipv4Address("10.0.0.77"), 8, 1);
        if (time == restartTime + SimTime(0.1) && learned())
            offer(destination, 20, mode == "ForwardReply" || mode == "ZeroForward");
        if (time == restartTime + SimTime(0.1) && helloReset()) helloFrom();
        if (time == restartTime + SimTime(0.11) && helloReset()) offer(destination, 20);
        if (time == restartTime + SimTime(0.12) && helloReset()) {
            auto interfaces = check_and_cast<IInterfaceTable *>(getModuleByPath("^.^.dut.interfaceTable"));
            check_and_cast<AodvRouteData *>(findRoute(destination)->getProtocolData())->addPrecursor(Ipv4Address("10.0.0.3"), interfaces->findInterfaceByName("wlan0")->getInterfaceId());
        }
        if (time == restartTime + SimTime(0.25) && helloReset()) {
            auto state = check_and_cast<AodvRouteData *>(findRoute(destination)->getProtocolData());
            ASSERT(!state->isActive() && state->getDestSeqNum() == 21);
        }
        if (time == restartTime + SimTime(0.25) && mode == "ExtendLearned") offer(destination, 22, false, false);
        if (time == restartTime + SimTime(0.1) && validationReset()) ask(dut, 10);
        if (time == restartTime + SimTime(0.12) && mode == "Advertisements") ask(dut, 12, 0, true);
        if (time == restartTime + SimTime(0.13) && mode == "Advertisements") ask(Ipv4Address("10.0.0.77"), 13);
        if (time == restartTime + SimTime(0.13) && mode == "CachedAdvertisement") ask(destination, 13, 1);
        if (time == restartTime + SimTime(0.15)) checkEarly();
        if (time == SimTime(0.3) && mode == "QueueRecovery") operate(new ModuleStopOperation());
        if (time == SimTime(0.35) && mode == "QueueRecovery") operate(new ModuleStartOperation());
        if (time == restartTime + SimTime(0.35) && (mode == "Extend" || mode == "ExtendLearned" || mode == "ExtensionFirst")) ASSERT(deliveries.empty());
        if (time == restartTime + SimTime(0.35) && mode == "Advertisements") ask(dut, 35);
        if (time == restartTime + SimTime(0.35) && (helloReset() || validationReset())) ask(dut, 35);
        if (time == restartTime + SimTime(0.36) && mode == "CachedAdvertisement") ask(destination, 36, 1);
        if (time == restartTime + SimTime(0.34) && (mode == "ForwardReply" || mode == "ZeroForward"))
            ask(Ipv4Address("10.0.0.77"), 34, 1);
        if (time == restartTime + SimTime(0.35) && (mode == "ForwardReply" || mode == "ZeroForward"))
            offer(destination, 21, true, false);
        if (time == restartTime + SimTime(0.8)) { finishCase(); return; }
        scheduleAt(times[++step], timer);
    }
    void socketDataArrived(UdpSocket *, Packet *packet) override {
        auto control = packet->peekAtFront<AodvControlPacket>();
        if (auto request = dynamicPtrCast<const Rreq>(control)) {
            if (request->getOriginatorAddr() == dut) requests.push_back(simTime());
            else if (simTime() >= restartTime && simTime() < restartTime + SimTime(0.3)) forbiddenAdvertisements++;
        }
        else if (auto ack = dynamicPtrCast<const RrepAck>(control)) acknowledgments++;
        else if (auto reply = dynamicPtrCast<const Rrep>(control)) {
            if (simTime() >= restartTime) {
                if (reply->getOriginatorAddr().isUnspecified()) {
                    if (simTime() < restartTime + SimTime(0.3)) forbiddenAdvertisements++;
                }
                else if (reply->getDestAddr() == dut) { selfReplies++; restoredSequence = reply->getDestSeqNum(); }
                else if (reply->getDestAddr() == destination) {
                    if (mode == "CachedAdvertisement") cachedReplies++;
                    else relayedReplies++;
                }
            }
            if (reply->getAckRequiredFlag() && !(validationReset() && simTime() < SimTime(0.1))) {
                auto ack = makeShared<RrepAck>(); ack->setPacketType(RREPACK); ack->setChunkLength(B(2));
                int output = packet->getTag<L3AddressInd>()->getDestAddress() == Ipv4Address("10.0.0.3") ? 1 : 0;
                transmit(ack, output);
            }
        }
        else if (auto error = dynamicPtrCast<const Rerr>(control)) {
            errors++;
            ASSERT(error->getUnreachableNodesArraySize() == 1 &&
                error->getUnreachableNodes(0).addr == (helloReset() || mode == "ExtendLearned" ? destination : L3Address(Ipv4Address("10.0.0.77"))));
            if (mode == "ExtendLearned") ASSERT(error->getUnreachableNodes(0).seqNum == 21);
        }
        delete packet;
    }
    void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *) override {
        Enter_Method("observeRecovery");
        auto packet = dynamic_cast<Packet *>(object);
        if (!packet || strncmp(packet->getName(), "queued-data-", 12)) return;
        if (source == getModuleByPath("^.ipv4.ip") && signal == packetReceivedFromLowerSignal) deliveries.push_back(simTime());
        if (source == dutIp && signal == packetReceivedFromLowerSignal &&
                L3Address(packet->peekAtFront<Ipv4Header>()->getDestAddress()) == (mode == "ExtendLearned" ? destination : L3Address(Ipv4Address("10.0.0.77"))))
            transitIngress = simTime();
    }
    void receiveSignal(cComponent *source, simsignal_t signal, long, cObject *) override {
        Enter_Method("restartAfterReinjection");
        ASSERT(mode == "Reentrant" && source == dutIp && !reentered);
        reentered = true;
        operate(new ModuleCrashOperation(), true);
        operate(new ModuleStartOperation(), true);
    }
};
Define_Module(AodvRecoveryPeer);
#endif

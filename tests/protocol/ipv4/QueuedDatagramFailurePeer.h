// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __QUEUED_DATAGRAM_FAILURE_PEER_H
#define __QUEUED_DATAGRAM_FAILURE_PEER_H

#include "inet/common/INETDefs.h"
#include "inet/common/InitStages.h"
#include "inet/common/lifecycle/LifecycleController.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/ModulePathAddress.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/Icmpv4ErrorTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/networklayer/contract/INetfilter.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv4/IcmpHeader_m.h"
#ifdef INET_WITH_IPv6
#include "inet/networklayer/common/Icmpv6ErrorTag_m.h"
#include "inet/networklayer/ipv6/Ipv6Header_m.h"
#include "inet/networklayer/icmpv6/Icmpv6Header_m.h"
#endif
#include "inet/transportlayer/contract/udp/UdpSocket.h"

using namespace inet;

class QueuedFailureTrackedPacket : public Packet
{
  public:
    static int destroyed;
    QueuedFailureTrackedPacket() : Packet("queued-failure-original") {}
    ~QueuedFailureTrackedPacket() override { destroyed++; }
};
int QueuedFailureTrackedPacket::destroyed = 0;

class QueuedDatagramFailurePeer : public cSimpleModule, public NetfilterBase::HookBase, public UdpSocket::ICallback, public LifecycleController, public IDoneCallback
{
  protected:
    UdpSocket socket;
    INetfilter *ip = nullptr;
    Packet *queued = nullptr;
    cMessage *timer = nullptr;
    std::string mode;
    bool using6 = false;
    int step = 0;
    int errors = 0;
    int queuedCount = 0;
    int forwarded = 0;
    int generatedErrors = 0;
    bool operationDone = false;
    const Packet *lifecycleIdentity = nullptr;
    L3Address target;

    int numInitStages() const override { return NUM_INIT_STAGES; }
    void initialize(int stage) override {
        if (stage != INITSTAGE_APPLICATION_LAYER)
            return;
        mode = par("scenario").stdstringValue();
        using6 = par("usingIpv6");
        auto node = mode == "Transit" || mode == "SuppressedIcmp" ? getModuleByPath("^.^.router") : getParentModule();
        auto layer = node->getSubmodule(mode == "NextHop" ? "generic" : using6 ? "ipv6" : "ipv4");
        ASSERT(layer);
        ip = check_and_cast<INetfilter *>(layer->getSubmodule(mode == "NextHop" ? "nextHop" : using6 ? "ipv6" : "ip"));
        ip->registerHook(0, this);
        socket.setOutputGate(gate("socketOut"));
        socket.setCallback(this);
        timer = new cMessage("queued-failure-step");
        scheduleAt(using6 ? 5 : 0.1, timer);
    }
    Result datagramPreRoutingHook(Packet *packet) override {
        if (mode == "Transit" || mode == "SuppressedIcmp")
            return intercept(packet);
        return ACCEPT;
    }
    Result datagramLocalOutHook(Packet *packet) override {
        if (mode == "Transit" || mode == "SuppressedIcmp") {
            if (using6) {
#ifdef INET_WITH_IPv6
                auto header = packet->peekAtFront<Ipv6Header>();
                if (header->getProtocolId() == IP_PROT_IPv6_ICMP &&
                        packet->peekDataAt<Icmpv6Header>(header->getChunkLength())->getType() < 128)
                    generatedErrors++;
#else
                throw cRuntimeError("IPv6 is disabled");
#endif
            }
            else {
                auto header = packet->peekAtFront<Ipv4Header>();
                if (header->getProtocolId() == IP_PROT_ICMP &&
                        packet->peekDataAt<IcmpHeader>(header->getChunkLength())->getType() == ICMP_DESTINATION_UNREACHABLE)
                    generatedErrors++;
            }
        }
        if (mode != "Transit" && mode != "SuppressedIcmp")
            return intercept(packet);
        return ACCEPT;
    }
    Result intercept(Packet *packet) {
        if (std::string(packet->getName()) != "queued-failure-original")
            return ACCEPT;
        ASSERT(!queued);
        queued = packet;
        queuedCount++;
        return QUEUE;
    }
    Result datagramForwardHook(Packet *packet) override {
        if (std::string(packet->getName()) == "queued-failure-original")
            forwarded++;
        return ACCEPT;
    }
    Result datagramPostRoutingHook(Packet *packet) override {
        if (std::string(packet->getName()) == "queued-failure-original")
            forwarded++;
        return ACCEPT;
    }
    Result datagramLocalInHook(Packet *) override { return ACCEPT; }
    void handleMessage(cMessage *message) override {
        if (message != timer) {
            socket.processMessage(message);
            return;
        }
        if (step++ == 0) {
            if (mode == "NextHop") {
                auto packet = new QueuedFailureTrackedPacket();
                packet->insertAtBack(makeShared<ByteCountChunk>(B(32)));
                packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::echo);
                packet->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::nextHopForwarding);
                packet->addTag<L3AddressReq>()->setDestAddress(ModulePathAddress(getModuleByPath("^.^.hostB.eth[0]")->getId()));
                send(packet, "socketOut");
                scheduleAfter(0.05, timer);
                return;
            }
            target = L3AddressResolver().resolve("hostB");
            auto source = L3AddressResolver().resolve("hostA");
            socket.bind(mode == "LocalBound" ? source : L3Address(), 41000);
            if (mode == "SuppressedIcmp") {
                auto packet = new Packet("queued-failure-original");
                if (using6) {
#ifdef INET_WITH_IPv6
                    auto header = makeShared<Icmpv6DestUnreachableMsg>();
                    header->setCode(NO_ROUTE_TO_DEST);
                    packet->insertAtFront(header);
#else
                    throw cRuntimeError("IPv6 is disabled");
#endif
                }
                else {
                    auto header = makeShared<IcmpHeader>();
                    header->setType(ICMP_DESTINATION_UNREACHABLE);
                    header->setCode(ICMP_DU_PORT_UNREACHABLE);
                    packet->insertAtFront(header);
                }
                packet->addTag<PacketProtocolTag>()->setProtocol(using6 ? &Protocol::icmpv6 : &Protocol::icmpv4);
                packet->addTag<DispatchProtocolReq>()->setProtocol(using6 ? &Protocol::ipv6 : &Protocol::ipv4);
                auto addresses = packet->addTag<L3AddressReq>();
                addresses->setSrcAddress(source);
                addresses->setDestAddress(target);
                send(packet, "socketOut");
            }
            else {
                auto packet = new QueuedFailureTrackedPacket();
                packet->insertAtBack(makeShared<ByteCountChunk>(B(32)));
                socket.sendTo(packet, target, 42000);
            }
            scheduleAfter(0.05, timer);
        }
        else if (step == 2) {
            ASSERT(queuedCount == 1 && queued);
            bool local = mode != "Transit" && mode != "SuppressedIcmp";
            if (local)
                ASSERT(QueuedFailureTrackedPacket::destroyed == 0);
            if (mode == "UnknownPointer") {
                auto unrelated = new Packet("not-owned-by-ip");
                ip->failQueuedDatagram(unrelated);
                ASSERT(unrelated->getOwner() == this);
                delete unrelated;
            }
            auto identity = queued;
            queued = nullptr;
            ip->failQueuedDatagram(identity);
            if (local)
                ASSERT(QueuedFailureTrackedPacket::destroyed == 1);
            if (mode == "RepeatedFailure") {
                ip->failQueuedDatagram(identity);
                ASSERT(QueuedFailureTrackedPacket::destroyed == 1);
            }
            ip->reinjectQueuedDatagram(identity); // the consumed identity must be absent
            scheduleAfter(0.2, timer);
        }
        else if (mode == "Lifecycle" && step == 3) {
            ASSERT(errors == 1);
            auto packet = new QueuedFailureTrackedPacket();
            packet->insertAtBack(makeShared<ByteCountChunk>(B(32)));
            socket.sendTo(packet, target, 42000);
            scheduleAfter(0.05, timer);
        }
        else if (mode == "Lifecycle" && step == 4) {
            ASSERT(queuedCount == 2 && queued && QueuedFailureTrackedPacket::destroyed == 1);
            lifecycleIdentity = queued;
            queued = nullptr;
            auto operation = new ModuleStopOperation();
            LifecycleOperation::StringMap parameters;
            operation->initialize(getParentModule(), parameters);
            if (initiateOperation(operation, this))
                operationDone = true;
            scheduleAfter(0.1, timer);
        }
        else {
            if (mode == "Lifecycle") {
                ASSERT(operationDone && QueuedFailureTrackedPacket::destroyed == 2);
                ip->failQueuedDatagram(lifecycleIdentity);
                ip->reinjectQueuedDatagram(lifecycleIdentity);
                ASSERT(QueuedFailureTrackedPacket::destroyed == 2);
            }
            ASSERT(errors == (mode == "SuppressedIcmp" || mode == "NextHop" ? 0 : 1));
            if (mode == "Transit" || mode == "SuppressedIcmp")
                ASSERT(generatedErrors == (mode == "Transit" ? 1 : 0));
            ASSERT(forwarded == 0);
            std::cout << "QUEUED DATAGRAM FAILURE " << mode << ": PASS\n";
        }
    }
    void socketDataArrived(UdpSocket *, Packet *packet) override { throw cRuntimeError("Unexpected data delivery"); }
    void socketErrorArrived(UdpSocket *, Indication *indication) override {
        errors++;
        const Packet *original = nullptr;
        if (using6) {
#ifdef INET_WITH_IPv6
            auto error = indication->getTag<Icmpv6ErrorInd>();
            ASSERT(error->getType() == ICMPv6_DESTINATION_UNREACHABLE && error->getCode() == NO_ROUTE_TO_DEST);
            original = error->getOriginalPacket();
#else
            throw cRuntimeError("IPv6 is disabled");
#endif
        }
        else {
            auto error = indication->getTag<Icmpv4ErrorInd>();
            ASSERT(error->getType() == ICMP_DESTINATION_UNREACHABLE && error->getCode() == ICMP_DU_HOST_UNREACHABLE);
            original = error->getOriginalPacket();
        }
        auto addresses = original->getTag<L3AddressInd>();
        auto ports = original->getTag<L4PortInd>();
        ASSERT(addresses->getDestAddress() == target);
        ASSERT(ports->getSrcPort() == 41000 && ports->getDestPort() == 42000);
        delete indication;
    }
    void socketClosed(UdpSocket *) override {}
  public:
    void invoke() override { Enter_Method("queuedFailureStopCompleted"); operationDone = true; }
    ~QueuedDatagramFailurePeer() override { cancelAndDelete(timer); }
};
Define_Module(QueuedDatagramFailurePeer);

#endif

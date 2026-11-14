// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __ARP_FAILURE_UNREACHABLE_PEER_H
#define __ARP_FAILURE_UNREACHABLE_PEER_H

#include "inet/common/INETDefs.h"
#include "inet/common/InitStages.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/common/lifecycle/LifecycleController.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/networklayer/arp/ipv4/ArpPacket_m.h"
#include "inet/networklayer/common/Icmpv4ErrorTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/INetfilter.h"
#include "inet/networklayer/ipv4/Ipv4.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv4/IcmpHeader_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

using namespace inet;

class ArpFailureTrackedPacket : public Packet
{
  public:
    inline static int created = 0;
    inline static int destroyed = 0;
    explicit ArpFailureTrackedPacket(const char *name) : Packet(name) { created++; }
    explicit ArpFailureTrackedPacket(const Packet& packet) : Packet(packet) { created++; }
    ~ArpFailureTrackedPacket() override { destroyed++; }
    Packet *dup() const override { return new Packet(*this); }
};

// Observe the router-owned raw packet lifetime without replacing any IP decision.
class ArpFailureIpv4 : public Ipv4
{
  protected:
    void handleIncomingDatagram(Packet *packet) override {
        if (!strcmp(packet->getName(), "arp-failure-original")) {
            auto tracked = new ArpFailureTrackedPacket(*packet);
            delete packet;
            packet = tracked;
        }
        Ipv4::handleIncomingDatagram(packet);
    }
};
Define_Module(ArpFailureIpv4);

class ArpFailureUnreachablePeer : public cSimpleModule, public NetfilterBase::HookBase, public UdpSocket::ICallback, public cListener, public LifecycleController, public IDoneCallback
{
    UdpSocket socket;
    INetfilter *ip = nullptr;
    cModule *owner = nullptr;
    cMessage *timer = nullptr;
    std::string mode;
    Ipv4Address target;
    int step = 0;
    int requests = 0;
    int drops = 0;
    int generatedErrors = 0;
    int errors = 0;
    bool operationDone = false;
    simtime_t firstRequest;
  protected:
    int numInitStages() const override { return NUM_INIT_STAGES; }
    void initialize(int stage) override {
        if (stage != INITSTAGE_APPLICATION_LAYER)
            return;
        mode = par("scenario").stdstringValue();
        owner = mode == "Local" ? getParentModule() : getModuleByPath("^.^.router");
        auto layer = owner->getSubmodule("ipv4");
        ip = check_and_cast<INetfilter *>(layer->getSubmodule("ip"));
        ip->registerHook(0, this);
        layer->getSubmodule("arp")->subscribe("arpRequestSent", this);
        layer->getSubmodule("ip")->subscribe(packetDroppedSignal, this);
        target = Ipv4Address(mode == "Local" ? "10.0.0.99" : "10.0.1.99");
        socket.setOutputGate(gate("socketOut"));
        socket.setCallback(this);
        socket.bind(31000);
        timer = new cMessage("arp-failure-observation");
        scheduleAt(0.1, timer);
    }
    Result datagramPreRoutingHook(Packet *) override { return ACCEPT; }
    Result datagramForwardHook(Packet *) override { return ACCEPT; }
    Result datagramPostRoutingHook(Packet *) override { return ACCEPT; }
    Result datagramLocalInHook(Packet *) override { return ACCEPT; }
    Result datagramLocalOutHook(Packet *packet) override {
        auto header = packet->peekAtFront<Ipv4Header>();
        if (header->getProtocolId() == IP_PROT_ICMP &&
                packet->peekDataAt<IcmpHeader>(header->getChunkLength())->getType() == ICMP_DESTINATION_UNREACHABLE)
            generatedErrors++;
        return ACCEPT;
    }
    void receiveSignal(cComponent *, simsignal_t signal, cObject *value, cObject *) override {
        auto packet = dynamic_cast<Packet *>(value);
        if (!packet)
            return;
        if (signal == cComponent::registerSignal("arpRequestSent")) {
            auto arp = packet->peekAtFront<ArpPacket>();
            if (arp->getOpcode() == ARP_REQUEST && arp->getDestIpAddress() == target) {
                if (!requests)
                    firstRequest = simTime();
                requests++;
            }
        }
        else if (signal == packetDroppedSignal && !strcmp(packet->getName(), "arp-failure-original"))
            drops++;
    }
    void handleMessage(cMessage *message) override {
        if (message != timer) {
            socket.processMessage(message);
            return;
        }
        if (step++ == 0) {
            int count = mode == "MultiplePackets" ? 3 : 1;
            for (int i = 0; i < count; ++i) {
                Packet *packet = mode == "Local" ? new ArpFailureTrackedPacket("arp-failure-original") : new Packet("arp-failure-original");
                if (mode == "SuppressedError") {
                    auto header = makeShared<IcmpHeader>();
                    header->setType(ICMP_DESTINATION_UNREACHABLE);
                    header->setCode(ICMP_DU_PORT_UNREACHABLE);
                    packet->insertAtFront(header);
                    packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::icmpv4);
                    packet->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);
                    auto addresses = packet->addTag<L3AddressReq>();
                    addresses->setSrcAddress(Ipv4Address("10.0.0.1"));
                    addresses->setDestAddress(target);
                    send(packet, "socketOut");
                }
                else {
                    packet->insertAtBack(makeShared<ByteCountChunk>(B(32)));
                    socket.sendTo(packet, target, 32000);
                }
            }
            scheduleAfter(0.05, timer);
        }
        else if (step == 2) {
            int count = mode == "MultiplePackets" ? 3 : 1;
            ASSERT(requests == 1 && errors == 0 && drops == 0);
            ASSERT(ArpFailureTrackedPacket::created == count && ArpFailureTrackedPacket::destroyed == 0);
            if (mode == "StopPending") {
                auto operation = new ModuleStopOperation();
                LifecycleOperation::StringMap parameters;
                operation->initialize(owner, parameters);
                if (initiateOperation(operation, this))
                    operationDone = true;
            }
            scheduleAt(0.75, timer);
        }
        else {
            int count = mode == "MultiplePackets" ? 3 : 1;
            bool reports = mode == "Transit" || mode == "MultiplePackets";
            ASSERT(requests == (mode == "StopPending" ? 1 : 3));
            ASSERT(errors == (reports ? count : 0));
            ASSERT(generatedErrors == (reports ? count : 0));
            ASSERT(drops == (mode == "StopPending" ? 0 : count));
            ASSERT(ArpFailureTrackedPacket::created == count && ArpFailureTrackedPacket::destroyed == count);
            if (mode == "StopPending")
                ASSERT(operationDone);
            std::cout << "ARP FAILURE UNREACHABLE " << mode << ": PASS\n";
        }
    }
    void socketDataArrived(UdpSocket *, Packet *) override { throw cRuntimeError("Absent member delivered data"); }
    void socketErrorArrived(UdpSocket *, Indication *indication) override {
        errors++;
        ASSERT(requests == 3 && simTime() >= firstRequest + SimTime(0.3));
        auto error = indication->getTag<Icmpv4ErrorInd>();
        ASSERT(error->getType() == ICMP_DESTINATION_UNREACHABLE && error->getCode() == ICMP_DU_HOST_UNREACHABLE);
        auto original = error->getOriginalPacket();
        auto addresses = original->getTag<L3AddressInd>();
        auto ports = original->getTag<L4PortInd>();
        ASSERT(addresses->getDestAddress() == target && ports->getSrcPort() == 31000 && ports->getDestPort() == 32000);
        delete indication;
    }
    void socketClosed(UdpSocket *) override {}
  public:
    void invoke() override { Enter_Method("arpFailureStopCompleted"); operationDone = true; }
    ~ArpFailureUnreachablePeer() override { cancelAndDelete(timer); }
};
Define_Module(ArpFailureUnreachablePeer);

#endif

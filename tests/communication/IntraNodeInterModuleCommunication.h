//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INTRANODEINTERMODULECOMMUNICATION_H
#define __INET_INTRANODEINTERMODULECOMMUNICATION_H

#include "inet/common/FunctionalEvent.h"
#include "inet/common/ModuleRefByGate.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/recorder/PcapWriter.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/SimulationContinuation.h"
#include "inet/common/SimulationTask.h"
#include "inet/linklayer/common/InterfaceTag_m.h"

namespace inet {
namespace communication {

using namespace std;

static void runSimulationTaskExample() {
    std::cout << "Parent started" << std::endl;
    int count = 3;
    int time = 100;
    SimulationTask parentTask;
    for (int i = 0; i < count; i++) {
        parentTask.spawnChild([=] () {
            std::cout << "Child " << i << " started" << std::endl;
            sleepSimulationTime(rand() % time);
            std::cout << "Child " << i << " finished" << std::endl;
        });
    }
    parentTask.joinChildren();
    std::cout << "Parent finished" << std::endl;
}

class IPassivePacketSink
{
  public:
    virtual void handlePushPacket(Packet *packet, cGate *gate) = 0;
};

class IActivePacketSink
{
  public:
    virtual void handlePullPacketChanged(cGate *gate) = 0;
};

class IPassivePacketSource
{
  public:
    virtual Packet *handlePullPacket(cGate *gate) = 0;
};

class ITcp
{
  public:
    virtual int createSocket() = 0;
    virtual int connect(int socketId, string address, int port) = 0;
    virtual void close(int socketId) = 0;
};

class TcpSocket
{
  protected:
    int id;
    ModuleRefByGate<ITcp> tcp;
    ModuleRefByGate<IPassivePacketSink> sink;

  public:
    void initialize(cGate *gate) {
        tcp.reference(gate, true);
        DispatchProtocolReq dispatchProtocolReq;
        dispatchProtocolReq.setProtocol(&Protocol::tcp);
        dispatchProtocolReq.setServicePrimitive(SP_REQUEST);
        sink.reference(gate, true, &dispatchProtocolReq); // lookup TCP protocol specific passive packet sink
    }

    void create() {
        EV << "TcpSocket::create" << endl;
        id = tcp->createSocket();
    }

    int connect(string address, int port) {
        EV << "TcpSocket::connect" << endl;
        return tcp->connect(id, address, port);
    }

    void send(Packet *packet) {
        EV << "TcpSocket::send" << endl;
        packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::tcp);
        sink->handlePushPacket(packet, sink.getReferencedGate());
    }

    void close() {
        EV << "TcpSocket::close" << endl;
        tcp->close(id);
    }
};

class TcpApp : public cSimpleModule
{
  protected:
    TcpSocket socket;

  protected:
    virtual void initialize() override {
        EV << "TcpApp::initialize" << endl;
        socket.initialize(gate("socketOut"));
        schedule("communicate", 0, [=] () { communicate(); });
    }

    void communicate() {
        EV << "TcpApp::communicate" << endl;
        socket.create();
        if (socket.connect("10.0.0.1", 42) != 0)
            throw cRuntimeError("Could not open TCP connection");
        EV << "connection created" << endl;
        socket.send(new Packet("packet", makeShared<ByteCountChunk>(B(42))));
        sleepSimulationTime(10);
        socket.close();
    }
};

class Tcp : public cSimpleModule, public virtual ITcp, public virtual IPassivePacketSink, public virtual IModuleInterfaceLookup
{
  protected:
    ModuleRefByGate<IPassivePacketSink> lowerLayerSink;

  protected:
    virtual void initialize() override {
        EV << "Tcp::initialize" << endl;
        DispatchProtocolReq dispatchProtocolReq;
        dispatchProtocolReq.setProtocol(&Protocol::ipv4);
        dispatchProtocolReq.setServicePrimitive(SP_REQUEST);
        lowerLayerSink.reference(gate("lowerLayerOut"), true, &dispatchProtocolReq); // lookup IPv4 protocol specific passive packet lowerLayerSink
    }

    virtual void pushPacket(Packet *packet, cGate *outputGate) {
        EV << "Tcp::pushPacket" << endl;
        ASSERT(outputGate == lowerLayerSink.getReferencingGate());
        packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);
        lowerLayerSink->handlePushPacket(packet, lowerLayerSink.getReferencedGate());
    }

  public:
    virtual int createSocket() override {
        Enter_Method("handleCreateSocket");
        EV << "Tcp::handleCreateSocket" << endl;
        return 0;
    }

    virtual int connect(int socketId, string address, int port) override {
        Enter_Method("connect");
        EV << "Tcp::connect" << endl;
        auto continuation = new SimulationContinuation();
        schedule("ResumeEvent", simTime() + 7, [&] () { continuation->resume(); });
        continuation->suspend();
        return 0;
    }

    virtual void handlePushPacket(Packet *packet, cGate *inputGate) override {
        Enter_Method("handlePushPacket");
        EV << "Tcp::handlePushPacket" << endl;
        ASSERT(inputGate->isName("upperLayerIn"));
        take(packet);
        pushPacket(packet, gate("lowerLayerOut"));
    }

    virtual void close(int socketId) override {
        Enter_Method("close");
        EV << "Tcp::close" << endl;
    }

    virtual cGate *lookupModuleInterface(cGate *gate, const type_info& type, const cObject *arguments, int direction) override {
        if (gate->isName("upperLayerIn")) {
            if (type == typeid(ITcp)) // handle socket control operations
                return gate;
            else if (type == typeid(IPassivePacketSink)) { // handle TCP SDUs
                auto dispatchProtocolReq = dynamic_cast<const DispatchProtocolReq *>(arguments);
                if (dispatchProtocolReq == nullptr || dispatchProtocolReq->getProtocol() == &Protocol::tcp)
                    return gate;
            }
        }
        return nullptr;
    }
};

class TcpProcessingDelay : public cSimpleModule, public virtual ITcp, public virtual IPassivePacketSink, public virtual IModuleInterfaceLookup
{
  protected:
    ModuleRefByGate<ITcp> tcp;
    ModuleRefByGate<IPassivePacketSink> sink;

    simtime_t time;

  protected:
    virtual void initialize() override {
        EV << "TcpSocketDelay::initialize" << endl;
        tcp.reference(gate("out"), true);
        DispatchProtocolReq dispatchProtocolReq;
        dispatchProtocolReq.setProtocol(&Protocol::tcp);
        dispatchProtocolReq.setServicePrimitive(SP_REQUEST);
        sink.reference(gate("out"), true, &dispatchProtocolReq); // lookup TCP protocol specific passive packet lowerLayerSink
    }

    virtual void pushPacket(Packet *packet, cGate *outputGate) {
        EV << "TcpSocketDelay::pushPacket" << endl;
        ASSERT(outputGate == sink.getReferencingGate());
        sink->handlePushPacket(packet, sink.getReferencedGate());
    }

  public:
    virtual int createSocket() override {
        Enter_Method("handleCreateSocket");
        EV << "TcpSocketDelay::handleCreateSocket" << endl;
        // TODO: check if any operation is in progress and terminate simulation if so
        return tcp->createSocket();
    }

    virtual int connect(int socketId, string address, int port) override {
        Enter_Method("connect");
        EV << "TcpSocketDelay::connect" << endl;
        auto continuation = new SimulationContinuation();
        time = std::max(time, simTime()) + 1;
        schedule("ResumeEvent", time, [&] () { continuation->resume(); });
        continuation->suspend();
        return tcp->connect(socketId, address, port);
    }

    virtual void handlePushPacket(Packet *packet, cGate *inputGate) override {
        Enter_Method("handlePushPacket");
        EV << "TcpSocketDelay::handlePushPacket" << endl;
        ASSERT(inputGate->isName("in"));
        take(packet);
        time = std::max(time, simTime()) + packet->getByteLength();
        schedule("PushPacketEvent", time, [=] () { pushPacket(packet, this->gate("out")); });
    }

    virtual void close(int socketId) override {
        Enter_Method("close");
        EV << "TcpSocketDelay::close" << endl;
        time = std::max(time, simTime()) + 1;
        schedule("CloseEvent", time, [=] () { tcp->close(socketId); });
    }

    virtual cGate *lookupModuleInterface(cGate *gate, const type_info& type, const cObject *arguments, int direction) override {
        if (gate->isName("in")) {
            if (type == typeid(ITcp)) // handle TCP socket control operations
                return gate;
            else if (type == typeid(IPassivePacketSink)) { // handle TCP SDUs
                auto dispatchProtocolReq = dynamic_cast<const DispatchProtocolReq *>(arguments);
                if (dispatchProtocolReq == nullptr || dispatchProtocolReq->getProtocol() == &Protocol::tcp)
                    return gate;
            }
        }
        return nullptr;
    }
};

class Dispatcher : public cSimpleModule, public virtual IPassivePacketSink, public virtual IModuleInterfaceLookup
{
  protected:
    cGate *forwardLookupModuleInterface(cGate *gate, const type_info& type, const cObject *arguments, int direction) {
        cGate *result = nullptr;
        int size = gateSize(gate->getType() == cGate::INPUT ? "out" : "in");
        for (int i = 0; i < size; i++) {
            if (i != gate->getIndex()) {
                cGate *referencingGate = this->gate(gate->getType() == cGate::INPUT ? "out" : "in", i);
                cGate *referencedGate = findModuleInterface(referencingGate, type, arguments);
                if (referencedGate != nullptr) {
                    auto referencedModule = referencedGate->getOwnerModule();
                    if (result != nullptr) {
                        // KLUDGE: to avoid ambiguity
                        bool resultIsMessageDispatcher = dynamic_cast<Dispatcher *>(result->getOwnerModule());
                        bool referencedModuleIsMessageDispatcher = dynamic_cast<Dispatcher *>(referencedModule);
                        if (!resultIsMessageDispatcher && referencedModuleIsMessageDispatcher)
                            break;
                        else if (referencedModuleIsMessageDispatcher || (!resultIsMessageDispatcher && !referencedModuleIsMessageDispatcher))
                            throw cRuntimeError("Referenced module is ambiguous for type %s (%s, %s)", opp_typename(type), check_and_cast<cModule *>(result->getOwnerModule())->getFullPath().c_str(), check_and_cast<cModule *>(referencedModule)->getFullPath().c_str());
                    }
                    result = referencedGate;
                }
            }
        }
        return result;
    }

  public:
    virtual cGate *lookupModuleInterface(cGate *gate, const type_info& type, const cObject *arguments, int direction) override {
        if (gate->isName("in")) {
            if (type == typeid(IPassivePacketSink)) // handle all packets
                return gate;
        }
        return forwardLookupModuleInterface(gate, type, arguments, direction); // forward all other interfaces
    }

    virtual void handlePushPacket(Packet *packet, cGate *inputGate) override {
        Enter_Method("handlePushPacket");
        EV << "Dispatcher::handlePushPacket" << endl;
        ASSERT(inputGate->isName("in"));
        take(packet);
        const auto& dispatchProtocolReq = packet->findTag<DispatchProtocolReq>();
        const auto& interfaceReq = packet->findTag<InterfaceReq>();
        cGate *referencedGate;
        if (dispatchProtocolReq != nullptr)
            referencedGate = forwardLookupModuleInterface(inputGate, typeid(IPassivePacketSink), dispatchProtocolReq.get(), 0);
        else if (interfaceReq != nullptr)
            referencedGate = forwardLookupModuleInterface(inputGate, typeid(IPassivePacketSink), interfaceReq.get(), 0);
        else
            throw cRuntimeError("Dispatch information not found");
        auto passivePacketSink = check_and_cast<IPassivePacketSink *>(referencedGate->getOwnerModule());
        passivePacketSink->handlePushPacket(packet, referencedGate);
    }
};

class PcapRecorder : public cSimpleModule, public virtual IPassivePacketSink, public virtual IModuleInterfaceLookup
{
  protected:
    ModuleRefByGate<IPassivePacketSink> sink;

    PcapWriter pcapWriter;

  protected:
    virtual void initialize() override {
        EV << "PcapRecorder::initialize" << endl;
        sink.reference(gate("out"), true);
        pcapWriter.open("results/out.pcap", 65536, 6); // lookup any passive packet lowerLayerSink
    }

    virtual void pushPacket(Packet *packet, cGate *outputGate) {
        EV << "PcapRecorder::pushPacket" << endl;
        ASSERT(outputGate == sink.getReferencingGate());
        sink->handlePushPacket(packet, sink.getReferencedGate());
    }

  public:
    virtual void handlePushPacket(Packet *packet, cGate *inputGate) override {
        Enter_Method("handlePushPacket");
        EV << "PcapRecorder::handlePushPacket" << endl;
        ASSERT(inputGate->isName("in"));
        take(packet);
        pcapWriter.writePacket(simTime(), packet, DIRECTION_OUTBOUND, nullptr, LINKTYPE_ETHERNET);
        pushPacket(packet, gate("out"));
    }

    virtual cGate *lookupModuleInterface(cGate *gate, const type_info& type, const cObject *arguments, int direction) override {
        if (gate->isName("in")) {
            if (type == typeid(IPassivePacketSink)) // handle all packets
                return gate;
        }
        return findModuleInterface(this->gate("out"), type, arguments); // forward all other interfaces
    }
};

class Ipv4 : public cSimpleModule, public virtual IPassivePacketSink, public virtual IModuleInterfaceLookup
{
  protected:
    ModuleRefByGate<IPassivePacketSink> lowerLayerSink;

  protected:
    virtual void pushPacket(Packet *packet, cGate *outputGate) {
        EV << "Ipv4::pushPacket" << endl;
        ASSERT(outputGate == lowerLayerSink.getReferencingGate());
        lowerLayerSink->handlePushPacket(packet, lowerLayerSink.getReferencedGate());
    }

  protected:
    virtual void initialize() override {
        EV << "Ipv4::initialize" << endl;
        DispatchProtocolReq dispatchProtocolReq;
        dispatchProtocolReq.setProtocol(&Protocol::ethernetMac);
        dispatchProtocolReq.setServicePrimitive(SP_REQUEST);
        lowerLayerSink.reference(gate("lowerLayerOut"), true, &dispatchProtocolReq); // lookup Ethernet MAC protocol specific passive packet lowerLayerSink
    }

  public:
    virtual void handlePushPacket(Packet *packet, cGate *inputGate) override {
        Enter_Method("handlePushPacket");
        EV << "Ipv4::handlePushPacket" << endl;
        ASSERT(inputGate->isName("upperLayerIn"));
        take(packet);
        packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(getModuleByPath("^.eth0")->getId());
        packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ethernetMac);
        pushPacket(packet, gate("lowerLayerOut"));
    }

    virtual cGate *lookupModuleInterface(cGate *gate, const type_info& type, const cObject *arguments, int direction) override {
        if (gate->isName("upperLayerIn")) {
            if (type == typeid(IPassivePacketSink)) { // handle IPv4 SDUs
                auto dispatchProtocolReq = dynamic_cast<const DispatchProtocolReq *>(arguments);
                if (dispatchProtocolReq == nullptr || dispatchProtocolReq->getProtocol() == &Protocol::ipv4)
                    return gate;
            }
        }
        return nullptr;
    }
};

class Ethernet : public cSimpleModule, public virtual IPassivePacketSink, public virtual IModuleInterfaceLookup
{
  protected:
    ModuleRefByGate<IPassivePacketSink> lowerLayerSink;

  protected:
    virtual void initialize() override {
        EV << "Ethernet::initialize" << endl;
        lowerLayerSink.reference(gate("lowerLayerOut"), true); // lookup any passive packet lowerLayerSink
    }

    virtual void pushPacket(Packet *packet, cGate *outputGate) {
        EV << "Ethernet::pushPacket" << endl;
        ASSERT(outputGate == lowerLayerSink.getReferencingGate());
        packet->removeTagIfPresent<DispatchProtocolReq>();
        lowerLayerSink->handlePushPacket(packet, lowerLayerSink.getReferencedGate());
    }

  public:
    virtual void handlePushPacket(Packet *packet, cGate *inputGate) override {
        Enter_Method("handlePushPacket");
        EV << "Ethernet::handlePushPacket" << endl;
        ASSERT(inputGate->isName("upperLayerIn"));
        take(packet);
        pushPacket(packet, gate("lowerLayerOut"));
    }

    virtual cGate *lookupModuleInterface(cGate *gate, const type_info& type, const cObject *arguments, int direction) override {
        if (gate->isName("upperLayerIn")) {
            if (type == typeid(IPassivePacketSink)) { // handle Ethernet MAC SDUs
                auto dispatchProtocolReq = dynamic_cast<const DispatchProtocolReq *>(arguments);
                if (dispatchProtocolReq == nullptr || dispatchProtocolReq->getProtocol() == &Protocol::ethernetMac)
                    return gate;
            }
        }
        return nullptr;
    }
};

class PacketQueue : public cSimpleModule, public virtual IPassivePacketSink, public virtual IPassivePacketSource, public virtual IModuleInterfaceLookup
{
  protected:
    ModuleRefByGate<IActivePacketSink> sink;
    cPacketQueue queue;

  protected:
    virtual void initialize() override {
        EV << "PacketQueue::initialize" << endl;
        sink.reference(gate("out"), true); // lookup any active packet lowerLayerSink
    }

  public:
    virtual void handlePushPacket(Packet *packet, cGate *inputGate) override {
        Enter_Method("handlePushPacket");
        EV << "PacketQueue::handlePushPacket" << endl;
        ASSERT(inputGate->isName("in"));
        take(packet);
        queue.insert(packet);
        sink->handlePullPacketChanged(sink.getReferencedGate());
    }

    virtual Packet *handlePullPacket(cGate *gate) override {
        Enter_Method("handlePullPacket");
        EV << "PacketQueue::handlePullPacket" << endl;
        return check_and_cast<Packet *>(queue.pop());
    }

    virtual cGate *lookupModuleInterface(cGate *gate, const type_info& type, const cObject *arguments, int direction) override {
        if (gate->isName("in")) {
            if (type == typeid(IPassivePacketSink)) // handle all packets
                return gate;
        }
        if (gate->isName("out")) {
            if (type == typeid(IPassivePacketSource)) // handle all packets
                return gate;
        }
        return nullptr;
    }
};

class PacketSink : public cSimpleModule, public virtual IPassivePacketSink, public virtual IModuleInterfaceLookup
{
  public:
    virtual void handlePushPacket(Packet *packet, cGate *inputGate) override {
        Enter_Method("handlePushPacket");
        EV << "PacketSink::handlePushPacket" << endl;
        ASSERT(inputGate->isName("in"));
        take(packet);
        delete packet;
    }

    virtual cGate *lookupModuleInterface(cGate *gate, const type_info& type, const cObject *arguments, int direction) override {
        if (gate->isName("in")) {
            if (type == typeid(IPassivePacketSink)) // handle all packets
                return gate;
        }
        return nullptr;
    }
};

class EthernetMac : public cSimpleModule, public virtual IActivePacketSink, public virtual IModuleInterfaceLookup
{
  protected:
    ModuleRefByGate<IPassivePacketSource> upperLayerSource;
    ModuleRefByGate<IPassivePacketSink> lowerLayerSink;

  protected:
    virtual void initialize() override {
        EV << "EthernetMac::initialize" << endl;
        upperLayerSource.reference(gate("upperLayerIn"), true); // lookup any passive packet source
        lowerLayerSink.reference(gate("lowerLayerOut"), true); // lookup any passive packet lowerLayerSink
    }

    virtual Packet *pullPacket(cGate *inputGate) {
        EV << "EthernetMac::pullPacket" << endl;
        ASSERT(inputGate == upperLayerSource.getReferencingGate());
        auto packet = upperLayerSource->handlePullPacket(upperLayerSource.getReferencedGate());
        take(packet);
        return packet;
    }

    virtual void pushPacket(Packet *packet, cGate *gate) {
        EV << "EthernetMac::pushPacket" << endl;
        ASSERT(gate == lowerLayerSink.getReferencingGate());
        lowerLayerSink->handlePushPacket(packet, lowerLayerSink.getReferencedGate());
    }

  public:
    virtual void handlePullPacketChanged(cGate *inputGate) override {
        Enter_Method("handlePullPacketChanged");
        EV << "EthernetMac::handlePullPacketChanged" << endl;
        auto packet = pullPacket(inputGate);
        pushPacket(packet, gate("lowerLayerOut"));
    }

    virtual cGate *lookupModuleInterface(cGate *gate, const type_info& type, const cObject *arguments, int direction) override {
        if (gate->isName("upperLayerIn")) {
            if (type == typeid(IActivePacketSink)) // handle all packets
                return gate;
        }
        return nullptr;
    }
};

class Udp : public cSimpleModule, public virtual IPassivePacketSink, public virtual IModuleInterfaceLookup
{
  public:
    virtual void handlePushPacket(Packet *packet, cGate *inputGate) override {
        Enter_Method("handlePushPacket");
        EV << "Udp::handlePushPacket" << endl;
        ASSERT(inputGate->isName("upperLayerIn"));
        take(packet);
        throw cRuntimeError("TODO");
    }

    virtual cGate *lookupModuleInterface(cGate *gate, const type_info& type, const cObject *arguments, int direction) override {
        if (gate->isName("upperLayerIn")) {
            if (type == typeid(IPassivePacketSink)) { // handle UDP SDUs
                auto dispatchProtocolReq = dynamic_cast<const DispatchProtocolReq *>(arguments);
                if (dispatchProtocolReq == nullptr || dispatchProtocolReq->getProtocol() == &Protocol::udp)
                    return gate;
            }
        }
        return nullptr;
    }
};

class Ipv6 : public cSimpleModule, public virtual IPassivePacketSink, public virtual IModuleInterfaceLookup
{
  public:
    virtual void handlePushPacket(Packet *packet, cGate *inputGate) override {
        Enter_Method("handlePushPacket");
        EV << "Ipv6::handlePushPacket" << endl;
        ASSERT(inputGate->isName("upperLayerIn"));
        take(packet);
        throw cRuntimeError("TODO");
    }

    virtual cGate *lookupModuleInterface(cGate *gate, const type_info& type, const cObject *arguments, int direction) override {
        if (gate->isName("upperLayerIn")) {
            if (type == typeid(IPassivePacketSink)) { // handle IPv6 SDUs
                auto dispatchProtocolReq = dynamic_cast<const DispatchProtocolReq *>(arguments);
                if (dispatchProtocolReq == nullptr || dispatchProtocolReq->getProtocol() == &Protocol::ipv6)
                    return gate;
            }
        }
        return nullptr;
    }
};

class Ppp : public cSimpleModule, public virtual IPassivePacketSink, public virtual IModuleInterfaceLookup
{
  public:
    virtual void handlePushPacket(Packet *packet, cGate *inputGate) override {
        Enter_Method("handlePushPacket");
        EV << "Ppp::handlePushPacket" << endl;
        ASSERT(inputGate->isName("upperLayerIn"));
        take(packet);
        throw cRuntimeError("TODO");
    }

    virtual cGate *lookupModuleInterface(cGate *gate, const type_info& type, const cObject *arguments, int direction) override {
        if (gate->isName("upperLayerIn")) {
            if (type == typeid(IPassivePacketSink)) { // handle PPP SDUs
                auto dispatchProtocolReq = dynamic_cast<const DispatchProtocolReq *>(arguments);
                if (dispatchProtocolReq == nullptr || dispatchProtocolReq->getProtocol() == &Protocol::ppp)
                    return gate;
            }
        }
        return nullptr;
    }
};

class NetworkInterface : public cModule, public virtual IModuleInterfaceLookup
{
  public:
    virtual cGate *lookupModuleInterface(cGate *gate, const type_info& type, const cObject *arguments, int direction) override {
        if (gate->isName("upperLayerIn")) {
            if (type == typeid(IPassivePacketSink)) { // handle network interface specific packets
                auto interfaceReq = dynamic_cast<const InterfaceReq *>(arguments);
                if (interfaceReq == nullptr || interfaceReq->getInterfaceId() == getId())
                    return findModuleInterface(gate, type, nullptr, 1);
            }
        }
        else if (gate->isName("physicalOut"))
            return findModuleInterface(gate, type, arguments); // forward all other interfaces
        return nullptr;
    }
};

} // namespace communication
} // namespace inet

#endif


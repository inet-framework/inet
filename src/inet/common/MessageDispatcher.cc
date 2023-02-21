//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/MessageDispatcher.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"

namespace inet {

Define_Module(MessageDispatcher);

// TODO optimize gate access throughout this class
// TODO factoring out some methods could also help

void MessageDispatcher::initialize(int stage)
{
#ifdef INET_WITH_QUEUEING
    PacketProcessorBase::initialize(stage);
#endif // #ifdef INET_WITH_QUEUEING
    if (stage == INITSTAGE_LOCAL) {
        forwardServiceRegistration = par("forwardServiceRegistration");
        forwardProtocolRegistration = par("forwardProtocolRegistration");
        WATCH_MAP(socketIdToGateIndex);
        WATCH_MAP(interfaceIdToGateIndex);
        WATCH_MAP(serviceToGateIndex);
        WATCH_MAP(protocolToGateIndex);
    }
}

void MessageDispatcher::arrived(cMessage *message, cGate *inGate, const SendOptions& options, simtime_t time)
{
    Enter_Method("arrived");
    cGate *outGate = nullptr;
    if (message->isPacket()) {
        auto packet = check_and_cast<Packet *>(message);
        outGate = handlePacket(packet, inGate);
#ifdef INET_WITH_QUEUEING
        handlePacketProcessed(packet);
#endif // #ifdef INET_WITH_QUEUEING
    }
    else
        outGate = handleMessage(check_and_cast<Message *>(message), inGate);
    outGate->deliver(message, options, time);
#ifdef INET_WITH_QUEUEING
    updateDisplayString();
#endif // #ifdef INET_WITH_QUEUEING
}

#ifdef INET_WITH_QUEUEING
bool MessageDispatcher::canPushSomePacket(cGate *inGate) const
{
    int size = gateSize("out");
    for (int i = 0; i < size; i++) {
        auto outGate = const_cast<MessageDispatcher *>(this)->gate("out", i);
        auto consumer = findConnectedModule<queueing::IPassivePacketSink>(outGate);
        if (consumer != nullptr && !dynamic_cast<MessageDispatcher *>(consumer) && !consumer->canPushSomePacket(outGate->getPathEndGate()))
            return false;
    }
    return true;
}

bool MessageDispatcher::canPushPacket(Packet *packet, cGate *inGate) const
{
    auto outGate = const_cast<MessageDispatcher *>(this)->handlePacket(packet, inGate);
    auto consumer = findConnectedModule<queueing::IPassivePacketSink>(outGate);
    return consumer != nullptr && !dynamic_cast<MessageDispatcher *>(consumer) && consumer->canPushPacket(packet, outGate->getPathEndGate());
}

void MessageDispatcher::pushPacket(Packet *packet, cGate *inGate)
{
    Enter_Method("pushPacket");
    take(packet);
    auto outGate = handlePacket(packet, inGate);
    auto consumer = findConnectedModule<IPassivePacketSink>(outGate);
    handlePacketProcessed(packet);
    pushOrSendPacket(packet, outGate, consumer);
    updateDisplayString();
}

void MessageDispatcher::pushPacketStart(Packet *packet, cGate *inGate, bps datarate)
{
    Enter_Method("pushPacketStart");
    take(packet);
    auto outGate = handlePacket(packet, inGate);
    auto consumer = findConnectedModule<IPassivePacketSink>(outGate);
    pushOrSendPacketStart(packet, outGate, consumer, datarate, packet->getTransmissionId());
    updateDisplayString();
}

void MessageDispatcher::pushPacketEnd(Packet *packet, cGate *inGate)
{
    Enter_Method("pushPacketEnd");
    take(packet);
    auto outGate = handlePacket(packet, inGate);
    auto consumer = findConnectedModule<IPassivePacketSink>(outGate);
    handlePacketProcessed(packet);
    pushOrSendPacketEnd(packet, outGate, consumer, packet->getTransmissionId());
    updateDisplayString();
}

void MessageDispatcher::handleCanPushPacketChanged(cGate *outGate)
{
    int size = gateSize("in");
    for (int i = 0; i < size; i++) {
        auto inGate = gate("in", i);
        auto producer = findConnectedModule<queueing::IActivePacketSource>(inGate);
        if (producer != nullptr && !dynamic_cast<MessageDispatcher *>(producer) && outGate->getOwnerModule() != inGate->getOwnerModule())
            producer->handleCanPushPacketChanged(inGate->getPathStartGate());
    }
}

void MessageDispatcher::handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
}

#endif // #ifdef INET_WITH_QUEUEING

cGate *MessageDispatcher::handlePacket(Packet *packet, cGate *inGate)
{
    const auto& socketInd = packet->findTag<SocketInd>();
    if (socketInd != nullptr) {
        int socketId = socketInd->getSocketId();
        auto it = socketIdToGateIndex.find(socketId);
        if (it != socketIdToGateIndex.end()) {
            auto outGate = gate("out", it->second);
            EV_INFO << "Dispatching packet to socket" << EV_FIELD(socketId) << EV_FIELD(inGate) << EV_FIELD(outGate) << EV_FIELD(packet) << EV_ENDL;
            return outGate;
        }
        else
            throw cRuntimeError("handlePacket(): Unknown socket: socketId = %d, pathStartGate = %s, pathEndGate = %s", socketId, inGate->getPathStartGate()->getFullPath().c_str(), inGate->getPathEndGate()->getFullPath().c_str());
    }
    const auto& dispatchProtocolReq = packet->findTag<DispatchProtocolReq>();
    if (dispatchProtocolReq != nullptr) {
        const auto& packetProtocolTag = packet->findTag<PacketProtocolTag>();
        auto servicePrimitive = dispatchProtocolReq->getServicePrimitive();
        // KLUDGE eliminate this by adding ServicePrimitive to every DispatchProtocolReq
        if (servicePrimitive == static_cast<ServicePrimitive>(-1)) {
            if (packetProtocolTag != nullptr && dispatchProtocolReq->getProtocol() == packetProtocolTag->getProtocol())
                servicePrimitive = SP_INDICATION;
            else
                servicePrimitive = SP_REQUEST;
        }
        auto protocol = dispatchProtocolReq->getProtocol();
        if (servicePrimitive == SP_REQUEST) {
            auto it = serviceToGateIndex.find(Key(protocol->getId(), servicePrimitive));
            if (it != serviceToGateIndex.end()) {
                auto outGate = gate("out", it->second);
                EV_INFO << "Dispatching packet to service" << EV_FIELD(protocol, *protocol) << EV_FIELD(servicePrimitive) << EV_FIELD(inGate) << EV_FIELD(outGate) << EV_FIELD(packet) << EV_ENDL;
                return outGate;
            }
            else {
                auto it = serviceToGateIndex.find(Key(-1, servicePrimitive));
                if (it != serviceToGateIndex.end()) {
                    auto outGate = gate("out", it->second);
                    EV_INFO << "Dispatching packet to any service" << EV_FIELD(servicePrimitive) << EV_FIELD(inGate) << EV_FIELD(outGate) << EV_FIELD(packet) << EV_ENDL;
                    return outGate;
                }
                else
                    throw cRuntimeError("handlePacket(): Unknown protocol: protocolId = %d, protocolName = %s, servicePrimitive = REQUEST, pathStartGate = %s, pathEndGate = %s", protocol->getId(), protocol->getName(), inGate->getPathStartGate()->getFullPath().c_str(), inGate->getPathEndGate()->getFullPath().c_str());
            }
        }
        else if (servicePrimitive == SP_INDICATION) {
            auto it = protocolToGateIndex.find(Key(protocol->getId(), servicePrimitive));
            if (it != protocolToGateIndex.end()) {
                auto outGate = gate("out", it->second);
                EV_INFO << "Dispatching packet to protocol" << EV_FIELD(protocol, *protocol) << EV_FIELD(servicePrimitive) << EV_FIELD(inGate) << EV_FIELD(outGate) << EV_FIELD(packet) << EV_ENDL;
                return outGate;
            }
            else {
                auto it = protocolToGateIndex.find(Key(-1, servicePrimitive));
                if (it != protocolToGateIndex.end()) {
                    auto outGate = gate("out", it->second);
                    EV_INFO << "Dispatching packet to any protocol" << EV_FIELD(servicePrimitive) << EV_FIELD(inGate) << EV_FIELD(outGate) << EV_FIELD(packet) << EV_ENDL;
                    return outGate;
                }
                else
                    throw cRuntimeError("handlePacket(): Unknown protocol: protocolId = %d, protocolName = %s, servicePrimitive = INDICATION, pathStartGate = %s, pathEndGate = %s", protocol->getId(), protocol->getName(), inGate->getPathStartGate()->getFullPath().c_str(), inGate->getPathEndGate()->getFullPath().c_str());
            }
        }
        else
            throw cRuntimeError("handlePacket(): Unknown service primitive: servicePrimitive = %d, pathStartGate = %s, pathEndGate = %s", static_cast<int>(servicePrimitive), inGate->getPathStartGate()->getFullPath().c_str(), inGate->getPathEndGate()->getFullPath().c_str());
    }
    const auto& interfaceReq = packet->findTag<InterfaceReq>();
    if (interfaceReq != nullptr) {
        int interfaceId = interfaceReq->getInterfaceId();
        auto it = interfaceIdToGateIndex.find(interfaceId);
        if (it != interfaceIdToGateIndex.end()) {
            auto outGate = gate("out", it->second);
            EV_INFO << "Dispatching packet to interface" << EV_FIELD(interfaceId) << EV_FIELD(inGate) << EV_FIELD(outGate) << EV_FIELD(packet) << EV_ENDL;
            return outGate;
        }
        else
            throw cRuntimeError("handlePacket(): Unknown interface: interfaceId = %d, pathStartGate = %s, pathEndGate = %s", interfaceId, inGate->getPathStartGate()->getFullPath().c_str(), inGate->getPathEndGate()->getFullPath().c_str());
    }
    throw cRuntimeError("handlePacket(): Unknown packet: %s(%s), pathStartGate = %s, pathEndGate = %s", packet->getName(), packet->getClassName(), inGate->getPathStartGate()->getFullPath().c_str(), inGate->getPathEndGate()->getFullPath().c_str());
}

cGate *MessageDispatcher::handleMessage(Message *message, cGate *inGate)
{
    const auto& socketReq = message->findTag<SocketReq>();
    if (socketReq != nullptr) {
        int socketReqId = socketReq->getSocketId();
        auto it = socketIdToGateIndex.find(socketReqId);
        if (it == socketIdToGateIndex.end())
            socketIdToGateIndex[socketReqId] = inGate->getIndex();
        else if (it->first != socketReqId)
            throw cRuntimeError("handleMessage(): Socket is already registered: socketId = %d, current gateIndex = %d, new gateIndex = %d, pathStartGate = %s, pathEndGate = %s", socketReqId, it->second, inGate->getIndex(), inGate->getPathStartGate()->getFullPath().c_str(), inGate->getPathEndGate()->getFullPath().c_str());
    }
    const auto& socketInd = message->findTag<SocketInd>();
    if (socketInd != nullptr) {
        int socketId = socketInd->getSocketId();
        auto it = socketIdToGateIndex.find(socketId);
        if (it != socketIdToGateIndex.end()) {
            auto outGate = gate("out", it->second);
            EV_INFO << "Dispatching message to socket" << EV_FIELD(socketId) << EV_FIELD(inGate) << EV_FIELD(outGate) << EV_FIELD(message) << EV_ENDL;
            return outGate;
        }
        else
            throw cRuntimeError("handleMessage(): Unknown socket: socketId = %d, pathStartGate = %s, pathEndGate = %s", socketId, inGate->getPathStartGate()->getFullPath().c_str(), inGate->getPathEndGate()->getFullPath().c_str());
    }
    const auto& dispatchProtocolReq = message->findTag<DispatchProtocolReq>();
    if (dispatchProtocolReq != nullptr) {
        auto servicePrimitive = dispatchProtocolReq->getServicePrimitive();
        // KLUDGE eliminate this by adding ServicePrimitive to every DispatchProtocolReq
        if (servicePrimitive == static_cast<ServicePrimitive>(-1))
            servicePrimitive = SP_REQUEST;
        auto protocol = dispatchProtocolReq->getProtocol();
        if (servicePrimitive == SP_REQUEST) {
            auto it = serviceToGateIndex.find(Key(protocol->getId(), servicePrimitive));
            if (it != serviceToGateIndex.end()) {
                auto outGate = gate("out", it->second);
                EV_INFO << "Dispatching message to service" << EV_FIELD(protocol, *protocol) << EV_FIELD(servicePrimitive) << EV_FIELD(inGate) << EV_FIELD(outGate) << EV_FIELD(message) << EV_ENDL;
                return outGate;
            }
            else {
                auto it = serviceToGateIndex.find(Key(-1, servicePrimitive));
                if (it != serviceToGateIndex.end()) {
                    auto outGate = gate("out", it->second);
                    EV_INFO << "Dispatching message to any service" << EV_FIELD(servicePrimitive) << EV_FIELD(inGate) << EV_FIELD(outGate) << EV_FIELD(message) << EV_ENDL;
                    return outGate;
                }
                else
                    throw cRuntimeError("handleMessage(): Unknown protocol: protocolId = %d, protocolName = %s, servicePrimitive = REQUEST, pathStartGate = %s, pathEndGate = %s", protocol->getId(), protocol->getName(), inGate->getPathStartGate()->getFullPath().c_str(), inGate->getPathEndGate()->getFullPath().c_str());
            }
        }
        else if (servicePrimitive == SP_INDICATION) {
            auto it = protocolToGateIndex.find(Key(protocol->getId(), servicePrimitive));
            if (it != protocolToGateIndex.end()) {
                auto outGate = gate("out", it->second);
                EV_INFO << "Dispatching message to protocol" << EV_FIELD(protocol, *protocol) << EV_FIELD(servicePrimitive) << EV_FIELD(inGate) << EV_FIELD(outGate) << EV_FIELD(message) << EV_ENDL;
                return outGate;
            }
            else {
                auto it = protocolToGateIndex.find(Key(-1, servicePrimitive));
                if (it != protocolToGateIndex.end()) {
                    auto outGate = gate("out", it->second);
                    EV_INFO << "Dispatching message to any protocol" << EV_FIELD(servicePrimitive) << EV_FIELD(inGate) << EV_FIELD(outGate) << EV_FIELD(message) << EV_ENDL;
                    return outGate;
                }
                else
                    throw cRuntimeError("handleMessage(): Unknown protocol: protocolId = %d, protocolName = %s, servicePrimitive = INDICATION, pathStartGate = %s, pathEndGate = %s", protocol->getId(), protocol->getName(), inGate->getPathStartGate()->getFullPath().c_str(), inGate->getPathEndGate()->getFullPath().c_str());
            }
        }
        else
            throw cRuntimeError("handleMessage(): Unknown service primitive: servicePrimitive = %d, pathStartGate = %s, pathEndGate = %s", static_cast<int>(servicePrimitive), inGate->getPathStartGate()->getFullPath().c_str(), inGate->getPathEndGate()->getFullPath().c_str());
    }
    const auto& interfaceReq = message->findTag<InterfaceReq>();
    if (interfaceReq != nullptr) {
        int interfaceId = interfaceReq->getInterfaceId();
        auto it = interfaceIdToGateIndex.find(interfaceId);
        if (it != interfaceIdToGateIndex.end()) {
            auto outGate = gate("out", it->second);
            EV_INFO << "Dispatching message to interface" << EV_FIELD(interfaceId) << EV_FIELD(inGate) << EV_FIELD(outGate) << EV_FIELD(message) << EV_ENDL;
            return outGate;
        }
        else
            throw cRuntimeError("handleMessage(): Unknown interface: interfaceId = %d, pathStartGate = %s, pathEndGate = %s", interfaceId, inGate->getPathStartGate()->getFullPath().c_str(), inGate->getPathEndGate()->getFullPath().c_str());
    }
    throw cRuntimeError("handleMessage(): Unknown message: %s(%s), pathStartGate = %s, pathEndGate = %s", message->getName(), message->getClassName(), inGate->getPathStartGate()->getFullPath().c_str(), inGate->getPathEndGate()->getFullPath().c_str());
}

void MessageDispatcher::handleRegisterService(const Protocol& protocol, cGate *g, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterService");
    if (registeringProtocol != nullptr) {
        if (registeringProtocol != &protocol)
            throw cRuntimeError("handleRegisterService(): cannot register protocol while another registration is being done");
    }
    else {
        registeringProtocol = &protocol;
        EV_INFO << "Handling service registration from " << printRegistrationPath() << EV_FIELD(protocol) << EV_FIELD(servicePrimitive) << EV_FIELD(gate, g) << EV_ENDL;
        auto key = Key(protocol.getId(), servicePrimitive);
        auto it = serviceToGateIndex.find(key);
        if (it != serviceToGateIndex.end()) {
            if (it->second != g->getIndex())
                throw cRuntimeError("handleRegisterService(): service is already registered: protocolId = %d, protocolName = %s, servicePrimitive = %d, pathStartGate = %s, pathEndGate = %s", protocol.getId(), protocol.str().c_str(), static_cast<int>(servicePrimitive), g->getPathStartGate()->getFullPath().c_str(), g->getPathEndGate()->getFullPath().c_str());
        }
        else {
            serviceToGateIndex[key] = g->getIndex();
            if (forwardServiceRegistration) {
                auto gateName = g->getType() == cGate::INPUT ? "out" : "in";
                int size = gateSize(gateName);
                for (int i = 0; i < size; i++)
                    registerService(protocol, gate(gateName, i), servicePrimitive);
            }
        }
        registeringProtocol = nullptr;
    }
}

void MessageDispatcher::handleRegisterAnyService(cGate *g, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterAnyService");
    if (!registeringAny) {
        registeringAny = true;
        EV_INFO << "Handling any service registration from " << printRegistrationPath() << EV_FIELD(servicePrimitive) << EV_FIELD(gate, g) << EV_ENDL;
        auto key = Key(-1, servicePrimitive);
        auto it = serviceToGateIndex.find(key);
        if (it != serviceToGateIndex.end()) {
            if (it->second != g->getIndex())
                throw cRuntimeError("handleRegisterAnyService(): any service is already registered: servicePrimitive = %d, pathStartGate = %s, pathEndGate = %s", static_cast<int>(servicePrimitive), g->getPathStartGate()->getFullPath().c_str(), g->getPathEndGate()->getFullPath().c_str());
        }
        else {
            serviceToGateIndex[key] = g->getIndex();
            if (forwardServiceRegistration) {
                auto gateName = g->getType() == cGate::INPUT ? "out" : "in";
                int size = gateSize(gateName);
                for (int i = 0; i < size; i++)
                    registerAnyService(gate(gateName, i), servicePrimitive);
            }
        }
        registeringAny = false;
    }
}

void MessageDispatcher::handleRegisterProtocol(const Protocol& protocol, cGate *g, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterProtocol");
    if (registeringProtocol != nullptr) {
        if (registeringProtocol != &protocol)
            throw cRuntimeError("handleRegisterProtocol(): cannot register protocol while another registration is being done");
    }
    else {
        registeringProtocol = &protocol;
        EV_INFO << "Handling protocol registration from " << printRegistrationPath() << EV_FIELD(protocol) << EV_FIELD(servicePrimitive) << EV_FIELD(gate, g) << EV_ENDL;
        auto key = Key(protocol.getId(), servicePrimitive);
        auto it = protocolToGateIndex.find(key);
        if (it != protocolToGateIndex.end()) {
            if (it->second != g->getIndex())
                throw cRuntimeError("handleRegisterProtocol(): protocol is already registered: protocolId = %d, protocolName = %s, servicePrimitive = %d, pathStartGate = %s, pathEndGate = %s", protocol.getId(), protocol.str().c_str(), static_cast<int>(servicePrimitive), g->getPathStartGate()->getFullPath().c_str(), g->getPathEndGate()->getFullPath().c_str());
        }
        else {
            protocolToGateIndex[key] = g->getIndex();
            if (forwardProtocolRegistration) {
                auto gateName = g->getType() == cGate::INPUT ? "out" : "in";
                int size = gateSize(gateName);
                for (int i = 0; i < size; i++)
                    registerProtocol(protocol, gate(gateName, i), servicePrimitive);
            }
        }
        registeringProtocol = nullptr;
    }
}

void MessageDispatcher::handleRegisterAnyProtocol(cGate *g, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterAnyProtocol");
    if (!registeringAny) {
        registeringAny = true;
        EV_INFO << "Handling any protocol registration from " << printRegistrationPath() << EV_FIELD(servicePrimitive) << EV_FIELD(gate, g) << EV_ENDL;
        auto key = Key(-1, servicePrimitive);
        auto it = protocolToGateIndex.find(key);
        if (it != protocolToGateIndex.end()) {
            if (it->second != g->getIndex())
                throw cRuntimeError("handleRegisterAnyProtocol(): any protocol is already registered: servicePrimitive = %d, pathStartGate = %s, pathEndGate = %s", static_cast<int>(servicePrimitive), g->getPathStartGate()->getFullPath().c_str(), g->getPathEndGate()->getFullPath().c_str());
        }
        else {
            protocolToGateIndex[key] = g->getIndex();
            if (forwardProtocolRegistration) {
                auto gateName = g->getType() == cGate::INPUT ? "out" : "in";
                int size = gateSize(gateName);
                for (int i = 0; i < size; i++)
                    registerAnyProtocol(gate(gateName, i), servicePrimitive);
            }
        }
        registeringAny = false;
    }
}

void MessageDispatcher::handleRegisterInterface(const NetworkInterface& interface, cGate *out, cGate *in)
{
    Enter_Method("handleRegisterInterface");
    EV_INFO << "Handling interface registration" << EV_FIELD(interface) << EV_FIELD(out, out) << EV_FIELD(in) << EV_ENDL;
    auto it = interfaceIdToGateIndex.find(interface.getInterfaceId());
    if (it != interfaceIdToGateIndex.end()) {
        if (it->second != out->getIndex())
            throw cRuntimeError("handleRegisterInterface(): interface is already registered: interfaceId = %d, interfaceName = %s, pathStartGate = %s, pathEndGate = %s", interface.getInterfaceId(), interface.str().c_str(), out->getPathStartGate()->getFullPath().c_str(), out->getPathEndGate()->getFullPath().c_str());
    }
    else {
        interfaceIdToGateIndex[interface.getInterfaceId()] = out->getIndex();
        int size = gateSize("out");
        for (int i = 0; i < size; i++)
            if (i != in->getIndex())
                registerInterface(interface, gate("in", i), gate("out", i));
    }
}

} // namespace inet


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
#include "inet/queueing/common/PassivePacketSinkRef.h"
#include "inet/queueing/base/AnimatePacket.h"

namespace inet {

Define_Module(MessageDispatcher);

// TODO optimize gate access throughout this class
// TODO factoring out some methods could also help

#ifdef INET_WITH_QUEUEING
bool MessageDispatcher::canPushSomePacket(const cGate *inGate) const
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

bool MessageDispatcher::canPushPacket(Packet *packet, const cGate *inGate) const
{
    auto outGate = const_cast<MessageDispatcher *>(this)->handlePacket(packet, inGate);
    auto consumer = findConnectedModule<queueing::IPassivePacketSink>(outGate);
    return consumer != nullptr && !dynamic_cast<MessageDispatcher *>(consumer) && consumer->canPushPacket(packet, outGate->getPathEndGate());
}

cGate *MessageDispatcher::handlePacket(Packet *packet, const cGate *inGate)
{
    const TagBase *tag = nullptr;
    const auto& dispatchProtocolReq = packet->findTag<DispatchProtocolReq>();
    const auto& interfaceReq = packet->findTag<InterfaceReq>();
    const auto& socketInd = packet->findTag<SocketInd>();
    // KLUDGE eliminate this by adding ServicePrimitive to every DispatchProtocolReq
    DispatchProtocolReq dispatchProtocolReqArgument;
    cGate *referencedGate;
    if (socketInd != nullptr) {
        tag = socketInd.get();
        auto it = socketIdMap.find(socketInd->getSocketId());
        if (it != socketIdMap.end())
            referencedGate = it->second;
        else {
            referencedGate = forwardLookupModuleInterface(inGate, typeid(IPassivePacketSink), socketInd.get(), 0);
            socketIdMap[socketInd->getSocketId()] = referencedGate;
        }
    }
    else if (dispatchProtocolReq != nullptr) {
        const auto& packetProtocolTag = packet->findTag<PacketProtocolTag>();
        // KLUDGE eliminate this by adding ServicePrimitive to every DispatchProtocolReq
        dispatchProtocolReqArgument = *dispatchProtocolReq;
        if (dispatchProtocolReq->getServicePrimitive() == static_cast<ServicePrimitive>(-1)) {
            if (packetProtocolTag != nullptr && dispatchProtocolReq->getProtocol() == packetProtocolTag->getProtocol())
                dispatchProtocolReqArgument.setServicePrimitive(SP_INDICATION);
            else
                dispatchProtocolReqArgument.setServicePrimitive(SP_REQUEST);
        }
        tag = &dispatchProtocolReqArgument;
        auto key = Key(dispatchProtocolReq->getProtocol()->getId(), dispatchProtocolReqArgument.getServicePrimitive());
        auto it = protocolIdMap.find(key);
        if (it != protocolIdMap.end())
            referencedGate = it->second;
        else {
            referencedGate = forwardLookupModuleInterface(inGate, typeid(IPassivePacketSink), &dispatchProtocolReqArgument, 0);
            protocolIdMap[key] = referencedGate;
        }
    }
    else if (interfaceReq != nullptr) {
        tag = interfaceReq.get();
        auto it = interfaceIdMap.find(interfaceReq->getInterfaceId());
        if (it != interfaceIdMap.end())
            referencedGate = it->second;
        else {
            referencedGate = forwardLookupModuleInterface(inGate, typeid(IPassivePacketSink), interfaceReq.get(), 0);
            interfaceIdMap[interfaceReq->getInterfaceId()] = referencedGate;
        }
    }
    else
        throw cRuntimeError("Cannot forward packet because no dispatch information was found, packet = %s, gate = %s, hint = %s",
                            printToStringIfPossible(packet, 0).c_str(),
                            printToStringIfPossible(inGate, 0).c_str(),
                            "the packet should have a DispatchProtocolReq, an InterfaceReq, or a SocketInd packet tag attached");
    if (referencedGate == nullptr)
        throw cRuntimeError("Cannot forward packet because no IPassivePacketSink module interface was found, packet = %s, gate = %s, tag = %s, hint = %s",
                            printToStringIfPossible(packet, 0).c_str(),
                            printToStringIfPossible(inGate, 0).c_str(),
                            printToStringIfPossible(tag, 0).c_str(),
                            "add @interface properties on one of the connected gates or implement the IModuleInterfaceLookup C++ interface with one of the connected modules");
    return referencedGate;
}

void MessageDispatcher::pushPacket(Packet *packet, const cGate *inGate)
{
    Enter_Method("pushPacket");
    ASSERT(inGate->isName("in"));
    take(packet);
    auto referencedGate = handlePacket(packet, inGate);
    auto passivePacketSink = check_and_cast<IPassivePacketSink *>(referencedGate->getOwnerModule());
    queueing::animatePushPacket(packet, referencedGate->getPathStartGate(), referencedGate);
    passivePacketSink->pushPacket(packet, referencedGate);
    updateDisplayString();
}

void MessageDispatcher::pushPacketStart(Packet *packet, const cGate *inGate, bps datarate)
{
    Enter_Method("pushPacketStart");
    take(packet);
    auto outGate = handlePacket(packet, inGate);
    queueing::PassivePacketSinkRef consumer;
    consumer.reference(outGate, false);
    pushOrSendPacketStart(packet, outGate, consumer, datarate, packet->getTransmissionId());
    updateDisplayString();
}

void MessageDispatcher::pushPacketEnd(Packet *packet, const cGate *inGate)
{
    Enter_Method("pushPacketEnd");
    take(packet);
    auto outGate = handlePacket(packet, inGate);
    queueing::PassivePacketSinkRef consumer;
    consumer.reference(outGate, false);
    handlePacketProcessed(packet);
    pushOrSendPacketEnd(packet, outGate, consumer, packet->getTransmissionId());
    updateDisplayString();
}

void MessageDispatcher::handleCanPushPacketChanged(const cGate *outGate)
{
    int size = gateSize("in");
    for (int i = 0; i < size; i++) {
        auto inGate = gate("in", i);
        auto producer = findConnectedModule<queueing::IActivePacketSource>(inGate);
        if (producer != nullptr && !dynamic_cast<MessageDispatcher *>(producer) && outGate->getOwnerModule() != inGate->getOwnerModule())
            producer->handleCanPushPacketChanged(inGate->getPathStartGate());
    }
}

void MessageDispatcher::handlePushPacketProcessed(Packet *packet, const cGate *gate, bool successful)
{
}

#endif // #ifdef INET_WITH_QUEUEING

bool MessageDispatcher::hasLookupModuleInterface(const cGate *gate, const std::type_info& type, const cObject *arguments, int direction)
{
    int size = gateSize(gate->getType() == cGate::INPUT ? "out" : "in");
    for (int i = 0; i < size; i++) {
        if (i != gate->getIndex()) {
            cGate *referencingGate = this->gate(gate->getType() == cGate::INPUT ? "out" : "in", i);
            if (findModuleInterface(referencingGate, type, arguments) != nullptr)
                return true;
        }
    }
    return false;
}

cGate *MessageDispatcher::forwardLookupModuleInterface(const cGate *gate, const std::type_info& type, const cObject *arguments, int direction)
{
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
                    bool resultIsMessageDispatcher = dynamic_cast<MessageDispatcher *>(result->getOwnerModule());
                    bool referencedModuleIsMessageDispatcher = dynamic_cast<MessageDispatcher *>(referencedModule);
                    if (!resultIsMessageDispatcher && referencedModuleIsMessageDispatcher)
                        break;
                    else if (referencedModuleIsMessageDispatcher || (!resultIsMessageDispatcher && !referencedModuleIsMessageDispatcher))
                        throw cRuntimeError("Cannot find module interface because the result is ambiguous, candidate1 = %s, candidate2 = %s, module = %s, gate = %s, type = %s, arguments = %s, direction = %s",
                                printToStringIfPossible(result->getOwnerModule(), 0).c_str(),
                                printToStringIfPossible(referencedModule, 0).c_str(),
                                printToStringIfPossible(gate->getOwnerModule(), 0).c_str(),
                                printToStringIfPossible(gate, 0).c_str(),
                                opp_typename(type),
                                printToStringIfPossible(arguments, 0).c_str(),
                                printToStringIfPossible(direction, 0).c_str());
                }
                result = referencedGate;
            }
        }
    }
    return result;
}

cGate *MessageDispatcher::lookupModuleInterface(cGate *gate, const std::type_info& type, const cObject *arguments, int direction)
{
    Enter_Method("lookupModuleInterface");
    EV_TRACE << "Looking up module interface" << EV_FIELD(gate) << EV_FIELD(type, opp_typename(type)) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_ENDL;
    if (gate->isName("in")) {
        if (type == typeid(IPassivePacketSink)) { // handle all packets
            if (arguments == nullptr)
                return gate;
            else if (hasLookupModuleInterface(gate, type, arguments, direction))
                return gate;
        }
    }
    return forwardLookupModuleInterface(gate, type, arguments, direction); // forward all other interfaces
}

} // namespace inet


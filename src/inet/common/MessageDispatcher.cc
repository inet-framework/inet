//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/applications/common/SocketTag_m.h"
#include "inet/common/MessageDispatcher.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"

namespace inet {

Define_Module(MessageDispatcher);

MessageDispatcher::MessageDispatcher()
{
}

void MessageDispatcher::initialize()
{
    WATCH_MAP(socketIdToUpperLayerGateIndex);
    WATCH_MAP(interfaceIdToLowerLayerGateIndex);
    WATCH_MAP(protocolIdToUpperLayerGateIndex);
    WATCH_MAP(protocolIdToLowerLayerGateIndex);
}

int MessageDispatcher::computeSocketReqSocketId(cMessage *message)
{
    auto *socketReq = message->getTag<SocketReq>();
    return socketReq != nullptr ? socketReq->getSocketId() : -1;
}

int MessageDispatcher::computeSocketIndSocketId(cMessage *message)
{
    auto *socketInd = message->getTag<SocketInd>();
    return socketInd != nullptr ? socketInd->getSocketId() : -1;
}

int MessageDispatcher::computeInterfaceId(cMessage *message)
{
    auto interfaceReq = message->getTag<InterfaceReq>();
    return interfaceReq != nullptr ? interfaceReq->getInterfaceId() : -1;
}

const Protocol *MessageDispatcher::computeProtocol(cMessage *message)
{
    auto protocolReq = message->getTag<DispatchProtocolReq>();;
    return protocolReq != nullptr ? protocolReq->getProtocol() : nullptr;
}

void MessageDispatcher::arrived(cMessage *message, cGate *inGate, simtime_t t) {
    Enter_Method_Silent();
    cGate *outGate = nullptr;
    if (!strcmp("upperLayerIn", inGate->getName())) {
        if (message->isPacket())
            outGate = handleUpperLayerPacket(message, inGate);
        else
            outGate = handleUpperLayerCommand(message, inGate);
    }
    else if (!strcmp("lowerLayerIn", inGate->getName())) {
        if (message->isPacket())
            outGate = handleLowerLayerPacket(message, inGate);
        else
            outGate = handleLowerLayerCommand(message, inGate);
    }
    else
        throw cRuntimeError("Message %s(%s) arrived on unknown '%s' gate", message->getName(), message->getClassName(), inGate->getFullName());
    outGate->deliver(message, t);
}

cGate *MessageDispatcher::handleUpperLayerPacket(cMessage *message, cGate *inGate)
{
    int interfaceId = computeInterfaceId(message);
    auto protocol = computeProtocol(message);
    if (protocol != nullptr) {
        auto it = protocolIdToLowerLayerGateIndex.find(protocol->getId());
        if (it != protocolIdToLowerLayerGateIndex.end())
            return gate("lowerLayerOut", it->second);
        else
            throw cRuntimeError("handleUpperLayerPacket(): Unknown protocol: id = %d, name = %s", protocol->getId(), protocol->getName());
    }
    else if (interfaceId != -1) {
        auto it = interfaceIdToLowerLayerGateIndex.find(interfaceId);
        if (it != interfaceIdToLowerLayerGateIndex.end())
            return gate("lowerLayerOut", it->second);
        else
            throw cRuntimeError("handleUpperLayerPacket(): Unknown interface: id = %d", interfaceId);
    }
    else
        throw cRuntimeError("handleUpperLayerPacket(): Unknown packet: %s(%s)", message->getName(), message->getClassName());
}

cGate *MessageDispatcher::handleLowerLayerPacket(cMessage *message, cGate *inGate)
{
    int socketId = computeSocketIndSocketId(message);
    auto protocol = computeProtocol(message);
    if (socketId != -1) {
        auto it = socketIdToUpperLayerGateIndex.find(socketId);
        if (it != socketIdToUpperLayerGateIndex.end())
            return gate("upperLayerOut", it->second);
        else
            throw cRuntimeError("handleLowerLayerPacket(): Unknown socket, id = %d", socketId);
    }
    else if (protocol != nullptr) {
        auto it = protocolIdToUpperLayerGateIndex.find(protocol->getId());
        if (it != protocolIdToUpperLayerGateIndex.end())
            return gate("upperLayerOut", it->second);
        else
            throw cRuntimeError("handleLowerLayerPacket(): Unknown protocol: id = %d, name = %s", protocol->getId(), protocol->getName());
    }
    else
        throw cRuntimeError("handleLowerLayerPacket(): Unknown packet: %s(%s)", message->getName(), message->getClassName());
}

cGate *MessageDispatcher::handleUpperLayerCommand(cMessage *message, cGate *inGate)
{
    int socketId = computeSocketReqSocketId(message);
    int interfaceId = computeInterfaceId(message);
    auto protocol = computeProtocol(message);
    if (socketId != -1) {
        auto it = socketIdToUpperLayerGateIndex.find(socketId);
        if (it == socketIdToUpperLayerGateIndex.end())
            socketIdToUpperLayerGateIndex[socketId] = inGate->getIndex();
        else if (it->first != socketId)
            throw cRuntimeError("handleUpperLayerCommand(): Socket is already registered: id = %d, gate = %d, new gate = %d", socketId, it->second, inGate->getIndex());
    }
    if (protocol != nullptr) {
        auto it = protocolIdToLowerLayerGateIndex.find(protocol->getId());
        if (it != protocolIdToLowerLayerGateIndex.end())
            return gate("lowerLayerOut", it->second);
        else
            throw cRuntimeError("handleUpperLayerCommand(): Unknown protocol: id = %d, name = %s", protocol->getId(), protocol->getName());
    }
    else if (interfaceId != -1) {
        auto it = interfaceIdToLowerLayerGateIndex.find(interfaceId);
        if (it != interfaceIdToLowerLayerGateIndex.end())
            return gate("lowerLayerOut", it->second);
        else
            throw cRuntimeError("handleUpperLayerCommand(): Unknown interface: id = %d", interfaceId);
    }
    else
        throw cRuntimeError("handleUpperLayerCommand(): Unknown message: %s(%s)", message->getName(), message->getClassName());
}

cGate *MessageDispatcher::handleLowerLayerCommand(cMessage *message, cGate *inGate)
{
    int socketId = computeSocketIndSocketId(message);
    auto protocol = computeProtocol(message);
    if (socketId != -1) {
        auto it = socketIdToUpperLayerGateIndex.find(socketId);
        if (it != socketIdToUpperLayerGateIndex.end())
            return gate("upperLayerOut", it->second);
        else
            throw cRuntimeError("handleLowerLayerCommand(): Unknown socket, id = %d", socketId);
    }
    else if (protocol != nullptr) {
        auto it = protocolIdToUpperLayerGateIndex.find(protocol->getId());
        if (it != protocolIdToUpperLayerGateIndex.end())
            return gate("uppwerLayerOut", it->second);
        else
            throw cRuntimeError("handleLowerLayerCommand(): Unknown protocol: id = %d", protocol->getId());
    }
    else
        throw cRuntimeError("handleLowerLayerCommand(): Unknown message: %s(%s)", message->getName(), message->getClassName());
}

void MessageDispatcher::handleRegisterProtocol(const Protocol& protocol, cGate *protocolGate)
{
    if (!strcmp("upperLayerIn", protocolGate->getName())) {
        if (protocolIdToUpperLayerGateIndex.find(protocol.getId()) != protocolIdToUpperLayerGateIndex.end())
            throw cRuntimeError("handleRegisterProtocol(): Upper layer protocol is already registered: %s", protocol.info().c_str());
        protocolIdToUpperLayerGateIndex[protocol.getId()] = protocolGate->getIndex();
        int size = gateSize("lowerLayerOut");
        for (int i = 0; i < size; i++)
            registerProtocol(protocol, gate("lowerLayerOut", i));
    }
    else if (!strcmp("lowerLayerIn", protocolGate->getName())) {
        if (protocolIdToLowerLayerGateIndex.find(protocol.getId()) != protocolIdToLowerLayerGateIndex.end())
            throw cRuntimeError("handleRegisterProtocol(): Lower layer protocol is already registered: %s", protocol.info().c_str());
        protocolIdToLowerLayerGateIndex[protocol.getId()] = protocolGate->getIndex();
        int size = gateSize("upperLayerOut");
        for (int i = 0; i < size; i++)
            registerProtocol(protocol, gate("upperLayerOut", i));
    }
    else
        throw cRuntimeError("handleRegisterProtocol(): Unknown gate: %s", protocolGate->getName());
}

void MessageDispatcher::handleRegisterInterface(const InterfaceEntry &interface, cGate *interfaceGate)
{
    if (!strcmp("upperLayerIn", interfaceGate->getName()))
        throw cRuntimeError("handleRegisterInterface(): Invalid gate: %s", interfaceGate->getName());
    else if (!strcmp("lowerLayerIn", interfaceGate->getName())) {
        if (interfaceIdToLowerLayerGateIndex.find(interface.getInterfaceId()) != interfaceIdToLowerLayerGateIndex.end())
            throw cRuntimeError("handleRegisterInterface(): Interface is already registered: %s", interface.info().c_str());
        interfaceIdToLowerLayerGateIndex[interface.getInterfaceId()] = interfaceGate->getIndex();
        int size = gateSize("upperLayerOut");
        for (int i = 0; i < size; i++)
            registerInterface(interface, gate("upperLayerOut", i));
    }
    else
        throw cRuntimeError("handleRegisterInterface(): Unknown gate: %s", interfaceGate->getName());
}

} // namespace inet


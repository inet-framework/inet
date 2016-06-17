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

#include "inet/common/MessageDispatcher.h"
#include "inet/common/IPacketControlInfo.h"
#include "inet/common/ISocketControlInfo.h"
#include "inet/common/IProtocolControlInfo.h"
#include "inet/common/IInterfaceControlInfo.h"

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

int MessageDispatcher::computeSocketId(cMessage *message)
{
    ISocketControlInfo *controlInfo = dynamic_cast<ISocketControlInfo *>(message->getControlInfo());
    return controlInfo != nullptr ? controlInfo->getSocketId() : -1;
}

int MessageDispatcher::computeInterfaceId(cMessage *message)
{
    IInterfaceControlInfo *controlInfo = dynamic_cast<IInterfaceControlInfo *>(message->getControlInfo());
    return controlInfo != nullptr ? controlInfo->getInterfaceId() : -1;
}

int MessageDispatcher::computeUpperLayerProtocolId(cMessage *message)
{
    IPacketControlInfo *controlInfo = dynamic_cast<IPacketControlInfo *>(message->getControlInfo());
    return controlInfo != nullptr ? controlInfo->getPacketProtocolId() : -1;
}

int MessageDispatcher::computeLowerLayerProtocolId(cMessage *message)
{
    IProtocolControlInfo *controlInfo = dynamic_cast<IProtocolControlInfo *>(message->getControlInfo());
    return controlInfo != nullptr ? controlInfo->getControlInfoProtocolId() : -1;
}

const char *MessageDispatcher::findProtocolName(int protocolId)
{
    const Protocol *protocol = Protocol::findProtocol(protocolId);
    return protocol != nullptr ? protocol->getName() : "<unknown>";
}

void MessageDispatcher::arrived(cMessage *message, cGate *inGate, simtime_t t) {
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
    int protocolId = computeLowerLayerProtocolId(message);
    if (interfaceId != -1) {
        auto it = interfaceIdToLowerLayerGateIndex.find(interfaceId);
        if (it != interfaceIdToLowerLayerGateIndex.end())
            return gate("lowerLayerOut", it->second);
        else
            throw cRuntimeError("handleUpperLayerPacket(): Unknown interface: id = %d", interfaceId);
    }
    else if (protocolId != -1) {
        auto it = protocolIdToLowerLayerGateIndex.find(protocolId);
        if (it != protocolIdToLowerLayerGateIndex.end())
            return gate("lowerLayerOut", it->second);
        else
            throw cRuntimeError("handleUpperLayerPacket(): Unknown protocol: id = %d, name = %s", protocolId, findProtocolName(protocolId));
    }
    else
        throw cRuntimeError("handleUpperLayerPacket(): Unknown packet: %s(%s)", message->getName(), message->getClassName());
}

cGate *MessageDispatcher::handleLowerLayerPacket(cMessage *message, cGate *inGate)
{
    int socketId = computeSocketId(message);
    int protocolId = computeUpperLayerProtocolId(message);
    if (socketId != -1) {
        auto it = socketIdToUpperLayerGateIndex.find(socketId);
        if (it != socketIdToUpperLayerGateIndex.end())
            return gate("upperLayerOut", it->second);
        else
            throw cRuntimeError("handleLowerLayerPacket(): Unknown socket, id = %d", socketId);
    }
    else if (protocolId != -1) {
        auto it = protocolIdToUpperLayerGateIndex.find(protocolId);
        if (it != protocolIdToUpperLayerGateIndex.end())
            return gate("upperLayerOut", it->second);
        else
            throw cRuntimeError("handleLowerLayerPacket(): Unknown protocol: id = %d, name = %s", protocolId, findProtocolName(protocolId));
    }
    else
        throw cRuntimeError("handleLowerLayerPacket(): Unknown packet: %s(%s)", message->getName(), message->getClassName());
}

cGate *MessageDispatcher::handleUpperLayerCommand(cMessage *message, cGate *inGate)
{
    int socketId = computeSocketId(message);
    int interfaceId = computeInterfaceId(message);
    int protocolId = computeLowerLayerProtocolId(message);
    if (socketId != -1)
        socketIdToUpperLayerGateIndex[socketId] = inGate->getIndex();
    if (interfaceId != -1) {
        auto it = interfaceIdToLowerLayerGateIndex.find(interfaceId);
        if (it != interfaceIdToLowerLayerGateIndex.end())
            return gate("lowerLayerOut", it->second);
        else
            throw cRuntimeError("handleUpperLayerCommand(): Unknown interface: id = %d", interfaceId);
    }
    else if (protocolId != -1) {
        auto it = protocolIdToLowerLayerGateIndex.find(protocolId);
        if (it != protocolIdToLowerLayerGateIndex.end())
            return gate("lowerLayerOut", it->second);
        else
            throw cRuntimeError("handleUpperLayerCommand(): Unknown protocol: id = %d, name = %s", protocolId, findProtocolName(protocolId));
    }
    else
        throw cRuntimeError("handleUpperLayerCommand(): Unknown message: %s(%s)", message->getName(), message->getClassName());
}

cGate *MessageDispatcher::handleLowerLayerCommand(cMessage *message, cGate *inGate)
{
    int socketId = computeSocketId(message);
    int protocolId = computeLowerLayerProtocolId(message);
    if (socketId != -1) {
        auto it = socketIdToUpperLayerGateIndex.find(socketId);
        if (it != socketIdToUpperLayerGateIndex.end())
            return gate("upperLayerOut", it->second);
        else
            throw cRuntimeError("handleLowerLayerCommand(): Unknown socket, id = %d", socketId);
    }
    else if (protocolId != -1) {
        auto it = protocolIdToUpperLayerGateIndex.find(protocolId);
        if (it != protocolIdToUpperLayerGateIndex.end())
            return gate("uppwerLayerOut", it->second);
        else
            throw cRuntimeError("handleLowerLayerCommand(): Unknown protocol: id = %d", protocolId);
    }
    else
        throw cRuntimeError("handleLowerLayerCommand(): Unknown message: %s(%s)", message->getName(), message->getClassName());
}

void MessageDispatcher::handleRegisterProtocol(const Protocol& protocol, cGate *protocolGate)
{
    if (!strcmp("upperLayerIn", protocolGate->getName())) {
        protocolIdToUpperLayerGateIndex[protocol.getId()] = protocolGate->getIndex();
        int size = gateSize("lowerLayerOut");
        for (int i = 0; i < size; i++)
            registerProtocol(protocol, gate("lowerLayerOut", i));
    }
    else if (!strcmp("lowerLayerIn", protocolGate->getName())) {
        protocolIdToLowerLayerGateIndex[protocol.getId()] = protocolGate->getIndex();
        int size = gateSize("upperLayerOut");
        for (int i = 0; i < size; i++)
            registerProtocol(protocol, gate("upperLayerOut", i));
    }
    else
        throw cRuntimeError("handleRegisterProtocol(): Unknown gate: %s", protocolGate->getName());
}

cModule *MessageDispatcher::handleLookupProtocol(const Protocol& protocol, cGate *protocolGate)
{
    if (!strcmp("upperLayerIn", protocolGate->getName())) {
        auto it = protocolIdToLowerLayerGateIndex.find(protocol.getId());
        if (it != protocolIdToLowerLayerGateIndex.end())
            return lookupProtocol(protocol, gate("lowerLayerOut", it->second));
        else
            return nullptr;
    }
    else if (!strcmp("lowerLayerIn", protocolGate->getName())) {
        auto it = protocolIdToUpperLayerGateIndex.find(protocol.getId());
        if (it != protocolIdToUpperLayerGateIndex.end())
            return lookupProtocol(protocol, gate("upperLayerOut", it->second));
        else
            return nullptr;
    }
    else
        throw cRuntimeError("handleLookupProtocol(): Unknown gate: %s", protocolGate->getName());
}

void MessageDispatcher::handleRegisterInterface(const InterfaceEntry &interface, cGate *interfaceGate)
{
    if (!strcmp("upperLayerIn", interfaceGate->getName()))
        throw cRuntimeError("handleRegisterInterface(): Invalid gate: %s", interfaceGate->getName());
    else if (!strcmp("lowerLayerIn", interfaceGate->getName())) {
        interfaceIdToLowerLayerGateIndex[interface.getInterfaceId()] = interfaceGate->getIndex();
        int size = gateSize("upperLayerOut");
        for (int i = 0; i < size; i++)
            registerInterface(interface, gate("upperLayerOut", i));
    }
    else
        throw cRuntimeError("handleRegisterInterface(): Unknown gate: %s", interfaceGate->getName());
}

cModule *MessageDispatcher::handleLookupInterface(const InterfaceEntry &interface, cGate *interfaceGate)
{
    if (!strcmp("upperLayerIn", interfaceGate->getName())) {
        auto it = interfaceIdToLowerLayerGateIndex.find(interface.getInterfaceId());
        if (it != interfaceIdToLowerLayerGateIndex.end())
            return lookupInterface(interface, gate("lowerLayerOut", it->second));
        else
            return nullptr;
    }
    else
        throw cRuntimeError("handleLookupInterface(): Unknown gate: %s", interfaceGate->getName());
}

} // namespace inet


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

void MessageDispatcher::initialize()
{
    WATCH_MAP(socketIdToGateIndex);
    WATCH_MAP(interfaceIdToGateIndex);
    WATCH_MAP(serviceToGateIndex);
    WATCH_MAP(protocolToGateIndex);
}

void MessageDispatcher::arrived(cMessage *message, cGate *inGate SENDOPTIONS_ARG, simtime_t t)
{
    Enter_Method_Silent();
    cGate *outGate = nullptr;
    if (message->isPacket())
        outGate = handlePacket(check_and_cast<Packet *>(message), inGate);
    else
        outGate = handleMessage(check_and_cast<Message *>(message), inGate);
    outGate->deliver(message SENDOPTIONS_ARG_NAME, t);
}

cGate *MessageDispatcher::handlePacket(Packet *packet, cGate *inGate)
{
    auto socketInd = packet->findTag<SocketInd>();
    if (socketInd != nullptr) {
        int socketId = socketInd->getSocketId();
        auto it = socketIdToGateIndex.find(socketId);
        if (it != socketIdToGateIndex.end())
            return gate("out", it->second);
        else
            throw cRuntimeError("handlePacket(): Unknown socket, id = %d", socketId);
    }
    auto dispatchProtocolReq = packet->findTag<DispatchProtocolReq>();;
    if (dispatchProtocolReq != nullptr) {
        auto packetProtocolTag = packet->findTag<PacketProtocolTag>();;
        auto servicePrimitive = dispatchProtocolReq->getServicePrimitive();
        // TODO: KLUDGE: eliminate this by adding ServicePrimitive to every DispatchProtocolReq
        if (servicePrimitive == static_cast<ServicePrimitive>(-1)) {
            if (packetProtocolTag != nullptr && dispatchProtocolReq->getProtocol() == packetProtocolTag->getProtocol())
                servicePrimitive = SP_INDICATION;
            else
                servicePrimitive = SP_REQUEST;
        }
        auto protocol = dispatchProtocolReq->getProtocol();
        auto it = serviceToGateIndex.find(Key(protocol->getId(), servicePrimitive));
        if (it != serviceToGateIndex.end())
            return gate("out", it->second);
        else
            throw cRuntimeError("handlePacket(): Unknown protocol: id = %d, name = %s", protocol->getId(), protocol->getName());
    }
    auto interfaceReq = packet->findTag<InterfaceReq>();
    if (interfaceReq != nullptr) {
        int interfaceId = interfaceReq->getInterfaceId();
        auto it = interfaceIdToGateIndex.find(interfaceId);
        if (it != interfaceIdToGateIndex.end())
            return gate("out", it->second);
        else
            throw cRuntimeError("handlePacket(): Unknown interface: id = %d", interfaceId);
    }
    throw cRuntimeError("handlePacket(): Unknown packet: %s(%s)", packet->getName(), packet->getClassName());
}

cGate *MessageDispatcher::handleMessage(Message *message, cGate *inGate)
{
    auto socketReq = message->findTag<SocketReq>();
    if (socketReq != nullptr) {
        int socketReqId = socketReq->getSocketId();
        auto it = socketIdToGateIndex.find(socketReqId);
        if (it == socketIdToGateIndex.end())
            socketIdToGateIndex[socketReqId] = inGate->getIndex();
        else if (it->first != socketReqId)
            throw cRuntimeError("handleMessage(): Socket is already registered: id = %d, gate = %d, new gate = %d", socketReqId, it->second, inGate->getIndex());
    }
    auto socketInd = message->findTag<SocketInd>();
    if (socketInd != nullptr) {
        int socketId = socketInd->getSocketId();
        auto it = socketIdToGateIndex.find(socketId);
        if (it != socketIdToGateIndex.end())
            return gate("out", it->second);
        else
            throw cRuntimeError("handleMessage(): Unknown socket, id = %d", socketId);
    }
    auto dispatchProtocolReq = message->findTag<DispatchProtocolReq>();;
    if (dispatchProtocolReq != nullptr) {
        auto servicePrimitive = dispatchProtocolReq->getServicePrimitive();
        // TODO: KLUDGE: eliminate this by adding ServicePrimitive to every DispatchProtocolReq
        if (servicePrimitive == static_cast<ServicePrimitive>(-1))
            servicePrimitive = SP_REQUEST;
        auto protocol = dispatchProtocolReq->getProtocol();
        auto it = serviceToGateIndex.find(Key(protocol->getId(), servicePrimitive));
        if (it != serviceToGateIndex.end())
            return gate("out", it->second);
        else
            throw cRuntimeError("handleMessage(): Unknown protocol: id = %d, name = %s", protocol->getId(), protocol->getName());
    }
    auto interfaceReq = message->findTag<InterfaceReq>();
    if (interfaceReq != nullptr) {
        int interfaceId = interfaceReq->getInterfaceId();
        auto it = interfaceIdToGateIndex.find(interfaceId);
        if (it != interfaceIdToGateIndex.end())
            return gate("out", it->second);
        else
            throw cRuntimeError("handleMessage(): Unknown interface: id = %d", interfaceId);
    }
    throw cRuntimeError("handleMessage(): Unknown message: %s(%s)", message->getName(), message->getClassName());
}

void MessageDispatcher::handleRegisterService(const Protocol& protocol, cGate *out, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterService");
    auto key = Key(protocol.getId(), servicePrimitive);
    if (serviceToGateIndex.find(key) != serviceToGateIndex.end())
        throw cRuntimeError("handleRegisterService(): service is already registered: %s", protocol.str().c_str());
    serviceToGateIndex[key] = out->getIndex();
    int size = gateSize("in");
    for (int i = 0; i < size; i++)
        if (i != out->getIndex())
            registerService(protocol, gate("in", i), servicePrimitive);
}

void MessageDispatcher::handleRegisterProtocol(const Protocol& protocol, cGate *in, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterProtocol");
    auto key = Key(protocol.getId(), servicePrimitive);
    if (protocolToGateIndex.find(key) != protocolToGateIndex.end())
        throw cRuntimeError("handleRegisterProtocol(): protocol is already registered: %s", protocol.str().c_str());
    protocolToGateIndex[key] = in->getIndex();
    int size = gateSize("out");
    for (int i = 0; i < size; i++)
        if (i != in->getIndex())
            registerProtocol(protocol, gate("out", i), servicePrimitive);
}

void MessageDispatcher::handleRegisterInterface(const InterfaceEntry &interface, cGate *out, cGate *in)
{
    Enter_Method("handleRegisterInterface");
    if (interfaceIdToGateIndex.find(interface.getInterfaceId()) != interfaceIdToGateIndex.end())
        throw cRuntimeError("handleRegisterInterface(): Interface is already registered: %s", interface.str().c_str());
    interfaceIdToGateIndex[interface.getInterfaceId()] = out->getIndex();
    int size = gateSize("out");
    for (int i = 0; i < size; i++)
        if (i != in->getIndex())
            registerInterface(interface, gate("in", i), gate("out", i));
}

} // namespace inet


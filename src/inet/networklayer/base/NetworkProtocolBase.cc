//
// (C) 2013 Opensim Ltd.
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// Author: Andras Varga (andras@omnetpp.org)
//

#include "inet/applications/common/SocketTag_m.h"
#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/base/NetworkProtocolBase.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/L3SocketCommand_m.h"

namespace inet {

NetworkProtocolBase::NetworkProtocolBase() :
    interfaceTable(nullptr)
{
}

void NetworkProtocolBase::initialize(int stage)
{
    LayeredProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        interfaceTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        registerService(getProtocol(), gate("transportIn"), gate("queueIn"));
        registerProtocol(getProtocol(), gate("queueOut"), gate("transportOut"));
    }
}

void NetworkProtocolBase::handleRegisterService(const Protocol& protocol, cGate *out, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterService");
}

void NetworkProtocolBase::handleRegisterProtocol(const Protocol& protocol, cGate *in, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterProtocol");
    if (in->isName("transportIn"))
        upperProtocols.insert(&protocol);
}

void NetworkProtocolBase::sendUp(cMessage *message)
{
    if (Packet *packet = dynamic_cast<Packet *>(message)) {
        const Protocol *protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
        const auto *addr = packet->getTag<L3AddressInd>();
        auto remoteAddress(addr->getSrcAddress());
        auto localAddress(addr->getDestAddress());
        bool hasSocket = false;
        for (const auto &elem: socketIdToSocketDescriptor) {
            if (elem.second->protocolId == protocol->getId()
                    && (elem.second->localAddress.isUnspecified() || elem.second->localAddress == localAddress)
                    && (elem.second->remoteAddress.isUnspecified() || elem.second->remoteAddress == remoteAddress)) {
                auto *packetCopy = packet->dup();
                packetCopy->setKind(L3_I_DATA);
                packetCopy->addTagIfAbsent<SocketInd>()->setSocketId(elem.second->socketId);
                EV_INFO << "Passing up to socket " << elem.second->socketId << "\n";
                emit(packetSentToUpperSignal, packetCopy);
                send(packetCopy, "transportOut");
                hasSocket = true;
            }
        }
        if (upperProtocols.find(protocol) != upperProtocols.end()) {
            EV_INFO << "Passing up to protocol " << protocol->getName() << "\n";
            emit(packetSentToUpperSignal, packet);
            send(packet, "transportOut");
        }
        else {
            if (!hasSocket) {
                EV_ERROR << "Transport protocol '" << protocol->getName() << "' not connected, discarding packet\n";
                //TODO send an ICMP error: protocol unreachable
                // sendToIcmp(datagram, inputInterfaceId, ICMP_DESTINATION_UNREACHABLE, ICMP_DU_PROTOCOL_UNREACHABLE);
            }
            delete packet;
        }
    }
    else
        send(message, "transportOut");
}

void NetworkProtocolBase::sendDown(cMessage *message, int interfaceId)
{
    if (message->isPacket())
        emit(packetSentToLowerSignal, message);
    if (interfaceId != -1) {
        auto& tags = getTags(message);
        delete tags.removeTagIfPresent<DispatchProtocolReq>();
        tags.addTagIfAbsent<InterfaceReq>()->setInterfaceId(interfaceId);
        send(message, "queueOut");
    }
    else {
        for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
            InterfaceEntry *interfaceEntry = interfaceTable->getInterface(i);
            if (interfaceEntry && !interfaceEntry->isLoopback()) {
                cMessage* duplicate = utils::dupPacketAndControlInfo(message);
                auto& tags = getTags(duplicate);
                delete tags.removeTagIfPresent<DispatchProtocolReq>();
                tags.addTagIfAbsent<InterfaceReq>()->setInterfaceId(interfaceEntry->getInterfaceId());
                send(duplicate, "queueOut");
            }
        }
        delete message;
    }
}

bool NetworkProtocolBase::isUpperMessage(cMessage *message)
{
    return message->getArrivalGate()->isName("transportIn");
}

bool NetworkProtocolBase::isLowerMessage(cMessage *message)
{
    return message->getArrivalGate()->isName("queueIn");
}

void NetworkProtocolBase::handleUpperCommand(cMessage *msg)
{
    auto request = dynamic_cast<Request *>(msg);
    if (auto *command = dynamic_cast<L3SocketBindCommand *>(msg->getControlInfo())) {
        int socketId = request->getTag<SocketReq>()->getSocketId();
        SocketDescriptor *descriptor = new SocketDescriptor(socketId, command->getProtocol()->getId(), command->getLocalAddress());
        socketIdToSocketDescriptor[socketId] = descriptor;
        delete msg;
    }
    else if (auto *command = dynamic_cast<L3SocketConnectCommand *>(msg->getControlInfo())) {
        int socketId = request->getTag<SocketReq>()->getSocketId();
        if (socketIdToSocketDescriptor.find(socketId) == socketIdToSocketDescriptor.end())
            throw cRuntimeError("Ipv4Socket: should use bind() before connect()");
        socketIdToSocketDescriptor[socketId]->remoteAddress = command->getRemoteAddress();
        delete msg;
    }
    else if (dynamic_cast<L3SocketCloseCommand *>(msg->getControlInfo()) != nullptr) {
        int socketId = request->getTag<SocketReq>()->getSocketId();
        auto it = socketIdToSocketDescriptor.find(socketId);
        if (it != socketIdToSocketDescriptor.end()) {
            delete it->second;
            socketIdToSocketDescriptor.erase(it);
            auto indication = new Indication("closed", L3_I_SOCKET_CLOSED);
            auto ctrl = new L3SocketClosedIndication();
            indication->setControlInfo(ctrl);
            indication->addTag<SocketInd>()->setSocketId(socketId);
            send(indication, "transportOut");
        }
        delete msg;
    }
    else
        LayeredProtocolBase::handleUpperCommand(msg);
}

} // namespace inet


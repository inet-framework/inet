//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/base/NetworkProtocolBase.h"

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/common/stlutils.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/L3SocketCommand_m.h"

namespace inet {

NetworkProtocolBase::NetworkProtocolBase()
{
}

void NetworkProtocolBase::initialize(int stage)
{
    LayeredProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        interfaceTable.reference(this, "interfaceTableModule", true);
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        registerService(getProtocol(), gate("transportIn"), gate("transportOut"));
        registerProtocol(getProtocol(), gate("queueOut"), gate("queueIn"));
    }
}

void NetworkProtocolBase::handleRegisterService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterService");
}

void NetworkProtocolBase::handleRegisterProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterProtocol");
    if (gate->isName("transportOut"))
        upperProtocols.insert(&protocol);
}

void NetworkProtocolBase::sendUp(cMessage *message)
{
    if (Packet *packet = dynamic_cast<Packet *>(message)) {
        const Protocol *protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
        const auto& addr = packet->getTag<L3AddressInd>();
        auto remoteAddress(addr->getSrcAddress());
        auto localAddress(addr->getDestAddress());
        bool hasSocket = false;
        for (const auto& elem : socketIdToSocketDescriptor) {
            if (elem.second->protocolId == protocol->getId() &&
                (elem.second->localAddress.isUnspecified() || elem.second->localAddress == localAddress) &&
                (elem.second->remoteAddress.isUnspecified() || elem.second->remoteAddress == remoteAddress))
            {
                auto *packetCopy = packet->dup();
                packetCopy->setKind(L3_I_DATA);
                packetCopy->addTagIfAbsent<SocketInd>()->setSocketId(elem.second->socketId);
                EV_INFO << "Passing up to socket " << elem.second->socketId << "\n";
                emit(packetSentToUpperSignal, packetCopy);
                send(packetCopy, "transportOut");
                hasSocket = true;
            }
        }
        if (contains(upperProtocols, protocol)) {
            EV_INFO << "Passing up to protocol " << protocol->getName() << "\n";
            emit(packetSentToUpperSignal, packet);
            send(packet, "transportOut");
        }
        else {
            if (!hasSocket) {
                EV_ERROR << "Transport protocol '" << protocol->getName() << "' not connected, discarding packet\n";
                // TODO send an ICMP error: protocol unreachable
//                sendToIcmp(datagram, inputInterfaceId, ICMP_DESTINATION_UNREACHABLE, ICMP_DU_PROTOCOL_UNREACHABLE);
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
        auto& tags = check_and_cast<ITaggedObject *>(message)->getTags();
        tags.addTagIfAbsent<InterfaceReq>()->setInterfaceId(interfaceId);
        auto networkInterface = interfaceTable->getInterfaceById(interfaceId);
        auto protocol = networkInterface->getProtocol();
        if (protocol != nullptr)
            tags.addTagIfAbsent<DispatchProtocolReq>()->setProtocol(protocol);
        else
            tags.removeTagIfPresent<DispatchProtocolReq>();
        send(message, "queueOut");
    }
    else {
        for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
            NetworkInterface *networkInterface = interfaceTable->getInterface(i);
            if (networkInterface && !networkInterface->isLoopback()) {
                cMessage *duplicate = utils::dupPacketAndControlInfo(message);
                auto& tags = check_and_cast<ITaggedObject *>(duplicate)->getTags();
                tags.addTagIfAbsent<InterfaceReq>()->setInterfaceId(networkInterface->getInterfaceId());
                auto protocol = networkInterface->getProtocol();
                if (protocol != nullptr)
                    tags.addTagIfAbsent<DispatchProtocolReq>()->setProtocol(protocol);
                else
                    tags.removeTagIfPresent<DispatchProtocolReq>();
                send(duplicate, "queueOut");
            }
        }
        delete message;
    }
}

bool NetworkProtocolBase::isUpperMessage(cMessage *message) const
{
    return message->getArrivalGate()->isName("transportIn");
}

bool NetworkProtocolBase::isLowerMessage(cMessage *message) const
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
        if (!containsKey(socketIdToSocketDescriptor, socketId))
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


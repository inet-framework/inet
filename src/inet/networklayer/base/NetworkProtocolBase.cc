//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/base/NetworkProtocolBase.h"

#include "inet/common/FunctionalEvent.h"
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
#include "inet/common/SimulationContinuation.h"

namespace inet {

NetworkProtocolBase::NetworkProtocolBase()
{
}

void NetworkProtocolBase::initialize(int stage)
{
    LayeredProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        interfaceTable.reference(this, "interfaceTableModule", true);
        queueSink.reference(gate("queueOut"), true);
        transportSink.reference(gate("transportOut"), true);
    }
}

void NetworkProtocolBase::sendUp(cMessage *message)
{
    Packet *packet = check_and_cast<Packet *>(message);
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
            yieldBeforePush();
            transportSink.pushPacket(packetCopy);
            hasSocket = true;
        }
    }
    if (contains(upperProtocols, protocol)) {
        EV_INFO << "Passing up to protocol " << protocol->getName() << "\n";
        emit(packetSentToUpperSignal, packet);
        yieldBeforePush();
        transportSink.pushPacket(packet);
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
        yieldBeforePush();
        queueSink.pushPacket(check_and_cast<Packet *>(message));
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
                yieldBeforePush();
                queueSink.pushPacket(check_and_cast<Packet *>(duplicate));
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
        bind(socketId, command->getProtocol(), command->getLocalAddress());
        delete msg;
    }
    else if (auto *command = dynamic_cast<L3SocketConnectCommand *>(msg->getControlInfo())) {
        int socketId = request->getTag<SocketReq>()->getSocketId();
        if (!containsKey(socketIdToSocketDescriptor, socketId))
            throw cRuntimeError("L3Socket: should use bind() before connect()");
        connect(socketId, command->getRemoteAddress());
        delete msg;
    }
    else if (dynamic_cast<L3SocketCloseCommand *>(msg->getControlInfo()) != nullptr) {
        int socketId = request->getTag<SocketReq>()->getSocketId();
        close(socketId);
        delete msg;
    }
    else if (dynamic_cast<L3SocketDestroyCommand *>(msg->getControlInfo()) != nullptr) {
        int socketId = request->getTag<SocketReq>()->getSocketId();
        destroy(socketId);
        delete msg;
    }
    else
        LayeredProtocolBase::handleUpperCommand(msg);
}

void NetworkProtocolBase::pushPacket(Packet *packet, const cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    packet->setArrival(getId(), gate->getId());
    handleMessage(packet);
}

void NetworkProtocolBase::setCallback(int socketId, ICallback *callback)
{
    Enter_Method("setCallback");
    socketIdToSocketDescriptor[socketId]->callback = callback;
}

void NetworkProtocolBase::bind(int socketId, const Protocol *protocol, const L3Address& localAddress)
{
    Enter_Method("bind");
    SocketDescriptor *descriptor = new SocketDescriptor(socketId, protocol ? protocol->getId() : -1, localAddress);
    socketIdToSocketDescriptor[socketId] = descriptor;
}

void NetworkProtocolBase::connect(int socketId, const L3Address& remoteAddress)
{
    Enter_Method("connect");
    socketIdToSocketDescriptor[socketId]->remoteAddress = remoteAddress;
}

void NetworkProtocolBase::close(int socketId)
{
    Enter_Method("close");
    auto it = socketIdToSocketDescriptor.find(socketId);
    if (it != socketIdToSocketDescriptor.end()) {
        auto callback = it->second->callback;
        delete it->second;
        socketIdToSocketDescriptor.erase(it);
        if (callback)
            inet::scheduleAfter("handleClose", 0, [=]() { callback->handleClosed(); });
    }
}

void NetworkProtocolBase::destroy(int socketId)
{
    Enter_Method("destroy");
    auto it = socketIdToSocketDescriptor.find(socketId);
    if (it != socketIdToSocketDescriptor.end()) {
        delete it->second;
        socketIdToSocketDescriptor.erase(it);
    }
}

} // namespace inet


//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/contract/ipv6/Ipv6Socket.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/ipv6/Ipv6SocketCommand_m.h"

namespace inet {

Ipv6Socket::Ipv6Socket(cGate *outputGate) :
    socketId(getActiveSimulationOrEnvir()->getUniqueNumber()),
    outputGate(outputGate)
{
}

void Ipv6Socket::setCallback(INetworkSocket::ICallback *callback)
{
    this->callback = callback;
}

bool Ipv6Socket::belongsToSocket(cMessage *msg) const
{
    auto& tags = check_and_cast<ITaggedObject *>(msg)->getTags();
    int msgSocketId = tags.getTag<SocketInd>()->getSocketId();
    return socketId == msgSocketId;
}

void Ipv6Socket::processMessage(cMessage *msg)
{
    ASSERT(belongsToSocket(msg));
    switch (msg->getKind()) {
        case IPv6_I_DATA:
            if (callback)
                callback->socketDataArrived(this, check_and_cast<Packet *>(msg));
            else
                delete msg;
            break;
        case IPv6_I_SOCKET_CLOSED:
            check_and_cast<Indication *>(msg);
            bound = isOpen_ = false;
            if (callback)
                callback->socketClosed(this);
            delete msg;
            break;
        default:
            throw cRuntimeError("Ipv6Socket: invalid msg kind %d, one of the IPv6_I_xxx constants expected", msg->getKind());
            break;
    }
}

void Ipv6Socket::bind(const Protocol *protocol, Ipv6Address localAddress)
{
    ASSERT(!bound);
    auto *command = new Ipv6SocketBindCommand();
    command->setProtocol(protocol);
    command->setLocalAddress(localAddress);
    auto request = new Request("bind", IPv6_C_BIND);
    request->setControlInfo(command);
    sendToOutput(request);
    bound = true;
    isOpen_ = true;
}

void Ipv6Socket::connect(Ipv6Address remoteAddress)
{
    isOpen_ = true;
    auto *command = new Ipv6SocketConnectCommand();
    command->setRemoteAddress(remoteAddress);
    auto request = new Request("connect", IPv6_C_CONNECT);
    request->setControlInfo(command);
    sendToOutput(request);
}

void Ipv6Socket::send(Packet *packet)
{
    sendToOutput(packet);
}

void Ipv6Socket::sendTo(Packet *packet, Ipv6Address destAddress)
{
    auto addressReq = packet->addTagIfAbsent<L3AddressReq>();
    addressReq->setDestAddress(destAddress);
    send(packet);
}

void Ipv6Socket::close()
{
    ASSERT(bound);
    Ipv6SocketCloseCommand *command = new Ipv6SocketCloseCommand();
    auto request = new Request("close", IPv6_C_CLOSE);
    request->setControlInfo(command);
    sendToOutput(request);
}

void Ipv6Socket::destroy()
{
    auto *command = new Ipv6SocketDestroyCommand();
    auto request = new Request("destroy", IPv6_C_DESTROY);
    request->setControlInfo(command);
    sendToOutput(request);
}

void Ipv6Socket::sendToOutput(cMessage *message)
{
    if (!outputGate)
        throw cRuntimeError("Ipv6Socket: setOutputGate() must be invoked before the socket can be used");
    auto& tags = check_and_cast<ITaggedObject *>(message)->getTags();
    tags.addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ipv6);
    tags.addTagIfAbsent<SocketReq>()->setSocketId(socketId);
    check_and_cast<cSimpleModule *>(outputGate->getOwnerModule())->send(message, outputGate);
}

} // namespace inet


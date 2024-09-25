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
    socketId(getActiveSimulationOrEnvir()->getUniqueNumber())
{
    if (outputGate != nullptr)
        setOutputGate(outputGate);
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
    ipv6->bind(socketId, protocol, localAddress);
    bound = true;
    isOpen_ = true;
}

void Ipv6Socket::connect(Ipv6Address remoteAddress)
{
    isOpen_ = true;
    ipv6->connect(socketId, remoteAddress);
}

void Ipv6Socket::send(Packet *packet)
{
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ipv6);
    packet->addTagIfAbsent<SocketReq>()->setSocketId(socketId);
    sink.pushPacket(packet);
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
    ipv6->close(socketId);
}

void Ipv6Socket::destroy()
{
    ipv6->destroy(socketId);
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


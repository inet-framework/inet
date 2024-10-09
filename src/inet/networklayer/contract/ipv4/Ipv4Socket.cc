//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/contract/ipv4/Ipv4Socket.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/ipv4/Ipv4SocketCommand_m.h"

namespace inet {

Ipv4Socket::Ipv4Socket(cGate *outputGate) :
    socketId(getActiveSimulationOrEnvir()->getUniqueNumber())
{
    if (outputGate != nullptr)
        setOutputGate(outputGate);
}

void Ipv4Socket::setCallback(INetworkSocket::ICallback *callback)
{
    this->callback = callback;
}

bool Ipv4Socket::belongsToSocket(cMessage *msg) const
{
    auto& tags = check_and_cast<ITaggedObject *>(msg)->getTags();
    const auto& socketInd = tags.findTag<SocketInd>();
    return socketInd != nullptr && socketInd->getSocketId() == socketId;
}

void Ipv4Socket::processMessage(cMessage *msg)
{
    ASSERT(belongsToSocket(msg));
    switch (msg->getKind()) {
        case IPv4_I_DATA:
            if (callback)
                callback->socketDataArrived(this, check_and_cast<Packet *>(msg));
            else
                delete msg;
            break;
        case IPv4_I_SOCKET_CLOSED:
            check_and_cast<Indication *>(msg);
            isOpen_ = false;
            bound = false;
            if (callback)
                callback->socketClosed(this);
            delete msg;
            break;
        default:
            throw cRuntimeError("Ipv4Socket: invalid msg kind %d, one of the IPv4_I_xxx constants expected", msg->getKind());
            break;
    }
}

void Ipv4Socket::bind(const Protocol *protocol, Ipv4Address localAddress)
{
    ASSERT(!bound);
    ipv4->bind(socketId, protocol, localAddress);
    ipv4->setCallback(socketId, this);
    bound = true;
    isOpen_ = true;
}

void Ipv4Socket::connect(Ipv4Address remoteAddress)
{
    ipv4->connect(socketId, remoteAddress);
    ipv4->setCallback(socketId, this);
    isOpen_ = true;
}

void Ipv4Socket::send(Packet *packet)
{
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);
    packet->addTagIfAbsent<SocketReq>()->setSocketId(socketId);
    sink.pushPacket(packet);
}

void Ipv4Socket::sendTo(Packet *packet, Ipv4Address destAddress)
{
    auto addressReq = packet->addTagIfAbsent<L3AddressReq>();
    addressReq->setDestAddress(destAddress);
    send(packet);
}

void Ipv4Socket::close()
{
    ASSERT(bound);
    ipv4->close(socketId);
}

void Ipv4Socket::destroy()
{
    ASSERT(bound);
    ipv4->destroy(socketId);
}

void Ipv4Socket::sendToOutput(cMessage *message)
{
    if (!outputGate)
        throw cRuntimeError("Ipv4Socket: setOutputGate() must be invoked before the socket can be used");
    auto& tags = check_and_cast<ITaggedObject *>(message)->getTags();
    tags.addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);
    tags.addTagIfAbsent<SocketReq>()->setSocketId(socketId);
    check_and_cast<cSimpleModule *>(outputGate->getOwnerModule())->send(message, outputGate);
}

} // namespace inet


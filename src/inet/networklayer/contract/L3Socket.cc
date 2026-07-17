//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/contract/L3Socket.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/L3SocketCommand_m.h"
#include "inet/common/SimulationContinuation.h"

namespace inet {

L3Socket::L3Socket(const Protocol *l3Protocol, cGate *outputGate) :
    l3Protocol(l3Protocol),
    socketId(getActiveSimulationOrEnvir()->getUniqueNumber())
{
    if (outputGate != nullptr)
        setOutputGate(outputGate);
}

void L3Socket::setCallback(INetworkSocket::ICallback *callback)
{
    this->callback = callback;
}

bool L3Socket::belongsToSocket(cMessage *msg) const
{
    auto& tags = check_and_cast<ITaggedObject *>(msg)->getTags();
    int msgSocketId = tags.getTag<SocketInd>()->getSocketId();
    return socketId == msgSocketId;
}

void L3Socket::processMessage(cMessage *msg)
{
    ASSERT(belongsToSocket(msg));
    switch (msg->getKind()) {
        case L3_I_DATA:
            if (callback)
                callback->socketDataArrived(this, check_and_cast<Packet *>(msg));
            else
                delete msg;
            break;
        case L3_I_SOCKET_CLOSED:
            check_and_cast<Indication *>(msg);
            bound = isOpen_ = false;
            if (callback)
                callback->socketClosed(this);
            delete msg;
            break;
        default:
            throw cRuntimeError("L3Socket: invalid msg kind %d, one of the L3_I_xxx constants expected", msg->getKind());
            break;
    }
}

void L3Socket::bind(const Protocol *protocol, L3Address localAddress)
{
    ASSERT(!bound);
    ASSERT(l3Protocol != nullptr);
    l3ProtocolModule->bind(socketId, protocol, localAddress);
    l3ProtocolModule->setCallback(socketId, this);
    bound = true;
    isOpen_ = true;
}

void L3Socket::connect(L3Address remoteAddress)
{
    l3ProtocolModule->connect(socketId, remoteAddress);
    l3ProtocolModule->setCallback(socketId, this);
    isOpen_ = true;
}

void L3Socket::send(Packet *packet)
{
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(l3Protocol);
    packet->addTagIfAbsent<SocketReq>()->setSocketId(socketId);
    deferrablePushPacket(sink, packet);
}

void L3Socket::sendTo(Packet *packet, L3Address destAddress)
{
    auto addressReq = packet->addTagIfAbsent<L3AddressReq>();
    addressReq->setDestAddress(destAddress);
    send(packet);
}

void L3Socket::close()
{
    ASSERT(bound);
    ASSERT(l3Protocol != nullptr);
    l3ProtocolModule->close(socketId);
}

void L3Socket::destroy()
{
    ASSERT(l3Protocol != nullptr);
    l3ProtocolModule->destroy(socketId);
}

void L3Socket::sendToOutput(cMessage *message)
{
    if (!outputGate)
        throw cRuntimeError("L3Socket: setOutputGate() must be invoked before the socket can be used");
    auto& tags = check_and_cast<ITaggedObject *>(message)->getTags();
    tags.addTagIfAbsent<DispatchProtocolReq>()->setProtocol(l3Protocol);
    tags.addTagIfAbsent<SocketReq>()->setSocketId(socketId);
    check_and_cast<cSimpleModule *>(outputGate->getOwnerModule())->send(message, outputGate);
}

} // namespace inet


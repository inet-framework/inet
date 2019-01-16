//
// Copyright (C) 2015 OpenSim Ltd.
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
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/ipv4/Ipv4Socket.h"
#include "inet/networklayer/contract/ipv4/Ipv4SocketCommand_m.h"

namespace inet {

Ipv4Socket::Ipv4Socket(cGate *outputGate) :
    socketId(getEnvir()->getUniqueNumber()),
    outputGate(outputGate)
{
}

void Ipv4Socket::setCallback(INetworkSocket::ICallback *callback)
{
    this->callback = callback;
}

bool Ipv4Socket::belongsToSocket(cMessage *msg) const
{
    auto& tags = getTags(msg);
    int msgSocketId = tags.getTag<SocketInd>()->getSocketId();
    return socketId == msgSocketId;
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
    Ipv4SocketBindCommand *command = new Ipv4SocketBindCommand();
    command->setProtocol(protocol);
    command->setLocalAddress(localAddress);
    auto request = new Request("bind", IPv4_C_BIND);
    request->setControlInfo(command);
    sendToOutput(request);
    bound = true;
    isOpen_ = true;
}

void Ipv4Socket::connect(Ipv4Address remoteAddress)
{
    Ipv4SocketConnectCommand *command = new Ipv4SocketConnectCommand();
    command->setRemoteAddress(remoteAddress);
    auto request = new Request("connect", IPv4_C_CONNECT);
    request->setControlInfo(command);
    sendToOutput(request);
    isOpen_ = true;
}

void Ipv4Socket::send(Packet *packet)
{
    sendToOutput(packet);
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
    Ipv4SocketCloseCommand *command = new Ipv4SocketCloseCommand();
    auto request = new Request("close", IPv4_C_CLOSE);
    request->setControlInfo(command);
    sendToOutput(request);
}

void Ipv4Socket::destroy()
{
    ASSERT(bound);
    auto command = new Ipv4SocketDestroyCommand();
    auto request = new Request("destroy", IPv4_C_DESTROY);
    request->setControlInfo(command);
    sendToOutput(request);
}

void Ipv4Socket::sendToOutput(cMessage *message)
{
    if (!outputGate)
        throw cRuntimeError("Ipv4Socket: setOutputGate() must be invoked before the socket can be used");
    auto& tags = getTags(message);
    tags.addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);
    tags.addTagIfAbsent<SocketReq>()->setSocketId(socketId);
    check_and_cast<cSimpleModule *>(outputGate->getOwnerModule())->send(message, outputGate);
}

} // namespace inet


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
#include "inet/networklayer/contract/L3Socket.h"
#include "inet/networklayer/contract/L3SocketCommand_m.h"

namespace inet {

L3Socket::L3Socket(const Protocol *l3Protocol, cGate *outputGate) :
    l3Protocol(l3Protocol),
    socketId(getEnvir()->getUniqueNumber()),
    outputGate(outputGate)
{
}

void L3Socket::setCallback(INetworkSocket::ICallback *callback)
{
    this->callback = callback;
}

bool L3Socket::belongsToSocket(cMessage *msg) const
{
    auto& tags = getTags(msg);
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
    L3SocketBindCommand *command = new L3SocketBindCommand();
    command->setProtocol(protocol);
    auto request = new Request("bind", L3_C_BIND);
    request->setControlInfo(command);
    sendToOutput(request);
    bound = true;
    isOpen_ = true;
}

void L3Socket::connect(L3Address remoteAddress)
{
    isOpen_ = true;
    auto *command = new L3SocketConnectCommand();
    command->setRemoteAddress(remoteAddress);
    auto request = new Request("connect", L3_C_CONNECT);
    request->setControlInfo(command);
    sendToOutput(request);
}

void L3Socket::send(Packet *packet)
{
    sendToOutput(packet);
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
    L3SocketCloseCommand *command = new L3SocketCloseCommand();
    auto request = new Request("close", L3_C_CLOSE);
    request->setControlInfo(command);
    sendToOutput(request);
}

void L3Socket::destroy()
{
    ASSERT(l3Protocol != nullptr);
    auto *command = new L3SocketDestroyCommand();
    auto request = new Request("destroy", L3_C_DESTROY);
    request->setControlInfo(command);
    sendToOutput(request);
}

void L3Socket::sendToOutput(cMessage *message)
{
    if (!outputGate)
        throw cRuntimeError("L3Socket: setOutputGate() must be invoked before the socket can be used");
    auto& tags = getTags(message);
    tags.addTagIfAbsent<DispatchProtocolReq>()->setProtocol(l3Protocol);
    tags.addTagIfAbsent<SocketReq>()->setSocketId(socketId);
    check_and_cast<cSimpleModule *>(outputGate->getOwnerModule())->send(message, outputGate);
}

} // namespace inet


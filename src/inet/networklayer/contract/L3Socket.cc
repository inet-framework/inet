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
#include "inet/networklayer/contract/L3Socket.h"
#include "inet/networklayer/contract/L3SocketCommand_m.h"

namespace inet {

L3Socket::L3Socket(const Protocol *controlInfoProtocol, cGate *outputGate) :
    l3Protocol(controlInfoProtocol),
    socketId(getEnvir()->getUniqueNumber()),
    outputGate(outputGate)
{
}

void L3Socket::setL3Protocol(const Protocol *controlInfoProtocol)
{
    ASSERT(!bound);
    l3Protocol = controlInfoProtocol;
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

void L3Socket::bind(const Protocol *protocol)
{
    ASSERT(!bound);
    ASSERT(l3Protocol != nullptr);
    L3SocketBindCommand *command = new L3SocketBindCommand();
    command->setProtocol(protocol);
    auto request = new Request("bind", L3_C_BIND);
    request->setControlInfo(command);
    sendToOutput(request);
    bound = true;
}

void L3Socket::send(cPacket *msg)
{
    sendToOutput(msg);
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

bool L3Socket::belongsToSocket(cMessage *msg) const
{
    auto& tags = getTags(msg);
    int msgSocketId = tags.getTag<SocketInd>()->getSocketId();
    return socketId == msgSocketId;
}

void L3Socket::setCallbackObject(ICallback *callback, void *userData)
{
    cb = callback;
    this->userData = userData;
}

void L3Socket::processMessage(cMessage *msg)
{
    ASSERT(belongsToSocket(msg));

    if (cb)
        cb->socketDataArrived(this, check_and_cast<Packet*>(msg));
    else
        delete msg;
}

} // namespace inet


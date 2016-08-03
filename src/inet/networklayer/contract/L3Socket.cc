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
#include "inet/networklayer/contract/L3Socket.h"
#include "inet/networklayer/contract/L3SocketCommand_m.h"

namespace inet {

L3Socket::L3Socket(int controlInfoProtocolId, cGate *outputGate) :
    controlInfoProtocolId(controlInfoProtocolId),
    socketId(getEnvir()->getUniqueNumber()),
    outputGate(outputGate)
{
}

void L3Socket::setControlInfoProtocolId(int _controlInfoProtocolId)
{
    ASSERT(!bound);
    controlInfoProtocolId = _controlInfoProtocolId;
}

void L3Socket::sendToOutput(cMessage *message)
{
    if (!outputGate)
        throw cRuntimeError("L3Socket: setOutputGate() must be invoked before the socket can be used");
    message->ensureTag<DispatchProtocolReq>()->setProtocol(Protocol::getProtocol(controlInfoProtocolId));
    message->ensureTag<SocketReq>()->setSocketId(socketId);
    check_and_cast<cSimpleModule *>(outputGate->getOwnerModule())->send(message, outputGate);
}

void L3Socket::bind(int protocolId)
{
    ASSERT(!bound);
    ASSERT(controlInfoProtocolId != -1);
    L3SocketBindCommand *command = new L3SocketBindCommand();
    command->setProtocolId(protocolId);
    cMessage *bind = new cMessage("bind");
    bind->setControlInfo(command);
    sendToOutput(bind);
    bound = true;
}

void L3Socket::send(cPacket *msg)
{
    sendToOutput(msg);
}

void L3Socket::close()
{
    ASSERT(bound);
    ASSERT(controlInfoProtocolId != -1);
    L3SocketCloseCommand *command = new L3SocketCloseCommand();
    cMessage *close = new cMessage("close");
    close->setControlInfo(command);
    sendToOutput(close);
}

} // namespace inet


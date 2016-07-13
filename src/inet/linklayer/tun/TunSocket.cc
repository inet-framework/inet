//
// Copyright (C) 2013 OpenSim Ltd.
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
#include "inet/linklayer/tun/TunSocket.h"
#include "inet/linklayer/tun/TunControlInfo_m.h"

namespace inet {

TunSocket::TunSocket()
{
    socketId = getEnvir()->getUniqueNumber();
}

void TunSocket::sendToTun(cMessage *msg)
{
    if (!outputGate)
        throw cRuntimeError("TunSocket: setOutputGate() must be invoked before socket can be used");
    msg->ensureTag<SocketReq>()->setSocketId(socketId);
    check_and_cast<cSimpleModule *>(outputGate->getOwnerModule())->send(msg, outputGate);
}

void TunSocket::open(int interfaceId)
{
    this->interfaceId = interfaceId;
    cMessage *message = new cMessage("OPEN");
    TunOpenCommand *command = new TunOpenCommand();
    command->setInterfaceId(interfaceId);
    message->setControlInfo(command);
    sendToTun(message);
}

void TunSocket::send(cPacket *packet)
{
    if (interfaceId == -1)
        throw cRuntimeError("Socket is closed");
    TunSendCommand *command = new TunSendCommand();
    command->setInterfaceId(interfaceId);
    packet->setControlInfo(command);
    sendToTun(packet);
}

void TunSocket::close()
{
    this->interfaceId = -1;
    cMessage *message = new cMessage("CLOSE");
    TunCloseCommand *command = new TunCloseCommand();
    command->setInterfaceId(interfaceId);
    message->setControlInfo(command);
    sendToTun(message);
}

} // namespace inet


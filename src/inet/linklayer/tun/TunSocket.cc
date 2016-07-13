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
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/tun/TunControlInfo_m.h"
#include "inet/linklayer/tun/TunSocket.h"

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
    msg->ensureTag<InterfaceReq>()->setInterfaceId(interfaceId);
    check_and_cast<cSimpleModule *>(outputGate->getOwnerModule())->send(msg, outputGate);
}

void TunSocket::open(int interfaceId)
{
    this->interfaceId = interfaceId;
    cMessage *message = new cMessage("OPEN");
    TunOpenCommand *command = new TunOpenCommand();
    message->setControlInfo(command);
    sendToTun(message);
}

void TunSocket::send(cPacket *packet)
{
    if (interfaceId == -1)
        throw cRuntimeError("Socket is closed");
    TunSendCommand *command = new TunSendCommand();
    packet->setControlInfo(command);
    sendToTun(packet);
}

void TunSocket::close()
{
    cMessage *message = new cMessage("CLOSE");
    TunCloseCommand *command = new TunCloseCommand();
    message->setControlInfo(command);
    sendToTun(message);
    this->interfaceId = -1;
}

} // namespace inet


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
#include "inet/common/packet/Message.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/tun/TunControlInfo_m.h"
#include "inet/linklayer/tun/TunSocket.h"

namespace inet {

TunSocket::TunSocket()
{
    socketId = getEnvir()->getUniqueNumber();
}

void TunSocket::setCallback(ICallback *callback)
{
    this->callback = callback;
}

bool TunSocket::belongsToSocket(cMessage *msg) const
{
    auto& tags = getTags(msg);
    int msgSocketId = tags.getTag<SocketInd>()->getSocketId();
    return socketId == msgSocketId;
}

void TunSocket::processMessage(cMessage *msg)
{
    ASSERT(belongsToSocket(msg));

    switch (msg->getKind()) {
        case TUN_I_DATA:
            if (callback)
                callback->socketDataArrived(this, check_and_cast<Packet*>(msg));
            else
                delete msg;
            break;
        case TUN_I_CLOSED:
            isOpen_ = false;
            if (callback)
                callback->socketClosed(this);
            delete msg;
            break;
        default:
            throw cRuntimeError("TunSocket: invalid msg kind %d, one of the TUN_I_xxx constants expected",
                msg->getKind());
    }
}

void TunSocket::open(int interfaceId)
{
    isOpen_ = true;
    this->interfaceId = interfaceId;
    auto request = new Request("OPEN", TUN_C_OPEN);
    TunOpenCommand *command = new TunOpenCommand();
    request->setControlInfo(command);
    sendToTun(request);
}

void TunSocket::send(Packet *packet)
{
    if (interfaceId == -1)
        throw cRuntimeError("Socket is closed");
//    TunSendCommand *command = new TunSendCommand();
//    packet->setControlInfo(command);
    packet->setKind(TUN_C_DATA);
    sendToTun(packet);
}

void TunSocket::close()
{
    auto request = new Request("CLOSE", TUN_C_CLOSE);
    TunCloseCommand *command = new TunCloseCommand();
    request->setControlInfo(command);
    sendToTun(request);
    this->interfaceId = -1;
}

void TunSocket::destroy()
{
    auto request = new Request("DESTROY", TUN_C_DESTROY);
    auto command = new TunDestroyCommand();
    request->setControlInfo(command);
    sendToTun(request);
    this->interfaceId = -1;
}

void TunSocket::sendToTun(cMessage *msg)
{
    if (!outputGate)
        throw cRuntimeError("TunSocket: setOutputGate() must be invoked before socket can be used");
    auto& tags = getTags(msg);
    tags.addTagIfAbsent<SocketReq>()->setSocketId(socketId);
    tags.addTagIfAbsent<InterfaceReq>()->setInterfaceId(interfaceId);
    check_and_cast<cSimpleModule *>(outputGate->getOwnerModule())->send(msg, outputGate);
}

} // namespace inet


//
// Copyright (C) 2018 OpenSim Ltd.
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
#include "inet/linklayer/common/Ieee802SapTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcSocket.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcSocketCommand_m.h"

namespace inet {

Ieee8022LlcSocket::Ieee8022LlcSocket()
{
    socketId = getEnvir()->getUniqueNumber();
}

void Ieee8022LlcSocket::setCallback(ICallback *callback)
{
    this->callback = callback;
}

bool Ieee8022LlcSocket::belongsToSocket(cMessage *msg) const
{
    auto& tags = getTags(msg);
    int msgSocketId = tags.getTag<SocketInd>()->getSocketId();
    return socketId == msgSocketId;
}

void Ieee8022LlcSocket::processMessage(cMessage *msg)
{
    ASSERT(belongsToSocket(msg));
    switch (msg->getKind()) {
        case IEEE8022_LLC_I_DATA:
            if (callback)
                callback->socketDataArrived(this, check_and_cast<Packet*>(msg));
            else
                delete msg;
            break;
        case IEEE8022_LLC_I_SOCKET_CLOSED:
            isOpen_ = false;
            if (callback)
                callback->socketClosed(this);
            delete msg;
            break;
        default:
            throw cRuntimeError("Ieee8022LlcSocket: invalid msg kind %d, one of the IEEE8022_LLC_I_xxx constants expected", msg->getKind());
            break;
    }
}

void Ieee8022LlcSocket::open(int interfaceId, int localSap)
{
    if (localSap < -1 || localSap > 255)
        throw cRuntimeError("LlcSocket::open(): Invalid localSap value: %d", localSap);
    this->interfaceId = interfaceId;
    this->localSap = localSap;
    auto request = new Request("LLC_OPEN", IEEE8022_LLC_C_OPEN);
    Ieee8022LlcSocketOpenCommand *command = new Ieee8022LlcSocketOpenCommand();
    command->setLocalSap(localSap);
    request->setControlInfo(command);
    isOpen_ = true;
    sendToLlc(request);
}

void Ieee8022LlcSocket::send(Packet *packet)
{
    if (! isOpen_)
        throw cRuntimeError("Socket is closed");
    sendToLlc(packet);
}

void Ieee8022LlcSocket::close()
{
    auto request = new Request("LLC_CLOSE", IEEE8022_LLC_C_CLOSE);
    Ieee8022LlcSocketCloseCommand *command = new Ieee8022LlcSocketCloseCommand();
    request->setControlInfo(command);
    sendToLlc(request);
    interfaceId = -1;
}

void Ieee8022LlcSocket::destroy()
{
    auto request = new Request("LLC_DESTROY", IEEE8022_LLC_C_DESTROY);
    auto command = new Ieee8022LlcSocketDestroyCommand();
    request->setControlInfo(command);
    sendToLlc(request);
    interfaceId = -1;
    isOpen_ = false;
}

void Ieee8022LlcSocket::sendToLlc(cMessage *msg)
{
    if (!outputGate)
        throw cRuntimeError("LlcSocket: setOutputGate() must be invoked before socket can be used");
    auto& tags = getTags(msg);
    tags.addTagIfAbsent<SocketReq>()->setSocketId(socketId);
    tags.addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ieee8022);
    tags.addTagIfAbsent<Ieee802SapReq>()->setSsap(localSap);
    if (interfaceId != -1)
        tags.addTagIfAbsent<InterfaceReq>()->setInterfaceId(interfaceId);
    check_and_cast<cSimpleModule *>(outputGate->getOwnerModule())->send(msg, outputGate);
}

} // namespace inet


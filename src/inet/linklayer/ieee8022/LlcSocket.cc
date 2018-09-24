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

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/applications/common/SocketTag_m.h"
#include "inet/linklayer/common/Ieee802SapTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/ieee8022/LlcSocketCommand_m.h"
#include "inet/linklayer/ieee8022/LlcSocket.h"

namespace inet {

LlcSocket::LlcSocket()
{
    socketId = getEnvir()->getUniqueNumber();
}

void LlcSocket::setCallback(ICallback *callback)
{
    this->callback = callback;
}

bool LlcSocket::belongsToSocket(cMessage *msg) const
{
    auto& tags = getTags(msg);
    int msgSocketId = tags.getTag<SocketInd>()->getSocketId();
    return socketId == msgSocketId;
}

void LlcSocket::processMessage(cMessage *msg)
{
    ASSERT(belongsToSocket(msg));

    if (callback)
        callback->socketDataArrived(this, check_and_cast<Packet*>(msg));
    else
        delete msg;
}

void LlcSocket::open(int interfaceId, int localSap)
{
    if (localSap < 0 || localSap > 255)
        throw cRuntimeError("LlcSocket::open(): Invalid localSap value: %d", localSap);
    this->interfaceId = interfaceId;
    this->localSap = localSap;
    auto request = new Request("OPEN", IEEE8022_LLC_C_OPEN);
    LlcSocketOpenCommand *command = new LlcSocketOpenCommand();
    command->setLocalSap(localSap);
    request->setControlInfo(command);
    isOpen = true;
    sendToLlc(request);
}

void LlcSocket::send(Packet *packet)
{
    if (! isOpen)
        throw cRuntimeError("Socket is closed");
    sendToLlc(packet);
}

void LlcSocket::close()
{
    auto request = new Request("CLOSE", IEEE8022_LLC_C_CLOSE);
    LlcSocketCloseCommand *command = new LlcSocketCloseCommand();
    request->setControlInfo(command);
    sendToLlc(request);
    interfaceId = -1;
    isOpen = false;
}

void LlcSocket::sendToLlc(cMessage *msg)
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


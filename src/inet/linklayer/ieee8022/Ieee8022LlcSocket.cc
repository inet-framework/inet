//
// Copyright (C) 2018 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/linklayer/ieee8022/Ieee8022LlcSocket.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/linklayer/common/Ieee802SapTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcSocketCommand_m.h"

namespace inet {

void Ieee8022LlcSocket::sendOut(cMessage *msg)
{
    auto& tags = check_and_cast<ITaggedObject *>(msg)->getTags();
    tags.addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ieee8022llc);
    tags.addTagIfAbsent<Ieee802SapReq>()->setSsap(localSap);
    if (interfaceId != -1)
        tags.addTagIfAbsent<InterfaceReq>()->setInterfaceId(interfaceId);
    SocketBase::sendOut(msg);
}

void Ieee8022LlcSocket::open(int interfaceId, int localSap)
{
    if (localSap < -1 || localSap > 255)
        throw cRuntimeError("LlcSocket::open(): Invalid localSap value: %d", localSap);
    this->interfaceId = interfaceId;
    this->localSap = localSap;
    auto request = new Request("LLC_OPEN", SOCKET_C_OPEN);
    Ieee8022LlcSocketOpenCommand *command = new Ieee8022LlcSocketOpenCommand();
    command->setLocalSap(localSap);
    request->setControlInfo(command);
    isOpen_ = true;
    sendOut(request);
}

void Ieee8022LlcSocket::processMessage(cMessage *msg)
{
    ASSERT(belongsToSocket(msg));
    switch (msg->getKind()) {
        case SOCKET_I_DATA:
            if (callback)
                callback->socketDataArrived(this, check_and_cast<Packet*>(msg));
            else
                delete msg;
            break;
        case SOCKET_I_CLOSED:
            isOpen_ = false;
            if (callback)
                callback->socketClosed(this);
            delete msg;
            break;
        default:
            throw cRuntimeError("Ieee8022LlcSocket: invalid msg kind %d, one of the SOCKET_I_xxx constants expected", msg->getKind());
            break;
    }
}

} // namespace inet


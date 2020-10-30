//
// Copyright (C) 2020 OpenSim Ltd.
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

#include "inet/common/socket/SocketTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/ieee8021q/Ieee8021qCommand_m.h"
#include "inet/linklayer/ieee8021q/Ieee8021qSocket.h"

namespace inet {

Ieee8021qSocket::Ieee8021qSocket()
{
    // don't allow user-specified socketIds because they may conflict with
    // automatically assigned ones.
    socketId = getEnvir()->getUniqueNumber();
    gateToIeee8021q = nullptr;
}

void Ieee8021qSocket::sendToIeee8021q(cMessage *msg)
{
    if (!gateToIeee8021q)
        throw cRuntimeError("Ieee8021qSocket: setOutputGate() must be invoked before socket can be used");

    cObject *ctrl = msg->getControlInfo();
    EV_TRACE << "Ieee8021qSocket: Send (" << msg->getClassName() << ")" << msg->getFullName();
    if (ctrl)
        EV_TRACE << "  control info: (" << ctrl->getClassName() << ")" << ctrl->getFullName();
    EV_TRACE << endl;

    auto& tags = check_and_cast<ITaggedObject *>(msg)->getTags();
    tags.addTagIfAbsent<InterfaceReq>()->setInterfaceId(networkInterface->getInterfaceId());
    tags.addTagIfAbsent<SocketReq>()->setSocketId(socketId);
    if (tags.findTag<DispatchProtocolReq>() == nullptr)
        tags.addTag<DispatchProtocolReq>()->setProtocol(protocol);
    check_and_cast<cSimpleModule *>(gateToIeee8021q->getOwnerModule())->send(msg, gateToIeee8021q);
}

void Ieee8021qSocket::bind(const Protocol *protocol, int vlanId, bool steal)
{
    auto request = new Request("BIND", IEEE8021Q_C_BIND);
    Ieee8021qBindCommand *ctrl = new Ieee8021qBindCommand();
    ctrl->setProtocol(protocol);
    ctrl->setVlanId(vlanId);
    ctrl->setSteal(steal);
    request->setControlInfo(ctrl);
    isOpen_ = true;
    sendToIeee8021q(request);
}

void Ieee8021qSocket::send(Packet *packet)
{
    packet->setKind(IEEE8021Q_C_DATA);
    isOpen_ = true;
    sendToIeee8021q(packet);
}

void Ieee8021qSocket::close()
{
    auto request = new Request("CLOSE", IEEE8021Q_C_CLOSE);
    auto *ctrl = new Ieee8021qCloseCommand();
    request->setControlInfo(ctrl);
    isOpen_ = true;
    sendToIeee8021q(request);
}

void Ieee8021qSocket::destroy()
{
    auto request = new Request("destroy", IEEE8021Q_C_DESTROY);
    auto *ctrl = new Ieee8021qDestroyCommand();
    request->setControlInfo(ctrl);
    sendToIeee8021q(request);
    isOpen_ = false;
}

void Ieee8021qSocket::setCallback(ICallback *callback)
{
    this->callback = callback;
}

bool Ieee8021qSocket::belongsToSocket(cMessage *msg) const
{
    auto& tags = check_and_cast<ITaggedObject *>(msg)->getTags();
    const auto& socketInd = tags.findTag<SocketInd>();
    return socketInd != nullptr && socketInd->getSocketId() == socketId;
}

void Ieee8021qSocket::processMessage(cMessage *msg)
{
    ASSERT(belongsToSocket(msg));
    switch (msg->getKind()) {
        case IEEE8021Q_I_DATA:
            if (callback)
                callback->socketDataArrived(this, check_and_cast<Packet *>(msg));
            else
                delete msg;
            break;
        case IEEE8021Q_I_SOCKET_CLOSED:
            isOpen_ = false;
            if (callback)
                callback->socketClosed(this);
            delete msg;
            break;
        default:
            throw cRuntimeError("Ieee8021qSocket: invalid msg kind %d, one of the ETHERNNET_I_xxx constants expected", msg->getKind());
            break;
    }
}

} // namespace inet


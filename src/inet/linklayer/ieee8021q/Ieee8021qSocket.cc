//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8021q/Ieee8021qSocket.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/ieee8021q/Ieee8021qCommand_m.h"

namespace inet {

void Ieee8021qSocket::sendOut(cMessage *msg)
{
    auto& tags = check_and_cast<ITaggedObject *>(msg)->getTags();
    tags.addTagIfAbsent<InterfaceReq>()->setInterfaceId(networkInterface->getInterfaceId());
    if (tags.findTag<DispatchProtocolReq>() == nullptr)
        tags.addTag<DispatchProtocolReq>()->setProtocol(protocol);
    SocketBase::sendOut(msg);
}

void Ieee8021qSocket::bind(const Protocol *protocol, int vlanId, bool steal)
{
    auto request = new Request("BIND", SOCKET_C_BIND);
    Ieee8021qBindCommand *ctrl = new Ieee8021qBindCommand();
    ctrl->setProtocol(protocol);
    ctrl->setVlanId(vlanId);
    ctrl->setSteal(steal);
    request->setControlInfo(ctrl);
    isOpen_ = true;
    sendOut(request);
}

void Ieee8021qSocket::processMessage(cMessage *msg)
{
    ASSERT(belongsToSocket(msg));
    switch (msg->getKind()) {
        case SOCKET_I_DATA:
            if (callback)
                callback->socketDataArrived(this, check_and_cast<Packet *>(msg));
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
            throw cRuntimeError("Ieee8021qSocket: invalid msg kind %d, one of the ETHERNNET_I_xxx constants expected", msg->getKind());
            break;
    }
}

} // namespace inet


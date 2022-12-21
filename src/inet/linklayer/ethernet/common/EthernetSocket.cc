//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/common/EthernetSocket.h"

#include "inet/common/packet/Message.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/ProtocolUtils.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/ethernet/common/EthernetCommand_m.h"

namespace inet {

void EthernetSocket::sendOut(cMessage *msg)
{
    auto& tags = check_and_cast<ITaggedObject *>(msg)->getTags();
    if (networkInterface != nullptr)
        tags.addTagIfAbsent<InterfaceReq>()->setInterfaceId(networkInterface->getInterfaceId());
    SocketBase::sendOut(msg);
}

void EthernetSocket::sendOut(Request *request)
{
    request->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ethernetMac);
    SocketBase::sendOut(request);
}

void EthernetSocket::sendOut(Packet *packet)
{
    appendEncapsulationProtocolReq(packet, &Protocol::ethernetMac);
    setDispatchProtocol(packet);
    SocketBase::sendOut(packet);
}

void EthernetSocket::bind(const MacAddress& localAddress, const MacAddress& remoteAddress, const Protocol *protocol, bool steal)
{
    auto request = new Request("BIND", SOCKET_C_BIND);
    EthernetBindCommand *ctrl = new EthernetBindCommand();
    ctrl->setLocalAddress(localAddress);
    ctrl->setRemoteAddress(remoteAddress);
    ctrl->setProtocol(protocol);
    ctrl->setSteal(steal);
    request->setControlInfo(ctrl);
    isOpen_ = true;
    sendOut(request);
}

void EthernetSocket::processMessage(cMessage *msg)
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
            throw cRuntimeError("EthernetSocket: invalid msg kind %d, one of the ETHERNNET_I_xxx constants expected", msg->getKind());
            break;
    }
}

} // namespace inet


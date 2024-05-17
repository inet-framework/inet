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
    if (networkInterface != nullptr)
        packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(networkInterface->getInterfaceId());
    packet->addTagIfAbsent<SocketReq>()->setSocketId(socketId);
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ethernetMac);
    sink.pushPacket(packet);
}

void EthernetSocket::bind(const MacAddress& localAddress, const MacAddress& remoteAddress, const Protocol *protocol, bool steal)
{
    isOpen_ = true;
    ethernet->bind(socketId, networkInterface->getInterfaceId(), localAddress, remoteAddress, protocol, steal);
}

void EthernetSocket::processMessage(cMessage *msg)
{
    ASSERT(belongsToSocket(msg));
    switch (msg->getKind()) {
        case SOCKET_I_DATA:
            if (callback) {
                auto packet = check_and_cast<Packet *>(msg);
                EV_INFO << "Receiving packet on socket" << EV_FIELD(socketId) << EV_FIELD(packet) << EV_ENDL;
                callback->socketDataArrived(this, packet);
            }
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


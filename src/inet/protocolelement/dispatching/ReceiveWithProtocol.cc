//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/dispatching/ReceiveWithProtocol.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/protocolelement/dispatching/ProtocolHeader_m.h"

namespace inet {

Define_Module(ReceiveWithProtocol);

void ReceiveWithProtocol::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    auto header = packet->popAtFront<ProtocolHeader>();
    auto protocol = Protocol::findProtocol(header->getProtocolId());
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(protocol);
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(protocol);
}

} // namespace inet


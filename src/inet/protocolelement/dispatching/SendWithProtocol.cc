//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/dispatching/SendWithProtocol.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/protocolelement/dispatching/ProtocolHeader_m.h"

namespace inet {

Define_Module(SendWithProtocol);

void SendWithProtocol::processPacket(Packet *packet)
{
    auto protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
    auto header = makeShared<ProtocolHeader>();
    header->setProtocolId(protocol->getId());
    packet->insertAtFront(header);
}

} // namespace inet


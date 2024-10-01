//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/forwarding/ReceiveWithHopLimit.h"

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"
#include "inet/protocolelement/forwarding/HopLimitHeader_m.h"

namespace inet {

Define_Module(ReceiveWithHopLimit);

void ReceiveWithHopLimit::processPacket(Packet *packet)
{
    auto header = packet->popAtFront<HopLimitHeader>();
    packet->popAtFront<HopLimitHeader>();
    packet->addTag<HopLimitInd>()->setHopLimit(header->getHopLimit());
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&AccessoryProtocol::forwarding);
}

bool ReceiveWithHopLimit::matchesPacket(const Packet *packet) const
{
    auto header = packet->peekAtFront<HopLimitHeader>();
    return header->getHopLimit() > 0;
}

} // namespace inet


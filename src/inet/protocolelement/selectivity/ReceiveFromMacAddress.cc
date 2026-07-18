//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/selectivity/ReceiveFromMacAddress.h"

#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/protocolelement/selectivity/SourceMacAddressHeader_m.h"

namespace inet {

Define_Module(ReceiveFromMacAddress);

void ReceiveFromMacAddress::processPacket(Packet *packet)
{
    auto header = packet->popAtFront<SourceMacAddressHeader>();
    packet->addTagIfAbsent<MacAddressInd>()->setSrcAddress(header->getSourceAddress());
}

} // namespace inet

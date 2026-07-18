//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/selectivity/SendFromMacAddress.h"

#include "inet/protocolelement/selectivity/SourceMacAddressHeader_m.h"

namespace inet {

Define_Module(SendFromMacAddress);

void SendFromMacAddress::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        address = MacAddress(par("address").stringValue());
}

void SendFromMacAddress::processPacket(Packet *packet)
{
    auto header = makeShared<SourceMacAddressHeader>();
    header->setSourceAddress(address);
    packet->insertAtFront(header);
}

} // namespace inet

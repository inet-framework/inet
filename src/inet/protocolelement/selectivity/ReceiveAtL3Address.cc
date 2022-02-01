//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/selectivity/ReceiveAtL3Address.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"
#include "inet/protocolelement/selectivity/DestinationL3AddressHeader_m.h"

namespace inet {

Define_Module(ReceiveAtL3Address);

void ReceiveAtL3Address::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        address = Ipv4Address(par("address").stringValue());
        registerService(AccessoryProtocol::destinationL3Address, nullptr, inputGate);
        registerProtocol(AccessoryProtocol::destinationL3Address, nullptr, outputGate);
    }
}

void ReceiveAtL3Address::processPacket(Packet *packet)
{
    packet->popAtFront<DestinationL3AddressHeader>();
}

bool ReceiveAtL3Address::matchesPacket(const Packet *packet) const
{
    auto header = packet->peekAtFront<DestinationL3AddressHeader>();
    return header->getDestinationAddress() == address;
}

} // namespace inet


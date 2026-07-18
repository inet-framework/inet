//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/selectivity/ReceiveAtMacAddress.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"
#include "inet/protocolelement/selectivity/DestinationMacAddressHeader_m.h"

namespace inet {

Define_Module(ReceiveAtMacAddress);

void ReceiveAtMacAddress::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        address = MacAddress(par("address").stringValue());
        registerService(AccessoryProtocol::destinationMacAddress, nullptr, inputGate);
        registerProtocol(AccessoryProtocol::destinationMacAddress, nullptr, outputGate);
    }
}

void ReceiveAtMacAddress::processPacket(Packet *packet)
{
    // pop the link-layer address header; the packet keeps whatever DispatchProtocolReq an earlier
    // element (e.g. ReceiveWithProtocol) set -- we do not force a downstream target here.
    packet->popAtFront<DestinationMacAddressHeader>();
}

bool ReceiveAtMacAddress::matchesPacket(const Packet *packet) const
{
    // accept frames addressed to this interface or broadcast; drop everything else (this is what
    // makes unicast work on a shared broadcast medium where every node hears every frame).
    auto destinationAddress = packet->peekAtFront<DestinationMacAddressHeader>()->getDestinationAddress();
    return destinationAddress == address || destinationAddress.isBroadcast();
}

} // namespace inet

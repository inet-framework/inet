//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/selectivity/ReceiveAtMacAddress.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"
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
        getContainingNicModule(this)->setMacAddress(address);
    }
}

void ReceiveAtMacAddress::processPacket(Packet *packet)
{
    packet->popAtFront<DestinationMacAddressHeader>();
    // KLUDGE
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&AccessoryProtocol::sequenceNumber);
}

bool ReceiveAtMacAddress::matchesPacket(const Packet *packet) const
{
    auto header = packet->peekAtFront<DestinationMacAddressHeader>();
    return header->getDestinationAddress() == address;
}

} // namespace inet


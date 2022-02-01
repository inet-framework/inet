//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/selectivity/SendToL3Address.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"
#include "inet/protocolelement/selectivity/DestinationL3AddressHeader_m.h"

namespace inet {

Define_Module(SendToL3Address);

void SendToL3Address::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        address = Ipv4Address(par("address").stringValue());
        registerService(AccessoryProtocol::destinationL3Address, inputGate, nullptr);
        registerProtocol(AccessoryProtocol::destinationL3Address, outputGate, nullptr);
    }
}

void SendToL3Address::processPacket(Packet *packet)
{
    auto header = makeShared<DestinationL3AddressHeader>();
    header->setDestinationAddress(address);
    packet->insertAtFront(header);
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&AccessoryProtocol::forwarding);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&AccessoryProtocol::destinationL3Address);
}

} // namespace inet


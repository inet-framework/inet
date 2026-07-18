//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/selectivity/SendFromL3Address.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"
#include "inet/protocolelement/selectivity/SourceL3AddressHeader_m.h"

namespace inet {

Define_Module(SendFromL3Address);

void SendFromL3Address::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        address = Ipv4Address(par("address").stringValue());
        registerService(AccessoryProtocol::sourceL3Address, inputGate, nullptr);
        registerProtocol(AccessoryProtocol::sourceL3Address, outputGate, nullptr);
    }
}

void SendFromL3Address::processPacket(Packet *packet)
{
    auto header = makeShared<SourceL3AddressHeader>();
    header->setSourceAddress(address);
    packet->insertAtFront(header);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&AccessoryProtocol::sourceL3Address);
}

} // namespace inet


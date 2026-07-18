//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/selectivity/ReceiveFromL3Address.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"
#include "inet/protocolelement/selectivity/SourceL3AddressHeader_m.h"

namespace inet {

Define_Module(ReceiveFromL3Address);

void ReceiveFromL3Address::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        registerService(AccessoryProtocol::sourceL3Address, nullptr, inputGate);
        registerProtocol(AccessoryProtocol::sourceL3Address, nullptr, outputGate);
    }
}

void ReceiveFromL3Address::processPacket(Packet *packet)
{
    auto header = packet->popAtFront<SourceL3AddressHeader>();
    packet->addTagIfAbsent<L3AddressInd>()->setSrcAddress(header->getSourceAddress());
}

} // namespace inet


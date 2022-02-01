//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/selectivity/ReceiveAtPort.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"
#include "inet/protocolelement/selectivity/DestinationPortHeader_m.h"

namespace inet {

Define_Module(ReceiveAtPort);

void ReceiveAtPort::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        port = par("port");
        registerService(AccessoryProtocol::destinationPort, nullptr, inputGate);
        registerProtocol(AccessoryProtocol::destinationPort, nullptr, outputGate);
    }
}

void ReceiveAtPort::processPacket(Packet *packet)
{
    packet->popAtFront<DestinationPortHeader>();
}

bool ReceiveAtPort::matchesPacket(const Packet *packet) const
{
    auto header = packet->peekAtFront<DestinationPortHeader>();
    return header->getDestinationPort() == port;
}

} // namespace inet


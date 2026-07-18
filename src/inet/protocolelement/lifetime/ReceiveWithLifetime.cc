//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/lifetime/ReceiveWithLifetime.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"
#include "inet/protocolelement/lifetime/LifetimeHeader_m.h"

namespace inet {

Define_Module(ReceiveWithLifetime);

void ReceiveWithLifetime::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        maxLifetime = par("maxLifetime");
        registerService(AccessoryProtocol::lifetime, nullptr, inputGate);
        registerProtocol(AccessoryProtocol::lifetime, nullptr, outputGate);
    }
}

void ReceiveWithLifetime::processPacket(Packet *packet)
{
    packet->popAtFront<LifetimeHeader>();
}

bool ReceiveWithLifetime::matchesPacket(const Packet *packet) const
{
    auto header = packet->peekAtFront<LifetimeHeader>();
    return simTime() - header->getCreationTime() <= maxLifetime;
}

} // namespace inet


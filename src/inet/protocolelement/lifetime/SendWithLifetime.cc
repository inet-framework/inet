//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/lifetime/SendWithLifetime.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"
#include "inet/protocolelement/lifetime/LifetimeHeader_m.h"

namespace inet {

Define_Module(SendWithLifetime);

void SendWithLifetime::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        registerService(AccessoryProtocol::lifetime, inputGate, nullptr);
        registerProtocol(AccessoryProtocol::lifetime, outputGate, nullptr);
    }
}

void SendWithLifetime::processPacket(Packet *packet)
{
    auto header = makeShared<LifetimeHeader>();
    header->setCreationTime(simTime());
    packet->insertAtFront(header);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&AccessoryProtocol::lifetime);
}

} // namespace inet


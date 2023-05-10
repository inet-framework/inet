//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/forwarding/SendWithHopLimit.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"
#include "inet/protocolelement/forwarding/HopLimitHeader_m.h"

namespace inet {

Define_Module(SendWithHopLimit);

void SendWithHopLimit::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        hopLimit = par("hopLimit");
        registerService(AccessoryProtocol::hopLimit, inputGate, nullptr);
        registerProtocol(AccessoryProtocol::hopLimit, outputGate, nullptr);
    }
}

void SendWithHopLimit::processPacket(Packet *packet)
{
    packet->removeTagIfPresent<DispatchProtocolReq>();
    auto header = makeShared<HopLimitHeader>();
    header->setHopLimit(hopLimit);
    packet->insertAtFront(header);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&AccessoryProtocol::hopLimit);
    pushOrSendPacket(packet, outputGate, consumer);
}

} // namespace inet


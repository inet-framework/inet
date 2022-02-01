//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/checksum/FcsHeaderInserter.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/protocolelement/checksum/header/FcsHeader_m.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"

namespace inet {

Define_Module(FcsHeaderInserter);

void FcsHeaderInserter::initialize(int stage)
{
    FcsInserterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        headerPosition = parseHeaderPosition(par("headerPosition"));
        registerService(AccessoryProtocol::fcs, inputGate, nullptr);
        registerProtocol(AccessoryProtocol::fcs, outputGate, nullptr);
    }
}

void FcsHeaderInserter::processPacket(Packet *packet)
{
    const auto& header = makeShared<FcsHeader>();
    auto fcs = computeFcs(packet, fcsMode);
    header->setFcs(fcs);
    header->setFcsMode(fcsMode);
    insertHeader<FcsHeader>(packet, header, headerPosition);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&AccessoryProtocol::fcs);
}

} // namespace inet


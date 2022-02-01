//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/checksum/CrcHeaderInserter.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/protocolelement/checksum/header/CrcHeader_m.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"

namespace inet {

Define_Module(CrcHeaderInserter);

void CrcHeaderInserter::initialize(int stage)
{
    CrcInserterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        headerPosition = parseHeaderPosition(par("headerPosition"));
        registerService(AccessoryProtocol::crc, inputGate, nullptr);
        registerProtocol(AccessoryProtocol::crc, outputGate, nullptr);
    }
}

void CrcHeaderInserter::processPacket(Packet *packet)
{
    const auto& header = makeShared<CrcHeader>();
    auto crc = computeCrc(packet, crcMode);
    header->setCrc(crc);
    header->setCrcMode(crcMode);
    insertHeader<CrcHeader>(packet, header, headerPosition);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&AccessoryProtocol::crc);
}

} // namespace inet


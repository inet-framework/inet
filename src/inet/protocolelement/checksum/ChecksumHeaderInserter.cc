//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/checksum/ChecksumHeaderInserter.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/protocolelement/checksum/header/ChecksumHeader_m.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"

namespace inet {

Define_Module(ChecksumHeaderInserter);

void ChecksumHeaderInserter::initialize(int stage)
{
    ChecksumInserterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        headerPosition = parseHeaderPosition(par("headerPosition"));
        registerService(AccessoryProtocol::checksum, inputGate, nullptr);
        registerProtocol(AccessoryProtocol::checksum, outputGate, nullptr);
    }
}

void ChecksumHeaderInserter::processPacket(Packet *packet)
{
    const auto& header = makeShared<ChecksumHeader>();
    auto checksum = computeChecksum(packet, checksumMode, checksumType);
    B checksumSize = B(getChecksumSizeInBytes(checksumType));
    header->setChecksum(checksum);
    header->setChunkLength(checksumSize);
    header->setChecksumMode(checksumMode);
    insertHeader<ChecksumHeader>(packet, header, headerPosition);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&AccessoryProtocol::checksum);
}

} // namespace inet


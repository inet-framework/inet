//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/checksum/base/ChecksumInserterBase.h"


namespace inet {

void ChecksumInserterBase::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        checksumType = parseChecksumType(par("checksumType"));
        checksumMode = parseChecksumMode(par("checksumMode"), true);
    }
}

uint64_t ChecksumInserterBase::computeDisabledChecksum(const Packet *packet) const
{
    return 0x0000;
}

uint64_t ChecksumInserterBase::computeDeclaredCorrectChecksum(const Packet *packet) const
{
    return 0xC00DC00D;
}

uint64_t ChecksumInserterBase::computeDeclaredIncorrectChecksum(const Packet *packet) const
{
    return 0xBAADBAAD;
}

uint64_t ChecksumInserterBase::computeComputedChecksum(const Packet *packet, ChecksumType checksumType) const
{
    return inet::computeChecksum(packet->peekDataAsBytes()->getBytes(), checksumType);
}

uint64_t ChecksumInserterBase::computeChecksum(const Packet *packet, ChecksumMode checksumMode, ChecksumType checksumType) const
{
    switch (checksumMode) {
        case CHECKSUM_DISABLED:
            return computeDisabledChecksum(packet);
        case CHECKSUM_DECLARED_CORRECT:
            return computeDeclaredCorrectChecksum(packet);
        case CHECKSUM_DECLARED_INCORRECT:
            return computeDeclaredIncorrectChecksum(packet);
        case CHECKSUM_COMPUTED:
            return computeComputedChecksum(packet, checksumType);
        default:
            throw cRuntimeError("Unknown checksum mode: %d", (int)checksumMode);
    }
}

} // namespace inet


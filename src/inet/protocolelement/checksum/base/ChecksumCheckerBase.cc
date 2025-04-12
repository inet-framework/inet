//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/checksum/base/ChecksumCheckerBase.h"


namespace inet {

void ChecksumCheckerBase::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        checksumType = parseChecksumType(par("checksumType"));
}

bool ChecksumCheckerBase::checkDisabledChecksum(const Packet *packet, uint64_t checksum) const
{
    if (checksum != 0x0000)
        throw cRuntimeError("Checksum value differs from expected");
    return true;
}

bool ChecksumCheckerBase::checkDeclaredCorrectChecksum(const Packet *packet, uint64_t checksum) const
{
    const auto& data = packet->peekData();
    if (checksum != 0xC00D && checksum != 0xC00DC00D)
        throw cRuntimeError("Checksum value differs from expected");
    return data->isCorrect() && !packet->hasBitError();
}

bool ChecksumCheckerBase::checkDeclaredIncorrectChecksum(const Packet *packet, uint64_t checksum) const
{
    if (checksum != 0xBAAD && checksum != 0xBAADBAAD)
        throw cRuntimeError("Checksum value differs from expected");
    return false;
}

bool ChecksumCheckerBase::checkComputedChecksum(const Packet *packet, ChecksumType checksumType, uint64_t receivedChecksum) const
{
    if (receivedChecksum == 0)
        return true; //TODO questionable
    else {
        const auto& data = packet->peekDataAsBytes();
        auto bytes = data->getBytes();
        size_t checksumSize = getChecksumSizeInBytes(checksumType);
        uint64_t computedChecksum = inet::computeChecksum(bytes.data(), packet->getByteLength() - checksumSize, checksumType);
        // NOTE: the correct bit must be checked, because the data may not be corrupted precisely depending on the corruption mode
        return receivedChecksum == computedChecksum && data->isCorrect() && !packet->hasBitError();
    }
}

bool ChecksumCheckerBase::checkChecksum(const Packet *packet, ChecksumMode checksumMode, ChecksumType checksumType, uint64_t checksum) const
{
    switch (checksumMode) {
        case CHECKSUM_DISABLED:
            return checkDisabledChecksum(packet, checksum);
        case CHECKSUM_DECLARED_CORRECT:
            return checkDeclaredCorrectChecksum(packet, checksum);
        case CHECKSUM_DECLARED_INCORRECT:
            return checkDeclaredIncorrectChecksum(packet, checksum);
        case CHECKSUM_COMPUTED:
            return checkComputedChecksum(packet, checksumType, checksum);
        default:
            throw cRuntimeError("Unknown checksum mode: %d", (int)checksumMode);
    }
}

} // namespace inet


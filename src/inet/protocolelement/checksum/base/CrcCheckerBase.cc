//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/checksum/base/CrcCheckerBase.h"

#include "inet/common/checksum/TcpIpChecksum.h"

namespace inet {

bool CrcCheckerBase::checkDisabledCrc(const Packet *packet, uint16_t crc) const
{
    if (crc != 0x0000)
        throw cRuntimeError("CRC value differs from expected");
    return true;
}

bool CrcCheckerBase::checkDeclaredCorrectCrc(const Packet *packet, uint16_t crc) const
{
    const auto& data = packet->peekData();
    if (crc != 0xC00D)
        throw cRuntimeError("CRC value differs from expected");
    return data->isCorrect() && !packet->hasBitError();
}

bool CrcCheckerBase::checkDeclaredIncorrectCrc(const Packet *packet, uint16_t crc) const
{
    if (crc != 0xBAAD)
        throw cRuntimeError("CRC value differs from expected");
    return false;
}

bool CrcCheckerBase::checkComputedCrc(const Packet *packet, uint16_t receivedCrc) const
{
    if (receivedCrc == 0x0000)
        return true;
    else {
        const auto& data = packet->peekDataAsBytes();
        auto bytes = data->getBytes();
        uint16_t computedCrc = TcpIpChecksum::checksum(bytes.data(), packet->getByteLength() - 2);
        // NOTE: the correct bit must be checked, because the data may not be corrupted precisely depending on the corruption mode
        return receivedCrc == computedCrc && data->isCorrect() && !packet->hasBitError();
    }
}

bool CrcCheckerBase::checkCrc(const Packet *packet, CrcMode crcMode, uint16_t crc) const
{
    switch (crcMode) {
        case CRC_DISABLED:
            return checkDisabledCrc(packet, crc);
        case CRC_DECLARED_CORRECT:
            return checkDeclaredCorrectCrc(packet, crc);
        case CRC_DECLARED_INCORRECT:
            return checkDeclaredIncorrectCrc(packet, crc);
        case CRC_COMPUTED:
            return checkComputedCrc(packet, crc);
        default:
            throw cRuntimeError("Unknown CRC mode: %d", (int)crcMode);
    }
}

} // namespace inet


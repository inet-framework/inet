//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/common/checksum/TcpIpChecksum.h"
#include "inet/protocol/checksum/base/CrcCheckerBase.h"

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
        uint16_t computedCrc = TcpIpChecksum::checksum(data->getBytes());
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


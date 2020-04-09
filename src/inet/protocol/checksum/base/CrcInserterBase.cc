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
#include "inet/protocol/checksum/base/CrcInserterBase.h"

namespace inet {

void CrcInserterBase::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        crcMode = parseCrcMode(par("crcMode"), true);
}

uint16_t CrcInserterBase::computeDisabledCrc(const Packet *packet) const
{
    return 0x0000;
}

uint16_t CrcInserterBase::computeDeclaredCorrectCrc(const Packet *packet) const
{
    return 0xC00D;
}

uint16_t CrcInserterBase::computeDeclaredIncorrectCrc(const Packet *packet) const
{
    return 0xBAAD;
}

uint16_t CrcInserterBase::computeComputedCrc(const Packet *packet) const
{
    auto data = packet->peekDataAsBytes();
    return TcpIpChecksum::checksum(data->getBytes());
}

uint16_t CrcInserterBase::computeCrc(const Packet *packet, CrcMode crcMode) const
{
    switch (crcMode) {
        case CRC_DISABLED:
            return computeDisabledCrc(packet);
        case CRC_DECLARED_CORRECT:
            return computeDeclaredCorrectCrc(packet);
        case CRC_DECLARED_INCORRECT:
            return computeDeclaredIncorrectCrc(packet);
        case CRC_COMPUTED:
            return computeComputedCrc(packet);
        default:
            throw cRuntimeError("Unknown CRC mode: %d", (int)crcMode);
    }
}

} // namespace inet


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

#include "inet/common/checksum/EthernetCRC.h"
#include "inet/protocol/checksum/base/FcsInserterBase.h"

namespace inet {

void FcsInserterBase::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        fcsMode = parseFcsMode(par("fcsMode"));
}

uint32_t FcsInserterBase::computeDisabledFcs(const Packet *packet) const
{
    return 0x00000000L;
}

uint32_t FcsInserterBase::computeDeclaredCorrectFcs(const Packet *packet) const
{
    return 0xC00DC00DL;
}

uint32_t FcsInserterBase::computeDeclaredIncorrectFcs(const Packet *packet) const
{
    return 0xBAADBAADL;
}

uint32_t FcsInserterBase::computeComputedFcs(const Packet *packet) const
{
    auto data = packet->peekDataAsBytes();
    auto bytes = data->getBytes();
    return ethernetCRC(bytes.data(), packet->getByteLength());
}

uint32_t FcsInserterBase::computeFcs(const Packet *packet, FcsMode fcsMode) const
{
    switch (fcsMode) {
        case FCS_DISABLED:
            return computeDisabledFcs(packet);
        case FCS_DECLARED_CORRECT:
            return computeDeclaredCorrectFcs(packet);
        case FCS_DECLARED_INCORRECT:
            return computeDeclaredIncorrectFcs(packet);
        case FCS_COMPUTED:
            return computeComputedFcs(packet);
        default:
            throw cRuntimeError("Unknown FCS mode: %d", (int)fcsMode);
    }
}

} // namespace inet


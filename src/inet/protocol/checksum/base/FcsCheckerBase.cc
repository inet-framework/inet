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
#include "inet/protocol/checksum/base/FcsCheckerBase.h"

namespace inet {

bool FcsCheckerBase::checkDisabledFcs(const Packet *packet, uint32_t fcs) const
{
    if (fcs != 0x00000000L)
        throw cRuntimeError("FCS value differs from expected");
    return true;
}

bool FcsCheckerBase::checkDeclaredCorrectFcs(const Packet *packet, uint32_t fcs) const
{
    const auto& data = packet->peekData();
    if (fcs != 0xC00DC00DL)
        throw cRuntimeError("FCS value differs from expected");
    return data->isCorrect() && !packet->hasBitError();
}

bool FcsCheckerBase::checkDeclaredIncorrectFcs(const Packet *packet, uint32_t fcs) const
{
    if (fcs != 0xBAADBAADL)
        throw cRuntimeError("FCS value differs from expected");
    return false;
}

bool FcsCheckerBase::checkComputedFcs(const Packet *packet, uint32_t receivedFcs) const
{
    // TODO: if (receivedFcs == 0x00000000L) -> disabled
    auto data = packet->peekDataAsBytes();
    auto bytes = data->getBytes();
    uint32_t computedFcs = ethernetCRC(bytes.data(), packet->getByteLength());
    // NOTE: the correct bit must be checked, because the data may not be corrupted precisely depending on the corruption mode
    return receivedFcs == computedFcs && data->isCorrect() && !packet->hasBitError();
}

bool FcsCheckerBase::checkFcs(const Packet *packet, FcsMode fcsMode, uint32_t fcs) const
{
    switch (fcsMode) {
        case FCS_DISABLED:
            return checkDisabledFcs(packet, fcs);
        case FCS_DECLARED_CORRECT:
            return checkDeclaredCorrectFcs(packet, fcs);
        case FCS_DECLARED_INCORRECT:
            return checkDeclaredIncorrectFcs(packet, fcs);
        case FCS_COMPUTED:
            return checkComputedFcs(packet, fcs);
        default:
            throw cRuntimeError("Unknown FCS mode: %d", (int)fcsMode);
    }
}

} // namespace inet


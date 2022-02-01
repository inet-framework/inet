//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/checksum/base/FcsCheckerBase.h"

#include "inet/common/checksum/EthernetCRC.h"

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
    // TODO if (receivedFcs == 0x00000000L) -> disabled
    auto data = packet->peekDataAsBytes();
    auto bytes = data->getBytes();
    uint32_t computedFcs = ethernetCRC(bytes.data(), packet->getByteLength() - 4);
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


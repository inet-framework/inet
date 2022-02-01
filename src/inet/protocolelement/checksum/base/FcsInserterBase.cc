//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/checksum/base/FcsInserterBase.h"

#include "inet/common/checksum/EthernetCRC.h"

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


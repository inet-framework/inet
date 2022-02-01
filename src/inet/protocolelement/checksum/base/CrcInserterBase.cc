//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/checksum/base/CrcInserterBase.h"

#include "inet/common/checksum/TcpIpChecksum.h"

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


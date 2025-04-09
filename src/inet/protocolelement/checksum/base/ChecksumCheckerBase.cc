//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/checksum/base/ChecksumCheckerBase.h"

#include "inet/common/checksum/TcpIpChecksum.h"
#include "inet/common/checksum/EthernetCRC.h"

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

        //TODO a similar switch occurs in ChecksumInserterBase too, could be factored out
        uint64_t computedChecksum;
        switch (checksumType) {
            case CHECKSUM_TYPE_UNDEFINED:
                throw cRuntimeError("Checksum type is undefined");
            case CHECKSUM_INTERNET:
                computedChecksum = TcpIpChecksum::checksum(bytes.data(), packet->getByteLength() - 2); //TODO assumes checksum is at the end and must be ignored, but e.g. position is configurable in ChecksumInserterBase
                break;
            case CHECKSUM_CRC32:
                computedChecksum = ethernetCRC(bytes.data(), packet->getByteLength() - 4);  //TODO assumes checksum is at the end and must be ignored, but e.g. position is configurable in ChecksumInserterBase
                break;
            case CHECKSUM_CRC8:
            case CHECKSUM_CRC16_IBM:
            case CHECKSUM_CRC16_CCITT:
            case CHECKSUM_CRC64:
                throw cRuntimeError("Unsupported checksum type: %d", (int)checksumType);
            default:
                throw cRuntimeError("Unknown checksum type: %d", (int)checksumType);
        }

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


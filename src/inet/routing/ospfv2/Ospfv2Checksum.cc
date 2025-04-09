//
// Copyright (C) 2019 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/routing/ospfv2/Ospfv2Checksum.h"

#include "inet/common/checksum/TcpIpChecksum.h"
#include "inet/routing/ospfv2/Ospfv2PacketSerializer.h"
#include "inet/routing/ospfv2/router/Ospfv2Common.h"

namespace inet {
namespace ospfv2 {

void setOspfChecksum(const Ptr<Ospfv2Packet>& ospfPacket, ChecksumMode checksumMode)
{
    ospfPacket->setChecksumMode(checksumMode);
    switch (checksumMode) {
        case ChecksumMode::CHECKSUM_DECLARED_CORRECT:
            ospfPacket->setChecksum(0xC00D);
            break;
        case ChecksumMode::CHECKSUM_DECLARED_INCORRECT:
            ospfPacket->setChecksum(0xBAAD);
            break;
        case ChecksumMode::CHECKSUM_COMPUTED: {
            AuthenticationKeyType authenticationKey;
            ospfPacket->setChecksum(0);
            // RFC 2328: OSPF checksum is calculated over the entire OSPF packet, excluding the 64-bit authentication field.
            for (int i = 0; i < 8; i++) {
                authenticationKey.bytes[i] = ospfPacket->getAuthentication(i);
                ospfPacket->setAuthentication(i, 0);
            }
            MemoryOutputStream stream;
            Chunk::serialize(stream, ospfPacket);
            uint16_t checksum = TcpIpChecksum::checksum(stream.getData());
            for (int i = 0; i < 8; i++) {
                ospfPacket->setAuthentication(i, authenticationKey.bytes[i]);
            }
            ospfPacket->setChecksum(checksum);
            break;
        }
        default:
            throw cRuntimeError("Unknown CHECKSUM mode: %d", (int)checksumMode);
    }
}

void setLsaChecksum(Ospfv2Lsa& lsa, ChecksumMode checksumMode)
{
    auto& lsaHeader = lsa.getHeaderForUpdate();
    lsaHeader.setLsChecksumMode(checksumMode);
    switch (checksumMode) {
        case ChecksumMode::CHECKSUM_DECLARED_CORRECT:
            lsaHeader.setLsChecksum(0xC00D);
            break;
        case ChecksumMode::CHECKSUM_DECLARED_INCORRECT:
            lsaHeader.setLsChecksum(0xBAAD);
            break;
        case ChecksumMode::CHECKSUM_COMPUTED: {
            lsaHeader.setLsChecksum(0);
            MemoryOutputStream stream;
            auto lsAge = lsaHeader.getLsAge();
            lsaHeader.setLsAge(0); // disable lsAge from CHECKSUM
            Ospfv2PacketSerializer::serializeLsa(stream, lsa);
            uint16_t checksum = TcpIpChecksum::checksum(stream.getData());
            lsaHeader.setLsAge(lsAge); // restore lsAge
            lsaHeader.setLsChecksum(checksum);
            break;
        }
        default:
            throw cRuntimeError("Unknown CHECKSUM mode: %d", (int)checksumMode);
    }
}

void setLsaHeaderChecksum(Ospfv2LsaHeader& lsaHeader, ChecksumMode checksumMode)
{
    lsaHeader.setLsChecksumMode(checksumMode);
    switch (checksumMode) {
        case ChecksumMode::CHECKSUM_DECLARED_CORRECT:
            lsaHeader.setLsChecksum(0xC00D);
            break;
        case ChecksumMode::CHECKSUM_DECLARED_INCORRECT:
            lsaHeader.setLsChecksum(0xBAAD);
            break;
        case ChecksumMode::CHECKSUM_COMPUTED: {
            lsaHeader.setLsChecksum(0);
            MemoryOutputStream stream;
            auto lsAge = lsaHeader.getLsAge();
            lsaHeader.setLsAge(0); // disable lsAge from CHECKSUM
            Ospfv2PacketSerializer::serializeLsaHeader(stream, lsaHeader);
            uint16_t checksum = TcpIpChecksum::checksum(stream.getData());
            lsaHeader.setLsAge(lsAge); // restore lsAge
            lsaHeader.setLsChecksum(checksum);
            break;
        }
        default:
            throw cRuntimeError("Unknown CHECKSUM mode: %d", (int)checksumMode);
    }
}

} // namespace ospfv2
} // namespace inet


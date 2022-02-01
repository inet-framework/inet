//
// Copyright (C) 2019 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/routing/ospfv2/Ospfv2Crc.h"

#include "inet/common/checksum/TcpIpChecksum.h"
#include "inet/routing/ospfv2/Ospfv2PacketSerializer.h"
#include "inet/routing/ospfv2/router/Ospfv2Common.h"

namespace inet {
namespace ospfv2 {

void setOspfCrc(const Ptr<Ospfv2Packet>& ospfPacket, CrcMode crcMode)
{
    ospfPacket->setCrcMode(crcMode);
    switch (crcMode) {
        case CrcMode::CRC_DECLARED_CORRECT:
            ospfPacket->setCrc(0xC00D);
            break;
        case CrcMode::CRC_DECLARED_INCORRECT:
            ospfPacket->setCrc(0xBAAD);
            break;
        case CrcMode::CRC_COMPUTED: {
            AuthenticationKeyType authenticationKey;
            ospfPacket->setCrc(0);
            // RFC 2328: OSPF checksum is calculated over the entire OSPF packet, excluding the 64-bit authentication field.
            for (int i = 0; i < 8; i++) {
                authenticationKey.bytes[i] = ospfPacket->getAuthentication(i);
                ospfPacket->setAuthentication(i, 0);
            }
            MemoryOutputStream stream;
            Chunk::serialize(stream, ospfPacket);
            uint16_t crc = TcpIpChecksum::checksum(stream.getData());
            for (int i = 0; i < 8; i++) {
                ospfPacket->setAuthentication(i, authenticationKey.bytes[i]);
            }
            ospfPacket->setCrc(crc);
            break;
        }
        default:
            throw cRuntimeError("Unknown CRC mode: %d", (int)crcMode);
    }
}

void setLsaCrc(Ospfv2Lsa& lsa, CrcMode crcMode)
{
    auto& lsaHeader = lsa.getHeaderForUpdate();
    lsaHeader.setLsCrcMode(crcMode);
    switch (crcMode) {
        case CrcMode::CRC_DECLARED_CORRECT:
            lsaHeader.setLsCrc(0xC00D);
            break;
        case CrcMode::CRC_DECLARED_INCORRECT:
            lsaHeader.setLsCrc(0xBAAD);
            break;
        case CrcMode::CRC_COMPUTED: {
            lsaHeader.setLsCrc(0);
            MemoryOutputStream stream;
            auto lsAge = lsaHeader.getLsAge();
            lsaHeader.setLsAge(0); // disable lsAge from CRC
            Ospfv2PacketSerializer::serializeLsa(stream, lsa);
            uint16_t crc = TcpIpChecksum::checksum(stream.getData());
            lsaHeader.setLsAge(lsAge); // restore lsAge
            lsaHeader.setLsCrc(crc);
            break;
        }
        default:
            throw cRuntimeError("Unknown CRC mode: %d", (int)crcMode);
    }
}

void setLsaHeaderCrc(Ospfv2LsaHeader& lsaHeader, CrcMode crcMode)
{
    lsaHeader.setLsCrcMode(crcMode);
    switch (crcMode) {
        case CrcMode::CRC_DECLARED_CORRECT:
            lsaHeader.setLsCrc(0xC00D);
            break;
        case CrcMode::CRC_DECLARED_INCORRECT:
            lsaHeader.setLsCrc(0xBAAD);
            break;
        case CrcMode::CRC_COMPUTED: {
            lsaHeader.setLsCrc(0);
            MemoryOutputStream stream;
            auto lsAge = lsaHeader.getLsAge();
            lsaHeader.setLsAge(0); // disable lsAge from CRC
            Ospfv2PacketSerializer::serializeLsaHeader(stream, lsaHeader);
            uint16_t crc = TcpIpChecksum::checksum(stream.getData());
            lsaHeader.setLsAge(lsAge); // restore lsAge
            lsaHeader.setLsCrc(crc);
            break;
        }
        default:
            throw cRuntimeError("Unknown CRC mode: %d", (int)crcMode);
    }
}

} // namespace ospfv2
} // namespace inet


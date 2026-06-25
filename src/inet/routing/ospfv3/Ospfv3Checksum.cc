//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/routing/ospfv3/Ospfv3Checksum.h"

#include "inet/common/MemoryOutputStream.h"
#include "inet/common/checksum/Checksum.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/routing/ospfv3/Ospfv3PacketSerializer.h"

namespace inet {
namespace ospfv3 {

void setOspfChecksum(const Ptr<Ospfv3Packet>& ospfPacket, ChecksumMode checksumMode,
        const Ipv6Address& srcAddress, const Ipv6Address& destAddress)
{
    ospfPacket->setChecksumMode(checksumMode);
    switch (checksumMode) {
        case CHECKSUM_DECLARED_CORRECT:
            ospfPacket->setChecksum(0xC00D);
            break;
        case CHECKSUM_DECLARED_INCORRECT:
            ospfPacket->setChecksum(0xBAAD);
            break;
        case CHECKSUM_COMPUTED: {
            ospfPacket->setChecksum(0);
            MemoryOutputStream ospfStream;
            Chunk::serialize(ospfStream, ospfPacket);
            const auto& ospfBytes = ospfStream.getData();
            // RFC 5340 2.5: the checksum is the standard Internet checksum over the IPv6
            // upper-layer pseudo-header followed by the OSPF packet (Checksum field = 0).
            MemoryOutputStream stream;
            stream.writeIpv6Address(srcAddress);
            stream.writeIpv6Address(destAddress);
            stream.writeUint32Be(ospfBytes.size());
            stream.writeUint16Be(0);
            stream.writeByte(0);
            stream.writeByte(IP_PROT_OSPF);
            stream.writeBytes(ospfBytes);
            ospfPacket->setChecksum(internetChecksum(stream.getData()));
            break;
        }
        default:
            throw cRuntimeError("Unknown CHECKSUM mode: %d", (int)checksumMode);
    }
}

void setLsaChecksum(Ospfv3Lsa& lsa, ChecksumMode checksumMode)
{
    auto& lsaHeader = lsa.getHeaderForUpdate();
    lsaHeader.setLsChecksumMode(checksumMode);
    switch (checksumMode) {
        case CHECKSUM_DECLARED_CORRECT:
            lsaHeader.setLsaChecksum(0xC00D);
            break;
        case CHECKSUM_DECLARED_INCORRECT:
            lsaHeader.setLsaChecksum(0xBAAD);
            break;
        case CHECKSUM_COMPUTED: {
            // RFC 2328 12.1.7: Fletcher checksum of the LSA contents excepting the LS Age field.
            lsaHeader.setLsaChecksum(0);
            MemoryOutputStream stream;
            Ospfv3PacketSerializer::serializeLsa(stream, lsa);
            const auto& bytes = stream.getData();
            // Checksummed region runs from the LS Type field (offset 2, skipping the 2-octet LS
            // Age) to the end; the two checksum octets sit at LSA offset 16, i.e. region offset 14.
            uint16_t checksum = fletcherChecksum(bytes.data() + 2, bytes.size() - 2, 14);
            lsaHeader.setLsaChecksum(checksum);
            break;
        }
        default:
            throw cRuntimeError("Unknown CHECKSUM mode: %d", (int)checksumMode);
    }
}

} // namespace ospfv3
} // namespace inet

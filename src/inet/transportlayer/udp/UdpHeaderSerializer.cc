//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/udp/UdpHeaderSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"

namespace inet {

Register_Serializer(UdpHeader, UdpHeaderSerializer);

void UdpHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& udpHeader = staticPtrCast<const UdpHeader>(chunk);
    stream.writeUint16Be(udpHeader->getSourcePort());
    stream.writeUint16Be(udpHeader->getDestinationPort());
    stream.writeUint16Be(udpHeader->getTotalLengthField().get<B>());
    auto checksumMode = udpHeader->getChecksumMode();
    if (checksumMode != CHECKSUM_DISABLED && checksumMode != CHECKSUM_COMPUTED)
        throw cRuntimeError("Cannot serialize UDP header without turned off or properly computed checksum, try changing the value of checksumMode parameter for Udp");
    stream.writeUint16Be(udpHeader->getChecksum());
}

const Ptr<Chunk> UdpHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto udpHeader = makeShared<UdpHeader>();
    udpHeader->setSourcePort(stream.readUint16Be());
    udpHeader->setDestinationPort(stream.readUint16Be());
    udpHeader->setTotalLengthField(B(stream.readUint16Be()));
    auto checksum = stream.readUint16Be();
    udpHeader->setChecksum(checksum);
    udpHeader->setChecksumMode(checksum == 0 ? CHECKSUM_DISABLED : CHECKSUM_COMPUTED);
    return udpHeader;
}

} // namespace inet


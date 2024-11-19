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
    auto crcMode = udpHeader->getCrcMode();
    if (crcMode != CRC_DISABLED && crcMode != CRC_COMPUTED)
        throw cRuntimeError("Cannot serialize UDP header without turned off or properly computed CRC, try changing the value of crcMode parameter for Udp");
    stream.writeUint16Be(udpHeader->getCrc());
}

const Ptr<Chunk> UdpHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto udpHeader = makeShared<UdpHeader>();
    udpHeader->setSourcePort(stream.readUint16Be());
    udpHeader->setDestinationPort(stream.readUint16Be());
    udpHeader->setTotalLengthField(B(stream.readUint16Be()));
    auto crc = stream.readUint16Be();
    udpHeader->setCrc(crc);
    udpHeader->setCrcMode(crc == 0 ? CRC_DISABLED : CRC_COMPUTED);
    return udpHeader;
}

} // namespace inet


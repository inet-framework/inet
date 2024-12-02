//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/linklayer/ieee802154/Ieee802154MacHeaderSerializer.h"
#include "inet/linklayer/ieee802154/Ieee802154MacHeader_m.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"

namespace inet {

Register_Serializer(Ieee802154MacHeader, Ieee802154MacHeaderSerializer);

void Ieee802154MacHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& header = staticPtrCast<const Ieee802154MacHeader>(chunk);

    // Frame Control Field (2 bytes)
    uint16_t frameControl = 0;
    frameControl |= 0x0001;  // Frame type = data frame
    frameControl |= 0x0C00;  // Dest addressing mode = 11 (64-bit extended)
    frameControl |= 0xC000;  // Source addressing mode = 11 (64-bit extended)
    stream.writeUint16Le(frameControl);

    // Sequence Number (1 byte)
    stream.writeUint8(header->getSequenceId() & 0xFF);

    // Addressing Fields
    // Destination PAN ID (2 bytes)
    stream.writeUint16Le(0xFFFF);  // Broadcast PAN ID

    // Destination Address (8 bytes)
    auto destAddr = header->getDestAddr();
    stream.writeUint48Le(destAddr.getInt());  // Writes all 6 bytes
    stream.writeUint16Le(0);  // Padding to 8 bytes

    // Source PAN ID (2 bytes)
    stream.writeUint16Le(0xFFFF);  // Broadcast PAN ID

    // Source Address (8 bytes)
    auto srcAddr = header->getSrcAddr();
    stream.writeUint48Le(srcAddr.getInt());  // Writes all 6 bytes
    stream.writeUint16Le(0);  // Padding to 8 bytes
}

const Ptr<Chunk> Ieee802154MacHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto header = makeShared<Ieee802154MacHeader>();

    // Frame Control Field (2 bytes)
    uint16_t frameControl = stream.readUint16Le();
    // We don't need to parse all frame control bits for our internal format
    ASSERT(frameControl == 0xCC01);

    // Sequence Number (1 byte)
    header->setSequenceId(stream.readUint8());

    // Skip Destination PAN ID (2 bytes)
    stream.readUint16Le();

    // Destination Address (8 bytes)
    MacAddress destAddr(stream.readUint48Le());  // Reads 6 bytes
    stream.readUint16Le();  // Skip padding
    header->setDestAddr(destAddr);

    // Skip Source PAN ID (2 bytes)
    stream.readUint16Le();

    // Source Address (8 bytes)
    MacAddress srcAddr(stream.readUint48Le());  // Reads 6 bytes
    stream.readUint16Le();  // Skip padding
    header->setSrcAddr(srcAddr);

    // Set default network protocol
    header->setNetworkProtocol(-1);

    return header;
}

} // namespace inet

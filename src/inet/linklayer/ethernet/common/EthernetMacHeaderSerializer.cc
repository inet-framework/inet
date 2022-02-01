//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/common/EthernetMacHeaderSerializer.h"

#include <algorithm>

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/linklayer/ethernet/common/EthernetControlFrame_m.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"

namespace inet {

Register_Serializer(EthernetMacAddressFields, EthernetMacAddressFieldsSerializer);
Register_Serializer(EthernetTypeOrLengthField, EthernetTypeOrLengthFieldSerializer);

Register_Serializer(EthernetMacHeader, EthernetMacHeaderSerializer);

Register_Serializer(EthernetPadding, EthernetPaddingSerializer);

Register_Serializer(EthernetFcs, EthernetFcsSerializer);
Register_Serializer(EthernetFragmentFcs, EthernetFcsSerializer);

void EthernetMacAddressFieldsSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& header = staticPtrCast<const EthernetMacAddressFields>(chunk);
    stream.writeMacAddress(header->getDest());
    stream.writeMacAddress(header->getSrc());
}

const Ptr<Chunk> EthernetMacAddressFieldsSerializer::deserialize(MemoryInputStream& stream) const
{
    auto header = makeShared<EthernetMacAddressFields>();
    header->setDest(stream.readMacAddress());
    header->setSrc(stream.readMacAddress());
    return header;
}

void EthernetTypeOrLengthFieldSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& header = staticPtrCast<const EthernetTypeOrLengthField>(chunk);
    stream.writeUint16Be(header->getTypeOrLength());
}

const Ptr<Chunk> EthernetTypeOrLengthFieldSerializer::deserialize(MemoryInputStream& stream) const
{
    auto header = makeShared<EthernetTypeOrLengthField>();
    header->setTypeOrLength(stream.readUint16Be());
    return header;
}

void EthernetMacHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& ethernetMacHeader = staticPtrCast<const EthernetMacHeader>(chunk);
    stream.writeMacAddress(ethernetMacHeader->getDest());
    stream.writeMacAddress(ethernetMacHeader->getSrc());
    stream.writeUint16Be(ethernetMacHeader->getTypeOrLength());
}

const Ptr<Chunk> EthernetMacHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    Ptr<EthernetMacHeader> ethernetMacHeader = makeShared<EthernetMacHeader>();
    ethernetMacHeader->setDest(stream.readMacAddress());
    ethernetMacHeader->setSrc(stream.readMacAddress());
    ethernetMacHeader->setTypeOrLength(stream.readUint16Be());
    return ethernetMacHeader;
}

void EthernetPaddingSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    stream.writeByteRepeatedly(0, B(chunk->getChunkLength()).get());
}

const Ptr<Chunk> EthernetPaddingSerializer::deserialize(MemoryInputStream& stream) const
{
    throw cRuntimeError("Invalid operation");
}

void EthernetFcsSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& ethernetFcs = staticPtrCast<const EthernetFcs>(chunk);
    if (ethernetFcs->getFcsMode() != FCS_COMPUTED)
        throw cRuntimeError("Cannot serialize Ethernet FCS without a properly computed FCS");
    stream.writeUint32Be(ethernetFcs->getFcs());
}

const Ptr<Chunk> EthernetFcsSerializer::deserialize(MemoryInputStream& stream) const
{
    auto ethernetFcs = makeShared<EthernetFcs>();
    ethernetFcs->setFcs(stream.readUint32Be());
    ethernetFcs->setFcsMode(FCS_COMPUTED);
    return ethernetFcs;
}

} // namespace inet


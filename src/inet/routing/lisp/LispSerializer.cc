//
// Copyright (C) 2013 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// LISP (Locator/ID Separation Protocol, RFC 6830) ported from the ANSAINET project.
// Original author: Vladimir Vesely (Brno University of Technology).
//

#include "inet/routing/lisp/LispSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/routing/lisp/LispMessages_m.h"

namespace inet {
namespace lisp {

Register_Serializer(LispHeader, LispHeaderSerializer);

void LispHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& header = staticPtrCast<const LispHeader>(chunk);
    // first octet: N|L|E|V|I flag bits followed by the 3-bit unused flags field
    stream.writeBit(header->getNonceBit());
    stream.writeBit(header->getLocStatBit());
    stream.writeBit(header->getEchoNonceBit());
    stream.writeBit(header->getVerMapBit());
    stream.writeBit(header->getInstanceBit());
    stream.writeNBitsOfUint64Be(header->getFlags(), 3);
    stream.writeUint24Be(header->getNonce());           // nonce / map-version (24 bits)
    stream.writeUint32Be(header->getInstanceId());      // instance id / locator-status-bits
}

const Ptr<Chunk> LispHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto header = makeShared<LispHeader>();
    header->setNonceBit(stream.readBit());
    header->setLocStatBit(stream.readBit());
    header->setEchoNonceBit(stream.readBit());
    header->setVerMapBit(stream.readBit());
    header->setInstanceBit(stream.readBit());
    header->setFlags(stream.readNBitsToUint64Be(3));
    header->setNonce(stream.readUint24Be());
    header->setInstanceId(stream.readUint32Be());
    return header;
}

} // namespace lisp
} // namespace inet

//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/checksum/serializer/CrcHeaderSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/protocolelement/checksum/header/CrcHeader_m.h"

namespace inet {

Register_Serializer(CrcHeader, CrcHeaderSerializer);

void CrcHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& crcHeader = staticPtrCast<const CrcHeader>(chunk);
    auto crcMode = crcHeader->getCrcMode();
    if (crcMode != CRC_DISABLED && crcMode != CRC_COMPUTED)
        throw cRuntimeError("Cannot serialize CRC header without turned off or properly computed CRC, try changing the value of crcMode parameter for CrcHeaderInserter");
    stream.writeUint16Be(crcHeader->getCrc());
}

const Ptr<Chunk> CrcHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto crcHeader = makeShared<CrcHeader>();
    auto crc = stream.readUint16Be();
    crcHeader->setCrc(crc);
    crcHeader->setCrcMode(crc == 0 ? CRC_DISABLED : CRC_COMPUTED);
    return crcHeader;
}

} // namespace inet


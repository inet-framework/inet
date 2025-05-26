//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/checksum/serializer/ChecksumHeaderSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/protocolelement/checksum/header/ChecksumHeader_m.h"

namespace inet {

Register_Serializer(ChecksumHeader, ChecksumHeaderSerializer);

void ChecksumHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& checksumHeader = staticPtrCast<const ChecksumHeader>(chunk);
    auto checksumMode = checksumHeader->getChecksumMode();
    if (checksumMode != CHECKSUM_DISABLED && checksumMode != CHECKSUM_COMPUTED)
        throw cRuntimeError("Cannot serialize checksum header without turned off or properly computed checksum, try changing the value of checksumMode parameter for ChecksumHeaderInserter");

    int64_t checksum = checksumHeader->getChecksum();
    B checksumSize = checksumHeader->getChunkLength();
    switch (checksumSize.get<b>()) {
        case 8:  stream.writeUint8(checksum); break;
        case 16: stream.writeUint16Be(checksum); break;
        case 32: stream.writeUint32Be(checksum); break;
        case 64: stream.writeUint64Be(checksum); break;
        default: throw cRuntimeError("Unsupported checksum size: %d", (int)checksumSize.get<B>());
    }
}

const Ptr<Chunk> ChecksumHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto checksumHeader = makeShared<ChecksumHeader>();
    B length = stream.getRemainingLength();
    int64_t checksum;
    switch (length.get<b>()) {
        case 8:  checksum = stream.readUint8(); break;
        case 16: checksum = stream.readUint16Be(); break;
        case 32: checksum = stream.readUint32Be(); break;
        case 64: checksum = stream.readUint64Be(); break;
        default: throw cRuntimeError("Unexpected checksum size or stream length: %d", (int)length.get<b>());
    }
    checksumHeader->setChecksum(checksum);
    checksumHeader->setChecksumMode(checksum == 0 ? CHECKSUM_DISABLED : CHECKSUM_COMPUTED);
    return checksumHeader;
}

} // namespace inet


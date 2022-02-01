//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/checksum/serializer/FcsHeaderSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/protocolelement/checksum/header/FcsHeader_m.h"

namespace inet {

Register_Serializer(FcsHeader, FcsHeaderSerializer);

void FcsHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& fcsHeader = staticPtrCast<const FcsHeader>(chunk);
    auto fcsMode = fcsHeader->getFcsMode();
    if (fcsMode != FCS_DISABLED && fcsMode != FCS_COMPUTED)
        throw cRuntimeError("Cannot serialize FCS header without turned off or properly computed FCS, try changing the value of fcsMode parameter for FcsHeaderInserter");
    stream.writeUint32Be(fcsHeader->getFcs());
}

const Ptr<Chunk> FcsHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto fcsHeader = makeShared<FcsHeader>();
    auto fcs = stream.readUint32Be();
    fcsHeader->setFcs(fcs);
    fcsHeader->setFcsMode(fcs == 0 ? FCS_DISABLED : FCS_COMPUTED);
    return fcsHeader;
}

} // namespace inet


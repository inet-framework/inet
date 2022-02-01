//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/fragmentation/serializer/FragmentNumberHeaderSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/protocolelement/fragmentation/header/FragmentNumberHeader_m.h"

namespace inet {

Register_Serializer(FragmentNumberHeader, FragmentNumberHeaderSerializer);

void FragmentNumberHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& fragmentNumberHeader = staticPtrCast<const FragmentNumberHeader>(chunk);
    uint8_t byte = (fragmentNumberHeader->getFragmentNumber() & 0x7F) + (fragmentNumberHeader->getLastFragment() ? 0x80 : 0x00);
    stream.writeUint8(byte);
}

const Ptr<Chunk> FragmentNumberHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto fragmentNumberHeader = makeShared<FragmentNumberHeader>();
    uint8_t byte = stream.readUint8();
    fragmentNumberHeader->setFragmentNumber(byte & 0x7F);
    fragmentNumberHeader->setLastFragment(byte & 0x80);
    return fragmentNumberHeader;
}

} // namespace inet


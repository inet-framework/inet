//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8022/Ieee8022LlcHeaderSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcHeader_m.h"
#include "inet/linklayer/ieee8022/Ieee8022SnapHeader_m.h"

namespace inet {

Register_Serializer(Ieee8022LlcHeader, Ieee8022LlcHeaderSerializer);
Register_Serializer(Ieee8022LlcSnapHeader, Ieee8022LlcHeaderSerializer);

void Ieee8022LlcHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& llcHeader = CHK(dynamicPtrCast<const Ieee8022LlcHeader>(chunk));
    stream.writeByte(llcHeader->getSsap());
    stream.writeByte(llcHeader->getDsap());
    auto control = llcHeader->getControl();
    stream.writeByte(control);
    if ((control & 3) != 3)
        stream.writeByte(control >> 8);
    if (auto snapHeader = dynamicPtrCast<const Ieee8022LlcSnapHeader>(chunk)) {
        stream.writeByte(snapHeader->getOui() >> 16);
        stream.writeByte(snapHeader->getOui() >> 8);
        stream.writeByte(snapHeader->getOui());
        stream.writeUint16Be(snapHeader->getProtocolId());
    }
}

const Ptr<Chunk> Ieee8022LlcHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    Ptr<Ieee8022LlcHeader> llcHeader = nullptr;
    uint8_t ssap = stream.readByte();
    uint8_t dsap = stream.readByte();
    uint16_t ctrl = stream.readByte();
    if ((ctrl & 3) != 3)
        ctrl |= ((uint16_t)stream.readByte()) << 8;
    if (dsap == 0xAA && ssap == 0xAA && ctrl == 0x03) {
        auto snapHeader = makeShared<Ieee8022LlcSnapHeader>();
        snapHeader->setOui(((uint32_t)stream.readByte() << 16) + stream.readUint16Be());
        snapHeader->setProtocolId(stream.readUint16Be());
        llcHeader = snapHeader;
    }
    else
        llcHeader = makeShared<Ieee8022LlcHeader>();
    llcHeader->setDsap(dsap);
    llcHeader->setSsap(ssap);
    llcHeader->setControl(ctrl);
    return llcHeader;
}

} // namespace inet


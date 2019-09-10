//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcHeader_m.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcHeaderSerializer.h"

namespace inet {

Register_Serializer(Ieee8022LlcHeader, Ieee8022LlcHeaderSerializer);
Register_Serializer(Ieee8022LlcSnapHeader, Ieee8022LlcHeaderSerializer);

void Ieee8022LlcHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& llcHeader = CHK(dynamicPtrCast<const Ieee8022LlcHeader>(chunk));
    // FIXME: ssap and dsap have incorrect (-1) values, which should not be serialized
    /*if (llcHeader->getSsap() < 0)
        throw cRuntimeError("Invalid ssap value.");
    stream.writeByte(llcHeader->getSsap());
    if (llcHeader->getDsap() < 0)
        throw cRuntimeError("Invalid dsap value.");*/
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
    uint8_t ssap = stream.readByte();
    uint8_t dsap = stream.readByte();
    uint16_t ctrl = stream.readByte();
    if ((ctrl & 3) != 3)
        ctrl |= ((uint16_t)stream.readByte()) << 8;
    if (dsap == 0xAA && ssap == 0xAA && ctrl == 0x03) {
        auto snapHeader = makeShared<Ieee8022LlcSnapHeader>();
        snapHeader->setOui(((uint32_t)stream.readByte() << 16) + stream.readUint16Be());
        snapHeader->setProtocolId(stream.readUint16Be());
        if (ssap == 255) snapHeader->setSsap(-1);
        else snapHeader->setSsap(ssap);
        if (dsap == 255) snapHeader->setDsap(-1);
        else snapHeader->setDsap(dsap);
        snapHeader->setControl(ctrl);
        return snapHeader;
    }
    Ptr<Ieee8022LlcHeader> llcHeader = makeShared<Ieee8022LlcHeader>();
    if (ssap == 255) llcHeader->setSsap(-1);
    else llcHeader->setSsap(ssap);
    if (dsap == 255) llcHeader->setDsap(-1);
    else llcHeader->setDsap(dsap);
    llcHeader->setControl(ctrl);
    return llcHeader;
}

} // namespace inet


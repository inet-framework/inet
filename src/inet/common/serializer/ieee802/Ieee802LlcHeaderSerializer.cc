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
#include "inet/common/serializer/ieee802/Ieee802LlcHeaderSerializer.h"
#include "inet/linklayer/ieee802/Ieee802LlcHeader_m.h"

namespace inet {

namespace serializer {

Register_Serializer(Ieee802LlcHeader, Ieee802LlcHeaderSerializer);
Register_Serializer(Ieee802SnapHeader, Ieee802LlcHeaderSerializer);

void Ieee802LlcHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<Chunk>& chunk) const
{
    const auto& llcHeader = CHK(std::dynamic_pointer_cast<const Ieee802LlcHeader>(chunk));
    stream.writeByte(llcHeader->getSsap());
    stream.writeByte(llcHeader->getDsap());
    stream.writeByte(llcHeader->getControl());
    if (auto snapHeader = std::dynamic_pointer_cast<const Ieee802SnapHeader>(chunk)) {
        stream.writeByte(snapHeader->getOui() >> 16);
        stream.writeByte(snapHeader->getOui() >> 8);
        stream.writeByte(snapHeader->getOui());
        stream.writeUint16Be(snapHeader->getProtocolId());
    }
}

Ptr<Chunk> Ieee802LlcHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    Ptr<Ieee802LlcHeader> llcHeader = nullptr;
    uint8_t ssap = stream.readByte();
    uint8_t dsap = stream.readByte();
    uint8_t ctrl = stream.readByte();
    if (dsap == 0xAA && ssap == 0xAA) { // snap frame
        auto snapHeader = std::make_shared<Ieee802SnapHeader>();
        snapHeader->setOui(((uint32_t)stream.readByte() << 16) + stream.readUint16Be());
        snapHeader->setProtocolId(stream.readUint16Be());
        llcHeader = snapHeader;
    }
    else
        llcHeader = std::make_shared<Ieee802LlcHeader>();
    llcHeader->setDsap(dsap);
    llcHeader->setSsap(ssap);
    llcHeader->setControl(ctrl);
    return llcHeader;
}

} // namespace serializer

} // namespace inet


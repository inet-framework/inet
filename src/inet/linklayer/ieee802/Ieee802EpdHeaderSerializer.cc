//
// Copyright (C) 2018 Raphael Riebl, TH Ingolstadt
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#include "inet/linklayer/ieee802/Ieee802EpdHeaderSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/linklayer/ieee802/Ieee802EpdHeader_m.h"

namespace inet {

Register_Serializer(Ieee802EpdHeader, Ieee802EpdHeaderSerializer);

void Ieee802EpdHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& llcHeader = CHK(dynamicPtrCast<const Ieee802EpdHeader>(chunk));
    stream.writeUint16Be(llcHeader->getEtherType());
}

const Ptr<Chunk> Ieee802EpdHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    Ptr<Ieee802EpdHeader> llcHeader = makeShared<Ieee802EpdHeader>();
    llcHeader->setEtherType(stream.readUint16Be());
    return llcHeader;
}

} // namespace inet


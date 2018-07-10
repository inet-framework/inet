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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/linklayer/ieee80211/llc/Ieee80211EtherTypeHeader_m.h"
#include "inet/linklayer/ieee80211/llc/Ieee80211EtherTypeHeaderSerializer.h"

namespace inet {
namespace ieee80211 {

Register_Serializer(Ieee80211EtherTypeHeader, Ieee80211EtherTypeHeaderSerializer);

void Ieee80211EtherTypeHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& llcHeader = CHK(dynamicPtrCast<const Ieee80211EtherTypeHeader>(chunk));
    stream.writeUint16Be(llcHeader->getEtherType());
}

const Ptr<Chunk> Ieee80211EtherTypeHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    Ptr<Ieee80211EtherTypeHeader> llcHeader = makeShared<Ieee80211EtherTypeHeader>();
    llcHeader->setEtherType(stream.readUint16Be());
    return llcHeader;
}

} // namepsace ieee80211
} // namespace inet


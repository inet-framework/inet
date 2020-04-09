//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/protocol/fragmentation/header/FragmentNumberHeader_m.h"
#include "inet/protocol/fragmentation/serializer/FragmentNumberHeaderSerializer.h"

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


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
#include "inet/protocol/checksum/header/FcsHeader_m.h"
#include "inet/protocol/checksum/serializer/FcsHeaderSerializer.h"

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


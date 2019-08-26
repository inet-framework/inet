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
#include "inet/linklayer/ieee802154/Ieee802154MacHeader_m.h"
#include "inet/linklayer/ieee802154/Ieee802154MacHeaderSerializer.h"

namespace inet {

Register_Serializer(Ieee802154MacHeader, Ieee802154MacHeaderSerializer);

void Ieee802154MacHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    b startPos = stream.getLength();
    const auto& macHeader = staticPtrCast<const Ieee802154MacHeader>(chunk);
    stream.writeByte(B(macHeader->getChunkLength()).get());
    stream.writeByte(macHeader->getSequenceId());
    stream.writeMacAddress(macHeader->getSrcAddr());
    stream.writeMacAddress(macHeader->getDestAddr());
    stream.writeUint16Be(macHeader->getNetworkProtocol());
    auto remainders = b(macHeader->getChunkLength() - (stream.getLength() - startPos)).get();
    if (remainders < 0)
        throw cRuntimeError("Ieee802154MacHeader length = %s smaller than required %s bits, try to increase the 'headerLength' parameter", macHeader->getChunkLength().str().c_str(), (stream.getLength() - startPos).str().c_str());
    if (remainders >= 8)
        stream.writeByteRepeatedly('?', remainders >> 3);
    if (remainders & 7)
        stream.writeBitRepeatedly(0, remainders & 7);
}

const Ptr<Chunk> Ieee802154MacHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    B startPos = stream.getPosition();
    auto macHeader = makeShared<Ieee802154MacHeader>();
    uint8_t length = stream.readByte();
    macHeader->setChunkLength(B(length));
    macHeader->setSequenceId(stream.readByte());
    macHeader->setSrcAddr(stream.readMacAddress());
    macHeader->setDestAddr(stream.readMacAddress());
    macHeader->setNetworkProtocol(stream.readUint16Be());
    while (B(length) - (stream.getPosition() - startPos) > B(0))
        stream.readByte();
    return macHeader;
}

} // namespace inet


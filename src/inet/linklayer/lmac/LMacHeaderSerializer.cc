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
#include "inet/linklayer/lmac/LMacHeader_m.h"
#include "inet/linklayer/lmac/LMacHeaderSerializer.h"

namespace inet {

Register_Serializer(LMacHeaderBase, LMacHeaderSerializer);
Register_Serializer(LMacControlFrame, LMacHeaderSerializer);
Register_Serializer(LMacDataFrameHeader, LMacHeaderSerializer);

void LMacHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    b startPos = stream.getLength();
    const auto& header = staticPtrCast<const LMacHeaderBase>(chunk);
    stream.writeByte(header->getType());
    stream.writeUint16Be(b(header->getChunkLength()).get());
    stream.writeMacAddress(header->getSrcAddr());
    stream.writeMacAddress(header->getDestAddr());
    stream.writeUint64Be(header->getMySlot());
    uint8_t numSlots = header->getOccupiedSlotsArraySize();
    stream.writeByte(numSlots);
    b minHeaderLengthWithZeroSlots = header->getType() == LMAC_DATA ? stream.getLength() - startPos + b(16) : stream.getLength() - startPos;
    b headerLengthWithoutSlots = header->getChunkLength() - b(numSlots * 6 * 8);
    auto remainderBits = b(headerLengthWithoutSlots - minHeaderLengthWithZeroSlots).get();
    if (remainderBits < 0)
        throw cRuntimeError("LMacHeader length = %s smaller than required %s, try to increase the 'headerLength' parameter", headerLengthWithoutSlots.str().c_str(), minHeaderLengthWithZeroSlots.str().c_str());
    switch (header->getType()) {
        case LMAC_CONTROL: {
            break;
        }
        case LMAC_DATA: {
            const auto& dataFrame = staticPtrCast<const LMacDataFrameHeader>(chunk);
            stream.writeUint16Be(dataFrame->getNetworkProtocol());
            break;
        }
        default:
            throw cRuntimeError("Unknown header type: %d", header->getType());
    }
    if (remainderBits >= 8)
        stream.writeByteRepeatedly('?', remainderBits >> 3);
    if (remainderBits & 7)
        stream.writeBitRepeatedly(0, remainderBits & 7);
    for (size_t i = 0; i < numSlots; ++i)
        stream.writeMacAddress(header->getOccupiedSlots(i));
}

const Ptr<Chunk> LMacHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    b startPos = stream.getPosition();
    LMacType type = static_cast<LMacType>(stream.readByte());
    b length = b(stream.readUint16Be());
    switch (type) {
        case LMAC_CONTROL: {
            auto ctrlFrame = makeShared<LMacControlFrame>();
            ctrlFrame->setType(type);
            ctrlFrame->setChunkLength(length);
            ctrlFrame->setSrcAddr(stream.readMacAddress());
            ctrlFrame->setDestAddr(stream.readMacAddress());
            ctrlFrame->setMySlot(stream.readUint64Be());
            uint8_t numSlots = stream.readByte();
            ctrlFrame->setOccupiedSlotsArraySize(numSlots);
            auto remainderBits = b(length - (stream.getPosition() - startPos)).get() - numSlots * 6 * 8;
            if (remainderBits >= 8)
                stream.readByteRepeatedly('?', remainderBits >> 3);
            if (remainderBits & 7)
                stream.readBitRepeatedly(0, remainderBits & 7);
            for (size_t i = 0; i < numSlots; ++i)
                ctrlFrame->setOccupiedSlots(i, stream.readMacAddress());
            return ctrlFrame;
        }
        case LMAC_DATA: {
            auto dataFrame = makeShared<LMacDataFrameHeader>();
            dataFrame->setType(type);
            dataFrame->setChunkLength(length);
            dataFrame->setSrcAddr(stream.readMacAddress());
            dataFrame->setDestAddr(stream.readMacAddress());
            dataFrame->setMySlot(stream.readUint64Be());
            uint8_t numSlots = stream.readByte();
            dataFrame->setOccupiedSlotsArraySize(numSlots);
            dataFrame->setNetworkProtocol(stream.readUint16Be());
            auto remainderBits = b(length - (stream.getPosition() - startPos)).get() - numSlots * 6 * 8;
            if (remainderBits >= 8)
                stream.readByteRepeatedly('?', remainderBits >> 3);
            if (remainderBits & 7)
                stream.readBitRepeatedly(0, remainderBits & 7);
            for (size_t i = 0; i < numSlots; ++i)
                dataFrame->setOccupiedSlots(i, stream.readMacAddress());
            return dataFrame;
        }
        default: {
            auto unknownFrame = makeShared<LMacHeaderBase>();
            unknownFrame->setType(type);
            unknownFrame->setChunkLength(length);
            auto remainderBits = b(length - (stream.getPosition() - startPos)).get();
            if (remainderBits >= 8)
                stream.readByteRepeatedly('?', remainderBits >> 3);
            if (remainderBits & 7)
                stream.readBitRepeatedly(0, remainderBits & 7);
            unknownFrame->markIncorrect();
            return unknownFrame;
        }
    }
}

} // namespace inet


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
#include "inet/linklayer/bmac/BMacHeader_m.h"
#include "inet/linklayer/bmac/BMacHeaderSerializer.h"

namespace inet {

Register_Serializer(BMacHeaderBase, BMacHeaderSerializer);
Register_Serializer(BMacControlFrame, BMacHeaderSerializer);
Register_Serializer(BMacDataFrameHeader, BMacHeaderSerializer);

void BMacHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    B startPos = B(stream.getLength());
    const auto& header = staticPtrCast<const BMacHeaderBase>(chunk);
    stream.writeByte(header->getType());
    stream.writeByte(B(header->getChunkLength()).get());
    stream.writeMacAddress(header->getSrcAddr());
    stream.writeMacAddress(header->getDestAddr());
    switch (header->getType()) {
        case BMAC_PREAMBLE:
        case BMAC_ACK: {
            break;
        }
        case BMAC_DATA: {
            const auto& dataFrame = staticPtrCast<const BMacDataFrameHeader>(chunk);
            stream.writeUint64Be(dataFrame->getSequenceId());
            stream.writeUint16Be(dataFrame->getNetworkProtocol());
            break;
        }
        default:
            throw cRuntimeError("Unknown header type: %d", header->getType());
    }
    if ((B(stream.getLength()) - startPos) > B(header->getChunkLength()))
        throw cRuntimeError("BMacHeader length = %d smaller than required %d bytes, try to increase the 'headerLength' parameter", B(header->getChunkLength()).get(), B(stream.getLength() - startPos).get());
    while ((B(stream.getLength()) - startPos) < B(header->getChunkLength()))
        stream.writeByte('?');
}

const Ptr<Chunk> BMacHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    B startPos = stream.getPosition();
    BMacType type = static_cast<BMacType>(stream.readByte());
    uint8_t length = stream.readByte();
    MacAddress srcAddr = stream.readMacAddress();
    MacAddress destAddr = stream.readMacAddress();
    switch (type) {
        case BMAC_PREAMBLE:
        case BMAC_ACK: {
            auto ctrlFrame = makeShared<BMacControlFrame>();
            ctrlFrame->setType(type);
            ctrlFrame->setChunkLength(B(length));
            ctrlFrame->setSrcAddr(srcAddr);
            ctrlFrame->setDestAddr(destAddr);
            while (B(length) - (stream.getPosition() - startPos) > B(0))
                stream.readByte();
            return ctrlFrame;
        }
        case BMAC_DATA: {
            auto dataFrame = makeShared<BMacDataFrameHeader>();
            dataFrame->setType(type);
            dataFrame->setChunkLength(B(length));
            dataFrame->setSrcAddr(srcAddr);
            dataFrame->setDestAddr(destAddr);
            dataFrame->setSequenceId(stream.readUint64Be());
            dataFrame->setNetworkProtocol(stream.readUint16Be());
            while (B(length) - (stream.getPosition() - startPos) > B(0))
                stream.readByte();
            return dataFrame;
        }
        default: {
            auto BMacHeader = makeShared<BMacHeaderBase>();
            while (B(length) - (stream.getPosition() - startPos) > B(0))
                stream.readByte();
            BMacHeader->markIncorrect();
            return BMacHeader;
        }
    }
}

} // namespace inet


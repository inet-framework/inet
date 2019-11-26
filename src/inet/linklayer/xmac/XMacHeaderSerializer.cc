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
#include "inet/linklayer/xmac/XMacHeader_m.h"
#include "inet/linklayer/xmac/XMacHeaderSerializer.h"

namespace inet {

Register_Serializer(XMacHeaderBase, XMacHeaderSerializer);
Register_Serializer(XMacControlFrame, XMacHeaderSerializer);
Register_Serializer(XMacDataFrameHeader, XMacHeaderSerializer);

void XMacHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    b startPos = stream.getLength();
    const auto& header = staticPtrCast<const XMacHeaderBase>(chunk);
    stream.writeByte(header->getType());
    stream.writeUint16Be(b(header->getChunkLength()).get());
    stream.writeMacAddress(header->getSrcAddr());
    stream.writeMacAddress(header->getDestAddr());
    switch (header->getType()) {
        case XMAC_PREAMBLE:
        case XMAC_ACK: {
            break;
        }
        case XMAC_DATA: {
            const auto& dataFrame = staticPtrCast<const XMacDataFrameHeader>(chunk);
            stream.writeUint64Be(dataFrame->getSequenceId());
            stream.writeUint16Be(dataFrame->getNetworkProtocol());
            break;
        }
        default:
            throw cRuntimeError("Unknown header type: %d", header->getType());
    }
    auto remainderBits = b(header->getChunkLength() - (stream.getLength() - startPos)).get();
    if (remainderBits < 0)
        throw cRuntimeError("XMacHeader length = %s smaller than required %s, try to increase the 'headerLength' parameter", header->getChunkLength().str().c_str(), (stream.getLength() - startPos).str().c_str());
    if (remainderBits >= 8)
        stream.writeByteRepeatedly('?', remainderBits >> 3);
    if (remainderBits & 7)
        stream.writeBitRepeatedly(0, remainderBits & 7);
}

const Ptr<Chunk> XMacHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    b startPos = stream.getPosition();
    XMacTypes type = static_cast<XMacTypes>(stream.readByte());
    b length = b(stream.readUint16Be());
    MacAddress srcAddr = stream.readMacAddress();
    MacAddress destAddr = stream.readMacAddress();
    switch (type) {
        case XMAC_PREAMBLE:
        case XMAC_ACK: {
            auto ctrlFrame = makeShared<XMacControlFrame>();
            ctrlFrame->setType(type);
            ctrlFrame->setChunkLength(length);
            ctrlFrame->setSrcAddr(srcAddr);
            ctrlFrame->setDestAddr(destAddr);
            auto remainderBits = b(length - (stream.getPosition() - startPos)).get();
            if (remainderBits >= 8)
                stream.readByteRepeatedly('?', remainderBits >> 3);
            if (remainderBits & 7)
                stream.readBitRepeatedly(0, remainderBits & 7);
            return ctrlFrame;
        }
        case XMAC_DATA: {
            auto dataFrame = makeShared<XMacDataFrameHeader>();
            dataFrame->setType(type);
            dataFrame->setChunkLength(length);
            dataFrame->setSrcAddr(srcAddr);
            dataFrame->setDestAddr(destAddr);
            dataFrame->setSequenceId(stream.readUint64Be());
            dataFrame->setNetworkProtocol(stream.readUint16Be());
            auto remainderBits = b(length - (stream.getPosition() - startPos)).get();
            if (remainderBits >= 8)
                stream.readByteRepeatedly('?', remainderBits >> 3);
            if (remainderBits & 7)
                stream.readBitRepeatedly(0, remainderBits & 7);
            return dataFrame;
        }
        default: {
            auto unknownFrame = makeShared<XMacHeaderBase>();
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


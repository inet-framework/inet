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
    B startPos = B(stream.getLength());
    const auto& header = staticPtrCast<const XMacHeaderBase>(chunk);
    stream.writeByte(header->getType());
    stream.writeByte(B(header->getChunkLength()).get());
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
    if ((B(stream.getLength()) - startPos) > B(header->getChunkLength()))
        throw cRuntimeError("XMacHeader length = %d smaller than required %d bytes, try to increase the 'headerLength' parameter", B(header->getChunkLength()).get(), B(stream.getLength() - startPos).get());
    while ((B(stream.getLength()) - startPos) < B(header->getChunkLength()))
        stream.writeByte('?');
}

const Ptr<Chunk> XMacHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    B startPos = stream.getPosition();
    XMacTypes type = static_cast<XMacTypes>(stream.readByte());
    uint8_t length = stream.readByte();
    MacAddress srcAddr = stream.readMacAddress();
    MacAddress destAddr = stream.readMacAddress();
    switch (type) {
        case XMAC_PREAMBLE:
        case XMAC_ACK: {
            auto ctrlFrame = makeShared<XMacControlFrame>();
            ctrlFrame->setType(type);
            ctrlFrame->setChunkLength(B(length));
            ctrlFrame->setSrcAddr(srcAddr);
            ctrlFrame->setDestAddr(destAddr);
            while (B(length) - (stream.getPosition() - startPos) > B(0))
                stream.readByte();
            return ctrlFrame;
        }
        case XMAC_DATA: {
            auto dataFrame = makeShared<XMacDataFrameHeader>();
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
            auto xMacHeader = makeShared<XMacHeaderBase>();
            while (B(length) - (stream.getPosition() - startPos) > B(0))
                stream.readByte();
            xMacHeader->markIncorrect();
            return xMacHeader;
        }
    }
}

} // namespace inet


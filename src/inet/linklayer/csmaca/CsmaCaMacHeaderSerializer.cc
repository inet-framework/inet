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
#include "inet/linklayer/csmaca/CsmaCaMacHeaderSerializer.h"

namespace inet {

Register_Serializer(CsmaCaMacHeader, CsmaCaMacHeaderSerializer);
Register_Serializer(CsmaCaMacDataHeader, CsmaCaMacHeaderSerializer);
Register_Serializer(CsmaCaMacAckHeader, CsmaCaMacHeaderSerializer);
Register_Serializer(CsmaCaMacTrailer, CsmaCaMacTrailerSerializer);

void CsmaCaMacHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    auto startPos = stream.getLength();
    if (auto macHeader = dynamicPtrCast<const CsmaCaMacDataHeader>(chunk)) {
        stream.writeUint8(macHeader->getType());
        auto length = macHeader->getHeaderLengthField();
        stream.writeUint8(length);
        stream.writeMacAddress(macHeader->getReceiverAddress());
        stream.writeMacAddress(macHeader->getTransmitterAddress());
        stream.writeUint16Be(macHeader->getNetworkProtocol());
        stream.writeByte(macHeader->getPriority());
        if (macHeader->getChunkLength() > stream.getLength() - startPos)
            stream.writeByteRepeatedly('?', B(macHeader->getChunkLength() - (stream.getLength() - startPos)).get());
    }
    else if (auto macHeader = dynamicPtrCast<const CsmaCaMacAckHeader>(chunk)) {
        stream.writeUint8(0x01);
        auto length = macHeader->getHeaderLengthField();
        stream.writeUint8(length);
        stream.writeMacAddress(macHeader->getReceiverAddress());
        stream.writeMacAddress(macHeader->getTransmitterAddress());
        if (macHeader->getChunkLength() > stream.getLength() - startPos)
            stream.writeByteRepeatedly('?', B(macHeader->getChunkLength() - (stream.getLength() - startPos)).get());
    }
    else
        throw cRuntimeError("CsmaCaMacSerializer: cannot serialize chunk");
}

const Ptr<Chunk> CsmaCaMacHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto startPos = stream.getPosition();
    CsmaCaMacHeaderType type = static_cast<CsmaCaMacHeaderType>(stream.readUint8());
    uint8_t length = stream.readUint8();
    switch(type) {
        case CSMA_DATA: {
            auto macHeader = makeShared<CsmaCaMacDataHeader>();
            macHeader->setType(type);
            macHeader->setHeaderLengthField(length);
            macHeader->setReceiverAddress(stream.readMacAddress());
            macHeader->setTransmitterAddress(stream.readMacAddress());
            macHeader->setNetworkProtocol(stream.readUint16Be());
            macHeader->setPriority(stream.readByte());
            if (B(length) > stream.getPosition() - startPos)
                stream.readByteRepeatedly('?', length - B(stream.getPosition() - startPos).get());
            return macHeader;
        }
        case CSMA_ACK: {
            auto macHeader = makeShared<CsmaCaMacAckHeader>();
            macHeader->setType(type);
            macHeader->setHeaderLengthField(length);
            macHeader->setReceiverAddress(stream.readMacAddress());
            macHeader->setTransmitterAddress(stream.readMacAddress());
            if (B(length) > stream.getPosition() - startPos)
                stream.readByteRepeatedly('?', length - B(stream.getPosition() - startPos).get());
            return macHeader;
        }
        default:
            throw cRuntimeError("CsmaCaMacSerializer: cannot deserialize chunk");
    }
    return nullptr;
}

void CsmaCaMacTrailerSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& macTrailer = dynamicPtrCast<const CsmaCaMacTrailer>(chunk);
    auto fcsMode = macTrailer->getFcsMode();
    if (fcsMode != FCS_COMPUTED)
        throw cRuntimeError("Cannot serialize CsmaCaMacTrailer without properly computed FCS, try changing the value of the fcsMode parameter (e.g. in the CsmaCaMac module)");
    stream.writeUint32Be(macTrailer->getFcs());
}

const Ptr<Chunk> CsmaCaMacTrailerSerializer::deserialize(MemoryInputStream& stream) const
{
    auto macTrailer = makeShared<CsmaCaMacTrailer>();
    auto fcs = stream.readUint32Be();
    macTrailer->setFcs(fcs);
    macTrailer->setFcsMode(FCS_COMPUTED);
    return macTrailer;
}

} // namespace inet


//
// Copyright (C) 2014 OpenSim Ltd.
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
#include "inet/linklayer/csmaca/CsmaCaMacSerializer.h"

namespace inet {

Register_Serializer(CsmaCaMacHeader, CsmaCaMacHeaderSerializer);
Register_Serializer(CsmaCaMacDataHeader, CsmaCaMacHeaderSerializer);
Register_Serializer(CsmaCaMacAckHeader, CsmaCaMacHeaderSerializer);
Register_Serializer(CsmaCaMacTrailer, CsmaCaMacTrailerSerializer);

void CsmaCaMacHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    if (auto macHeader = dynamicPtrCast<const CsmaCaMacDataHeader>(chunk)) {
        stream.writeByte(0x00);
        stream.writeMacAddress(macHeader->getReceiverAddress());
        stream.writeMacAddress(macHeader->getTransmitterAddress());
        stream.writeUint16Be(macHeader->getNetworkProtocol());
        stream.writeByte(macHeader->getPriority());
    }
    else if (auto macHeader = dynamicPtrCast<const CsmaCaMacAckHeader>(chunk)) {
        stream.writeByte(0x01);
        stream.writeMacAddress(macHeader->getReceiverAddress());
        stream.writeMacAddress(macHeader->getTransmitterAddress());
    }
    else
        throw cRuntimeError("CsmaCaMacSerializer: cannot serialize chunk");
}

const Ptr<Chunk> CsmaCaMacHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    uint8_t type = stream.readByte();
    switch(type) {
        case 0x00: {
            auto macHeader = makeShared<CsmaCaMacDataHeader>();
            macHeader->setReceiverAddress(stream.readMacAddress());
            macHeader->setTransmitterAddress(stream.readMacAddress());
            macHeader->setNetworkProtocol(stream.readUint16Be());
            macHeader->setPriority(stream.readByte());
            return macHeader;
        }
        case 0x01: {
            auto macHeader = makeShared<CsmaCaMacAckHeader>();
            macHeader->setReceiverAddress(stream.readMacAddress());
            macHeader->setTransmitterAddress(stream.readMacAddress());
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


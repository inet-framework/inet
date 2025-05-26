//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/apsk/packetlevel/ApskPhyHeaderSerializer.h"

#include "inet/common/ProtocolGroup.h"
#include "inet/common/checksum/Checksum.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"

namespace inet {
namespace physicallayer {

Register_Serializer(ApskPhyHeader, ApskPhyHeaderSerializer);

void ApskPhyHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk, b offset, b length) const
{
    auto startPosition = stream.getLength();
    const auto& phyHeader = staticPtrCast<const ApskPhyHeader>(chunk);
    stream.writeUint16Be(phyHeader->getHeaderLengthField().get<b>());
    stream.writeUint16Be(phyHeader->getPayloadLengthField().get<b>());
    auto fcsMode = phyHeader->getFcsMode();
    if (fcsMode != FCS_DISABLED && fcsMode != FCS_COMPUTED)
        throw cRuntimeError("Cannot serialize Apsk Phy header without turned off or properly computed FCS, try changing the value of fcsMode parameter for Udp");
    stream.writeUint16Be(phyHeader->getFcs());
    stream.writeUint16Be(ProtocolGroup::getInetPhyProtocolGroup()->getProtocolNumber(phyHeader->getPayloadProtocol()));

    b remainders = phyHeader->getChunkLength() - (stream.getLength() - startPosition);
    if (remainders < b(0))
        throw cRuntimeError("ApskPhyHeader length = %d smaller than required %d bits", (int)phyHeader->getChunkLength().get<b>(),
                (int)(stream.getLength() - startPosition).get<b>());
    uint8_t remainderbits = remainders.get() % 8;
    stream.writeByteRepeatedly('?', (remainders - b(remainderbits)).get<B>());
    stream.writeBitRepeatedly(false, remainderbits);
}

const Ptr<Chunk> ApskPhyHeaderSerializer::deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const
{
    auto startPosition = stream.getPosition();
    auto phyHeader = makeShared<ApskPhyHeader>();
    b headerLength = b(stream.readUint16Be());
    phyHeader->setHeaderLengthField(headerLength);
    phyHeader->setChunkLength(headerLength);
    phyHeader->setPayloadLengthField(b(stream.readUint16Be()));
    auto fcs = stream.readUint16Be();
    phyHeader->setFcs(fcs);
    phyHeader->setFcsMode(fcs == 0 ? FCS_DISABLED : FCS_COMPUTED);
    phyHeader->setPayloadProtocol(ProtocolGroup::getInetPhyProtocolGroup()->findProtocol(stream.readUint16Be()));

    b curLength = stream.getPosition() - startPosition;
    b remainders = headerLength - curLength;
    if (remainders < b(0)) {
        phyHeader->markIncorrect();
    }
    else {
        uint8_t remainderbits = remainders.get() % 8;
        stream.readByteRepeatedly('?', (remainders - b(remainderbits)).get<B>());
        stream.readBitRepeatedly(false, remainderbits);
    }
    phyHeader->setChunkLength(stream.getPosition() - startPosition);
    return phyHeader;
}

} // namespace physicallayer
} // namespace inet


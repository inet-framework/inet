//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/rtp/RtcpPacketSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/transportlayer/rtp/RtcpPacket_m.h"

namespace inet {
namespace rtp {

Register_Serializer(RtcpPacket, RtcpPacketSerializer);
Register_Serializer(RtcpReceiverReportPacket, RtcpPacketSerializer);
Register_Serializer(RtcpSdesPacket, RtcpPacketSerializer);
Register_Serializer(RtcpByePacket, RtcpPacketSerializer);
Register_Serializer(RtcpSenderReportPacket, RtcpPacketSerializer);

namespace {

void serializeReceptionReport(MemoryOutputStream& stream, const ReceptionReport *receptionReport) {
    if (receptionReport != nullptr) {
        stream.writeUint32Be(receptionReport->getSsrc());
        stream.writeByte(receptionReport->getFractionLost());
        stream.writeUint24Be(receptionReport->getPacketsLostCumulative());
        stream.writeUint32Be(receptionReport->getSequenceNumber());
        stream.writeUint32Be(receptionReport->getJitter());
        stream.writeUint32Be(receptionReport->getLastSR());
        stream.writeUint32Be(receptionReport->getDelaySinceLastSR());
    }
    else
        throw cRuntimeError("Cannot serialize RTCP packet: receptionReport is a null pointer.");
}

void deserializeReceptionReport(MemoryInputStream& stream, ReceptionReport& receptionReport) {
    receptionReport.setSsrc(stream.readUint32Be());
    receptionReport.setFractionLost(stream.readByte());
    receptionReport.setPacketsLostCumulative(stream.readNBitsToUint64Be(24));
    receptionReport.setSequenceNumber(stream.readUint32Be());
    receptionReport.setJitter(stream.readUint32Be());
    receptionReport.setLastSR(stream.readUint32Be());
    receptionReport.setDelaySinceLastSR(stream.readUint32Be());
}

void serializeSdesChunk(MemoryOutputStream& stream, const SdesChunk *sdesChunk) {
    stream.writeUint32Be(sdesChunk->getSsrc());
    uint64_t numBytes = 4;
    for (int e = 0; e < sdesChunk->size(); ++e) {
        const SdesItem *sdesItem = static_cast<const SdesItem *>(sdesChunk->get(e));
        if (sdesItem != nullptr) {
            stream.writeByte(sdesItem->getType());
            uint8_t length = sdesItem->getLengthField();
            stream.writeByte(length);
            stream.writeBytes((uint8_t *)sdesItem->getContent(), B(length));
            numBytes += 2 + length;
        }
        else
            throw cRuntimeError("Cannot serialize RTCP packet: sdesItem is a null pointer.");
    }
    ASSERT(B(numBytes) == B(sdesChunk->getLength()));
    stream.writeByte(0);
    ++numBytes;
    if (numBytes % 4 != 0)
        stream.writeByteRepeatedly(0, 4 - (numBytes % 4));
}

void deserializeSdesChunk(MemoryInputStream& stream, const Ptr<RtcpPacket> rtcpPacket, SdesChunk& sdesChunk) {
    sdesChunk.setName("SdesChunk");
    sdesChunk.setSsrc(stream.readUint32Be());
    uint8_t type = stream.readByte();
    uint64_t numBytes = 1;
    while (type != 0) {
        uint8_t count = stream.readByte();
        char *content = new char[count];
        stream.readBytes((uint8_t *)content, B(count));
        SdesItem *sdesItem = new SdesItem(static_cast<SdesItem::SdesItemType>(type), content);
        sdesChunk.addSDESItem(sdesItem);
        type = stream.readByte();
        numBytes += 2 + count;
    }
    if (numBytes % 4 != 0)
        if (!stream.readByteRepeatedly(0, 4 - (numBytes % 4)))
            rtcpPacket->markIncorrect();
}

} // namespace

void RtcpPacketSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& rtcpPacket = staticPtrCast<const RtcpPacket>(chunk);
    B start_position = B(stream.getLength());
    stream.writeNBitsOfUint64Be(rtcpPacket->getVersion(), 2);
    stream.writeBit(rtcpPacket->getPadding());
    short count = rtcpPacket->getCount();
    stream.writeNBitsOfUint64Be(count, 5);
    stream.writeByte(rtcpPacket->getPacketType());
    stream.writeUint16Be(rtcpPacket->getRtcpLength());
    switch (rtcpPacket->getPacketType()) {
        case RTCP_PT_SR: {
            const auto& rtcpSenderReportPacket = staticPtrCast<const RtcpSenderReportPacket>(chunk);
            stream.writeUint32Be(rtcpSenderReportPacket->getSsrc());
            auto& senderReport = rtcpSenderReportPacket->getSenderReport();
            stream.writeUint64Be(senderReport.getNTPTimeStamp());
            stream.writeUint32Be(senderReport.getRTPTimeStamp());
            stream.writeUint32Be(senderReport.getPacketCount());
            stream.writeUint32Be(senderReport.getByteCount());
            ASSERT(count == rtcpSenderReportPacket->getReceptionReports().size());
            for (short i = 0; i < count; ++i) {
                serializeReceptionReport(stream, static_cast<const ReceptionReport *>(rtcpSenderReportPacket->getReceptionReports()[i]));
            }
            ASSERT(rtcpSenderReportPacket->getChunkLength() == B(4) + B(24) + B(count * 24));
            break;
        }
        case RTCP_PT_RR: {
            const auto& rtcpReceiverReportPacket = staticPtrCast<const RtcpReceiverReportPacket>(chunk);
            stream.writeUint32Be(rtcpReceiverReportPacket->getSsrc());
            for (short i = 0; i < count; ++i) {
                serializeReceptionReport(stream, static_cast<const ReceptionReport *>(rtcpReceiverReportPacket->getReceptionReports()[i]));
            }
            ASSERT(rtcpReceiverReportPacket->getChunkLength() == B(4) + B(4) + B(count * 24));
            break;
        }
        case RTCP_PT_SDES: {
            const auto& rtcpSdesPacket = staticPtrCast<const RtcpSdesPacket>(chunk);
            for (short i = 0; i < count; ++i) {
                serializeSdesChunk(stream, static_cast<const SdesChunk *>(rtcpSdesPacket->getSdesChunks()[i]));
            }
            ASSERT(rtcpSdesPacket->getChunkLength() == (stream.getLength() - start_position));
            break;
        }
        case RTCP_PT_BYE: {
            const auto& rtcpByePacket = staticPtrCast<const RtcpByePacket>(chunk);
            stream.writeUint32Be(rtcpByePacket->getSsrc());
            ASSERT(rtcpByePacket->getChunkLength() == (stream.getLength() - start_position));
            break;
        }
        default: {
            throw cRuntimeError("Can not serialize RTCP packet: type %d not supported.", rtcpPacket->getPacketType());
            break;
        }
    }
}

const Ptr<Chunk> RtcpPacketSerializer::deserialize(MemoryInputStream& stream) const
{
    auto rtcpPacket = makeShared<RtcpPacket>();
    rtcpPacket->setVersion(stream.readNBitsToUint64Be(2));
    rtcpPacket->setPadding(stream.readBit());
    short count = stream.readNBitsToUint64Be(5);
    rtcpPacket->setCount(count);
    rtcpPacket->setPacketType((RtcpPacketType)stream.readByte());
    rtcpPacket->setRtcpLength(stream.readUint16Be());
    switch (rtcpPacket->getPacketType()) {
        case RTCP_PT_SR: {
            auto rtcpSenderReportPacket = makeShared<RtcpSenderReportPacket>();
            rtcpSenderReportPacket->setVersion(rtcpPacket->getVersion());
            rtcpSenderReportPacket->setPadding(rtcpPacket->getPadding());
            rtcpSenderReportPacket->setCount(0);
            rtcpSenderReportPacket->setPacketType(rtcpPacket->getPacketType());
            rtcpSenderReportPacket->setRtcpLength(rtcpPacket->getRtcpLength());
            rtcpSenderReportPacket->setSsrc(stream.readUint32Be());
            auto& senderReport = rtcpSenderReportPacket->getSenderReportForUpdate();
            senderReport.setNTPTimeStamp(stream.readUint64Be());
            senderReport.setRTPTimeStamp(stream.readUint32Be());
            senderReport.setPacketCount(stream.readUint32Be());
            senderReport.setByteCount(stream.readUint32Be());
            for (short i = 0; i < count; ++i) {
                ReceptionReport *receptionReport = new ReceptionReport();
                deserializeReceptionReport(stream, *receptionReport);
                rtcpSenderReportPacket->addReceptionReport(receptionReport);
            }
            return rtcpSenderReportPacket;
        }
        case RTCP_PT_RR: {
            auto rtcpReceiverReportPacket = makeShared<RtcpReceiverReportPacket>();
            rtcpReceiverReportPacket->setVersion(rtcpPacket->getVersion());
            rtcpReceiverReportPacket->setPadding(rtcpPacket->getPadding());
            rtcpReceiverReportPacket->setCount(0);
            rtcpReceiverReportPacket->setPacketType(rtcpPacket->getPacketType());
            rtcpReceiverReportPacket->setRtcpLength(rtcpPacket->getRtcpLength());
            rtcpReceiverReportPacket->setSsrc(stream.readUint32Be());
            for (short i = 0; i < count; ++i) {
                ReceptionReport *receptionReport = new ReceptionReport();
                deserializeReceptionReport(stream, *receptionReport);
                rtcpReceiverReportPacket->addReceptionReport(receptionReport);
            }
            return rtcpReceiverReportPacket;
        }
        case RTCP_PT_SDES: {
            auto rtcpSdesPacket = makeShared<RtcpSdesPacket>();
            rtcpSdesPacket->setVersion(rtcpPacket->getVersion());
            rtcpSdesPacket->setPadding(rtcpPacket->getPadding());
            rtcpSdesPacket->setCount(0);
            rtcpSdesPacket->setPacketType(rtcpPacket->getPacketType());
            rtcpSdesPacket->setRtcpLength(rtcpPacket->getRtcpLength());
            for (short i = 0; i < count; ++i) {
                SdesChunk *sdesChunk = new SdesChunk();
                deserializeSdesChunk(stream, rtcpSdesPacket, *sdesChunk);
                rtcpSdesPacket->addSDESChunk(sdesChunk);
            }
            return rtcpSdesPacket;
        }
        case RTCP_PT_BYE: {
            auto rtcpByePacket = makeShared<RtcpByePacket>();
            rtcpByePacket->setVersion(rtcpPacket->getVersion());
            rtcpByePacket->setPadding(rtcpPacket->getPadding());
            rtcpByePacket->setCount(1);
            rtcpByePacket->setPacketType(rtcpPacket->getPacketType());
            rtcpByePacket->setRtcpLength(rtcpPacket->getRtcpLength());
            rtcpByePacket->setSsrc(stream.readUint32Be());
            // more SSRC and optional data may be included
            while (B(stream.getRemainingLength()) != B(0)) {
                stream.readByte();
            }
            return rtcpByePacket;
        }
        default: {
            rtcpPacket->markIncorrect();
            return rtcpPacket;
        }
    }
}

} // namespace rtp
} // namespace inet


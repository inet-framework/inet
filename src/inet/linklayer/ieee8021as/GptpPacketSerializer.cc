//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8021as/GptpPacketSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"

namespace inet {

Register_Serializer(GptpBase, GptpPacketSerializer);
Register_Serializer(GptpFollowUp, GptpPacketSerializer);
Register_Serializer(GptpPdelayReq, GptpPacketSerializer);
Register_Serializer(GptpPdelayResp, GptpPacketSerializer);
Register_Serializer(GptpPdelayRespFollowUp, GptpPacketSerializer);
Register_Serializer(GptpSync, GptpPacketSerializer);

clocktime_t GptpPacketSerializer::readClock8(MemoryInputStream& stream) const
{
    int64_t ns16 = stream.readUint64Be();
    simtime_t t1(ns16>>16, SIMTIME_NS);
    ns16 = (ns16 & 0xFFFF) * SimTime::getScale() / 1000000000LL;
    return SIMTIME_AS_CLOCKTIME(t1 + SimTime::fromRaw(ns16>>16));
}

void GptpPacketSerializer::writeClock8(MemoryOutputStream& stream, const clocktime_t& clock) const
{
    int64_t outValue;
    simtime_t outRemainder;
    simtime_t t = CLOCKTIME_AS_SIMTIME(clock);
    t.split(SIMTIME_NS, outValue, outRemainder);
    outValue <<= 16;
    int64_t ns16 = outRemainder.raw() * 1000000000LL / SimTime::getScale();
    stream.writeUint64Be(outValue + ns16);
}

clocktime_t GptpPacketSerializer::readTimestamp(MemoryInputStream& stream) const
{
    // 10 bytes
    int64_t s = stream.readUint48Be();
    int64_t ns = stream.readUint32Be();
    simtime_t t1(s, SIMTIME_S);
    simtime_t t2(ns, SIMTIME_NS);
    return SIMTIME_AS_CLOCKTIME(t1 + t2);
}

void GptpPacketSerializer::writeTimestamp(MemoryOutputStream& stream, const clocktime_t& clock) const
{
    // 10 bytes
    simtime_t t = CLOCKTIME_AS_SIMTIME(clock);
    int64_t outValue;
    t.split(SIMTIME_S, outValue, t);
    stream.writeUint48Be(outValue);
    t.split(SIMTIME_NS, outValue, t);
    stream.writeUint32Be(outValue);
}

clocktime_t GptpPacketSerializer::readScaledNS(MemoryInputStream& stream) const
{
    // The ScaledNs type represents signed values of time and time interval in units of 2^–16 ns.
    // 12 bytes
    int32_t x = stream.readUint32Be();
    int64_t ns16 = stream.readUint64Be();
    if ((ns16 < 0 && x != -1) || (ns16 >= 0 && x != 0))
        throw cRuntimeError("too big number");
    simtime_t t1(ns16>>16, SIMTIME_NS);
    ns16 = (ns16 & 0xFFFF) * SimTime::getScale() / 1000000000LL;
    return SIMTIME_AS_CLOCKTIME(t1 + SimTime::fromRaw(ns16>>16));
}

void GptpPacketSerializer::writeScaledNS(MemoryOutputStream& stream, const clocktime_t& clock) const
{
    // The ScaledNs type represents signed values of time and time interval in units of 2^–16 ns.
    // 12 bytes
    simtime_t t = CLOCKTIME_AS_SIMTIME(clock);
    int64_t outValue;
    simtime_t outRemainder;
    t.split(SIMTIME_NS, outValue, outRemainder);
    outValue <<= 16;
    int64_t ns16 = outRemainder.raw() * 1000000000LL / SimTime::getScale();
    stream.writeUint32Be(clock >= CLOCKTIME_ZERO ? 0 : 0xFFFFFFFF);
    stream.writeUint64Be(outValue + ns16);
}

void GptpPacketSerializer::writePortIdentity(MemoryOutputStream& stream, const PortIdentity& portIdentity) const
{
    stream.writeUint64Be(portIdentity.clockIdentity);
    stream.writeUint16Be(portIdentity.portNumber);
}

PortIdentity GptpPacketSerializer::readPortIdentity(MemoryInputStream& stream) const
{
    PortIdentity portIdentity;
    portIdentity.clockIdentity = stream.readUint64Be();
    portIdentity.portNumber = stream.readUint16Be();
    return portIdentity;
}

void GptpPacketSerializer::writeGptpBase(MemoryOutputStream& stream, const GptpBase& gptpPacket) const
{
    stream.writeUint4(gptpPacket.getMajorSdoId());
    stream.writeUint4((int8_t)(gptpPacket.getMessageType()));
    stream.writeUint4(gptpPacket.getMinorVersionPTP());
    stream.writeUint4(gptpPacket.getVersionPTP());
    stream.writeUint16Be(gptpPacket.getMessageLengthField());
    stream.writeUint8(gptpPacket.getDomainNumber());
    stream.writeUint8(gptpPacket.getMinorSdoId());
    stream.writeUint16Be(gptpPacket.getFlags());
    writeClock8(stream, gptpPacket.getCorrectionField());
    stream.writeUint32Be(gptpPacket.getMessageTypeSpecific());
    writePortIdentity(stream, gptpPacket.getSourcePortIdentity());
    stream.writeUint16Be(gptpPacket.getSequenceId());
    stream.writeUint8(gptpPacket.getControlField());
    stream.writeUint8(gptpPacket.getLogMessageInterval());
}

void GptpPacketSerializer::readGptpBase(MemoryInputStream& stream, GptpBase& gptpPacket) const
{
    gptpPacket.setMajorSdoId(stream.readUint4());
    gptpPacket.setMessageType(static_cast<GptpMessageType>(stream.readUint4()));
    gptpPacket.setMinorVersionPTP(stream.readUint4());
    gptpPacket.setVersionPTP(stream.readUint4());
    gptpPacket.setMessageLengthField(stream.readUint16Be());
    gptpPacket.setDomainNumber(stream.readUint8());
    gptpPacket.setMinorSdoId(stream.readUint8());
    gptpPacket.setFlags(stream.readUint16Be());
    gptpPacket.setCorrectionField(readClock8(stream));
    gptpPacket.setMessageTypeSpecific(stream.readUint32Be());
    gptpPacket.setSourcePortIdentity(readPortIdentity(stream));
    gptpPacket.setSequenceId(stream.readUint16Be());
    gptpPacket.setControlField(stream.readUint8());
    gptpPacket.setLogMessageInterval(stream.readUint8());
}

void GptpPacketSerializer::readGptpFollowUpPart(MemoryInputStream& stream, GptpFollowUp& gptpPacket) const
{
    gptpPacket.setPreciseOriginTimestamp(readTimestamp(stream));
    readGptpFollowUpInformationTlv(stream, gptpPacket.getFollowUpInformationTLVForUpdate());
}

void GptpPacketSerializer::writeGptpFollowUpPart(MemoryOutputStream& stream, const GptpFollowUp& gptpPacket) const
{
    writeTimestamp(stream, gptpPacket.getPreciseOriginTimestamp());
    writeGptpFollowUpInformationTlv(stream, gptpPacket.getFollowUpInformationTLV());
}

void GptpPacketSerializer::readGptpPdelayReqPart(MemoryInputStream& stream, GptpPdelayReq& gptpPacket) const
{
    stream.readByteRepeatedly(0, 10);
    stream.readByteRepeatedly(0, 10);
}

void GptpPacketSerializer::writeGptpPdelayReqPart(MemoryOutputStream& stream, const GptpPdelayReq& gptpPacket) const
{
    stream.writeByteRepeatedly(0, 10);
    stream.writeByteRepeatedly(0, 10);
}

void GptpPacketSerializer::readGptpPdelayRespPart(MemoryInputStream& stream, GptpPdelayResp& gptpPacket) const
{
    gptpPacket.setRequestReceiptTimestamp(readTimestamp(stream));
    gptpPacket.setRequestingPortIdentity(readPortIdentity(stream));
}

void GptpPacketSerializer::writeGptpPdelayRespPart(MemoryOutputStream& stream, const GptpPdelayResp& gptpPacket) const
{
    writeTimestamp(stream, gptpPacket.getRequestReceiptTimestamp());
    writePortIdentity(stream, gptpPacket.getRequestingPortIdentity());
}

void GptpPacketSerializer::readGptpPdelayRespFollowUpPart(MemoryInputStream& stream, GptpPdelayRespFollowUp& gptpPacket) const
{
    gptpPacket.setResponseOriginTimestamp(readTimestamp(stream));
    gptpPacket.setRequestingPortIdentity(readPortIdentity(stream));
}

void GptpPacketSerializer::writeGptpPdelayRespFollowUpPart(MemoryOutputStream& stream, const GptpPdelayRespFollowUp& gptpPacket) const
{
    writeTimestamp(stream, gptpPacket.getResponseOriginTimestamp());
    writePortIdentity(stream, gptpPacket.getRequestingPortIdentity());
}

void GptpPacketSerializer::readGptpSyncPart(MemoryInputStream& stream, GptpSync& gptpPacket) const
{
    if (gptpPacket.getFlags() & twoStepFlag)
        stream.readByteRepeatedly(0, 10); // read "reserved"
    else
        throw cRuntimeError("The GptpSync without twoStepFlag is unimplemented yet");
}

void GptpPacketSerializer::writeGptpSyncPart(MemoryOutputStream& stream, const GptpSync& gptpPacket) const
{
    if (gptpPacket.getFlags() & twoStepFlag)
        stream.writeByteRepeatedly(0, 10); // write "reserved"
    else
        throw cRuntimeError("The GptpSync without twoStepFlag is unimplemented yet");
}

void GptpPacketSerializer::readGptpFollowUpInformationTlv(MemoryInputStream& stream, GptpFollowUpInformationTlv& tlv) const
{
    tlv.setTlvType((GptpTlvType)stream.readUint16Be());
    tlv.setLengthField(stream.readUint16Be());
    tlv.setOrganizationId(stream.readUint24Be());
    tlv.setOrganizationSubType(stream.readUint24Be());
    tlv.setCumulativeScaledRateOffset(stream.readUint32Be());
    tlv.setGmTimeBaseIndicator(stream.readUint16Be());
    tlv.setLastGmPhaseChange(readScaledNS(stream));
    tlv.setScaledLastGmFreqChange(stream.readUint32Be());
}

void GptpPacketSerializer::writeGptpFollowUpInformationTlv(MemoryOutputStream& stream, const GptpFollowUpInformationTlv& tlv) const
{
    stream.writeUint16Be(tlv.getTlvType());
    stream.writeUint16Be(tlv.getLengthField());
    stream.writeUint24Be(tlv.getOrganizationId());
    stream.writeUint24Be(tlv.getOrganizationSubType());
    stream.writeUint32Be(tlv.getCumulativeScaledRateOffset());
    stream.writeUint16Be(tlv.getGmTimeBaseIndicator());
    writeScaledNS(stream, tlv.getLastGmPhaseChange());
    stream.writeUint32Be(tlv.getScaledLastGmFreqChange());
}

void GptpPacketSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& gptpPacket = staticPtrCast<const GptpBase>(chunk);
    writeGptpBase(stream, *gptpPacket.get());

    // TODO Type-specific code:
    switch(gptpPacket->getMessageType()) {
        case GPTPTYPE_FOLLOW_UP: {
            const auto& gptp = CHK(dynamicPtrCast<const GptpFollowUp>(gptpPacket));
            writeGptpFollowUpPart(stream, *gptp.get());
            break;
        }
        case GPTPTYPE_PDELAY_REQ: {
            const auto& gptp = CHK(dynamicPtrCast<const GptpPdelayReq>(gptpPacket));
            writeGptpPdelayReqPart(stream, *gptp.get());
            break;
        }
        case GPTPTYPE_PDELAY_RESP: {
            const auto& gptp = CHK(dynamicPtrCast<const GptpPdelayResp>(gptpPacket));
            writeGptpPdelayRespPart(stream, *gptp.get());
            break;
        }
        case GPTPTYPE_PDELAY_RESP_FOLLOW_UP: {
            const auto& gptp = CHK(dynamicPtrCast<const GptpPdelayRespFollowUp>(gptpPacket));
            writeGptpPdelayRespFollowUpPart(stream, *gptp.get());
            break;
        }
        case GPTPTYPE_SYNC: {
            const auto& gptp = CHK(dynamicPtrCast<const GptpSync>(gptpPacket));
            writeGptpSyncPart(stream, *gptp.get());
            break;
        }
        default:
            throw cRuntimeError("unknown GptpMessageType");
    }
}

const Ptr<Chunk> GptpPacketSerializer::deserialize(MemoryInputStream& stream) const
{
    auto startPos = stream.getPosition();
    stream.readUint4();
    GptpMessageType type = static_cast<GptpMessageType>(stream.readUint4());
    stream.seek(startPos);

    switch(type) {
        case GPTPTYPE_FOLLOW_UP: {
            auto gptpPacket = makeShared<GptpFollowUp>();
            readGptpBase(stream, *gptpPacket.get());
            readGptpFollowUpPart(stream, *gptpPacket.get());
            return gptpPacket;
        }
        case GPTPTYPE_PDELAY_REQ: {
            auto gptpPacket = makeShared<GptpPdelayReq>();
            readGptpBase(stream, *gptpPacket.get());
            readGptpPdelayReqPart(stream, *gptpPacket.get());
            return gptpPacket;
        }
        case GPTPTYPE_PDELAY_RESP: {
            auto gptpPacket = makeShared<GptpPdelayResp>();
            readGptpBase(stream, *gptpPacket.get());
            readGptpPdelayRespPart(stream, *gptpPacket.get());
            return gptpPacket;
        }
        case GPTPTYPE_PDELAY_RESP_FOLLOW_UP: {
            auto gptpPacket = makeShared<GptpPdelayRespFollowUp>();
            readGptpBase(stream, *gptpPacket.get());
            readGptpPdelayRespFollowUpPart(stream, *gptpPacket.get());
            return gptpPacket;
        }
        case GPTPTYPE_SYNC: {
            auto gptpPacket = makeShared<GptpSync>();
            readGptpBase(stream, *gptpPacket.get());
            readGptpSyncPart(stream, *gptpPacket.get());
            return gptpPacket;
        }
        default:
            throw cRuntimeError("unknown GptpMessageType");
    }
}

} // namespace inet


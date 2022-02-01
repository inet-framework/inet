//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_GPTPPACKETSERIALIZER_H
#define __INET_GPTPPACKETSERIALIZER_H

#include "inet/clock/common/ClockEvent.h"
#include "inet/common/packet/serializer/FieldsChunkSerializer.h"
#include "inet/linklayer/ieee8021as/GptpPacket_m.h"

namespace inet {

/**
 * Converts between GptpPacket and binary (network byte order) Gptp packet.
 */
class INET_API GptpPacketSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

    void readGptpBase(MemoryInputStream& stream, GptpBase& gptpPacket) const;
    void writeGptpBase(MemoryOutputStream& stream, const GptpBase& gptpPacket) const;

    void readGptpFollowUpPart(MemoryInputStream& stream, GptpFollowUp& gptpPacket) const;
    void writeGptpFollowUpPart(MemoryOutputStream& stream, const GptpFollowUp& gptpPacket) const;

    void readGptpPdelayReqPart(MemoryInputStream& stream, GptpPdelayReq& gptpPacket) const;
    void writeGptpPdelayReqPart(MemoryOutputStream& stream, const GptpPdelayReq& gptpPacket) const;

    void readGptpPdelayRespPart(MemoryInputStream& stream, GptpPdelayResp& gptpPacket) const;
    void writeGptpPdelayRespPart(MemoryOutputStream& stream, const GptpPdelayResp& gptpPacket) const;

    void readGptpPdelayRespFollowUpPart(MemoryInputStream& stream, GptpPdelayRespFollowUp& gptpPacket) const;
    void writeGptpPdelayRespFollowUpPart(MemoryOutputStream& stream, const GptpPdelayRespFollowUp& gptpPacket) const;

    void readGptpSyncPart(MemoryInputStream& stream, GptpSync& gptpPacket) const;
    void writeGptpSyncPart(MemoryOutputStream& stream, const GptpSync& gptpPacket) const;

    void readGptpFollowUpInformationTlv(MemoryInputStream& stream, GptpFollowUpInformationTlv& gptpPacket) const;
    void writeGptpFollowUpInformationTlv(MemoryOutputStream& stream, const GptpFollowUpInformationTlv& gptpPacket) const;

    clocktime_t readClock8(MemoryInputStream& stream) const;
    void writeClock8(MemoryOutputStream& stream, const clocktime_t& clock) const;

    clocktime_t readTimestamp(MemoryInputStream& stream) const;
    void writeTimestamp(MemoryOutputStream& stream, const clocktime_t& clock) const;

    clocktime_t readScaledNS(MemoryInputStream& stream) const;
    void writeScaledNS(MemoryOutputStream& stream, const clocktime_t& clock) const;

    PortIdentity readPortIdentity(MemoryInputStream& stream) const;
    void writePortIdentity(MemoryOutputStream& stream, const PortIdentity& portIdentity) const;

  public:
    GptpPacketSerializer() : FieldsChunkSerializer() {}
};

} // namespace inet

#endif


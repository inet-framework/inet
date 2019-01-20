/***************************************************************************
                          RtpSenderInfo.cc  -  description
                             -------------------
    begin                : Wed Dec 5 2001
    copyright            : (C) 2001 by Matthias Oppitz
    email                : Matthias.Oppitz@gmx.de
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "inet/transportlayer/rtp/Reports_m.h"
#include "inet/transportlayer/rtp/RtpPacket_m.h"
#include "inet/transportlayer/rtp/RtpSenderInfo.h"

namespace inet {
namespace rtp {

Register_Class(RtpSenderInfo);

RtpSenderInfo::RtpSenderInfo(uint32 ssrc) : RtpParticipantInfo(ssrc)
{
    _startTime = 0.0;
    _clockRate = 0;
    _timeStampBase = 0;
    _sequenceNumberBase = 0;
    _packetsSent = 0;
    _bytesSent = 0;
}

RtpSenderInfo::RtpSenderInfo(const RtpSenderInfo& senderInfo) : RtpParticipantInfo(senderInfo)
{
    copy(senderInfo);
}

RtpSenderInfo::~RtpSenderInfo()
{
}

RtpSenderInfo& RtpSenderInfo::operator=(const RtpSenderInfo& senderInfo)
{
    if (this == &senderInfo)
        return *this;
    RtpParticipantInfo::operator=(senderInfo);
    copy(senderInfo);
    return *this;
}

void RtpSenderInfo::copy(const RtpSenderInfo& senderInfo)
{
    RtpParticipantInfo::operator=(senderInfo);
    _startTime = senderInfo._startTime;
    _clockRate = senderInfo._clockRate;
    _timeStampBase = senderInfo._timeStampBase;
    _sequenceNumberBase = senderInfo._sequenceNumberBase;
    _packetsSent = senderInfo._packetsSent;
    _bytesSent = senderInfo._bytesSent;
}

RtpSenderInfo *RtpSenderInfo::dup() const
{
    return new RtpSenderInfo(*this);
}

void RtpSenderInfo::processRTPPacket(Packet *packet, int id, simtime_t arrivalTime)
{
    const auto& rtpHeader = packet->peekAtFront<RtpHeader>();
    _packetsSent++;
    _bytesSent = _bytesSent + packet->getByteLength() - B(rtpHeader->getChunkLength()).get();

    // call corresponding method of superclass
    // for setting _silentIntervals
    // it deletes the packet !!!
    RtpParticipantInfo::processRTPPacket(packet, id, arrivalTime);
}

void RtpSenderInfo::processReceptionReport(const ReceptionReport *report, simtime_t arrivalTime)
{
}

SenderReport *RtpSenderInfo::senderReport(simtime_t now)
{
    if (isSender()) {
        SenderReport *senderReport = new SenderReport();
        // ntp time stamp is 64 bit integer

        uint64 ntpSeconds = (uint64)SIMTIME_DBL(now);
        uint64 ntpFraction = (uint64)((SIMTIME_DBL(now) - ntpSeconds * 65536.0) * 65536.0);

        senderReport->setNTPTimeStamp((uint64)(ntpSeconds << 32) + ntpFraction);
        senderReport->setRTPTimeStamp(SIMTIME_DBL(now - _startTime) * _clockRate);
        senderReport->setPacketCount(_packetsSent);
        senderReport->setByteCount(_bytesSent);
        return senderReport;
    }
    else {
        return nullptr;
    }
}

void RtpSenderInfo::setStartTime(simtime_t startTime)
{
    _startTime = startTime;
}

void RtpSenderInfo::setClockRate(int clockRate)
{
    _clockRate = clockRate;
}

void RtpSenderInfo::setTimeStampBase(uint32 timeStampBase)
{
    _timeStampBase = timeStampBase;
}

void RtpSenderInfo::setSequenceNumberBase(uint16 sequenceNumberBase)
{
    _sequenceNumberBase = sequenceNumberBase;
}

} // namespace rtp
} // namespace inet


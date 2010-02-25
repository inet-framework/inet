/***************************************************************************
                          RTPSenderInfo.cc  -  description
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


/** \file RTPSenderInfo.cc
 * This file contains the implementation of member functions of the class RTPSenderInfo.
 */

#include "RTPSenderInfo.h"


Register_Class(RTPSenderInfo);

RTPSenderInfo::RTPSenderInfo(uint32 ssrc) : RTPParticipantInfo(ssrc)
{
    _startTime = 0.0;
    _clockRate = 0;
    _timeStampBase = 0;
    _sequenceNumberBase = 0;
    _packetsSent = 0;
    _bytesSent = 0;
}


RTPSenderInfo::RTPSenderInfo(const RTPSenderInfo& senderInfo) : RTPParticipantInfo()
{
    operator=(senderInfo);
}


RTPSenderInfo::~RTPSenderInfo()
{

}


RTPSenderInfo& RTPSenderInfo::operator=(const RTPSenderInfo& senderInfo)
{
    RTPParticipantInfo::operator=(senderInfo);
    _startTime = senderInfo._startTime;
    _clockRate = senderInfo._clockRate;
    _timeStampBase = senderInfo._timeStampBase;
    _sequenceNumberBase = senderInfo._sequenceNumberBase;
    _packetsSent = senderInfo._packetsSent;
    _bytesSent = senderInfo._bytesSent;
    return *this;
}


RTPSenderInfo *RTPSenderInfo::dup() const
{
    return new RTPSenderInfo(*this);
}


void RTPSenderInfo::processRTPPacket(RTPPacket *packet, int id,  simtime_t arrivalTime)
{
    _packetsSent++;
    _bytesSent = _bytesSent + packet->getPayloadLength();

    // call corresponding method of superclass
    // for setting _silentIntervals
    // it deletes the packet !!!
    RTPParticipantInfo::processRTPPacket(packet, id, arrivalTime);
}


void RTPSenderInfo::processReceptionReport(ReceptionReport *report, simtime_t arrivalTime)
{
    delete report;
}


SenderReport *RTPSenderInfo::senderReport(simtime_t now)
{
    if (isSender()) {
        SenderReport *senderReport = new SenderReport();
        // ntp time stamp is 64 bit integer

        uint64 ntpSeconds = (uint64)SIMTIME_DBL(now);
        uint64 ntpFraction = (uint64)( (SIMTIME_DBL(now) - ntpSeconds*65536.0) * 65536.0);

        senderReport->setNTPTimeStamp((uint64)(ntpSeconds << 32) + ntpFraction);
        senderReport->setRTPTimeStamp(SIMTIME_DBL(now - _startTime) * _clockRate);
        senderReport->setPacketCount(_packetsSent);
        senderReport->setByteCount(_bytesSent);
        return senderReport;
    }
    else {
        return NULL;
    }
}


void RTPSenderInfo::setStartTime(simtime_t startTime)
{
    _startTime = startTime;
}


void RTPSenderInfo::setClockRate(int clockRate)
{
    _clockRate = clockRate;
}


void RTPSenderInfo::setTimeStampBase(uint32 timeStampBase)
{
    _timeStampBase = timeStampBase;
}


void RTPSenderInfo::setSequenceNumberBase(uint16 sequenceNumberBase)
{
    _sequenceNumberBase = sequenceNumberBase;
}


bool RTPSenderInfo::toBeDeleted(simtime_t now)
{
    return false;
}

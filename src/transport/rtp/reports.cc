/***************************************************************************
                          reports.cc  -  description
                             -------------------
    begin                : Mon Nov 26 2001
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

/** \file reports.cc
 * This file contains the implementations of member functions of the
 * class INET_API SenderReport and ReceptionReport.
 */

#include "reports.h"


//
// SenderReport
//

Register_Class(SenderReport);


SenderReport::SenderReport() : cObject()
{
    _ntpTimeStamp = 0;
    _rtpTimeStamp = 0;
    _packetCount = 0;
    _byteCount = 0;
}


SenderReport::SenderReport(const SenderReport& senderReport) : cObject()
{
    operator=(senderReport);
}


SenderReport::~SenderReport()
{
}


SenderReport& SenderReport::operator=(const SenderReport& senderReport)
{
    cObject::operator=(senderReport);
    _ntpTimeStamp = senderReport._ntpTimeStamp;
    _rtpTimeStamp = senderReport._rtpTimeStamp;
    _packetCount = senderReport._packetCount;
    _byteCount = senderReport._byteCount;
    return *this;
}


SenderReport *SenderReport::dup() const
{
    return new SenderReport(*this);
}


std::string SenderReport::info()
{
    std::stringstream out;
    out << "SenderReport.timeStamp=" << _rtpTimeStamp;
    return out.str();
}


void SenderReport::dump(std::ostream& os) const
{
    os << "SenderReport:" << endl;
    os << "  ntpTimeStamp = " << _ntpTimeStamp << endl;
    os << "  rtpTimeStamp = " << _rtpTimeStamp << endl;
    os << "  packetCount = " << _packetCount << endl;
    os << "  byteCount = " << _byteCount << endl;
}


uint64 SenderReport::getNTPTimeStamp()
{
    return _ntpTimeStamp;
}


void SenderReport::setNTPTimeStamp(uint64 ntpTimeStamp)
{
    _ntpTimeStamp = ntpTimeStamp;
}


uint32 SenderReport::getRTPTimeStamp()
{
    return _rtpTimeStamp;
}


void SenderReport::setRTPTimeStamp(uint32 rtpTimeStamp)
{
    _rtpTimeStamp = rtpTimeStamp;
}


uint32 SenderReport::getPacketCount()
{
    return _packetCount;
}


void SenderReport::setPacketCount(uint32 packetCount)
{
    _packetCount = packetCount;
}


uint32 SenderReport::getByteCount()
{
    return _byteCount;
}


void SenderReport::setByteCount(uint32 byteCount)
{
    _byteCount = byteCount;
}

//
// ReceptionReport
//

Register_Class(ReceptionReport);


ReceptionReport::ReceptionReport() : cObject()
{
    _ssrc = 0;
    _fractionLost = 0;
    _packetsLostCumulative = 0;
    _extendedHighestSequenceNumber = 0;
    _jitter = 0;
    _lastSR = 0;
    _delaySinceLastSR = 0;
}


ReceptionReport::ReceptionReport(const ReceptionReport& receptionReport) : cObject()
{
    operator=(receptionReport);
}


ReceptionReport::~ReceptionReport()
{
}


ReceptionReport& ReceptionReport::operator=(const ReceptionReport& receptionReport)
{
    cObject::operator=(receptionReport);
    _ssrc = receptionReport._ssrc;
    _fractionLost = receptionReport._fractionLost;
    _packetsLostCumulative = receptionReport._packetsLostCumulative;
    _extendedHighestSequenceNumber = receptionReport._extendedHighestSequenceNumber;
    _jitter = receptionReport._jitter;
    _lastSR = receptionReport._lastSR;
    _delaySinceLastSR = receptionReport._delaySinceLastSR;
    return *this;
}


ReceptionReport *ReceptionReport::dup() const
{
    return new ReceptionReport(*this);
}


std::string ReceptionReport::info()
{
    std::stringstream out;
    out << "ReceptionReport.ssrc=" << _ssrc;
    return out.str();
}


void ReceptionReport::dump(std::ostream& os) const
{
    os << "ReceptionReport:" << endl;
    os << "  ssrc = " << _ssrc << endl;
    os << "  fractionLost = " << (int)_fractionLost << endl;
    os << "  packetsLostCumulative = " << _packetsLostCumulative << endl;
    os << "  extendedHighestSequenceNumber = " << _extendedHighestSequenceNumber << endl;
    os << "  jitter = " << _jitter << endl;
    os << "  lastSR = " << _lastSR << endl;
    os << "  delaySinceLastSR = " << _delaySinceLastSR << endl;
}


uint32 ReceptionReport::getSSRC()
{
    return _ssrc;
}


void ReceptionReport::setSSRC(uint32 ssrc)
{
    _ssrc = ssrc;
}


uint8 ReceptionReport::getFractionLost()
{
    return _fractionLost;
}


void ReceptionReport::setFractionLost(uint8 fractionLost)
{
    _fractionLost = fractionLost;
}


int ReceptionReport::getPacketsLostCumulative()
{
    return _packetsLostCumulative;
}


void ReceptionReport::setPacketsLostCumulative(int packetsLostCumulative)
{
    _packetsLostCumulative = packetsLostCumulative;
}


uint32 ReceptionReport::getSequenceNumber()
{
    return _extendedHighestSequenceNumber;
}


void ReceptionReport::setSequenceNumber(uint32 sequenceNumber)
{
    _extendedHighestSequenceNumber = sequenceNumber;
}


int ReceptionReport::getJitter()
{
    return _jitter;
}


void ReceptionReport::setJitter(int jitter)
{
    _jitter = jitter;
}


int ReceptionReport::getLastSR()
{
    return _lastSR;
}


void ReceptionReport::setLastSR(int lastSR)
{
    _lastSR = lastSR;
}


int ReceptionReport::getDelaySinceLastSR()
{
    return _delaySinceLastSR;
}


void ReceptionReport::setDelaySinceLastSR(int delaySinceLastSR)
{
    _delaySinceLastSR = delaySinceLastSR;
}

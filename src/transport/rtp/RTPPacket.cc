/***************************************************************************
                          RTPPacket.cc  -  description
                             -------------------
    begin                : Mon Oct 22 2001
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

/** \file RTPPacket.cc
 * This file contains the implementaion of member functions of the class RTPPacket.
 */

#include "RTPPacket.h"

Register_Class(RTPPacket);


RTPPacket::RTPPacket(const char *name) : cPacket(name)
{
    _version = 2;
    _padding = 0;
    _extension = 0;
    _csrcCount = 0;
    _marker = 0;
    _payloadType = 0;
    _sequenceNumber = 0;
    _timeStamp = 0;
    _ssrc = 0;

    // a standard rtp packet without csrcs and data has a length of 12 bytes
    setByteLength(getFixedHeaderLength());
}


RTPPacket::RTPPacket(const RTPPacket& packet) : cPacket()
{
    setName(packet.getName());
    operator=(packet);
}


RTPPacket::~RTPPacket()
{
    // when csrcList is implemented this
    // should free the memory used for it
}


RTPPacket *RTPPacket::dup() const
{
    return new RTPPacket(*this);
}


RTPPacket& RTPPacket::operator=(const RTPPacket& packet)
{
    cPacket::operator=(packet);
    _version = packet._version;
    _padding = packet._padding;
    _extension = packet._extension;
    _csrcCount = packet._csrcCount;
    _marker = packet._marker;
    _payloadType = packet._payloadType;
    _sequenceNumber = packet._sequenceNumber;
    _timeStamp = packet._timeStamp;
    _ssrc = packet._ssrc;
    return *this;
}


std::string RTPPacket::info()
{
    std::stringstream out;
    out << "RTPPacket: payloadType=" << _payloadType << " payloadLength=" << getPayloadLength();
    return out.str();
}


void RTPPacket::dump()
{
    ev << "RTPPacket:" << endl;
    ev << "  payloadType = " << _payloadType << endl;
    ev << "  sequenceNumber = " << _sequenceNumber << endl;
    ev << "  timeStamp = " << _timeStamp << endl;
    ev << "  payloadLength = " << getPayloadLength() << endl;
}


int RTPPacket::getMarker()
{
    return _marker;
}


void RTPPacket::setMarker(int marker)
{
    _marker = marker;
}


int RTPPacket::getPayloadType()
{
    return _payloadType;
}


void RTPPacket::setPayloadType(int payloadType)
{
    _payloadType = payloadType;
}


uint16 RTPPacket::getSequenceNumber()
{
    return _sequenceNumber;
}


void RTPPacket::setSequenceNumber(uint16 sequenceNumber)
{
    _sequenceNumber = sequenceNumber;
}


uint32 RTPPacket::getTimeStamp()
{
    return _timeStamp;
}


void RTPPacket::setTimeStamp(uint32 timeStamp)
{
    _timeStamp = timeStamp;
}


uint32 RTPPacket::getSSRC()
{
    return _ssrc;
}


void RTPPacket::setSSRC(uint32 ssrc)
{
    _ssrc = ssrc;
}

int RTPPacket::getFixedHeaderLength()
{
    return 12;
}

int RTPPacket::getHeaderLength()
{
    // fixed header is 12 bytes long,
    // add 4 bytes for every csrc identifier
    return(getFixedHeaderLength() + 4 * _csrcCount);
}


int RTPPacket::getPayloadLength()
{
    return(getByteLength() - getHeaderLength());
}




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


#include "RTPPacket.h"


Register_Class(RTPPacket);

std::string RTPPacket::info() const
{
    std::stringstream out;
    out << "RTPPacket: payloadType=" << payloadType_var
        << " payloadLength=" << getPayloadLength();
    return out.str();
}

void RTPPacket::dump() const
{
    ev << "RTPPacket:" << endl;
    ev << "  payloadType = " << payloadType_var << endl;
    ev << "  sequenceNumber = " << sequenceNumber_var << endl;
    ev << "  timeStamp = " << timeStamp_var << endl;
    ev << "  payloadLength = " << getPayloadLength() << endl;
}

int RTPPacket::getHeaderLength() const
{
    // fixed header is 12 bytes long,
    // add 4 bytes for every csrc identifier
    return (RTPPACKET_FIX_HEADERLENGTH + 4 * getCsrcArraySize());
}

int RTPPacket::getPayloadLength() const
{
    return (getByteLength() - getHeaderLength());
}

/***************************************************************************
                          RtpPacket.cc  -  description
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

#include "inet/transportlayer/rtp/RtpPacket_m.h"

namespace inet {

namespace rtp {

std::string RtpHeader::str() const
{
    std::stringstream out;
    out << "RtpHeader: payloadType=" << payloadType;
    return out.str();
}

void RtpHeader::dump() const
{
    EV_INFO << "RtpHeader:" << endl;
    EV_INFO << "  payloadType = " << payloadType << endl;
    EV_INFO << "  sequenceNumber = " << sequenceNumber << endl;
    EV_INFO << "  timeStamp = " << timeStamp << endl;
}

} // namespace rtp

} // namespace inet


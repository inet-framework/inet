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


#include "reports.h"


//
// SenderReport
//

Register_Class(SenderReport);


std::string SenderReport::info() const
{
    std::stringstream out;
    out << "SenderReport.timeStamp=" << RTPTimeStamp_var;
    return out.str();
}

void SenderReport::dump(std::ostream& os) const
{
    os << "SenderReport:" << endl;
    os << "  ntpTimeStamp = " << NTPTimeStamp_var << endl;
    os << "  rtpTimeStamp = " << RTPTimeStamp_var << endl;
    os << "  packetCount = " << packetCount_var << endl;
    os << "  byteCount = " << byteCount_var << endl;
}


//
// ReceptionReport
//

Register_Class(ReceptionReport);

std::string ReceptionReport::info() const
{
    std::stringstream out;
    out << "ReceptionReport.ssrc=" << ssrc_var;
    return out.str();
}

void ReceptionReport::dump(std::ostream& os) const
{
    os << "ReceptionReport:" << endl;
    os << "  ssrc = " << ssrc_var << endl;
    os << "  fractionLost = " << (int)fractionLost_var << endl;
    os << "  packetsLostCumulative = " << packetsLostCumulative_var << endl;
    os << "  extendedHighestSequenceNumber = " << sequenceNumber_var << endl;
    os << "  jitter = " << jitter_var << endl;
    os << "  lastSR = " << lastSR_var << endl;
    os << "  delaySinceLastSR = " << delaySinceLastSR_var << endl;
}

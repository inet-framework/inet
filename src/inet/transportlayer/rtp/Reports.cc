//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
/***************************************************************************
                          reports.cc  -  description
                             -------------------
    begin                : Mon Nov 26 2001
    copyright            : (C) 2001 by Matthias Oppitz
    email                : Matthias.Oppitz@gmx.de
***************************************************************************/

#include "inet/transportlayer/rtp/Reports_m.h"

namespace inet {

namespace rtp {

//
// SenderReport
//

Register_Class(SenderReport);

std::string SenderReport::str() const
{
    std::stringstream out;
    out << "SenderReport.timeStamp=" << getRTPTimeStamp();
    return out.str();
}

void SenderReport::dump(std::ostream& os) const
{
    os << "SenderReport:" << endl;
    os << "  ntpTimeStamp = " << getNTPTimeStamp() << endl;
    os << "  rtpTimeStamp = " << getRTPTimeStamp() << endl;
    os << "  packetCount = " << getPacketCount() << endl;
    os << "  byteCount = " << getByteCount() << endl;
}

//
// ReceptionReport
//

Register_Class(ReceptionReport);

std::string ReceptionReport::str() const
{
    std::stringstream out;
    out << "ReceptionReport.ssrc=" << getSsrc();
    return out.str();
}

void ReceptionReport::dump(std::ostream& os) const
{
    os << "ReceptionReport:" << endl;
    os << "  ssrc = " << getSsrc() << endl;
    os << "  fractionLost = " << (int)getFractionLost() << endl;
    os << "  packetsLostCumulative = " << getPacketsLostCumulative() << endl;
    os << "  extendedHighestSequenceNumber = " << getSequenceNumber() << endl;
    os << "  jitter = " << getJitter() << endl;
    os << "  lastSR = " << getLastSR() << endl;
    os << "  delaySinceLastSR = " << getDelaySinceLastSR() << endl;
}

} // namespace rtp

} // namespace inet


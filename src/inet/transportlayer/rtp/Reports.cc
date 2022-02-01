//
// Copyright (C) 2001 Matthias Oppitz <Matthias.Oppitz@gmx.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

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


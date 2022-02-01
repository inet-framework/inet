//
// Copyright (C) 2001 Matthias Oppitz <Matthias.Oppitz@gmx.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

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


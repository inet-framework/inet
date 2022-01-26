//
// Copyright (C) 2001 Matthias Oppitz <Matthias.Oppitz@gmx.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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


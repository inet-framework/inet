//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

import inet.transportlayer.rtp.RTCPPacket1;

cplusplus {{
#include "inet/transportlayer/rtp/reports.h"
#include "inet/transportlayer/rtp/sdes.h"
#include "inet/transportlayer/rtp/RTCPPacket1.h"
}}

namespace inet::rtp;

packet RTCPReceiverReportPacket extends RTCPPacket
{
    packetType = RTCP_PT_RR;
    @customize(true);  // see the generated C++ header for more info
    uint32 ssrc = 0;
    cArray receptionReports;
}

packet RTCPSDESPacket extends RTCPPacket
{
    packetType = RTCP_PT_SDES;
    @customize(true);  // see the generated C++ header for more info
    cArray sdesChunks;
}

packet RTCPByePacket extends RTCPPacket
{
    packetType = RTCP_PT_BYE;
    count = 1;
    @customize(true);  // see the generated C++ header for more info
    uint32 ssrc = 0;
}

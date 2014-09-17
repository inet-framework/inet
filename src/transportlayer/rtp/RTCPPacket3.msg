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

cplusplus {{
#include "inet/transportlayer/rtp/RTCPPacket2.h"
#include "inet/transportlayer/rtp/reports.h"
#include "inet/transportlayer/rtp/sdes.h"
}}

namespace inet::rtp;

class noncobject SenderReport;
packet RTCPReceiverReportPacket;

packet RTCPSenderReportPacket extends RTCPReceiverReportPacket
{
    packetType = RTCP_PT_SR;
    @customize(true);  // see the generated C++ header for more info
    SenderReport senderReport;
}

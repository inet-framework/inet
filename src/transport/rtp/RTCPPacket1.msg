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

//
// TODO generated message class
//

class cArray;

enum RTCPPacketType
{
    RTCP_PT_UNDEF =   0; // default value undefined
    RTCP_PT_SR    = 200; // sender report
    RTCP_PT_RR    = 201; // receiver report
    RTCP_PT_SDES  = 202; // source description
    RTCP_PT_BYE   = 203; // bye
};

packet RTCPPacket
{
    byteLength = 4;
    @customize(true);  // see the generated C++ header for more info
    int8 version = 2;
    bool padding = 0;
    short count = 0;
    short packetType enum(RTCPPacketType) = RTCP_PT_UNDEF; 
    abstract int rtcpLength;
}

packet RTCPCompoundPacket
{
    byteLength = 0;
    @customize(true);  // see the generated C++ header for more info
    cArray rtcpPackets;
}

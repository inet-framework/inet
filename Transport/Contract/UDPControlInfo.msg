//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 Andras Varga
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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


cplusplus {{
#include "IPvXAddress.h"
}}

class noncobject IPvXAddress;

//
// \UDP command codes, sent by the application to UDP. These constants
// should be set as message kind on a message sent to the UDP entity.
// UDP_C_BIND (UDP_C_UNBIND) tells UDP that the app is interested
// (no longer interested) in receiving \UDP packets on the given port;
// the message object is discarded, and only the control info's srcPort
// field is significant.
//
// @see UDPControlInfo
//
enum UDPCommandCode
{
    UDP_C_DATA = 0;   // datagram to send (must carry UDPControlInfo)
    UDP_C_BIND = 1;   // only UDPControlInfo's srcPort should be filled
    UDP_C_UNBIND = 2; // only UDPControlInfo's srcPort should be filled
};


//
// Control info for sending/receiving data via UDP. Travels between
// application and UDP.
//
// @see UDPCommandCode
//
class UDPControlInfo
{
    fields:
        IPvXAddress srcAddr;  // source \IP or \IPv6 address
        IPvXAddress destAddr; // destination \IP or \IPv6 address
        int srcPort;   // \UDP source port in packet
        int destPort;  // \UDP destination port in packet
        int inputPort; // gate index of interface on which the packet was received
        int outputPort = -1; // gate index of interface on where the packet should go 
                             // (for multicast; currently it works for IPv4 only)
};


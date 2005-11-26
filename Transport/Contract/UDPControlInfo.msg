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
// @see UDPControlInfo, UDPStatusInd, 2UDP
//
enum UDPCommandCode
{
    UDP_C_DATA = 0;   // datagram to send (must carry UDPControlInfo)
    UDP_C_BIND = 1;   // at least UDPControlInfo's srcPort and sockId must be filled
    UDP_C_UNBIND = 2; // UDPControlInfo's sockId must be filled
};

//
// \UDP indications, sent by UDP to the application. UDP will set these
// constants as message kind on messages it sends to the application.
//
// @see UDPControlInfo, UDPCommandCode, UDP
//
enum UDPStatusInd
{
    UDP_I_DATA = 0;        // data packet (set on data packet)
    UDP_I_PEER_CLOSED = 1; // ICMP error received on a sent datagram
};


//
// Control info for sending/receiving data via UDP. Travels between
// application and UDP.
//
//# TODO explain userId, sockId, etc.
//
// @see UDPCommandCode
//
class UDPControlInfo
{
    properties:
        omitGetVerb = true;
    fields:
        int sockId = -1;   // identifies the socket within the application
        int userId = -1;   // id than can be freely used by the app
        IPvXAddress srcAddr;  // source \IP or \IPv6 address, or local address with BIND
        IPvXAddress destAddr; // destination \IP or \IPv6 address
        int srcPort;   // \UDP source port in packet, or local port with BIND
        int destPort;  // \UDP destination port in packet
        int interfaceId = -1; // interface on which pk was received/should be sent (see InterfaceTable)
};


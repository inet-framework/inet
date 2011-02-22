//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


class noncobject IPAddress;

class noncobject MACAddress;

cplusplus {{
#include "IPAddress.h"
#include "MACAddress.h"

// ARP header length for IPv4 (4-byte addresses) and 802 LANs (6-byte MAC addrs)
#define ARP_HEADER_BYTES   28
}}



//
// The list of practically important ARP opcodes
//
enum ARPOpcode
{

    ARP_REQUEST = 1;      // Request. RFC 826
    ARP_REPLY = 2;        // Reply. RFC 826, RFC 1868
    ARP_RARP_REQUEST = 3; // Request Reverse. RFC 903
    ARP_RARP_REPLY = 4;   // Reply Reverse. RFC 903
}


//
// ARP packet. This is a specialized version: prepared for IEEE 802 hardware
// addresses and IPv4. Packet fields are therefore represented by C++ classes
// MACAddress and IPAddress. Also, some ARP protocol header fields are
// not modelled explicitly (their values are implied):
//   - hardwareType (not needed for modelling);
//   - protocol type (0x800 IPv4)
//   - hardware address length (6)
//   - protocol address length (4)
//
packet ARPPacket
{
    int opcode enum(ARPOpcode);
    MACAddress srcMACAddress;
    MACAddress destMACAddress;
    IPAddress srcIPAddress;
    IPAddress destIPAddress;
}



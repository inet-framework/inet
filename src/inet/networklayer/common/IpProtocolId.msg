//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, see <http://www.gnu.org/licenses/>.
//


import inet.common.INETDefs;

namespace inet;

//
// Protocol field: taken from RFC 1700.
// Replacing old RFC1700 with newer (28/06/01) Protocol Numbers
//
enum IpProtocolId
{

    //  IP_PROT_HOPOPT    = 0;
    IP_PROT_ICMP = 1;
    IP_PROT_IGMP = 2;
    IP_PROT_IP = 4; // used for IP tunneling. FIXME: RFC1700 specifies =3 (old IPSuite) ???
    IP_PROT_TCP = 6;
    IP_PROT_EGP = 8;
    IP_PROT_IGP = 9;
    IP_PROT_UDP = 17;
    IP_PROT_XTP = 36;
    IP_PROT_IPv6 = 41;
    IP_PROT_RSVP = 46;
    IP_PROT_IPv6_ICMP = 58;
    IP_PROT_NONE = 59;
    IP_PROT_EIGRP = 88;
    IP_PROT_OSPF = 89;
    IP_PROT_OSPFv3 = 90;
    IP_PROT_PIM = 103;
    IP_PROT_VRRP = 112;
    IP_PROT_SCTP = 132;
    IP_PROT_DSR  = 48;
    IP_PROT_MANET = 138;

    IP_PROT_IPv6EXT_HOP = 0;       // Hop-by-hop option header.
    IP_PROT_IPv6EXT_DEST = 60;     // Destination options header.
    IP_PROT_IPv6EXT_ROUTING = 43;  // Routing header.
    IP_PROT_IPv6EXT_FRAGMENT = 44; // Fragmentation/reassembly header.
    IP_PROT_IPv6EXT_AUTH = 51;     // Authentication header.
    IP_PROT_IPv6EXT_ESP = 50;      // Encapsulating security payload.
    IP_PROT_IPv6EXT_MOB = 135;      // Mobility Header for MIPv6 support.

    IP_PROT_FLOODING = 250;    // INET specific: Probabilistic Network Protocol
    IP_PROT_PROBABILISTIC = 251;    // INET specific: Probabilistic Network Protocol
    IP_PROT_WISE = 252;    // INET specific: Probabilistic Network Protocol
    IP_PROT_NEXT_HOP_FORWARDING = 253; // INET specific: Next Hop Forwarding
    IP_PROT_ECHO = 254; // INET specific: Echo Protocol
}


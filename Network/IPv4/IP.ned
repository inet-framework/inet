//
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


//
// Implements the \IP protocol. This is the all-in-one version of IPProcessing.
//
// The \IP protocol header is represented by the IPDatagram message class.
//
// To send a packet over IP, fill in an IPControlInfo object,
// attach it to the packet with the C++ method setControlInfo(),
// the send it to the IP module.
//
// Similarly, IP will attach IPControlInfo to the packets it sends
// up to the higher layers.
//
// IP can serve several higher-layer protocols. When delivering packets
// to them, the output gate is determined from the Protocol field in the \IP header.
// The protocol-to-gateindex mapping must be given in the protocolmapping
// string parameter, in the following format:
// <pre>
// protocolnumber:gateindex, protocolnumber:gateindex, ...
// </pre>
//
// The recommended setting is:
// - TCP (6) : 0
// - UDP (17) : 1
// - ICMP (1) : 2
// - IGMP (2) : 3
// - \RSVP (46) : 4
// That is, <tt>protocolMapping="6:0,17:1,1:2,2:3,46:4"</tt>
//
// @see IPProcessing
//
// @author Andras Varga
//
simple IP
    parameters:
        timeToLive : numeric const,
        multicastTimeToLive : numeric const,
        protocolMapping : string,
        fragmentTimeout : numeric const;
    gates:
        in: transportIn[];
        out: transportOut[];
        in: queueIn[];
        out: queueOut[];
endsimple


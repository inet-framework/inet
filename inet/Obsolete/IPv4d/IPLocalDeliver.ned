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


//
// Handles local delivery of \IP datagrams, the main task being defragmentation.
//
// Receive \IP datagram for local delivery:
// - strip off \IP header
// - buffer fragments
// - wait until all fragments of one fragment number are received
// - discard without notification if not all fragments arrives within timeout
//
// Once all fragments have arrived:
// - send encapsulated \IP packet (tunnelling) to preRoutingOut
// - send other packets on corresponding transportOut[] gate
//
// The output gate is determined from the Protocol field in the \IP header.
// The protocol-to-gateindex mapping must be given in the protocolmapping
// string parameter, in the following format:
// <pre>
// protocolnumber:gateindex, protocolnumber:gateindex, ...
// </pre>
//
// The recommended setting is:
// - \TCP (6) : 0
// - \UDP (17) : 1
// - \ICMP (1) : 2
// - \IGMP (2) : 3
// - \RSVP (46) : 4
// That is, <tt>protocolMapping="6:0,17:1,1:2,2:3,46:4"</tt>
//
// Author: Jochen Reber, date: 2.5.00, 9.5.00, 20.5.00, 9.6.00
// Rewrite: Andras Varga, 2004
//
simple IPLocalDeliver
{
    parameters:
        string protocolMapping;
        double fragmentTimeout;
        @display("i=block/fork");
    gates:
        input in[];
        output transportOut[];
        output preRoutingOut;
}



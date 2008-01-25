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
// Receives packet from transport layer, ICMP or IPTunneling (IP tunneled
// datagram) and encapsulates it into an IPDatagram.
//  - sets version
//  - sets TOS (currently  constant 0!!!)
//  - sets ds.codepoint
//  - sets TTL
//  - chooses and sets fragmentation identifier
//  - sets fragment offset = 0, more fragments = 0 (fragmentation occurs in IPFragmentation)
//  - sets 'don't Fragment'-bit to received value (default 0)
//  - sets Protocol to received value
//  - sets destination address to received value
//  - sends datagram to IPRouting
//  - if IPControlInfo is invalid (e.g. invalid source address), it is thrown away
//    without feedback
//
// Author: Jochen Reber
// Redesign: Andras Varga, 02.2004
//
simple IPSend
{
    parameters:
        volatile double timeToLive;
        volatile double multicastTimeToLive;
        @display("i=block/join");
    gates:
        input in[];
        output routingOut;
}



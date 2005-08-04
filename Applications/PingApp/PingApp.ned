//
// Copyright (C) 2001 Monash University, Australia
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
// Generates ping requests and calculates the packet loss and round trip
// parameters of the replies. 
//
// Start/stop time, interval etc can be specified via parameters. To disable,
// specify empty destAddr or stopTime<=startTime.
//
// Every ping request is sent out with a sequence number, and replies are
// expected to arrive in the same order. Whenever there's a jump in the
// in the received ping responses' sequence number (e.g. 1, 2, 3, 5), then
// the missing pings (number 4 in this example) is counted as lost.
// Then if it still arrives later (that is, a reply with a sequence number
// smaller than the largers one received so far) it will be counted as
// out-of-sequence arrival. So the number of really lost pings will be
// "lost" minus "out-of-order" (assuming there's no duplicate or bogus reply).
//
// Uses PingPayload as payload for the ICMP(v6) Echo Request/Reply packets.
//
// @see PingPayload, ICMP, ICMPv6Core
//
simple PingApp
    parameters:
        destAddr: string, // destination IP or IPv6 address
        srcAddr: string, // source IP or IPv6 address (useful with multi-homing)
        packetSize: numeric const, // of ping payload, in bytes
        interval: numeric, // time to wait between pings (can be random)
        hopLimit: numeric const, // TTL or hopLimit for IP packets
        count: numeric const, // stop after count ping requests, 0 means continuously
        startTime: numeric const, // send first ping at startTime
        stopTime: numeric const, // send no pings after stopTime, 0 means forever 
        printPing: bool; // dump on stdout
    gates:
        in: pingIn;
        out: pingOut;
        in: pingv6In;
        out: pingv6Out;
endsimple

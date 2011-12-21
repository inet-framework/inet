//
// Copyright (C) 2006 Andras Varga
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


package inet.transport.tcp_common;

import inet.transport.ITCP;


//
// Sends fabricated TCP packets. This is a base implementation,
// you'll probably have to customize it in C++ according to
// what you want to send and when you want to send it.
//
// Compatible with both IPv4 (~IPv4) and ~IPv6.
//
simple TCPSpoof like ITCP
{
    parameters:
        string srcAddress = default("");  // local address; may be left empty ("")
        string destAddress = default(""); // destination address
        int srcPort; // local port number
        int destPort; // destination port number
        int seqNo; // sequence number (-1 for TCP ISS)
        bool isSYN = default(true); // whether to set SYN bit on packet
        double t @unit("s"); // simulation time to send at
        @display("i=block/square");
        @signal[sentPk](type=cPacket);
        @statistic[sentPk](title="packets sent"; source=sentPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);

    gates:
        output ipOut @labels(TCPSegment,IPv4ControlInfo/down);
        output ipv6Out @labels(TCPSegment,IPv6ControlInfo/down);

        input appIn[] @labels(TCPCommand/down); // Not used!
        input ipIn @labels(TCPSegment,IPv4ControlInfo/up); // Not used!
        input ipv6In @labels(TCPSegment,IPv6ControlInfo/up); // Not used!
        output appOut[] @labels(TCPCommand/up); // Not used!
}


//
// Copyright (C) 2006 Andras Varga
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
// Sends fabricated TCP packets. This is a base implementation,
// you'll probably have to customize it in C++ according to
// what you want to send and when you want to send it.
//
// Compatible with both IPv4 and IPv6.
//
simple TCPSpoof
    parameters:
        srcAddress: string,  // local address; may be left empty ("")
        destAddress: string, // destination address
        srcPort: numeric const, // local port number
        destPort: numeric const, // destination port number
        seqNo: numeric const, // sequence number (-1 for TCP ISS)
        isSYN: bool, // whether to set SYN bit on packet
        t: numeric const; // simulation time to send at
    gates:
        out: ipv4Out;
        out: ipv6Out;
endsimple


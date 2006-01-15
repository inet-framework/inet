//
// (C) 2005 Vojta Janota
// (C) 2004 Andras Varga
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//


//
// Handles and processes LDP messages.
//
// The LDP protocol is used by one LSR (Label Switched \Router) to inform
// another LSR of the label bindings it has made. The LSR uses this protocol
// to establish label switched paths through a network by mapping network layer
// routing information directly to data-link layer switched paths.
//
// Operations
//
// All LDP message types are subclassed from LDPPacket, and include
// LDPHello, LDPNotify, LDPLabelRequest, LDPIni and LDPAddress.
//
// Message processing in the LDP module is explained in
// <a href="ldp-processing.html">LDP Message Processing</a>
//
// @author Andras Varga, Vojta Janota
//
simple LDP
    parameters:
        holdTime : numeric,
        helloInterval : numeric;
    gates:
        in: tcpIn;
        in: udpIn;
        out: tcpOut;
        out: udpOut;
endsimple

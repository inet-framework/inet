//
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
// LDPHello, LDPLabelMapping, LDPLabelRequest, LDPIni and LDPAddress.
//
// Message processing in NewLDP module is explained in
// <a href="ldp-processing.html">LDP Message Processing</a>
//
// Collaborations
//
// NewLDP is implemented as a simple module. It has buffers to store Label
// Request messages when finding the paths for FEC in these messages. NewLDP
// can communicate directly with MPLSModule. The following messages are
// exchanged between these two modules:
//
// - label requests for unknown FECs from MPLSModule to NewLDP
// - label replies for particular FECs from NewLDP module to MPLSModule
//
// @author Andras Varga, based on original implementation LDPproc
//
simple NewLDP

    parameters:
        routerId : string,
        helloTimeout : numeric,
        isIR: bool,
        isER: bool;

    gates:
        in: from_mpls_switch;  // receives messages here by direct sending
        in: from_tcp_interface;
        in: from_udp_interface;
        out: to_tcp_interface;
        out: to_udp_interface;

endsimple

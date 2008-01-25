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
// Message processing in LDPproc module is explained in
// <a href="ldp-processing.html">LDP Message Processing</a>
//
// Collaborations
//
// LDPproc is implemented as a simple module. It has buffers to store Label
// Request messages when finding the paths for FEC in these messages. LDPproc
// can communicate directly with MPLSModule. There are three types of
// messages exchanged between these two modules:
// - Messages of label request for unknown FEC from MPLS module to LDPproc
//   module
// - Messages of label replied for a particular FEC from LDPproc module to
//   MPLSModule
// - Messages of ready state signaling from LDPproc module to MPLSModule to
//   signal the MPLSModule that the signalling protocol has passed the
//   initialization stage.
//
simple LDPproc

{
    parameters:
        string local_addr;
        volatile double helloTimeout;
        volatile double peerNo;

    gates:
        input from_mpls_switch;  // receives messages here by direct sending
        input from_tcp_interface;
        input from_udp_interface;
        output to_mpls_switch;
        output to_tcp_interface;
        output to_udp_interface;

}

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
// Implements the MPLS protocol.
//
// MPLS module is added between layer 2 and 3 in the OSI model for fast packet
// switching. MPLS should be transparent to signaling protocols. Two signaling
// protocols currently implemented in this simulation are \RSVP-TE and LDP.
//
// Operations
//
// MPLS packets are represented by the message class MPLSPacket.
// The local LIB (Label Information Base) is stored in a LIBTable module
// in the LSR.
//
// For most of the time, the MPLS module will do the label swapping and
// message forwarding. Upon receiving a labelled packet from another LSR, the
// MPLS first extract the incoming interface and incoming label pair, and then
// look up the local LIB table. If an outgoing label and an outgoing interface
// can be found, the module will perform appropriate label operations (PUSH,
// POP, SWAP) based on the "opcode" presented in the LIB. Otherwise, it makes
// a query of label mapping to the LDP component of the parent LSR.
//
// Algorithm
//
// Upon receiving a packet (not signalling message), the MPLS module, based on
// the packet types, calls appropriate methods to process the packet.
// See <a href="mpls-pseudocode.html">MPLS Operation</a> for details.
//
// Upon receiving a replied message from the signalling module, the MPLS
// module will locate all queued packets which generated the request message
// for this replied message and encapsulate them in MPLS packets with label
// taken from the replied message. These MPLS packets are then forwarded to an
// appropriate next hop.
//
// Collaborations
//
// MPLS module is required to interact with L2 (Link Layer) and L3 (Network
// Layer) in the OSI model. In addition, it needs to obtain label information
// from the LIB component and label query result from the LDP module. Messages
// the model communicates with L2 and Network Layers are L2 packets (PPP,
// Frame Relay, ATM, Ethernet, etc.), and \IP native packets. Specifically,
// MPLS module encapsulates \IP packet and is encapsulated in L2 packet.
// Different L2 protocols may require different methods of encapsulation to
// inherit the L2’s QoS. This implementation follows a generic approach; it
// assumes no information of QoS from the link layer.
//
// Usages
//
// Values of IsIR and IsER must be input though LSR composite module for the
// MPLS to operate correctly. Both of them should be set to true if
// bidirectional LSPs (in this context, it means LSPs are setup from the
// sender to receiver and in opposite directions, these LSPs might be
// different) are desired.
//
simple MPLSModule
    parameters:
        classifier : numeric, // 1: by dest address; 2: by src & dest address
        isER : bool, // is Egress router?
        isIR : bool; // is Ingress router?

    gates:
        in: fromL3[];
        out: toL3[];
        in: fromL2[];
        out: toL2[];
        in: fromSignalModule;
        out: toSignalModule;

endsimple

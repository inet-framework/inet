//
// (C) 2005 Vojtech Janota
// (C) 2003 Xuan Thang Nguyen
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

package inet.networklayer.mpls;

//
// Implements the MPLS protocol.
//
// ~Mpls module is added between layer 2 and 3 in the OSI model for fast packet
// switching. ~Mpls should be transparent to signalling protocols. Signalling
// protocol currently implemented in this simulation is RSVP-TE.
//
// Operations
//
// MPLS packets are represented by the MplsPacket class.
// The local LIB (Label Information Base) is stored in a ~LibTable module
// in the LSR.
//
// For most of the time, the ~Mpls module will do the label swapping and
// message forwarding. Upon receiving a labelled packet from another LSR, the
// ~Mpls first extract the incoming interface and incoming label pair, and then
// look up the local LIB table. If an outgoing label and an outgoing interface
// can be found, the module will perform appropriate label operations (PUSH,
// POP, SWAP) based on the "outLabel" vector containing label and operation
// pairs.
//
// Collaborations
//
// ~Mpls module is required to interact with L2 (Link Layer) and L3 (Network
// Layer) in the OSI model. In addition, it needs to obtain label information
// from the LIB component and label query result from the ~Ldp module. Messages
// the model communicates with L2 and Network Layers are L2 packets (PPP,
// Frame Relay, ATM, Ethernet, etc.), and IP native packets. Specifically,
// ~Mpls module encapsulates IP packet and is encapsulated in L2 packet.
// Different L2 protocols may require different methods of encapsulation to
// inherit the L2's QoS. This implementation follows a generic approach; it
// assumes no information of QoS from the link layer.
//
simple Mpls
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string libTableModule;
        string classifierModule; // The path to the module which implements the IIngressClassifier C++ interface
        @display("i=block/switch");
    gates:
        input netwIn @labels(Ipv4Header);
        output netwOut @labels(Ipv4Header);
        input ifIn @labels(MplsPacket,Ipv4Header);
        output ifOut @labels(MplsPacket,Ipv4Header);
}


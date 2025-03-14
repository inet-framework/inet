//
// Copyright (C) 2010 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.contract;

//
// Module interface for Ethernet MAC (Media Access Control) implementations that handle
// frame transmission and reception according to the Ethernet protocol. Provides functionality
// for queuing and transmitting frames using CSMA/CD in half-duplex mode or direct transmission
// in full-duplex mode, handling collisions, and processing PAUSE frames for flow control.
//
// Expected environment:
// - phys$i and phys$o should be connected to the "network"
// - `upperLayerIn` and `upperLayerOut` are usually connected to ~EthernetEncapsulation (in hosts)
//   or ~IMacRelayUnit (in a switch)
//
// When a frame is received from the higher layers, it must be an Ethernet frame,
// and with all protocol fields filled out (including the destination MAC address).
// The source address, if left empty, will be filled in. Then the frame is queued
// and transmitted according to the CSMA/CD protocol.
//
// Data frames received from the network are Ethernet frames. They are passed to
// the higher layers without modification.
//
// Also, the module properly responds to PAUSE frames, but never sends them
// by itself -- however, it transmits PAUSE frames received from upper layers.
// See <a href="ether-pause.html">PAUSE handling</a> for more info.
//
// @see EthernetCsmaMacPhy, EthernetMacPhy, EthernetEncapsulation
//
moduleinterface IEtherMac
{
    parameters:
        bool promiscuous;       // If true, all packets are received, otherwise only the
                                // ones with matching destination MAC address
        bool duplexMode;        // Selects full-duplex (true) or half-duplex (false) operation
        int mtu @unit(B);
        @display("i=block/rxtx");
    gates:
        input upperLayerIn @labels(EtherFrame);    // To ~EthernetEncapsulation or ~IMacRelayUnit
        output upperLayerOut @labels(EtherFrame);  // To ~EthernetEncapsulation or ~IMacRelayUnit
        inout phys @labels(EthernetSignal); // To the network
}

//
// Copyright (C) 2010 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.contract;

//
// Interface for Ethernet MAC implementations. All Ethernet MAC implementations
// should implement this (i.e. declared as: EthernetCsmaMac like IEtherMac).
// The existing implementations are these: ~EthernetCsmaMac and ~EthernetMac.
//
// Doesn't do encapsulation/decapsulation; see ~EthernetEncapsulation for that.
//
// Expected environment:
// - phys$i and phys$o should be connected to the "network"
// - upperLayerIn and upperLayerOut are usually connected to ~EthernetEncapsulation (in hosts)
//   or ~IMacRelayUnit (in a switch)
//
// The module does not perform encapsulation or decapsulation of frames --
// this is done by higher layers (~EthernetEncapsulation).
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
moduleinterface IEtherMac
{
    parameters:
        bool promiscuous;       // if true, all packets are received, otherwise only the
                                // ones with matching destination MAC address
        bool duplexMode;        // selects full-duplex (true) or half-duplex (false) operation
        int mtu @unit(B);
        @display("i=block/rxtx");
    gates:
        input upperLayerIn @labels(EtherFrame);    // to ~EthernetEncapsulation or ~IMacRelayUnit
        output upperLayerOut @labels(EtherFrame);  // to ~EthernetEncapsulation or ~IMacRelayUnit
        inout phys @labels(EthernetSignal); // to the network
}


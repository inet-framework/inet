//
// Copyright (C) 2010 Zoltan Bojthe
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

package inet.linklayer.contract;

//
// Interface for Ethernet MAC implementations. All Ethernet MAC implementations
// should implement this (i.e. declared as: EtherMac like IEtherMac).
// The existing implementations are these: ~EtherMac and ~EtherMacFullDuplex.
//
// Doesn't do encapsulation/decapsulation; see ~EtherEncap for that.
//
// Expected environment:
// - phys$i and phys$o should be connected to the "network"
// - upperLayerIn and upperLayerOut are usually connected to ~EtherEncap (in hosts)
//   or ~IMacRelayUnit (in a switch)
//
// The module does not perform encapsulation or decapsulation of frames --
// this is done by higher layers (~EtherEncap).
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
        input upperLayerIn @labels(EtherFrame);    // to ~EtherEncap or ~IMacRelayUnit
        output upperLayerOut @labels(EtherFrame);  // to ~EtherEncap or ~IMacRelayUnit
        inout phys @labels(EtherFrame); // to physical layer or the network
}


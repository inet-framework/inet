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


package inet.linklayer;

//
// Interface for Ethernet MAC implementations. All Ethernet MAC implementations
// should implement this (i.e. declared as: EtherMAC like IEtherMAC).
// The existing implementations are these: ~EtherMAC and ~EtherMACFullDuplex.
//
// Doesn't do encapsulation/decapsulation; see ~EtherLLC and ~EtherEncap for
// that.
//
// Expected environment:
// - phys$i and phys$o should be connected to the "network"
// - upperLayerIn and upperLayerOut are usually connected to ~EtherLLC (in hosts)
//   or ~IMACRelayUnit (in a switch)
//
// The module does not perform encapsulation or decapsulation of frames --
// this is done by higher layers (~EtherLLC or ~EtherEncap).
//
// When a frame is received from the higher layers, it must be an ~EtherFrame,
// and with all protocol fields filled out
// (including the destination MAC address). The source address, if left empty,
// will be filled in. Then frame is queued and transmitted according
// to the CSMA/CD protocol.
//
// Data frames received from the network are EtherFrames. They are passed to
// the higher layers without modification.
// Also, the module properly responds to PAUSE frames, but never sends them
// by itself -- however, it transmits PAUSE frames received from upper layers.
// See <a href="ether-pause.html">PAUSE handling</a> for more info.
//
moduleinterface IEtherMAC
{
    parameters:
        bool promiscuous;       // if true, all packets are received, otherwise only the
                                // ones with matching destination MAC address
        string address;         // MAC address as hex string (12 hex digits), or
                                // "auto". "auto" values will be replaced by
                                // a generated MAC address in init stage 0.
        bool duplexMode;        // selects full-duplex (true) or half-duplex (false) operation
        int txQueueLimit;       // maximum number of frames queued up for transmission;
                                // additional frames are dropped. Only used if queueModule==""
        string queueModule;     // name of optional external queue module
        int mtu @unit("B");
        @display("i=block/rxtx");
    gates:
        input upperLayerIn @labels(EtherFrame);    // to ~EtherLLC or ~IMACRelayUnit
        output upperLayerOut @labels(EtherFrame);  // to ~EtherLLC or ~IMACRelayUnit
        inout phys @labels(EtherFrame); // to physical layer or the network
}


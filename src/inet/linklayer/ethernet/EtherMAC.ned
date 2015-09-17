//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
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


package inet.linklayer.ethernet;

import inet.linklayer.contract.IEtherMAC;


//
// Ethernet MAC layer. MAC performs transmission and reception of frames.
// See the ~IEtherMAC for the Ethernet MAC layer general informations.
// Doesn't do encapsulation/decapsulation; see ~EtherLLC and ~EtherEncap for
// that.
//
// Supported variations:
// - 10Mb Ethernet (duplex and half-duplex, coaxial cable or twisted pair)
// - 100Mb Ethernet (duplex and half-duplex)
// - 1Gb Ethernet (duplex and half-duplex)
// - 10Gb Ethernet
// - 40Gb Ethernet
// - 100Gb Ethernet
//
// Supports all three Ethernet frame types. (It handles ~EtherFrame message class;
// specific frame classes (Ethernet-II, IEEE 802.3) are subclassed from that one.)
// RAW mode (only used by the IPX protocol) is not supported.
//
// Expected environment:
// - phys$i and phys$o should be connected to the "network"
// - upperLayerIn and upperLayerOut are usually connected to ~EtherLLC (in hosts)
//   or ~MACRelayUnit (in a switch)
//
// <b>Operation</b>
//
// Processing of frames received from higher layers:
// - if src address in the frame is empty, fill it out
// - frames get queued up until transmission
// - transmit according to the CSMA/CD protocol
// - can send PAUSE message if requested by higher layers (PAUSE protocol,
//   used in switches).
//
// Processing of frames incoming from the network:
// - receive according to the CSMA/CD protocol
// - CRC checking (frames with the error bit set are discarded).
// - respond to PAUSE frames
// - in promiscuous mode, pass up all received frames;
//   otherwise, only frames with matching MAC addresses and
//   broadcast frames are passed up.
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
// For more info see <a href="ether-overview.html">Ethernet Model Overview</a>.
//
// <b>Disabling and disconnecting</b>
//
// If the MAC is not connected to the network ("cable unplugged"), it will
// start up in "disabled" mode. A disabled MAC simply discards any messages
// it receives. It is currently not supported to dynamically connect/disconnect
// a MAC.
//
//
// <b>Queueing</b>
//
// In routers, MAC relies on an external queue module (see ~IOutputQueue)
// to model finite buffer, implement QoS and/or RED, and requests packets
// from this external queue one-by-one.
//
// In hosts, no such queue is used, so MAC contains an internal
// queue named txQueue to queue up packets waiting for transmission.
// Conceptually, txQueue is of infinite size, but for better diagnostics
// one can specify a hard limit in the txQueueLimit parameter -- if this is
// exceeded, the simulation stops with an error.
//
//
// <b>Physical layer messaging</b>
//
// Please see <a href="physical.html">Messaging on the physical layer</a>.
//
// @see ~EtherMACFullDuplex, ~EthernetInterface, ~IOutputQueue, ~EtherEncap, ~EtherLLC
// @see ~EtherFrame, ~EthernetIIFrame, ~EtherFrameWithLLC, ~Ieee802Ctrl
//
simple EtherMAC like IEtherMAC
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        bool promiscuous = default(false);  // if true, all packets are received, otherwise only the
                                            // ones with matching destination MAC address
        string address = default("auto");   // MAC address as hex string (12 hex digits), or
                                            // "auto". "auto" values will be replaced by
                                            // a generated MAC address in init stage 0.
        bool duplexMode = default(true);    // selects full-duplex (true) or half-duplex (false) operation
        int txQueueLimit = default(1000);   // maximum number of frames queued up for transmission in the internal queue (only used if queueModule==""); additional frames cause a runtime error
        string queueModule = default("");   // name of optional external queue module
        bool frameBursting = default(true); // enable/disable frame bursting mode in Gigabit Ethernet
        int mtu @unit("B") = default(1500B);
        bool connectionColoring = default(true); // colors the connection when transmitting
        @display("i=block/rxtx");

        @signal[txPk](type=EtherFrame);
        @signal[rxPkOk](type=EtherFrame);
        @signal[txPausePkUnits](type=long);
        @signal[rxPausePkUnits](type=long);
        @signal[rxPkFromHL](type=EtherFrame);
        @signal[dropPkNotForUs](type=EtherFrame);
        @signal[dropPkBitError](type=EtherFrame);
        @signal[dropPkFromHLIfaceDown](type=EtherFrame);
        @signal[dropPkIfaceDown](type=EtherTraffic);        // emitted at begin of receiving
        @signal[packetSentToLower](type=EtherTraffic);
        @signal[packetReceivedFromLower](type=EtherTraffic);
        @signal[packetSentToUpper](type=EtherFrame);
        @signal[packetReceivedFromUpper](type=EtherFrame);
        @signal[collision](type=long);
        @signal[backoff](type=long);
        @signal[transmitState](type=long); // enum=MACTransmitState
        @signal[receiveState](type=long); // enum=MACReceiveState

        @statistic[txPk](title="packets transmitted"; source=txPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[rxPkOk](title="packets received OK"; source=rxPkOk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[passedUpPk](title="packets passed to higher layer"; source=packetSentToUpper; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[txPausePkUnits](title="pause units sent"; record=count,sum,vector; interpolationmode=none);
        @statistic[rxPausePkUnits](title="pause units received"; record=count,sum,vector; interpolationmode=none);
        @statistic[rxPkFromHL](title="packet bytes from higher layer"; source=rxPkFromHL; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[droppedPkIfaceDown](title="packets dropped/interface down"; source=dropPkIfaceDown; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[droppedPkBitError](title="packets dropped/bit error"; source=dropPkBitError; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[droppedPkNotForUs](title="packets dropped/not for us"; source=dropPkNotForUs; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[collision](title="collision"; record=count,vector; interpolationmode=none);
        @statistic[backoff](title="backoff"; record=count,vector; interpolationmode=none);

    gates:
        input upperLayerIn @labels(EtherFrame);    // to ~EtherLLC or ~EtherEncap or ~IMACRelayUnit
        output upperLayerOut @labels(EtherFrame);  // to ~EtherLLC or ~EtherEncap or ~IMACRelayUnit
        inout phys @labels(EtherFrame); // to physical layer or the network
}


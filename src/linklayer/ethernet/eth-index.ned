//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
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


//
// @page eth-index.html, Ethernet Model Index
//
// The Ethernet model contains MAC models (a full-featured ~EtherMAC,
// and a simplified, duplex-only ~EtherMACFullDuplex);
// encapsulation/decapsulation models (~EtherEncap, ~EtherLLC);
// bus and hub models ((~EtherBus for modelling coaxial cable, and ~EtherHub);
// a switch model (~EtherSwitch) with various relay units (~IMACRelayUnit,
// ~MACRelayUnitNP and ~MACRelayUnitPP); some traffic generators
// (~EtherTrafGen, ~EtherAppCli/~EtherAppSrv); and example host models
// (~EtherHost, ~EtherHost2). In INET hosts, an Ethernet interface is
// represented by ~EthernetInterface.
//
// A few useful topics:
//  - example networks: ~MixedLAN, ~LargeNet
//  - <a href="ether-overview.html">model overview</a>
//  - <a href="eth-standards.html">implemented standards</a>
//  - <a href="llc-app.html">communication between LLC and higher layers</a>
//  - <a href="appreg.html">application registration</a>
//  - <a href="physical.html">physical layer messaging</a>
//  - click <i>selected topics</i> in the navigation frame to see more topics
//
//
// @page ether-overview.html, Ethernet Model Overview
//
// <h3>Data packets and Ethernet frames</h3>
//
// Ethernet frames carry data packets as encapsulated cPacket objects.
// Data packets can be of any message type (cPacket or cPacket subclass).
//
// The model encapsulates data packets in Ethernet frames using the <tt>encapsulate()</tt>
// method of cPacket. Encapsulate() updates the length of the Ethernet frame too,
// so the model doesn't have to take care of that.
//
// The models supports various Ethernet frame types: Ethernet II, 803.2 with
// LLC header, and 803.3 with LLC and SNAP headers. The corresponding classes are:
// ~EthernetIIFrame, ~EtherFrameWithLLC and ~EtherFrameWithSNAP. They all subclass
// from ~EtherFrame which only represents the basic MAC frame with source and
// destination addresses. ~EtherMAC and ~EtherMACFullDuplex only deal with EtherFrames,
// and do not care about the specific subclass.
//
//
// @page eth-standards.html, Ethernet: Implemented Standards
//
// The Ethernet model operates according to the following standards:
// - Gigabit Ethernet: IEEE 802.3z-1998
// - Full-Duplex Ethernet with Flow Control: IEEE 802.3x-1997
// - Fast Ethernet: IEEE 802.3u-1995
// - Ethernet: IEEE 802.3-1998
//
// Note: switches don't implement the Spanning Tree Protocol. You need to
// avoid cycles in the LAN topology.
//
//
// <h3>MAC and higher layers</h3>
//
// MAC and LLC are implemented as separate modules (~EtherMAC and
// ~EtherLLC/~EtherEncap) because encapsulation/decapsulation functionality
// is not always needed. (Switches don't do encapsulation/decapsulation.)
// In switches, ~EtherMAC is used with ~IMACRelayUnit.
//
//
// <h3>~EtherLLC and higher layers</h3>
//
// The ~EtherLLC module can serve several applications (higher layer protocols),
// and dispatch data to them. Higher layers are identified by DSAP.
// See <a href="appreg.html">Application registration</a> for more info.
//
// ~EtherEncap doesn't have the functionality to dispatch to different
// higher layers because in practice it'll always be used with ~IPv4.
//
//
// @page llc-app.html, Ethernet: Communication between LLC and Higher Layers
//
// Higher layers (applications or protocols) talk to the ~EtherLLC module
// in the following way.
//
// When a higher layer wants to send a packet via Ethernet, it just
// passes the data packet (a cPacket or any subclass) to ~EtherLLC.
// The message kind has to be set to IEEE802CTRL_DATA.
//
// In general, if ~EtherLLC receives a packet from the higher layers,
// it interprets the message kind as a command. The commands include
// IEEE802CTRL_DATA (send a frame), IEEE802CTRL_REGISTER_DSAP (register highher layer)
// IEEE802CTRL_DEREGISTER_DSAP (deregister higher layer) and IEEE802CTRL_SENDPAUSE
// (send PAUSE frame) -- see ~EtherLLC for a more complete list.
//
// The arguments to the command are in an ~Ieee802Ctrl control info object,
// attached to the packet (see the getControlInfo() method of cPacket.)
//
// For example, to send a packet to a given MAC address and protocol
// identifier, the application sets the data packet's message kind
// to ETH_DATA ("please send this data packet" command),
// fills in the ~Ieee802Ctrl structure with the destination MAC address and
// the protocol identifier, adds the control info to the message, then sends
// the packet to ~EtherLLC.
//
// When the command doesn't involve a data packet (e.g.
// IEEE802CTRL_(DE)REGISTER_DSAP, IEEE802CTRL_SENDPAUSE), a dummy packet
// (empty cMessage) is used.
//
//
// @page appreg.html, EtherLLC: SAP Registration
//
// The Ethernet model supports multiple applications or higher layer
// protocols.
//
// So that data arriving from the network can be dispatched to the
// correct applications (higher layer protocols), applications
// have to register themselves in ~EtherLLC. The registration
// is done with the IEEE802CTRL_REGISTER_DSAP command
// (see <a href="llc-app.html">communication between LLC and higher layers</a>)
// which associates a SAP with the LLC port. Different applications
// have to connect to different ports of ~EtherLLC.
//
// The ETHERCTRL_REGISTER_DSAP and IEEE802CTRL_DEREGISTER_DSAP commands use
// only the dsap field in the ~Ieee802Ctrl structure.
//
//
// @page physical.html, Ethernet: Messaging on the Physical Layer
//
// Messages sent by ~EtherMAC mark the beginning of a transmission.
// The end of a transmission is not explicitly represented by a message,
// but instead, the ~EtherMAC calculates it from the frame length and
// the transmission rate. Frames are represented by ~EtherFrame.
//
// When frames collide, the transmission is aborted -- in this case
// ~EtherMAC makes use of the modelled jam signals to figure out
// when colliding transmissions end.
//
// When a transmitting station senses a collision, it transmits a jam signal.
// Jam signals are represented by a ~EtherJam message.
// When ~EtherMAC received a jam signal, it knows that one transmission
// has ended in jamming -- thus when it receives as many jam messages
// as colliding frames, it can be sure all transmissions have been aborted.
//
// Receiving a jam message marks the beginning (and not the end)
// of a jam signal, so actually ~EtherMAC has to wait for the duration
// of the jamming before assuming the channel is free again.
//
// In burst mode, filled inter-frame gaps are represented with EtherIFG
// packets.
//
// Carrier extension is modelled by simply adding to the frame length,
// using the addBitLength() method of cPacket.
//
//
// @page ether-pause.html, Ethernet: PAUSE handling
//
// The 802.3x standard supports PAUSE frames as a means of flow
// control. The frame contains a timer value, expressed as a multiple
// of 512 bit-times, that specifies how long the transmitter should
// remain quiet. If the receiver becomes uncongested before the
// transmitter's pause timer expires, the receiver may elect to send
// another PAUSE frame to the transmitter with a timer value of zero,
// allowing the transmitter to resume immediately.
//
// ~EtherMAC will properly respond to PAUSE frames it receives
// (~EtherPauseFrame class), but it will never send a PAUSE frame by itself.
// (For one thing, it doesn't have an input buffer that can overflow.)
//
// ~EtherMAC, however, transmits PAUSE frames received by higher layers,
// and ~EtherLLC and ~EtherEncap can be instructed by a command to send
// a PAUSE frame to MAC.
//
// ~IMACRelayUnit types (and thus ~EtherSwitch) currently implement a very simple
// scheme for sending PAUSE frames -- this can be refined if the need arises.
//


package inet.linklayer.ethernet;

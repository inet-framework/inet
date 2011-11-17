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


//# @-titlepage (would conflict with main titlepage)
//# <h1>Ethernet Model Documentation</h1>
//
// @page eth-index.html, Ethernet Model -- Start Page
//
//# This documents the Ethernet model created by David Wu and refined by Andras
//# Varga at CTIE, Monash University, Melbourne, Australia.
//
// The Ethernet model contains a MAC model (EtherMAC), LLC model (EtherLLC) as well
// as a bus (EtherBus, for modelling coaxial \cable) and a hub (EtherHub) model.
// A switch model (EtherSwitch) is also provided.
//
// A few useful topics:
//  - <a href="ether-overview.html">model overview</a>
//  - <a href="eth-standards.html">implemented standards</a>
//  - <a href="llc-app.html">communication between LLC and higher layers</a>
//  - <a href="appreg.html">application registration</a>
//  - <a href="physical.html">physical layer messaging</a>
//  - click <i>selected topics</i> in the navigation frame to see more topics
//
// If you can't wait to see the model, start here:
//  - the MixedLAN model contains hosts, switch, hub and bus;
//  - the <a href="largenet.html">\LargeNet model</a> contains hundreds of
//    computers, switches and hubs (numbers depend on model configuration in
//    largenet.ini), and mixes all kinds of Ethernet technologies;
//  - EtherHost is a sample node with an Ethernet NIC;
//  - EtherSwitch, EtherBus, EtherHub model switching hub, repeating hub and
//    the old coxial \cable;
//  - basic compnents of the model: EtherMAC, EtherLLC/EtherEncap module types,
//    MACRelayUnit (MACRelayUnitNP and MACRelayUnitPP), EtherFrame message type,
//    MACAddress class
//
// Related documentation:
//   - <a href="../doxy/index.html" target="_top">C++ source documentation</a>
//
//
//
// @page ether-overview.html, Ethernet Model Overview
//
// <h3>Data packets and Ethernet frames</h3>
//
// Ethernet frames carry data packets as encapsulated cMessage objects.
// Data packets can be of any message type (cMessage or cMessage subclass).
//
// The model encapsulates data packets in Ethernet frames using the <tt>encapsulate()</tt>
// method of cMessage. Encapsulate() updates the length of the Ethernet frame too,
// so the model doesn't have to take care of that.
//
// The models supports various Ethernet frame types: Ethernet II, 803.2 with
// LLC header, and 803.3 with LLC and SNAP headers. The corresponding classes are:
// EthernetIIFrame, EtherFrameWithLLC and EtherFrameWithSNAP. They all class
// from EtherFrame which only represents the basic MAC frame with source and
// destination addresses. EtherMAC only deals with EtherFrames, and does not
// care about the specific subclass.
//
//
// <h3>Autoconfiguration</h3>
//
// In order to facilitate building large models, EtherMAC and other Ethernet model
// components provide some degree of auto-configuration. Specifically, transmission
// rate and half duplex/full duplex mode can be chosen automatically so that
// connecting Ethernet MACs have matching settings. The purpose is similar to
// Ethernet Auto-Negotiation; however the mechanism is NOT a model of
// Auto-Negotiation (e.g. EtherBus and EtherHub also actively participate,
// which obviously does not happen in a real Ethernet.)
//
// What it does:
//  - the txrate parameters of EtherMAC represent the highest speed supported
//    by that station, or 0 for full autoconfiguration. Autoconfig will choose
//    the largest common denominator (the speed of the slowest station in the
//    collision domain) for all stations. If all stations are set to auto
//    txrate, 100Mb will be chosen, or 10Mb if there's a bus (EtherBus) in
//    the collision domain.
//  - the duplexEnabled parameter of EtherMAC means whether the station supports
//    duplex operation. However, duplex operation will actually be chosen only
//    if it's a DTE-to-DTE direct connection (there's no shared media like
//    EtherHub or EtherBus) and both sides have duplexEnabled=true set.
//
// How it works:
//
// Auto-configuration occurs at the beginning of the simulation, by
// Ethernet model components (EtherMAC, EtherHub and EtherBus) exchanging
// EtherAutoconfig messages with each other. See description of EtherAutoconfig
// for more info.
//
// <h3>MAC and higher layers</h3>
//
// MAC and LLC are implemented as separate modules (EtherMAC and
// EtherLLC/EtherEncap) because encapsulation/decapsulation functionality
// is not always needed. (Switches don't do encapsulation/decapsulation.)
// In switches, EtherMAC is used with MACRelayUnit.
//
//
// <h3>EtherLLC and higher layers</h3>
//
// The EtherLLC module can serve several applications (higher layer protocols),
// and dispatch data to them. Higher layers are identified by DSAP.
// See <a href="appreg.html">Application registration</a> for more info.
//
// EtherEncap doesn't have the functionality to dispatch to different
// higher layers because in practice it'll always be used with IP.
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
// @page llc-app.html, Ethernet: Communication between LLC and Higher Layers
//
// Higher layers (applications or protocols) talk to the EtherLLC module.
//
// When a higher layer wants to send a packet via Ethernet, it just
// passes the data packet (a cMessage or any subclass) to EtherLLC.
// The message kind has to be set to IEEE802CTRL_DATA.
//
// In general, if EtherLLC receives a packet from the higher layers,
// it interprets the message kind as a command. The commands include
// IEEE802CTRL_DATA (send a frame), IEEE802CTRL_REGISTER_DSAP (register highher layer)
// IEEE802CTRL_DEREGISTER_DSAP (deregister higher layer) and IEEE802CTRL_SENDPAUSE
// (send PAUSE frame) -- see EtherLLC for a more complete list.
//
// The arguments to the command are NOT inside the data packet but
// in a "control info" data structure of class Ieee802Ctrl, attached to
// the packet. See controlInfo() method of cMessage (OMNeT++ 3.0).
//
// For example, to send a packet to a given MAC address and protocol
// identifier, the application sets the data packet's message kind
// to ETH_DATA ("please send this data packet" command),
// fills in the Ieee802Ctrl structure with the destination MAC address and
// the protocol identifier, adds the control info to the message, then sends
// the packet to EtherLLC.
//
// When the command doesn't involve a data packet (e.g.
// IEEE802CTRL_(DE)REGISTER_DSAP, IEEE802CTRL_SENDPAUSE), a dummy packet
// (empty cMessage) is used.
//
// <h3>Rationale</h3>
//
// The alternative of the above communications would be:
// - adding the parameters such as destination address into the data
//   packet. This would be a poor solution since it would make the
//   higher layers specific to the Ethernet model.
// - encapsulating a data packet into an <i>interface packet</i> which
//   contains the destination address and other parameters. The
//   disadvantages of this approach is the overhead associated with
//   creating and destroying the interface packets.
//
// Using a control structure is more efficient than the interface packet
// approach, because the control structure can be created once inside
// the higher layer and be reused for every packet.
//
// It may also appear to be more intuitive in Tkenv because one can observe
// data packets travelling between the higher layer and Ethernet
// modules -- as opposed to "interface" packets.
//
//
// @page appreg.html, EtherLLC: SAP Registration
//
// The Ethernet model supports multiple applications or higher layer
// protocols.
//
// So that data arriving from the network can be dispatched to the
// correct applications (higher layer protocols), applications
// have to register themselves in EtherLLC. The registration
// is done with the IEEE802CTRL_REGISTER_DSAP command
// (see <a href="llc-app.html">communication between LLC and higher layers</a>)
// which associates a SAP with the LLC port. Different applications
// have to connect to different ports of EtherLLC.
//
// The ETHERCTRL_REGISTER_DSAP/IEEE802CTRL_DEREGISTER_DSAP commands use only the
// dsap field in the Ieee802Ctrl structure.
//
//
// @page physical.html, Ethernet: Messaging on the Physical Layer
//
// Messages sent by EtherMAC mark the beginning of a transmission.
// The end of a transmission is not explicitly represented by a message,
// but instead, the EtherMAC calculates it from the frame length and
// the transmission rate. Frames are represented by EtherFrame.
//
// When frames collide, the transmission is aborted -- in this case
// EtherMAC makes use of the modelled jam signals to figure out
// when colliding transmissions end.
//
// When a transmitting station senses a collision, it transmits a jam signal.
// Jam signals are represented by a EtherJam message.
// When EtherMAC received a jam signal, it knows that one transmission
// has ended in jamming -- thus when it receives as many jam messages
// as colliding frames, it can be sure all transmissions have been aborted.
//
// Receiving a jam message marks the beginning (and not the end)
// of a jam signal, so actually EtherMAC has to wait for the duration
// of the jamming before assuming the channel is free again.
//
//
//
// @page ether-pause.html, Ethernet: PAUSE handling
//
// The 802.3x standard supports PAUSE frames as a means of flow
// control. The frame contains a timer value, expressed as a multiple
// of 512 bit-times, that specifies how long the transmitter should
// remain quiet. If the receiver becomes uncongested before the
// transmitter's pause timer expires, the receiver may elect to send
// another Pause frame to the transmitter with a timer value of zero,
// allowing the transmitter to resume immediately.
//
// EtherMAC will properly respond to PAUSE frames it receives
// (EtherPauseFrame class),
// however it will never send a PAUSE frame by itself. (For one thing,
// it doesn't have an input buffer that can overflow.)
//
// EtherMAC, however, transmits PAUSE frames received by higher layers,
// and EtherLLC can be instructed by a command to send a PAUSE frame to MAC.
//
// MACRelayUnit types (and thus EtherSwitch) currently implement a very simple
// scheme for sending PAUSE frames -- this can be refined if the need arises.
//
// For background on PAUSE, see e.g.
// http://www.computer.org/proceedings/lcn/0309/03090160abs.htm,
// http://www.nwfusion.com/netresources/0913flow.html, and
// http://www.nwfusion.com/netresources/0913flow2.html.
//


//
// @page largenet.html, LargeNet model description
//
// The LargeNet model demonstrates how one can put together models of large
// LANs with little effort, making use of MAC auto-configuration.
//
// LargeNet models a large Ethernet campus backbone. As configured in the
// default omnetpp.ini, it contains altogether about 8000 computers
// and 900 switches and hubs. This results in about 165MiB process size
// on my (32-bit) linux box when I run the simulation.
// The model mixes all kinds of Ethernet technology: Gigabit Ethernet,
// 100Mb full duplex, 100Mb half duplex, 10Mb UTP, 10Mb bus ("thin Ethernet"),
// switched hubs, repeating hubs.
//
// The topology is in LargeNet.ned, and it looks like this: there's chain
// of n=15 large "backbone" switches (switchBB[]) as well as four more
// large switches (switchA, switchB, switchC, switchD) connected to
// somewhere the middle of the backbone (switchBB[4]). These 15+4 switches
// make up the backbone; the n=15 number is configurable in omnetpp.ini.
//
// Then there're several smaller LANs hanging off each backbone switch.
// There're three types of LANs: small, medium and large (represented by
// compound module types SmallLAN, MediumLAN, LargeLAN). A small LAN
// consists of a few computers on a hub (100Mb half duplex); a medium
// LAN consists of a smaller switch with a hub on one of its port
// (and computers on both); the large one also has a switch and a hub,
// plus an Ethernet bus hanging of one port of the hub (there's still hubs
// around with one BNC connector besides the UTP ones).
// By default there're 5..15 LANs of each type hanging off each backbone
// switch. (These numbers are also omnetpp.ini parameters like the length
// of the backbone.)
//
// The application model which generates load on the simulated LAN is
// simple yet powerful. It can be used as a rough model for any
// request-response based protocol such as SMB/CIFS (the Windows file
// sharing protocol), HTTP, or a database client-server protocol.
//
// Every computer runs a client application (EtherAppCli) which connects
// to one of the servers. There's one server attached to switches A, B,
// C and D each: serverA, serverB, serverC and serverD -- server selection
// is configured in omnetpp.ini). The servers run EtherAppSrv.
// Clients periodically send a request to the server, and the request
// packet contains how many bytes the client wants the server to send back
// (this can mean one or more Ethernet frames, depending on the byte count).
//  Currently the request and reply lengths are configured in omnetpp.ini
// as intuniform(50,1400) and truncnormal(5000,5000).
//
// The volume of the traffic can most easily be controlled with the
// time period between sending requests; this is currently
// set in omnetpp.ini to exponential(0.50) (that is, average 2
// requests per second). This already causes frames to be dropped
// in some of the backbone switches, so the network is a bit
// overloaded with the current settings.
//
// The model generates extensive statistics. All MACs (and most other
// modules too) write statistics into omnetpp.sca at the end
// of the simulation: number of frames sent, received, dropped, etc.
// These are only basic statistics, however it still makes the
// scalar file to be several ten megabytes in size. You can use
// the analysis tools provided with OMNeT++ to visualized the data
// in this file. (If the file size is too big, writing statistics
// can be disabled, by putting **.record-scalar=false in the ini file.)
// The model can also record output vectors, but this is currently
// disabled in omnetpp.ini because the generated file can easily reach
// gigabyte sizes.
//

package inet.linklayer.ethernet;

//
// Copyright (C) 2003 CTIE, Monash University
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


//#FIXME titlepage conflicts with main titlepage
// @titlepage
// <h1>Ethernet Model Documentation</h1>
//
// This documents the Ethernet model created by David Wu and refined by Andras
// Varga at CTIE, Monash University, Melbourne, Australia.
//
// The model contains a MAC model (EtherMAC), LLC model (EtherLLC) as well
// as a bus (EtherBus, for modelling coaxial cable) and a hub (EtherHub) model.
// A switch model (EtherSwitch) is also provided.
//
// A few useful starting points for browsing the documentation:
//  - LANNetwork demonstrates switch, hub and bus
//  - see the EtherMAC, EtherLLC module types, the EtherFrame message type
//    and the MACAddress class
//  - <a href="ether-overview.html">model overview</a>
//  - <a href="standards.html">implemented standards</a>
//  - <a href="llc-app.html">communication between LLC and higher layers</a>
//  - <a href="appreg.html">application registration</a>
//  - <a href="physical.html">physical layer messaging</a>
//  - click <i>selected topics</i> in the navigation frame to see more topics
//
// Related documentation:
//   - <a href="../doxy/index.html" target="_top">C++ source documentation</a>
//
//
//
// @page ether-overview.html, Overview
//
// <h3>Data packets</h3>
//
// Ethernet frames carry data packets as encapsulated cMessage objects.
// Data packets can be of any message type (cMessage or anything subclassed from that).
//
// The model encapsulates data packets in Ethernet frames using the <tt>encapsulate()</tt>
// method of cMessage. Encapsulate() updates the length of the Ethernet frame too,
// so the model doesn't have to take care of that.
//
// <h3>MAC and LLC</h3>
//
// MAC and LLC are implemented in two different modules (EtherMAC and EtherLLC)
// because encapsulation/decapsulation functionality (done in EtherLLC)
// is not always needed (e.g. switches need only MAC). In switches,
// EtherMAC MAC is used with MACRelayUnit.
//
// <h3>LLC and higher layers</h3>
//
// The EtherLLC module can serve several applications (higher layer protocols),
// and dispatch data to them. Higher layers are identified by DSAP.
//
//
//
// @page standards.html, Standards
//
// The models operate according to the following standards:
// - Gigabit Ethernet: IEEE 802.3z-1998
// - Full-Duplex Ethernet with Flow Control: IEEE 802.3x-1997
// - Fast Ethernet: IEEE 802.3u-1995
// - Ethernet: IEEE 802.3-1998
//
//
//
// @page llc-app.html, Communication between LLC and higher layers
//
// Higher layers (applications or protocols) talk to the EtherLLC module.
//
// When a higher layer wants to send a packet via Ethernet, it just
// passes the data packet (a cMessage or any subclass) to EtherLLC.
// The message kind has to be set to ETHCTRL_DATA.
//
// In general, if EtherLLC receives a packet from the higher layers,
// it interprets the message kind as a command. The commands include
// ETHCTRL_DATA (send a frame), ETHCTRL_REGISTER_DSAP (register highher layer)
// ETHCTRL_DEREGISTER_DSAP (deregister higher layer) and ETHCTRL_SENDPAUSE
// (send PAUSE frame) -- see EtherLLC for a more complete list.
//
// The arguments to the command are NOT inside the data packet but
// in a "control" data structure (EtherCtrl class) kept
// by the higher layer module, and the packet refers back to it via
// the <b><tt>contextPointer()</tt></b> of cMessage.
//
// For example, to send a packet to a given MAC address and protocol
// identifier, the application sets the data packet's message kind
// to ETH_DATA ("please send this data packet" command),
// fills in the EtherCtrl structure with the destination MAC address and
// the protocol identifier, sets the data packet's context pointer
// to the EtherCtrl structure, then sends the packet to EtherLLC.
//
// When the command doesn't involve a data packet (e.g. ETHCTRL_(DE)REGISTER_DSAP,
// ETHCTRL_SENDPAUSE), a dummy packet (empty cMessage) is used.
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
// The main disadvantage of the contextPointer() approach is
// that special care has to be taken not to overwrite the contents of
// the control structure prematurely, i.e. before Ethernet had a chance to
// look at it. However, in most cases the control structure doesn't
// need to be changed after creation for the following reasons:
//
// If we talk about an application session directly talking to the Ethernet
// layer, that application usually has a session peer which it talks to
// (and the protocol identifier is constant), so the contents
// of the control structure would stay the same for all ETH_DATA commands.
// If several the protocol identifiers or several peers are in use,
// it is usually not inpractical to keep as many control structures
// within the application.
//
// If we talk about higher layer protocols: in the TCP/IP suite the
// destination is usually specified as an IP address in the first place,
// and it is mapped to MAC address in the ARP cache. For our purpose, the model of
// the ARP cache might just be programmed so that the entries in the cache
// are represented by (contain) Ethernet control structures, and so the
// data packets sent from IP to Ethernet may safely point back to the control
// structures in the ARP cache. Care needs to be taken when deleting control
// structures.
//
//
// @page appreg.html, Application registration
//
// The Ethernet model also supports multiple applications or higher layer
// protocols.
//
// So that data arriving from the network can be dispatched to the
// correct applications (higher layer protocols), applications
// have to register themselves in EtherLLC. The registration
// is done with the ETHCTRL_REGISTER_DSAP command
// (see <a href="llc-app.html">communication between LLC and higher layers</a>)
// which associates a SAP with the LLC port. Different applications
// have to connect to different ports of EtherLLC.
//
// The ETHERCTRL_REGISTER_DSAP/ETHCTRL_DEREGISTER_DSAP commands use only the
// dsap field in the EtherCtrl structure.
//
//
// @page physical.html, Messaging on the physical layer
//
// Messages sent by EtherMAC mark the beginning of a transmission.
// The end of a transmission is not explicitly represented by a message,
// but instead, the EtherMAC calculates it from the frame length and
// the transmission rate. Frames are represented by EtherFrame, with
// message kind=ETH_FRAME.
//
// When frames collide, the transmission is aborted -- in this case
// EtherMAC makes use of the modelled jam signals to figure out
// when colliding transmissions end.
//
// When a transmitting station senses a collision, it transmits a jam signal.
// Jam signals are represented by a message with kind=JAM_SIGNAL.
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
// @page ether-pause.html, PAUSE handling
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
// (EtherPauseFrame class, ETH_PAUSE message kind),
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

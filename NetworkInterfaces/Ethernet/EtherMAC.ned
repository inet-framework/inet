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


//
// Ethernet MAC layer. MAC performs transmission and reception of frames.
//
// Supported variations:
// - 10Mb Ethernet bus or twisted pair
// - 100Mb Ethernet duplex or half-duplex
// - 1Gb Ethernet
//
// IEEE 802.3 frame type is assumed. (Frames contain SSDAP/DSAP bytes
// instead of Ethernet-II's 2-byte Ethertype field).
// RAW mode (only used by the IPX protocol) is not supported.
//
// Expected environment:
// - LowerLayer_in and LowerLayer_out should be connected to the "network"
// - UpperLayer_in and UpperLayer_out are usually connected to EtherLLC (in hosts)
//   or MACRelayUnitPP (in a switch)
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
// this is done by higher layers (EtherLLC).
//
// When a frame is received from the higher layers, it must be an EtherFrame
// with message kind set to ETH_FRAME, and with all protocol fields filled out
// (including the destination MAC address). The source address, if left empty,
// will be filled in. Then frame is queued and transmitted according
// to the CSMA/CD protocol.
//
// Data frames received from the network are EtherFrames, with message kind set
// to ETH_FRAME. They are passed to the higher layers without modification.
// Also, the module properly responds to PAUSE frames, but never sends them
// by itself -- however, it transmits PAUSE frames received from upper layers.
// See <a href="ether-pause.html">PAUSE handling</a> for more info.
//
// <b>Disabling and disconnecting</b>
//
// There is no explicit support for <i>disabling</i> an EtherMAC, mainly
// because there is no need for such functionality. For example, if
// an Ethernet switch wants to "disable" a port, the MACRelayUnit should
// simply not forward frames to that EtherMAC, and discard frames
// arriving from that EtherMAC.
//
// Disconnecting and reconnecting the EtherMAC is not yet supported.
//
// <b>Physical layer messaging</b>
//
// Please see <a href="physical.html">Messaging on the physical layer</a>.
//
// <b>Statistics</b>
//
// Output vectors and WATCHes:
// - framesSent: number of frames sent
// - framesReceivedOK: number of frames received without collision or CRC error
// - bytesSent: bytes sent, including Ethernet frame fields (but excluding preamble and SFD)
// - bytesReceivedOK: total bytes received, including Ethernet frame fields
//   (but excluding preamble and SFD), including discarded frames (see also
//   framesPassedToHL)
// - droppedQueueFull: number of frames from higher layer dropped
// - droppedBitError: number of frames dropped because of bit errors
// - droppedNotForUs: number of frames dropped because destination address didn't match
// - framesPassedToHL: number of frames actually passed up to higher layer
// - pauseFramesRcvd: number of PAUSE frames received from network
// - pauseFramesSent: number of PAUSE frames sent out
// - collisions: number of collisions (NOT number of collided frames!) sensed
// - backoffs: number of retransmissions
//
// Output scalars (written in the finish() function) include the final values of
// the above variables and throughput.
//
// Hints: to get an average throughput (bits/sec) graph in Plove, create
// an Awk expression filter <tt>8*x/t</tt>.
//
simple EtherMAC
    parameters:
        promiscuous : bool,     // if true, all packets are received, otherwise only the
                                // ones with matching destination MAC address
        address : string,       // MAC address as hex string (12 hex digits), or
                                // "auto". "auto" values will be replaced by 
                                // a generated MAC address in init stage 0.
        frameBursting : bool,   // enables transmitting frame bursts (Gigabit Ethernet only)
        txrate : numeric,       // data rate (bit/s)
        duplexChannel : bool,   // channel connecting to MAC is full duplex or not;
                                // can only be be turned on when connecting to
                                // a switch, but not with shared media like bus or hub
        maxQueueSize : numeric; // maximum number of frames queued up for transmission;
                                // additional frames are dropped
    gates:
        in: LowerLayer_in;    // to physical layer or the network
        out: LowerLayer_out;  // to physical layer or the network
        in: UpperLayer_in;    // to EtherLLC or MACRelayUnitPP
        out: UpperLayer_out;  // to EtherLLC or MACRelayUnitPP
endsimple


//
// Copyright (C) 2010 Kyeong Soo (Joseph) Kim
// Copyright (C) 2006 Levente Meszaros
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

//
// Implementation of EtherMAC2 with frame loss handling at txQueue.
// For details other than queueing and statistics, refer to EtherMAC2.
//
// <b>Queueing</b>
//
// In routers, MAC relies on an external queue module (see OutputQueue)
// to model finite buffer, implement QoS and/or RED, and requests packets
// from this external queue one-by-one.
//
// In hosts and Ethernet switches, no such queue is used, so MAC contains
// an internal queue named txQueue to queue up packets waiting for transmission.
// Conceptually, txQueue is of infinite size, but for better diagnostics
// one can specify a hard limit in the txQueueLimit parameter -- <i>if this is
// exceeded, frames are dropped and recorded (modified behaviour!)</i>.
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
// - droppedIfaceDown: number of frames from higher layer dropped
// - droppedBitError: number of frames dropped because of bit errors
// - droppedNotForUs: number of frames dropped because destination address didn't match
// - droppedTxQueueOverflow: number of frames dropped because txQueue overflows (<i>new!</i>)
// - framesPassedToHL: number of frames actually passed up to higher layer
// - pauseFramesRcvd: number of PAUSE frames received from network
// - pauseFramesSent: number of PAUSE frames sent out
// - collisions: number of collisions (NOT number of collided frames!) sensed
// - backoffs: number of retransmissions
//
// Output scalars (written in the finish() function) include the final values of
// the above variables and throughput.
//
// @see EtherMAC2
//
simple EtherMAC3
{
    parameters:
        bool promiscuous = default(false);     // if true, all packets are received, otherwise only the
                                // ones with matching destination MAC address
        string address = default("auto");       // MAC address as hex string (12 hex digits), or
                                // "auto". "auto" values will be replaced by
                                // a generated MAC address in init stage 0.
        int txQueueLimit = default(1000); // maximum number of frames queued up for transmission;
                                // additional frames are dropped. Only used if queueModule==""
        string queueModule = default("");    // name of optional external queue module
        int mtu = default(1500);
        @display("i=block/queue");
    gates:
        input upperLayerIn @labels(EtherFrame);    // to EtherLLC or MACRelayUnitPP
        output upperLayerOut @labels(EtherFrame);  // to EtherLLC or MACRelayUnitPP
        inout phys @labels(EtherFrame);            // to physical layer or the network
}

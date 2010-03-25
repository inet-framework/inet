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

package inet.linklayer.ethernet;

//
// Provides Ethernet 802.3 encapsulation/decapsulation and dispatching
// to the appropriate higher layer by DSAP values.
//
// Expected environment:
// - lowerLayerIn, lowerLayerOut gates should be connected
//   to Ethernet EtherMAC
// - upperLayerIn[], upperLayerOut[] gates should be connected to
//   higher layer protocols or applications
//
// Functionality:
//
// Processes commands received from upper layers, as described in
// <a href="llc-app.html">Communication between LLC and higher layers</a>.
// The following commands are supported:
// - IEEE802CTRL_DATA: send a frame.
//   Encapsulates packet into EtherFrameWithLLC. This includes assigning
//   dest address, ssap and dsap from the Ieee802Ctrl structure acompanying
//   the message. Src address will be filled in by EtherMAC.
// - IEEE802CTRL_REGISTER_DSAP: register application in LLC, as described in
//   <a href="appreg.html">Application registration</a>.
// - IEEE802CTRL_DEREGISTER_DSAP: deregister application, as described in
//   <a href="appreg.html">Application registration</a>.
// - IEEE802CTRL_SENDPAUSE: send PAUSE frame, as described in
//   <a href="ether-pause.html">PAUSE frames</a>.
//
// Processing of packets received from the lower layers:
// - decapsulate frames received from EtherMAC
// - sends received frames to corresponding application, based on dsap.
// - only servicetype=0 is implemented (connectionless)
// - reception of non-EtherFrameWithLLC objects (e.g. EthernetIIFrame,
//   EtherFrameWithSNAP) will cause a runtime error.
//
simple EtherLLC
{
    parameters:
        @display("i=block/fork");
    gates:
        input upperLayerIn[];   // higher layer protocols or applications
        output upperLayerOut[]; // higher layer protocols or applications
        input lowerLayerIn @labels(EtherFrame);     // to Ethernet MAC
        output lowerLayerOut @labels(EtherFrame);   // to Ethernet MAC
}


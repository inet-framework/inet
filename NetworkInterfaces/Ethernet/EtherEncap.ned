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
// Performs Ethernet II encapsulation/decapsulation.
//
// Expected environment:
// - lowerLayerIn, lowerLayerOut gates should be connected to Ethernet EtherMAC
// - upperLayerIn, upperLayerOut gates should be connected to higher layer
//   protocol or application
//
// Functionality:
// - packets coming from upper layers will be encapsulated in EthernetIIFrame
//   and sent to MAC. EtherType is taken from the Ieee802Ctrl structure attached
//   to the message as control info (see setControlInfo(), controlInfo()
//   methods). Src address will be filled in by EtherMAC.
// - also accepts SENDPAUSE command (message kind=IEEE802CTRL_SENDPAUSE) from
//   higher layer: send PAUSE frame, as described in
//   <a href="ether-pause.html">PAUSE frames</a>.
// - Ethernet frames arriving from MAC will be decapsulated, and sent up to the
//   higher layers. Accepts EtherFrameWithLLC and EtherFrameWithSNAP too.
//
simple EtherEncap
    parameters:
        writeScalars: bool; // enable/disable recording statistics in omnetpp.sca
    gates:
        in: upperLayerIn;   // higher layer
        out: upperLayerOut; // higher layer
        in: lowerLayerIn;   // to Ethernet MAC
        out: lowerLayerOut; // to Ethernet MAC
endsimple


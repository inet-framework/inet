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
// Ethernet LLC layer.
//
// Expected environment:
// - LowerLayer_in, LowerLayer_out gates should be connected to Ethernet EtherMAC
// - UpperLayer_in, UpperLayer_out gates should be connected to higher layer
//   protocol or application
//
// Functionality:
//
// Processes commands received from upper layers, as described in
// <a href="llc-app.html">Communication between LLC and higher layers</a>.
// The following commands are supported:
// - ETHCTRL_DATA: send a frame.
//   Encapsulates packets received from upper layers. This includes assigning
//   dest address, ssap and dsap from the control info acompanying
//   the message. Src address will be filled in by EtherMAC.
// - ETHCTRL_SENDPAUSE: send PAUSE frame, as described in
//   <a href="ether-pause.html">PAUSE frames</a>.
//
// Processing of packets received from the lower layers:
// - decapsulate frames received from EtherMAC
// - send up received frame to higher layer
//
simple EtherLLC
    gates:
        in: UpperLayer_in;   // higher layer
        out: UpperLayer_out; // higher layer
        in: LowerLayer_in;     // to Ethernet MAC
        out: LowerLayer_out;   // to Ethernet MAC
endsimple


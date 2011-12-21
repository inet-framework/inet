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
// Performs Ethernet II or Ethernet with SNAP encapsulation/decapsulation.
//
// Expected environment:
// - lowerLayerIn, lowerLayerOut gates should be connected to an ~EtherMAC
// - upperLayerIn, upperLayerOut gates should be connected to higher layer
//   protocol or application
//
// Functionality:
// - packets coming from upper layers will be encapsulated in ~EthernetIIFrame
//   or ~EtherFrameWithSNAP (depending on the useSNAP module parameter),
//   and sent to MAC. ~EtherType is taken from the ~Ieee802Ctrl structure
//   attached to the message as control info (see setControlInfo(),
//   controlInfo() methods). The source address will be filled in by ~EtherMAC.
// - also accepts SENDPAUSE command (message kind=IEEE802CTRL_SENDPAUSE) from
//   higher layer: it will send a PAUSE frame, as described in
//   <a href="ether-pause.html">PAUSE frames</a>.
// - Ethernet frames arriving from MAC will be decapsulated, and sent up to the
//   higher layers. Accepts ~EtherFrameWithLLC and ~EtherFrameWithSNAP too.
//
simple EtherEncap
{
    parameters:
        bool useSNAP = default(false);  // create EtherFrameWithSNAP frames instead of EthernetIIFrame
        @display("i=block/square");
        @signal[encapPk](type=cPacket);
        @signal[decapPk](type=cPacket);
        @signal[pauseSent](type=long); // pause units
        @statistic[encapPk](title="packets encapsulated";source=encapPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[decapPk](title="packets decapsulated";source=decapPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[pauseSent](title="pause sent";record=count,sum,vector);
    gates:
        input upperLayerIn @labels(Ieee802Ctrl/down); // higher layer
        output upperLayerOut @labels(Ieee802Ctrl/up); // higher layer
        input lowerLayerIn @labels(EtherFrame);   // to Ethernet MAC
        output lowerLayerOut @labels(EtherFrame); // to Ethernet MAC
}


//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

package inet.linklayer.ethernet.basic;

import inet.linklayer.ethernet.contract.IEthernetEncapsulation;
import inet.linklayer.ieee8022.Ieee8022Llc;

//
// Performs Ethernet II or Ethernet with LLC/SNAP encapsulation/decapsulation.
//
// Expected environment:
// - lowerLayerIn, lowerLayerOut gates should be connected to an ~EthernetCsmaMac
// - upperLayerIn, upperLayerOut gates should be connected to higher layer
//   protocol or application
//
// Functionality:
// - packets coming from upper layers will be encapsulated in EthernetII frame
//   (~EthernetMacHeader) or Ethernet SNAP frame (~Ieee8022LlcSnapHeader)
//   depending on the useSNAP module parameter,
//   and sent to MAC. The ~EtherType is taken from the ~PacketProtocolTag tag
//   attached to the packet. The source address will be filled in by ~EthernetCsmaMac.
// - also accepts SENDPAUSE command (message kind=IEEE802CTRL_SENDPAUSE) from
//   higher layer: it will send a PAUSE frame, as described in
//   <a href="ether-pause.html">PAUSE frames</a>.
// - Ethernet frames arriving from MAC will be decapsulated, and sent up to the
//   higher layers. Accepts ~EthernetMacHeader, ~Ieee8022LlcHeader, and
//   ~Ieee8022LlcSnapHeader too.
//
simple EthernetEncapsulation extends Ieee8022Llc like IEthernetEncapsulation
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        bool useSNAP = default(false);  // create EtherFrameWithSNAP frames instead of EthernetIIFrame
        string fcsMode @enum("declared","computed");
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // timeout value for lifecycle stop operation
        @class(EthernetEncapsulation);
        @display("i=block/square");
        @signal[encapPk](type=cPacket);
        @signal[decapPk](type=cPacket);
        @signal[pauseSent](type=long); // pause units
        @signal[packetDropped](type=inet::Packet);
        @statistic[encapPk](title="packets encapsulated"; source=encapPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[decapPk](title="packets decapsulated"; source=decapPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[pauseSent](title="pause sent"; record=count,sum,vector);
}


//
// Copyright (C) 2013 OpenSim Ltd
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
// author: Zoltan Bojthe
//

package inet.linklayer.acking;

import inet.queueing.contract.IPacketQueue;
import inet.linklayer.contract.IWirelessInterface;
import inet.networklayer.common.InterfaceEntry;
import inet.physicallayer.contract.packetlevel.IRadio;

//
// Highly abstracted wireless NIC that consists of a unit disk radio and a trivial
// MAC protocol. It offers simplicity for scenarios where Layer 1 and 2 effects
// can be completely ignored, for example testing the basic functionality
// of a wireless ad-hoc routing protocol.
//
// The most important parameter this model accepts is the transmission range.
// When a radio transmits a frame, all other radios within transmission range
// will receive the frame correctly, and radios that are out of range will not be
// affected at all.
//
// This module requires an ~UnitDiskRadioMedium instance in the network.
//
module AckingWirelessInterface extends InterfaceEntry like IWirelessInterface
{
    parameters:
        @class(InterfaceEntry);
        @display("i=block/ifcard");
        string interfaceTableModule;
        string energySourceModule = default("");
        double bitrate @unit(bps);
        *.interfaceTableModule = default(absPath(this.interfaceTableModule));
        *.energySourceModule = default(absPath(this.energySourceModule));
        **.bitrate = this.bitrate;
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input radioIn @labels(Signal);
    submodules:
        mac: AckingMac {
            parameters:
                @display("p=100,100");
        }
        radio: <default("UnitDiskRadio")> like IRadio {
            parameters:
                @display("p=100,200");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> mac.upperLayerIn;
        mac.lowerLayerOut --> radio.upperLayerIn;
        mac.upperLayerOut --> { @display("m=n"); } --> upperLayerOut;
        radioIn --> { @display("m=s"); } --> radio.radioIn;
        radio.upperLayerOut --> mac.lowerLayerIn;
}


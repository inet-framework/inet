//
// Copyright (C) 2006 OpenSim Ltd.
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

package inet.linklayer.ieee802154;

import inet.queueing.contract.IPacketQueue;
import inet.linklayer.acking.AckingMac;
import inet.linklayer.contract.IWirelessInterface;
import inet.networklayer.common.NetworkInterface;
import inet.physicallayer.wireless.common.contract.packetlevel.IRadio;

//
// This module implements an IEEE 802.15.4 UWB-IR network interface.
//
module Ieee802154UwbIrInterface extends NetworkInterface like IWirelessInterface
{
    parameters:
        @class(NetworkInterface);
        string interfaceTableModule;
        *.interfaceTableModule = default(absPath(interfaceTableModule));
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input radioIn @labels(IWirelessSignal);
    submodules:
        mac: AckingMac {
            parameters:
                @display("p=300,100");
        }
        radio: <default("Ieee802154UwbIrRadio")> like IRadio {
            parameters:
                @display("p=300,200");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> mac.upperLayerIn;
        mac.lowerLayerOut --> radio.upperLayerIn;
        
        radioIn --> { @display("m=s"); } --> radio.radioIn;
        radio.upperLayerOut --> mac.lowerLayerIn;
        mac.upperLayerOut --> { @display("m=n"); } --> upperLayerOut;
}


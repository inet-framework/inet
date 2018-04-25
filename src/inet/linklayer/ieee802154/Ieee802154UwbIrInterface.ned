//
// Copyright (C) 2006 Andras Varga
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

package inet.linklayer.ieee802154;

import inet.common.queue.IOutputQueue;
import inet.linklayer.acking.AckingMac;
import inet.linklayer.contract.IWirelessInterface;
import inet.physicallayer.contract.packetlevel.IRadio;

module Ieee802154UwbIrInterface like IWirelessInterface
{
    parameters:
        @class(InterfaceEntry);
        string interfaceTableModule;
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        @display("i=block/ifcard");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input radioIn @labels(ISignal);
    submodules:
        queue: <default("DropTailQueue")> like IOutputQueue {
            parameters:
                @display("p=100,100;q=l2queue");
        }
        mac: AckingMac { // TODO use Ieee802154Mac or something like Ieee802154NarrowbandMac
            parameters:
                @display("p=250,200");
        }
        radio: <default("Ieee802154UwbIrRadio")> like IRadio {
            parameters:
                @display("p=250,350");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> queue.in;
        queue.out --> mac.upperLayerIn;
        mac.lowerLayerOut --> radio.upperLayerIn;
        mac.upperLayerOut --> { @display("m=n"); } --> upperLayerOut;
        radioIn --> { @display("m=s"); } --> radio.radioIn;
        radio.upperLayerOut --> mac.lowerLayerIn;
}


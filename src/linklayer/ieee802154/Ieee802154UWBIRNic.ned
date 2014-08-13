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
import inet.linklayer.IWirelessNic;
import inet.linklayer.ideal.IdealMac;
import inet.physicallayer.contract.IRadio;


module Ieee802154UWBIRNic like IWirelessNic
{
    parameters:
        string interfaceTableModule;
        string queueType = default("DropTailQueue");    // DropTailQueue, a Diffserv queue, etc.
        string radioType = default("Ieee802154UWBIRRadio");
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        @display("i=block/ifcard");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input radioIn @labels(IRadioFrame);
    submodules:
        queue: <queueType> like IOutputQueue {
            parameters:
                @display("p=100,50;q=l2queue");
        }
        mac: IdealMac {
            parameters:
                @display("p=100,150");
        }
        radio: <radioType> like IRadio {
            parameters:
                @display("p=100,250");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> queue.in;
        queue.out --> mac.upperLayerIn;
        mac.lowerLayerOut --> radio.upperLayerIn;
        mac.upperLayerOut --> { @display("m=n"); } --> upperLayerOut;
        radioIn --> { @display("m=s"); } --> radio.radioIn;
        radio.upperLayerOut --> mac.lowerLayerIn;
}

//
// Copyright (C) OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

package inet.protocol.service;

import inet.protocol.selectivity.ReceiveAtL3Address;
import inet.protocol.selectivity.ReceiveAtPort;
import inet.protocol.selectivity.SendToL3Address;
import inet.protocol.selectivity.SendToPort;

module SelectivityService
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        sendToPort: SendToPort {
            @display("p=150,100");
        }
        sendToL3Address: SendToL3Address {
            @display("p=150,200");
        }
        receiveAtPort: ReceiveAtPort {
            @display("p=350,100");
        }
        receiveAtL3Address: ReceiveAtL3Address {
            @display("p=350,200");
        }
    connections allowunconnected:
        upperLayerIn --> sendToPort.in;
        sendToPort.out --> sendToL3Address.in;
        sendToL3Address.out --> { @display("m=s"); } --> lowerLayerOut;
        lowerLayerIn --> { @display("m=s"); } --> receiveAtL3Address.in;
        receiveAtL3Address.out --> receiveAtPort.in;
        receiveAtPort.out --> upperLayerOut;
}

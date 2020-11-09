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

package inet.protocolelement.service;

import inet.common.MessageDispatcher;
import inet.protocolelement.forwarding.Forwarding;
import inet.protocolelement.forwarding.ReceiveWithHopLimit;
import inet.protocolelement.forwarding.SendWithHopLimit;

module ForwardingService
{
    parameters:
        @display("i=block/routing");
    gates:
        input upperLayerIn @loose;
        output upperLayerOut @loose;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        d1: MessageDispatcher {
            @display("p=300,100");
        }
        forwarding: Forwarding {
            @display("p=300,200");
        }
        sendWithHopLimit: SendWithHopLimit {
            @display("p=150,300");
        }
        receiveWithHopLimit: ReceiveWithHopLimit {
            @display("p=450,300");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> d1.in++;
        d1.out++ --> forwarding.in;
        forwarding.out --> d1.in++;
        d1.out++ --> sendWithHopLimit.in;
        sendWithHopLimit.out --> { @display("m=s"); } --> lowerLayerOut;
        lowerLayerIn --> { @display("m=s"); } --> receiveWithHopLimit.in;
        receiveWithHopLimit.out --> d1.in++;
        d1.out++ --> { @display("m=n"); } --> upperLayerOut;
}

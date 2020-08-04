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

package inet.linklayer.ethernet.layered;

import inet.protocol.ethernet.IIeee8022LlcLayer;
import inet.queueing.contract.IPacketFlow;
import inet.queueing.contract.ITrafficConditioner;

moduleinterface ISocketLayer
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}

module OmittedSocketLayer like ISocketLayer
{
    parameters:
        @class(OmittedModule);
        @display("i=block/layer,black,50");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    connections:
        upperLayerIn --> lowerLayerOut;
        lowerLayerIn --> upperLayerOut;
}

moduleinterface IMeasurementLayer
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}

module OmittedMeasurementLayer like IMeasurementLayer
{
    parameters:
        @class(OmittedModule);
        @display("i=block/layer,black,50");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    connections:
        upperLayerIn --> lowerLayerOut;
        lowerLayerIn --> upperLayerOut;
}

module MeasurementLayer like IMeasurementLayer
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        measurementStarter: <default("TimingMeasurementStarter")> like IPacketFlow {
            parameters:
                @display("p=150,100");
        }
        measurementMaker: <default("TimingMeasurementMaker")> like IPacketFlow {
            parameters:
                @display("p=350,100");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> measurementStarter.in;
        measurementStarter.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> measurementMaker.in;
        measurementMaker.out --> { @display("m=n"); } --> upperLayerOut;
}

moduleinterface ITrafficConditionerLayer
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}

module OmittedTrafficConditionerLayer like ITrafficConditionerLayer
{
    parameters:
        @class(OmittedModule);
        @display("i=block/layer,black,50");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    connections:
        upperLayerIn --> lowerLayerOut;
        lowerLayerIn --> upperLayerOut;
}

module TrafficConditionerLayer like ITrafficConditionerLayer
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        ingressConditioner: <default("OmittedTrafficConditioner")> like ITrafficConditioner {
            @display("p=100,100");
        }
        egressConditioner: <default("OmittedTrafficConditioner")> like ITrafficConditioner {
            @display("p=300,100");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> ingressConditioner.in;
        ingressConditioner.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> egressConditioner.in;
        egressConditioner.out --> { @display("m=n"); } --> upperLayerOut;
}

module OmittedIeee8022LlcLayer like IIeee8022LlcLayer
{
    parameters:
        @class(OmittedModule);
        @display("i=block/layer,black,50");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    connections:
        upperLayerIn --> lowerLayerOut;
        lowerLayerIn --> upperLayerOut;
}

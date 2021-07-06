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

package inet.protocolelement.measurement;

import inet.queueing.contract.IPacketFlow;

module MultiMeasurementLayer like IMeasurementLayer
{
    parameters:
        @display("i=block/layer");
        int numMeasurementModules = default(1);
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        measurementStarter[numMeasurementModules]: <default("FlowMeasurementStarter")> like IPacketFlow {
            parameters:
                @display("p=150,100,column,100");
        }
        measurementMaker[numMeasurementModules]: <default("FlowMeasurementRecorder")> like IPacketFlow {
            parameters:
                @display("p=350,100,column,100");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> measurementStarter[0].in;
        for i=0..numMeasurementModules-2 {
            measurementStarter[i].out --> { @display("m=n"); } --> measurementStarter[i+1].in;
        }
        measurementStarter[numMeasurementModules-1].out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> measurementMaker[numMeasurementModules-1].in;
        for i=0..numMeasurementModules-2 {
            measurementMaker[i+1].out --> { @display("m=n"); } --> measurementMaker[i].in;
        }
        measurementMaker[0].out --> { @display("m=n"); } --> upperLayerOut;
}


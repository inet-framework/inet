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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package inet.protocol.ethernet;

module EthernetCutthroughLayer
{
    parameters:
        string macTableModule;
        *.macTableModule = default(macTableModule);
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
        input cutthroughIn;
        output cutthroughOut;
    submodules:
        cutthroughSink: EthernetCutthroughSink {
            @display("p=200,100");
        }
        cutthroughSource: EthernetCutthroughSource {
            @display("p=500,100");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> cutthroughSink.in;
        cutthroughSink.out --> { @display("m=s"); } --> lowerLayerOut;
        cutthroughIn --> { @display("m=w"); } --> cutthroughSink.cutthroughIn;

        lowerLayerIn --> { @display("m=s"); } --> cutthroughSource.in;
        cutthroughSource.out --> { @display("m=n"); } --> upperLayerOut;
        cutthroughSource.cutthroughOut --> { @display("m=e"); } --> cutthroughOut;
}


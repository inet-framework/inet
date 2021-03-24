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

package inet.protocolelement.redundancy;

import inet.protocolelement.contract.IProtocolLayer;
import inet.queueing.contract.IPacketFilter;
import inet.queueing.contract.IPacketFlow;
import inet.queueing.contract.IPacketPusher;

module StreamLayer like IProtocolLayer
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        streamMerger: <default("StreamMerger")> like IPacketFilter if typename != "" {
            parameters:
                @display("p=200,150;i=block/join");
        }
        streamDecoder: <default("StreamDecoder")> like IPacketFlow if typename != "" {
            parameters:
                @display("p=200,300;i=block/blackboard");
        }
        streamIdentifier: <default("StreamIdentifier")> like IPacketFlow if typename != "" {
            parameters:
                @display("p=500,150;i=block/blackboard");
        }
        streamSplitter: <default("StreamSplitter")> like IPacketPusher if typename != "" {
            parameters:
                @display("p=500,300;i=block/fork");
        }
        streamEncoder: <default("StreamEncoder")> like IPacketFlow if typename != "" {
            parameters:
                @display("p=500,450;i=block/blackboard");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> streamIdentifier.in;
        streamIdentifier.out --> streamSplitter.in;
        streamSplitter.out --> streamEncoder.in;
        streamEncoder.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> streamDecoder.in;
        streamDecoder.out --> streamMerger.in;
        streamMerger.out --> { @display("m=n"); } --> upperLayerOut;
}

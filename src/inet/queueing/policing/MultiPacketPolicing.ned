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

package inet.queueing.policing;

import inet.queueing.common.PacketMultiplexer;
import inet.queueing.contract.IPacketClassifier;
import inet.queueing.contract.IPacketFilter;
import inet.queueing.contract.IPacketMeter;
import inet.queueing.contract.IPacketPolicing;

//
// This module combines multiple packet policing modules into one.
//
module MultiPacketPolicing like IPacketPolicing
{
    parameters:
        int numPaths;
    gates:
        input in @labels(push);
        output out @labels(push);
    submodules:
        classifier: <default("PacketClassifier")> like IPacketClassifier {
            parameters:
                @display("p=125,120");
        }
        meter[numPaths]: <default("DualRateThreeColorMeter")> like IPacketMeter {
            parameters:
                @display("p=325,120");
        }
        filter[numPaths]: <default("LabelFilter")> like IPacketFilter {
            parameters:
                @display("p=525,120");
        }
        multiplexer: PacketMultiplexer {
            parameters:
                @display("p=725,120");
        }
    connections:
        in --> { @display("m=w"); } --> classifier.in;
        for i=0..numPaths-1 {
            classifier.out++ --> meter[i].in;
            meter[i].out --> filter[i].in;
            filter[i].out --> multiplexer.in++;
        }
        multiplexer.out --> { @display("m=e"); } --> out;
}


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

package inet.linklayer.ieee8021q;

import inet.queueing.common.PacketMultiplexer;
import inet.queueing.contract.IPacketClassifier;
import inet.queueing.contract.IPacketFilter;
import inet.queueing.contract.IPacketGate;
import inet.queueing.contract.IPacketMeter;
import inet.queueing.contract.IPassivePacketSink;

module Ieee8021qciFilter
{
    parameters:
        int numStreams;
        @display("i=block/filter");
    gates:
        input in;
        output out;
    submodules:
        classifier: <default("StreamClassifier")> like IPacketClassifier {
            parameters:
                @display("p=100,100");
        }
        meter[numStreams]: <default("DualRateThreeColorMeter")> like IPacketMeter {
            parameters:
                @display("p=300,100,column,150");
        }
        filter[numStreams]: <default("LabelFilter")> like IPacketFilter {
            parameters:
                includeLabels = default(["green"]);
                @display("p=500,100,column,150");
        }
        gate[numStreams]: <default("OmittedPacketGate")> like IPacketGate {
            parameters:
                @display("p=700,100,column,150");
        }
        multiplexer: PacketMultiplexer {
            parameters:
                @display("p=900,100");
        }
    connections:
        in --> { @display("m=w"); } --> classifier.in;
        for i=0..numStreams-1 {
            classifier.out++ --> meter[i].in;
            meter[i].out --> filter[i].in;
            filter[i].out --> gate[i].in;
            gate[i].out --> multiplexer.in++;
        }
        multiplexer.out --> { @display("m=e"); } --> out;
}


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

//
// This module combines two meters and their corresponding filters per path. This
// is primarily useful for combining a token bucket based metering with an
// asynchronous packet shaper. Note that the asynchronous packet shaper also has
// parts in the network interface queue module.
//
// @see ~EligibilityTimeQueue, EligibilityTimeGate
//
module DualIeee8021qciFilter like IPacketFilter
{
    parameters:
        int numStreams; // number of streams
        bool hasDirectPath = default(false); // specifies if the non-filtered direct path should exist
        int startY = default(hasDirectPath ? 200 : 100); // used for submodule layout
        @display("i=block/filter");
    gates:
        input in;
        output out;
    submodules:
        classifier: <default("StreamClassifier")> like IPacketClassifier {
            @display("p=100,100");
        }
        meter1[numStreams]: <default("DualRateThreeColorMeter")> like IPacketMeter {
            @display("p=300,$startY,column,150");
        }
        filter1[numStreams]: <default("LabelFilter")> like IPacketFilter {
            labelFilter = default("green");
            @display("p=500,$startY,column,150");
        }
        meter2[numStreams]: <default("EligibilityTimeMeter")> like IPacketMeter {
            @display("p=700,$startY,column,150");
        }
        filter2[numStreams]: <default("EligibilityTimeFilter")> like IPacketFilter {
            @display("p=900,$startY,column,150");
        }
        gate[numStreams]: <default("InteractiveGate")> like IPacketGate {
            @display("p=1100,$startY,column,150");
        }
        multiplexer: PacketMultiplexer {
            @display("p=1300,100");
        }
    connections:
        in --> { @display("m=w"); } --> classifier.in;
        classifier.out++ --> multiplexer.in++ if hasDirectPath;
        for i=0..numStreams-1 {
            classifier.out++ --> meter1[i].in;
            meter1[i].out --> filter1[i].in;
            filter1[i].out --> meter2[i].in;
            meter2[i].out --> filter2[i].in;
            filter2[i].out --> gate[i].in;
            gate[i].out --> multiplexer.in++;
        }
        multiplexer.out --> { @display("m=e"); } --> out;
}

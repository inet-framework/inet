//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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


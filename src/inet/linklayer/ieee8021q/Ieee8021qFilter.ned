//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.queueing.common.BackPressureBarrier;
import inet.queueing.common.PacketMultiplexer;
import inet.queueing.contract.IPacketClassifier;
import inet.queueing.contract.IPacketFilter;
import inet.queueing.contract.IPacketGate;
import inet.queueing.contract.IPacketMeter;

//
// This module implements the IEEE 802.1Q per-stream filtering and policing.
// The relationship between streams, gates and meters is not one-to-one. The
// number of streams, gates and meters can be different and the module will take
// care about the connections between the submodules based on the streamFilterTable
// parameter.
//
module Ieee8021qFilter like IPacketFilter
{
    parameters:
        int numStreams; // number of different streams to filter for
        int numGates; // number of gates
        int numMeters; // number of traffic meters
        object streamFilterTable; // an array of objects where each object contains a stream name, gate index and meter index, for example: [{stream: "s0", gate: 0, meter: 0}, ...]
        @display("i=block/filter");
    gates:
        input in;
        output out;
    submodules:
        classifier: <default("PriorityClassifier")> like IPacketClassifier {
            @display("p=100,100");
        }
        streamFilter[numStreams]: <default("StreamFilter")> like IPacketFilter {
            streamNameFilter = default(parent.streamFilterTable.get(index).get("stream"));
            @display("p=300,100,column,150");
        }
        sm[numGates]: PacketMultiplexer {
            displayStringTextFormat = default("");
            @display("p=500,100,column,150");
        }
        gate[numGates]: <default("InteractiveGate")> like IPacketGate {
            @display("p=700,100,column,150");
        }
        gc[numGates]: <default("StreamClassifier")> like IPacketClassifier {
            displayStringTextFormat = default("");
            @display("p=900,100,column,150;i=-;b=10,10,oval,grey,,1");
        }
        gm[numMeters]: PacketMultiplexer {
            displayStringTextFormat = default("");
            @display("p=1100,100,column,150");
        }
        meter[numMeters]: <default("DualRateThreeColorMeter")> like IPacketMeter {
            @display("p=1300,100,column,150");
        }
        flowFilter[numMeters]: <default("LabelFilter")> like IPacketFilter {
            labelFilter = default("green");
            @display("p=1500,100,column,150");
        }
        multiplexer: PacketMultiplexer {
            @display("p=1700,100");
        }
        barrier: BackPressureBarrier {
            @display("p=1900,100");
        }
    connections allowunconnected:
        in --> { @display("m=w"); } --> classifier.in;
        for i=0..numStreams-1 {
            classifier.out++ --> streamFilter[i].in;
        }
        for i=0..numStreams-1, for j=0..numGates-1 {
            streamFilter[i].out --> sm[j].in++ if streamFilterTable.get(i).get("gate") == j;
        }
        for i=0..numGates-1 {
            sm[i].out --> gate[i].in;
            gate[i].out --> gc[i].in;
        }
        for i=0..numGates-1, for j=0..numMeters-1 {
            gc[i].out++ --> gm[j].in++ if findArrayObjectElement(streamFilterTable, "gate", i, "meter", j) != null;
        }
        for i=0..numMeters-1 {
            gm[i].out --> meter[i].in;
            meter[i].out --> flowFilter[i].in;
            flowFilter[i].out --> multiplexer.in++;
        }
        multiplexer.out --> barrier.in;
        barrier.out --> { @display("m=e"); } --> out;
}

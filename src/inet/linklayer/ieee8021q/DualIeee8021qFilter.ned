//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
module DualIeee8021qFilter like IPacketFilter
{
    parameters:
        int numStreams = default(0); // number of streams
        bool hasDefaultPath = default(true); // specifies if the non-filtered direct path should exist
        int startY = default(hasDefaultPath ? 200 : 100); // used for submodule layout
        @display("i=block/filter");
    gates:
        input in;
        output out;
    submodules:
        classifier: <default("StreamClassifier")> like IPacketClassifier {
            gateIndexOffset = default(parent.hasDefaultPath ? 1 : 0);
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
        classifier.out++ --> multiplexer.in++ if hasDefaultPath;
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

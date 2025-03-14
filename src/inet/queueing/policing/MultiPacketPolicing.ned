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
// Implements a composite packet policing mechanism that combines multiple
// packet policing paths into a single module. Allows for different traffic classes
// to be metered and filtered according to different policies simultaneously.
//
// Key features:
// - Supports multiple parallel policing paths for different traffic classes
// - Each path consists of a meter and filter component
// - Configurable classifier to direct packets to appropriate policing paths
// - Multiplexer to combine the outputs of all policing paths
//
// Operation mechanism:
// 1. The classifier directs incoming packets to the appropriate policing path
//    based on packet attributes (e.g., protocol, priority, flow ID)
// 2. Each path's meter measures the traffic characteristics (e.g., rate, burst size)
//    and marks packets accordingly (e.g., green, yellow, red)
// 3. The filter in each path allows or drops packets based on their markings
// 4. The multiplexer combines the outputs from all paths into a single stream
//
// By default, this module uses PacketClassifier, DualRateThreeColorMeter, and LabelFilter
// components, but these can be customized to implement various policing strategies.
//
// @see ~PacketPolicing, ~DualRateThreeColorMeter, ~SingleRateThreeColorMeter, ~LabelFilter
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

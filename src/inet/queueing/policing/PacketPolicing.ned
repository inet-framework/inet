//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.policing;

import inet.queueing.contract.IPacketFilter;
import inet.queueing.contract.IPacketMeter;
import inet.queueing.contract.IPacketPolicing;

//
// Implements a basic packet policing mechanism by combining a packet meter
// and a packet filter in sequence. Enforces traffic policies by measuring traffic
// characteristics and then selectively dropping packets that exceed defined limits.
//
// Key features:
// - Two-stage traffic policing (metering followed by filtering)
// - Configurable metering algorithm to measure traffic characteristics
// - Configurable filtering criteria for packet acceptance/dropping
// - Push-based packet processing model
// - Implements standard traffic policing patterns like token bucket and leaky bucket
//
// Operation mechanism:
// 1. The meter component measures traffic characteristics (e.g., rate, burst size)
//    and attaches meta information to packets (typically color markings)
// 2. The filter component examines the meta information and decides whether to
//    forward or drop each packet based on configured criteria
//
// By default, this module uses DualRateThreeColorMeter and LabelFilter components,
// which implement the two rate three color marker (trTCM) algorithm as defined in
// RFC 2698, but these can be customized to implement various policing strategies.
//
// @see ~MultiPacketPolicing, ~DualRateThreeColorMeter, ~SingleRateThreeColorMeter, ~LabelFilter
//
module PacketPolicing like IPacketPolicing
{
    gates:
        input in @labels(push);
        output out @labels(push);
    submodules:
        meter: <default("DualRateThreeColorMeter")> like IPacketMeter {
            parameters:
                @display("p=125,120");
        }
        filter: <default("LabelFilter")> like IPacketFilter {
            parameters:
                @display("p=325,120");
        }
    connections:
        in --> { @display("m=w"); } --> meter.in;
        meter.out --> filter.in;
        filter.out --> { @display("m=e"); } --> out;
}

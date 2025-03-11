//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// Base module for various packet gate modules in the queueing framework.
// Packet gates control the flow of packets by selectively allowing or blocking
// transmission based on their open/closed state.
//
// Packet gates are essential components for implementing:
// - Time-sensitive networking (TSN) features
// - Traffic shaping and bandwidth allocation
// - Scheduled transmission of packets
// - Flow control mechanisms
// - Quality of Service (QoS) policies
//
// When a gate is open, packets can flow through it; when closed, packets are blocked.
// Gates can operate in both push and pull mode, allowing them to be used in various
// packet processing pipelines.
//
// Parameters:
// - bitrate: Specifies the transmission rate when the gate is open
// - extraLength: Accounts for additional packet overhead (e.g., headers)
// - extraDuration: Adds extra processing time for each packet
//
// The gate state (open/closed) is tracked and can be visualized during simulation.
// Derived modules implement specific gate opening/closing behaviors, such as
// periodic scheduling or credit-based operation.
//
// @see ~PeriodicGate, ~CreditBasedGate, ~PacketGate, ~IPacketGate
//
simple PacketGateBase extends PacketFlowBase
{
    parameters:
        displayStringTextFormat = default("passed %p pk (%l)");
        double bitrate @unit(bps) = default(nan bps);
        int extraLength @unit(b) = default(0b);
        double extraDuration @unit(s) = default(0s);
        @class(PacketGateBase);
        @display("i=block/cogwheel");
        @signal[gateStateChanged](type=bool);
        @statistic[gateState](title="gate state"; type=enum; enum=CLOSED, OPEN; source=gateStateChanged; record=vector; interpolationmode=sample-hold);
}

//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// Module interface for packet gate components in the queueing framework.
// Packet gates control the flow of packets by opening and closing according
// to various scheduling mechanisms.
//
// Key characteristics:
// - Has one input and one output
// - Can operate in both push and pull modes
// - Allows or blocks packet flow based on its open/closed state
// - Can be controlled by time-based schedules or external signals
// - Preserves packet order and integrity
// - Provides temporal isolation between traffic streams
//
// Packet gates are commonly used in network devices to:
// - Implement time-sensitive networking (TSN) mechanisms
// - Control traffic bursts and provide traffic shaping
// - Enforce time-based access control policies
// - Create time-division multiplexing schemes
// - Implement credit-based traffic control
//
// Common implementations include PacketGate, PeriodicGate, CreditBasedGate,
// and GateControlList. When a gate is not needed in a model, OmittedPacketGate
// can be used as a placeholder.
//
moduleinterface IPacketGate extends IPacketSink, IPacketSource
{
    parameters:
        @omittedTypename(OmittedPacketGate);
        @display("i=block/star");
}

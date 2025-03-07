//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// Module interface for packet filter components in the queueing framework.
// Packet filters selectively pass or drop packets based on specified criteria
// without modifying the packets themselves.
//
// Key characteristics:
// - Has one input and one output
// - Can operate in both push and pull modes
// - Evaluates each packet against filter criteria
// - Passes matching packets through without delay or reordering
// - Drops non-matching packets
// - Preserves packet integrity (no modifications)
//
// Packet filters are commonly used in network devices to:
// - Implement access control and traffic filtering
// - Enforce security policies
// - Perform traffic shaping and rate limiting
// - Support quality of service mechanisms
// - Filter out unwanted or malicious traffic
//
// Common implementations include ContentBasedFilter, LabelFilter,
// OrdinalBasedDropper, and StatisticalRateLimiter. When a filter is not
// needed in a model, OmittedPacketFilter can be used as a placeholder.
//
moduleinterface IPacketFilter extends IPacketFlow
{
    parameters:
        @omittedTypename(OmittedPacketFilter);
        @display("i=block/filter");
}

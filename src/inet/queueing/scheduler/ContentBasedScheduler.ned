//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.scheduler;

import inet.queueing.base.PacketSchedulerBase;
import inet.queueing.contract.IPacketScheduler;

//
// Deep packet inspection scheduler that selects packets based on their content
// using configurable filter expressions. This enables sophisticated traffic
// management based on packet data and metadata.
//
// Operation mechanism:
// - When pulled for a packet, the scheduler evaluates each packet against the
//   configured filter expressions in order
// - The first matching filter expression determines which input gate to pull from
// - If no filter matches, the defaultGateIndex is used
// - Filter expressions can examine any aspect of the packet, including headers,
//   payload, and attached tags
//
// The packetFilters parameter contains an array of filter expressions that are
// evaluated in order, with the first match determining the input gate to use.
//
// @see ~LabelScheduler, ~PriorityScheduler
//
simple ContentBasedScheduler extends PacketSchedulerBase like IPacketScheduler
{
    parameters:
        int defaultGateIndex = default(0); // Default gate index if no matching labels are found
        object packetFilters; // Array of packet filter expressions
        @class(ContentBasedScheduler);
}

//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// Module interface for packet marker components in the queueing framework.
// Packet markers attach metadata to packets as they pass through, without
// changing the packet's content or flow.
//
// Key characteristics:
// - Has one input and one output
// - Can operate in both push and pull modes
// - Processes packets individually
// - Attaches labels, tags, or other metadata to packets
// - Does not delay, reorder, or drop packets
// - Marks packets based on their content, metadata, or external criteria
//
// Packet markers are commonly used in network devices to:
// - Set quality of service (QoS) indicators like DSCP or priority values
// - Add flow identification information
// - Mark packets for special handling by downstream components
// - Implement Explicit Congestion Notification (ECN)
// - Support differentiated services in routers and switches
//
// Common implementations include PacketLabeler, PacketTagger, ContentBasedLabeler,
// ContentBasedTagger, and EcnMarker.
//
moduleinterface IPacketMarker extends IPacketFlow
{
    parameters:
        @display("i=block/star");
}

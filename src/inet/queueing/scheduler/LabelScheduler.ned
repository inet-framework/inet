//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.scheduler;

import inet.queueing.base.PacketSchedulerBase;
import inet.queueing.contract.IPacketScheduler;

//
// Packet scheduler that selects packets based on labels attached to them via
// the LabelsTag. This enables content-aware scheduling where the packet's
// metadata determines its priority or routing.
//
// The scheduler examines each packet's LabelsTag and compares the labels against
// a configured priority list. It selects the first input gate that contains a
// packet with the highest priority matching label. If no matching labels are found,
// it uses the defaultGateIndex.
//
// The labels parameter specifies a space-separated list of labels in order of
// priority, with the highest priority label listed first.
//
// @see ~LabelsTag, ~PriorityScheduler
//
simple LabelScheduler extends PacketSchedulerBase like IPacketScheduler
{
    parameters:
        int defaultGateIndex = default(0); // Default gate index if no matching labels are found
        string labels; // Space separated list of labels in the order of priority (highest first)
        @class(LabelScheduler);
}

//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.scheduler;

import inet.queueing.base.PacketSchedulerBase;
import inet.queueing.contract.IPacketScheduler;

//
// Schedules packets based on the attached labels in a ~LabelsTag.
//
simple LabelScheduler extends PacketSchedulerBase like IPacketScheduler
{
    parameters:
        int defaultGateIndex = default(0); // Default gate index if no matching labels are found
        string labels; // Space separated list of labels in the order of priority (highest first)
        @class(LabelScheduler);
}

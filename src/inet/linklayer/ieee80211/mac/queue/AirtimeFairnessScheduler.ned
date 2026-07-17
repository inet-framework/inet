//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.queue;

import inet.queueing.base.PacketSchedulerBase;
import inet.queueing.contract.IPacketScheduler;

//
// The scheduling half of the ~AirtimeFairnessQueue compound queue; the counterpart of
// ~AirtimeFairnessGate. It runs a deficit round robin over the per-station gates, as in
// Linux `mac80211` airtime fairness: it serves the first backlogged station whose gate is
// still open (airtime deficit non-negative), and grants an ineligible-but-backlogged
// station one `quantum * weight` of airtime credit before moving the cursor on. Over time
// every backlogged station gets an equal share of on-air time.
//
// The airtime deficit lives in the gates (charged a-posteriori from actual on-air time);
// this scheduler owns only the rotation and the top-up trigger, so the two form a matched
// pair and the scheduler must be wired to ~AirtimeFairnessGate inputs.
//
// With `fairnessEnabled = false` the gates stay open and this degrades to a plain
// per-station round robin (the frame-fair anomaly baseline).
//
// @see ~AirtimeFairnessQueue, ~AirtimeFairnessGate, ~DynamicClassifier
//
simple AirtimeFairnessScheduler extends PacketSchedulerBase like IPacketScheduler
{
    parameters:
        bool fairnessEnabled = default(true); // when false, degrades to a plain per-station round robin
        @class(AirtimeFairnessScheduler);
}

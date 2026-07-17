//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.queue;

import inet.queueing.base.PacketGateBase;
import inet.queueing.contract.IPacketGate;

//
// A per-station airtime-fairness gate for the ~AirtimeFairnessQueue compound queue. One
// gate sits on the transmit path of exactly one receiver (destination station); it holds
// that station's airtime deficit and is open while the station is eligible to transmit
// (deficit >= 0) and closed otherwise. This mirrors the way a ~PeriodicGate gates one
// sub-queue in a ~GatingPriorityQueue, except that eligibility is driven by on-air time
// rather than by a schedule.
//
// The deficit is charged a-posteriori from the actual on-air duration of each transmitted
// frame (retries included), reported by the coordination function (~Dcf/~Hcf) via the
// `frameTransmittedAirtime` signal (see ~Ieee80211AirtimeInd). The gate learns which
// station it serves from the receiver address of the first frame that passes through it.
//
// The gate forms a matched pair with ~AirtimeFairnessScheduler: the scheduler owns the
// deficit round-robin rotation and tops up an ineligible-but-backlogged gate by one
// `quantum * weight` when its turn comes; the gate owns the deficit state and the
// open/closed eligibility.
//
// With `fairnessEnabled = false` the gate stays permanently open, degrading the compound
// queue to a plain per-station round robin (the frame-fair anomaly baseline).
//
// @see ~AirtimeFairnessQueue, ~AirtimeFairnessScheduler, ~DynamicClassifier
//
simple AirtimeFairnessGate extends PacketGateBase like IPacketGate
{
    parameters:
        double quantum @unit(s) = default(1500us); // airtime top-up granted per round-robin turn (scaled by weight); ~one max-length frame's airtime
        double weight = default(1); // per-station airtime weight; equal weight means equal airtime share
        bool fairnessEnabled = default(true); // when false the gate stays open (plain per-station round robin)
        displayStringTextFormat = default("deficit {deficit}\npassed %p pk (%l)");
        @class(AirtimeFairnessGate);
        @signal[deficitChanged](type=double);
        // the statistical value is the current airtime deficit of the served station
        @statistic[deficit](title="airtime deficit"; source=deficitChanged; record=vector; unit=s; interpolationmode=sample-hold);
}

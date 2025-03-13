//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.queueing.gate.CreditBasedGate;

//
// Implements the IEEE 802.1Q Credit-Based Shaper (CBS) mechanism for traffic
// shaping in Time-Sensitive Networking (TSN) applications. This module controls
// packet flow based on a credit system that regulates transmission opportunities
// according to configured bandwidth parameters.
//
// The Credit-Based Shaper operates by:
// - Accumulating credits at the idleSlope rate when no transmission is occurring
// - Spending credits at the sendSlope rate during packet transmission
// - Allowing transmission only when credit is non-negative
// - Limiting maximum credit accumulation to prevent traffic bursts
//
// This mechanism ensures that each traffic stream receives its allocated bandwidth
// while preventing any single stream from monopolizing the network. It is particularly
// useful for applications requiring guaranteed bandwidth with bounded latency, such
// as audio/video streaming, industrial control, and automotive networks.
//
// The module extends the basic ~CreditBasedGate with specific parameters and
// calculations defined in the IEEE 802.1Q standard. It can be combined with a
// packet queue to form a complete credit-based shaper implementation.
//
// @see ~Ieee8021qCreditBasedShaper, ~Ieee8021qTimeAwareShaper
//
simple Ieee8021qCreditBasedGate extends CreditBasedGate
{
    parameters:
        double idleSlope @unit(bps); // Data rate of the accumulated credits when the transmission channel is idle
        double sendSlope @unit(bps) = default(idleSlope - bitrate); // Data rate of the spent credits when the transmission channel is in use
        double maxInterferenceSize @unit(b) = default(inf b);
        idleCreditGainRate = default(dropUnit(idleSlope));
        transmitCreditSpendRate = default(-dropUnit(sendSlope));
        maxCredit = default(dropUnit(maxInterferenceSize) * dropUnit(idleSlope / bitrate));
}

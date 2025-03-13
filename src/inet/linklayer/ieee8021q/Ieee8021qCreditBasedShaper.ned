//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

//
// Provides a complete implementation of the IEEE 802.1Q Credit-Based Shaper (CBS)
// for Time-Sensitive Networking (TSN) applications. This module extends the
// ~Ieee8021qCreditBasedGate to create a standalone traffic shaping component.
//
// The Credit-Based Shaper is one of the key traffic shaping mechanisms defined in
// IEEE 802.1Q for providing deterministic quality of service. It ensures that each
// traffic stream receives its allocated bandwidth while preventing any single stream
// from monopolizing network resources.
//
// This module inherits all the functionality of the ~Ieee8021qCreditBasedGate,
// including:
// - Credit accumulation during idle periods
// - Credit spending during transmission
// - Transmission control based on credit availability
// - Maximum credit limitation to prevent traffic bursts
//
// The CBS is particularly useful in applications requiring guaranteed bandwidth
// allocation with bounded latency, such as industrial automation, automotive
// networks, and professional audio/video systems.
//
// @see ~Ieee8021qCreditBasedGate, ~Ieee8021qTimeAwareShaper
//
module Ieee8021qCreditBasedShaper extends Ieee8021qCreditBasedGate
{
}

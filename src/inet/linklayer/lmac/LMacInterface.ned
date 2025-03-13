//
// Copyright (C) 2017 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.lmac;

import inet.linklayer.common.WirelessInterface;

//
// Provides a complete wireless network interface that uses the L-MAC protocol
// for energy-efficient, collision-free communication in wireless sensor networks.
// This interface combines the L-MAC protocol with appropriate radio and queue
// components to create a ready-to-use network interface.
//
// The interface uses:
// - L-MAC: A time-slotted MAC protocol where each node claims a unique timeslot
//   for transmission, enabling collision-free and energy-efficient communication
// - ApskRadio: Default radio model for wireless transmission
// - DropTailQueue: Simple FIFO queue with configurable capacity
//
// This interface is particularly suitable for static wireless sensor networks
// where predictable, energy-efficient communication is required. The time-slotted
// approach of L-MAC helps extend battery life while providing deterministic
// medium access.
//
// @see ~LMac, ~WirelessInterface
//
module LMacInterface extends WirelessInterface
{
    parameters:
        radio.typename = default("ApskRadio");
        mac.typename = "LMac";
        queue.typename = default("DropTailQueue");
        queue.packetCapacity = default(10);
}

//
// Copyright (C) 2017 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.bmac;

import inet.linklayer.common.WirelessInterface;

//
// Provides a complete wireless network interface that uses the B-MAC protocol
// for energy-efficient communication in wireless sensor networks. This interface
// combines the B-MAC protocol with appropriate radio and queue components to
// create a ready-to-use network interface.
//
// The interface uses:
// - B-MAC: An energy-efficient MAC protocol that uses preamble sampling and
//   low power listening to reduce energy consumption
// - ApskRadio: Default radio model for wireless transmission
// - DropTailQueue: Simple FIFO queue with configurable capacity
//
// This interface is particularly suitable for battery-powered devices in
// wireless sensor networks where energy efficiency is critical. The low power
// listening approach of B-MAC helps extend battery life while maintaining
// acceptable latency for most sensor network applications.
//
// @see ~BMac, ~WirelessInterface
//
module BMacInterface extends WirelessInterface
{
    parameters:
        radio.typename = default("ApskRadio");
        mac.typename = "BMac";
        queue.typename = default("DropTailQueue");
        queue.packetCapacity = default(20);
}

//
// Copyright (C) 2017 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.xmac;

import inet.linklayer.common.WirelessInterface;

//
// Provides a complete wireless network interface that uses the X-MAC protocol
// for energy-efficient communication in wireless sensor networks. This interface
// combines the X-MAC protocol with appropriate radio and queue components to
// create a ready-to-use network interface.
//
// The interface uses:
// - X-MAC: An energy-efficient MAC protocol that reduces power consumption through
//   preamble sampling and early acknowledgment
// - ApskRadio: Default radio model for wireless transmission
// - DropTailQueue: Simple FIFO queue with configurable capacity
//
// This interface is particularly suitable for battery-powered devices in
// low-data-rate sensor networks where energy efficiency is critical. The duty
// cycling approach of X-MAC helps extend battery life while maintaining
// acceptable latency for most sensor network applications.
//
// @see ~XMac, ~WirelessInterface
//
module XMacInterface extends WirelessInterface
{
    parameters:
        radio.typename = default("ApskRadio");
        mac.typename = "XMac";
        queue.typename = default("DropTailQueue");
        queue.packetCapacity = default(20);
}

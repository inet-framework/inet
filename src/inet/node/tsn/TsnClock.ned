//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.tsn;

import inet.linklayer.ieee8021as.GptpMaster;

//
// A specialized network device that serves as the master clock for Time-Sensitive
// Networking (TSN) simulations.
//
// This module represents a dedicated hardware device with a high-precision
// oscillator-based clock. It implements the IEEE 802.1AS-2020 (gPTP) protocol
// and acts as the grandmaster clock, distributing time information to all other
// devices in the network to achieve network-wide time synchronization.
//
// The TsnClock is a critical component in TSN networks, as precise time synchronization
// is fundamental to many TSN mechanisms such as time-aware scheduling, frame preemption,
// and redundancy management.
//
// @see ~TsnSwitch, ~TsnDevice
//
module TsnClock extends GptpMaster
{
    parameters:
        bool hasCutthroughSwitching = default(false); // Enable cut-through switching support
        clock.typename = default("OscillatorBasedClock"); // Master clocks cannot be set
        ethernet.typename = default("EthernetLayer"); // Use the Ethernet protocol layer outside network interfaces
        eth[*].typename = default("LayeredEthernetInterface"); // Switch to modular Ethernet interface
        eth[*].phyLayer.typename = default("EthernetStreamingPhyLayer"); // We need packet streaming to use the receptionStartedSignal
        @display("i=device/card"); // Change icon to emphasize hardware device
}

//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.common;

import inet.linklayer.base.MacRelayUnitBase;
import inet.linklayer.contract.IMacRelayUnit;

//
// Implements an Ethernet switch (or bridge) that performs MAC address learning
// and frame forwarding. This module provides the core functionality of a layer 2
// switch by relaying frames between different ports based on destination MAC addresses.
//
// The switch maintains a MAC address table (forwarding database) that maps MAC
// addresses to outgoing ports. This table is dynamically populated through a
// learning process: when a frame arrives on a port, the switch records the source
// MAC address and the incoming port in its table. When forwarding frames, the switch
// looks up the destination MAC address in its table and:
// - If found, forwards the frame only to the associated port
// - If not found, floods the frame to all ports except the incoming one (broadcast)
// - If the destination is a broadcast/multicast address, floods to all ports except the incoming one
//
// This implementation extends ~MacRelayUnitBase, which provides the common infrastructure
// for MAC relay units, and focuses on the specific switching behavior. It can be used
// in various network topologies to connect multiple Ethernet segments.
//
// @see ~MacRelayUnitBase, ~IMacRelayUnit, ~MacForwardingTable
//
simple MacRelayUnit extends MacRelayUnitBase like IMacRelayUnit
{
    parameters:
        @class(MacRelayUnit);
}

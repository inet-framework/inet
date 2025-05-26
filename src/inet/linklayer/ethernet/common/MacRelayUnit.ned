//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.common;

import inet.linklayer.base.MacRelayUnitBase;
import inet.linklayer.contract.IMacRelayUnit;

//
// Implements Ethernet switch functionality by relaying frames between different
// ports based on destination MAC addresses. Maintains a MAC address table that
// maps addresses to ports, learns addresses from incoming frames, and forwards
// frames appropriately. Handles unicast, multicast, and broadcast traffic
// according to standard Ethernet switching rules, with support for VLANs.
//
simple MacRelayUnit extends MacRelayUnitBase like IMacRelayUnit
{
    parameters:
        @class(MacRelayUnit);
}

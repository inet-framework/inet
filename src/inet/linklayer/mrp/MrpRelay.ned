//
// Copyright (C) 2024 Daniel Zeitler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.mrp;

import inet.linklayer.base.MacRelayUnitBase;
import inet.linklayer.contract.IMacRelayUnit;
import inet.linklayer.ieee8021d.relay.Ieee8021dRelay;

//
// Relay unit with support for the Media Redundancy Protocol (MRP), defined
// in IEC 62439-2.
//
// This module forwards Ethernet frames based on their destination MAC addresses
// to appropriate ports. It handles frame forwarding in accordance with
// interface data from both STP and MRP, and blocks traffic if ports are set to
// BLOCKED. If a frame specific to STP or MRP is received, this frame is
// processed according to the rules in ~MrpMacForwardingTable, and then
// forwarded to the appropriate upper layer MRP or STP instance.
//
// @see ~MrpMacForwardingTable, ~Mrp, ~MrpInterconnection
//
simple MrpRelay extends Ieee8021dRelay like IMacRelayUnit
{
    parameters:
        @class(MrpRelay);

        // Switches take a certain amount of time for processing each packet.
        // This parameter sets the processsing delay applied before sending
        // packets out.
        volatile double switchingDelay @unit(s) = default(truncnormal(8.61us,5.42us));
}


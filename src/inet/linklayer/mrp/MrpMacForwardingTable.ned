//
// Copyright (C) 2024 Daniel Zeitler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.linklayer.mrp;

import inet.linklayer.contract.IMacForwardingTable;

//
// Implements a specialized MAC forwarding table with support for the Media
// Redundancy Protocol (MRP) defined in IEC 62439-2. This module extends the
// functionality of the standard ~MacForwardingTable to handle the specific
// requirements of MRP ring topologies.
//
// The MrpMacForwardingTable maintains MAC address to port mappings while
// respecting the port states managed by the MRP protocol. It works in conjunction
// with the MRP protocol to ensure proper frame forwarding in ring topologies
// while maintaining a loop-free logical topology.
//
// Key features include:
// - Support for MRP-specific port states (FORWARDING, BLOCKED)
// - Handling of topology changes triggered by MRP events
// - Clearing of forwarding database entries upon topology changes
// - Proper interaction with MRP control frames
// - Compatibility with both MRP and STP/RSTP protocols when both are present
//
// This module is an essential component in MRP-enabled switches and is used
// by the ~MrpRelay module to make forwarding decisions based on destination
// MAC addresses while respecting the logical topology created by MRP.
//
// @see ~MrpRelay, ~Mrp, ~MrpInterconnection, ~MacForwardingTable
//
simple MrpMacForwardingTable like IMacForwardingTable
{
    parameters:
        @display("i=block/table2");
        double agingTime @unit(s) = default(120s);
        object forwardingTable @mutable = default([]);
        string forwardingTableFile = default("");
        string interfaceTableModule;
        string displayStringTextFormat = default("addr: %a\nvlan: %v");
}

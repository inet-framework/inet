//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.common;

import inet.linklayer.contract.IMacForwardingTable;

//
// Maintains a forwarding database that maps MAC addresses to outgoing interfaces
// in Ethernet switches and bridges. This module is a key component of layer 2
// switching, enabling frames to be forwarded to the correct destination port.
//
// The forwarding table (also known as a MAC address table or filtering database)
// contains entries that associate MAC addresses with the ports on which those
// addresses were last seen. Each entry includes:
// - MAC address: The hardware address of a connected device
// - Interface: The port through which the device can be reached
// - VLAN ID: Optional VLAN identifier for VLAN-aware switching
// - Age: Time since the entry was last refreshed
//
// Key features:
// - Dynamic learning: Entries are automatically added when frames are received
// - Aging: Entries are removed after a configurable aging time to handle topology changes
// - Pre-loading: Tables can be initialized from configuration files
// - VLAN support: Entries can be associated with specific VLANs
//
// The table can be pre-loaded from text files at the beginning of the simulation
// using the `forwardingTableFile` parameter. File format:
//   line oriented table with space or TAB delimited columns.
//   columns:
//     VLAN ID:      numeric value, 0 means no vlan
//     MAC address:  string, resolved with ~L3AddressResolver as MAC_ADDR
//     Interface:    interface name, e.g. eth3 (or interface id, e.g. 103)
//   empty lines and commented out lines starting with '#' are allowed.
//
// @see ~MacRelayUnit, ~IMacForwardingTable
//
simple MacForwardingTable like IMacForwardingTable
{
    parameters:
        @display("i=block/table2");
        double agingTime @unit(s) = default(120s);
        object forwardingTable @mutable = default([]);
        string forwardingTableFile = default("");
        string interfaceTableModule;
        string displayStringTextFormat = default("addr: %a\nvlan: %v");
}

//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.common;

import inet.linklayer.contract.IMacForwardingTable;

//
// This module handles the mapping between ports and MAC addresses.
//
// Entries are deleted if their age exceeds a certain limit.
//
// If needed, address tables can be pre-loaded from text files at the beginning
// of the simulation; this controlled by the forwardingTableFile module parameter.
// File format:
//   line oriented table with space or TAB delimited columns.
//   columns:
//     VLAN ID:      numeric value, 0 means no vlan
//     MAC address:  string, resolving with ~L3AddressResolver as MAC_ADDR
//     Interface:    interface name, e.g. eth3 (or interface id, e.g. 103)
//   empty lines and commented out lines starts with '#' are allowed.
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


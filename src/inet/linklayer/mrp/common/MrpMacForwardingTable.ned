// Copyright (C) 2024 Daniel Zeitler
// SPDX-License-Identifier: LGPL-3.0-or-later

package inet.linklayer.mrp.common;

import inet.linklayer.contract.IMacForwardingTable;

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


//
// Copyright (C) 2024 Daniel Zeitler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.linklayer.mrp;

import inet.common.SimpleModule;
import inet.linklayer.contract.IMacForwardingTable;

//
// This is a variant of ~MacForwardingTable, with added support for the Media
// Redundancy Protocol (MRP) defined in IEC 62439-2.
//
// @see ~MrpRelay, ~Mrp, ~MrpInterconnection
//
simple MrpMacForwardingTable extends SimpleModule like IMacForwardingTable
{
    parameters:
        @class(MrpMacForwardingTable);
        @display("i=block/table2");
        double agingTime @unit(s) = default(120s);
        object forwardingTable @mutable = default([]);
        string forwardingTableFile = default("");
        string interfaceTableModule;
        displayStringTextFormat = default("addr: %a\nvlan: %v");
}


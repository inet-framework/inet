//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.contract;

//
// Interface for MAC address tables, used by MAC relay units in Ethenet switches.
//
moduleinterface IMacForwardingTable
{
    parameters:
        @display("i=block/table2");
        double agingTime @unit(s);
        string forwardingTableFile;
}


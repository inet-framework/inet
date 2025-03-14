//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.contract;

//
// Module interface for MAC forwarding tables that map MAC addresses to outgoing interfaces
// in Ethernet switches and bridges. These tables maintain the association between destination
// MAC addresses and the ports through which those addresses can be reached, enabling
// efficient frame forwarding in layer 2 networks. The tables support dynamic learning,
// aging of entries, and can be pre-loaded from configuration files.
//
// @see MacForwardingTable, MrpMacForwardingTable, MacRelayUnit
//
moduleinterface IMacForwardingTable
{
    parameters:
        @display("i=block/table2");
        double agingTime @unit(s);
        string forwardingTableFile;
}

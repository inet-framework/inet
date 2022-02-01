//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.nexthop;

import inet.networklayer.contract.IRoutingTable;

//
// This module stores next hop routes used by the next hop forwarding protocol.
//
simple NextHopRoutingTable like IRoutingTable
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        @display("i=block/table");
        string addressType @enum("mac","modulepath","moduleid") = default("modulepath");
        string routerId = default("auto"); // for routers, the router id using address dotted
                          // notation; specify "auto" to select the highest
                          // interface address; should be left empty ("") for hosts
        bool forwarding = default(true);  // turns IP forwarding on/off
        bool multicastForwarding = default(false); // turns multicast forwarding on/off
        @signal[routeAdded](type=inet::NextHopRoute);
        @signal[routeDeleted](type=inet::NextHopRoute);
        @signal[routeChanged](type=inet::NextHopRoute);
}


//
// Copyright (C) 2010 Helene Lageber
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.routing.contract;

import inet.applications.contract.IApp;

//
// Interface for BGP routing module.
//
moduleinterface IBgp extends IApp
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        string ospfRoutingModule;
        @display("i=block/app");
}


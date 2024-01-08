//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.routing.leach;

import inet.routing.contract.IManetRouting;

simple LeachBS like IManetRouting
{
    parameters:
        @class("leach::LeachBS");
        @display("i=block/routing");
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
    gates:
        input ipIn;
        output ipOut;
}

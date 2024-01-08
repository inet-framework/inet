//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.routing.leach;

import inet.routing.contract.IManetRouting;

simple Leach like IManetRouting
{
    parameters:
        @class("leach::Leach");
        @display("i=block/routing");
        string interfaceTableModule;   // The path to the InterfaceTable module
        double maxVariance = default(1); 
        int numNodes = default(10);
        double clusterHeadPercentage = default(0.5);
        string netInterface = default("wlan");
    gates:
        input ipIn;
        output ipOut;
}

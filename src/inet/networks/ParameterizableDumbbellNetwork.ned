//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.common.ILink;

//
// A parameterizable dumbbell network topology.
//
// Two routers connected by a single bottleneck link, each with a set of hosts.
// Commonly used for congestion and queuing studies.
//
// <pre>
//   [h0]--+            +--[h0]
//   [h1]--+--[R0]--[R1]--+--[h1]
//   [h2]--+            +--[h2]
//    ...                  ...
// </pre>
//
network DumbbellNetwork extends WiredNetworkBase
{
    parameters:
        int numLeftHosts; // hosts on the left side
        int numRightHosts; // hosts on the right side
    submodules:
        leftRouter: <default("EthernetSwitch")> like IEthernetNetworkNode {
        }
        rightRouter: <default("EthernetSwitch")> like IEthernetNetworkNode {
        }
        leftHost[numLeftHosts]: <default("StandardHost")> like IEthernetNetworkNode {
            @display("p=300,100,column,100");
        }
        rightHost[numRightHosts]: <default("StandardHost")> like IEthernetNetworkNode {
            @display("p=900,100,column,100");
        }
    connections:
        // Bottleneck link
        leftRouter.ethg++ <--> <default("EthernetLink")> like ILink <--> rightRouter.ethg++;
        // Left hosts
        for i=0..numLeftHosts-1 {
            leftHost[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leftRouter.ethg++;
        }
        // Right hosts
        for i=0..numRightHosts-1 {
            rightHost[i].ethg++ <--> <default("EthernetLink")> like ILink <--> rightRouter.ethg++;
        }
}

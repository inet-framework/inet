//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.topology;

import inet.common.ILink;
import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;

//
// A parameterizable star network topology.
//
// A single central node connects to every peripheral node.
// Commonly used in small LANs, campus, and hub-and-spoke WAN deployments.
//
// <pre>
//       [1]  [2]
//        \  /
//   [0]--[center]--[3]
//        /  \
//      [n-1] [4]
// </pre>
//
network StarNetwork extends WiredNetworkBase
{
    parameters:
        int numNodes; // number of peripheral nodes
    submodules:
        center: <default("EthernetSwitch")> like IEthernetNetworkNode {
            @display("p=400,300");
        }
        node[numNodes]: <default("StandardHost")> like IEthernetNetworkNode {
            @display("p=400,300,ring,200");
        }
    connections:
        for i=0..numNodes-1 {
            center.ethg++ <--> <default("EthernetLink")> like ILink <--> node[i].ethg++;
        }
}

//
// Small star: 4 peripheral nodes.
//
network SmallStarNetwork extends StarNetwork
{
    parameters:
        numNodes = default(4);
}

//
// Medium star: 8 peripheral nodes.
//
network MediumStarNetwork extends StarNetwork
{
    parameters:
        numNodes = default(8);
}

//
// Large star: 16 peripheral nodes.
//
network LargeStarNetwork extends StarNetwork
{
    parameters:
        numNodes = default(16);
}

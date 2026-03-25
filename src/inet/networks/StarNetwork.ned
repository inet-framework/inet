//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.common.ILink;

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
        center: <default("EthernetSwitch")> like IEthernetNetworkNode;
        node[numNodes]: <default("StandardHost")> like IEthernetNetworkNode;
    connections:
        for i=0..numNodes-1 {
            center.ethg++ <--> <default("EthernetLink")> like ILink <--> node[i].ethg++;
        }
}

//
// Small star: 9 peripheral nodes + 1 center = 10 nodes.
//
network SmallStarNetwork extends StarNetwork
{
    parameters:
        numNodes = default(9);
}

//
// Medium star: 99 peripheral nodes + 1 center = 100 nodes.
//
network MediumStarNetwork extends StarNetwork
{
    parameters:
        numNodes = default(99);
}

//
// Large star: 999 peripheral nodes + 1 center = 1000 nodes.
//
network LargeStarNetwork extends StarNetwork
{
    parameters:
        numNodes = default(999);
}

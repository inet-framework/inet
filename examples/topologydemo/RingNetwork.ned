//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.topologydemo;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.node.ethernet.EthernetLink;

//
// A parameterizable ring network topology.
//
// Each node connects to its two neighbors forming a closed ring.
// Commonly used in backbone/WAN, metro, and token-ring networks.
//
// <pre>
//       [0]---[1]
//      /         \
//   [n-1]       [2]
//      \         /
//      [n-2]-[3]
// </pre>
//
network RingNetwork extends WiredNetworkBase
{
    parameters:
        int numNodes; // number of nodes in the ring
    submodules:
        node[numNodes]: <default("EthernetSwitch")> like IEthernetNetworkNode {
            @display("p=300,300,ring,200");
        }
    connections:
        for i=0..numNodes-1 {
            node[i].ethg++ <--> EthernetLink <--> node[(i+1) % numNodes].ethg++;
        }
}

//
// Small ring: 4 nodes.
//
network SmallRingNetwork extends RingNetwork
{
    parameters:
        numNodes = default(4);
}

//
// Medium ring: 8 nodes.
//
network MediumRingNetwork extends RingNetwork
{
    parameters:
        numNodes = default(8);
}

//
// Large ring: 16 nodes.
//
network LargeRingNetwork extends RingNetwork
{
    parameters:
        numNodes = default(16);
}

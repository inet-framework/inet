 //
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.common.ILink;

//
// A parameterizable ring network topology.
//
// Each node connects to its two neighbors forming a closed ring.
// Commonly used in backbone/WAN, metro, and token-ring networks.
//
// <pre>
//       [0]---[1]
//      /  \   /  \
//  [h0,h1] [h2,h3]
//     |      |
//   [n-1]   [2]
//      \   /   /
//      [n-2]-[3]
// </pre>
//
network RingNetwork extends WiredNetworkBase
{
    parameters:
        int numNodes; // number of nodes in the ring
        volatile int numHostsPerSwitch = default(0); // number of hosts connected to each switch
        object numHosts = default(repeatPar("numHostsPerSwitch", numNodes));
    submodules:
        node[numNodes]: <default("EthernetSwitch")> like IEthernetNetworkNode {
        }
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        for i=0..numNodes-1 {
            node[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node[(i+1) % numNodes].ethg++;
        }
        for i=0..sizeof(host)-1 {
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node[groupIndex(numHosts,i)].ethg++;
        }
}

//
// Small ring: 10 nodes.
//
network SmallRingNetwork extends RingNetwork
{
    parameters:
        numNodes = default(10);
}

//
// Medium ring: 100 nodes.
//
network MediumRingNetwork extends RingNetwork
{
    parameters:
        numNodes = default(100);
}

//
// Large ring: 1000 nodes.
//
network LargeRingNetwork extends RingNetwork
{
    parameters:
        numNodes = default(1000);
}

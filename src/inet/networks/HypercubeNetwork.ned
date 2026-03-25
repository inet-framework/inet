//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.common.ILink;

//
// A parameterizable hypercube network topology.
//
// A d-dimensional hypercube has 2^d nodes. Two nodes are connected
// if and only if their binary indices differ in exactly one bit.
// Each node has exactly d neighbors.
//
// Classic topology for parallel computing and routing algorithm studies.
//
// <pre>
//   d=2 (4 nodes):        d=3 (8 nodes):
//   [00]---[01]           [000]---[001]
//    |       |             | \     | \
//   [10]---[11]           [010]--[011]
//                          |       |
//                         [100]--[101]
//                          | /     | /
//                         [110]--[111]
// </pre>
//
network HypercubeNetwork extends WiredNetworkBase
{
    parameters:
        int numDimensions; // number of dimensions (d), total nodes = 2^d
        int numNodes = int(2^numDimensions); // computed
        volatile int numHostsPerSwitch = default(0); // number of hosts connected to each switch
        object numHosts = default(repeatPar("numHostsPerSwitch", numNodes));
    submodules:
        node[numNodes]: <default("EthernetSwitch")> like IEthernetNetworkNode;
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        // Connect nodes that differ in exactly bit d: node i <-> node i XOR (1<<d)
        // Only connect when i < i XOR (1<<d) to avoid duplicates
        for d=0..numDimensions-1, for i=0..numNodes-1 {
            node[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node[int(i + int(2^d))].ethg++ if (i % int(2^(d+1))) < int(2^d);
        }
        for i=0..sizeof(host)-1 {
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node[groupIndex(numHosts,i)].ethg++;
        }
}

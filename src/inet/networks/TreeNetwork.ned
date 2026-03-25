//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.common.ILink;

//
// A parameterizable k-ary tree network topology.
//
// A hierarchical tree where each internal node has exactly k children.
// Total nodes = (k^(depth+1) - 1) / (k - 1) for k > 1.
// Commonly used in campus, enterprise, and aggregation networks.
//
// <pre>
//              [root]                     depth 0
//           /    |    \
//        [0]   [1]   [2]                 depth 1
//       / | \ / | \ / | \
//      [h] ...  [h] ...  [h]             depth 2 (leaves)
// </pre>
//
// For k=2, depth=2: 7 nodes (1 root + 2 internal + 4 leaves).
// For k=3, depth=2: 13 nodes (1 root + 3 internal + 9 leaves).
//
network TreeNetwork extends WiredNetworkBase
{
    parameters:
        int k = default(2); // branching factor (children per node)
        int depth = default(2); // tree depth (root is at depth 0)
        int totalNodes = int((int(k^(depth+1)) - 1) / (k - 1)); // computed total nodes
        volatile int numHostsPerSwitch = default(0); // number of hosts connected to each switch
        object numHosts = default(repeatPar("numHostsPerSwitch", totalNodes));
    submodules:
        node[totalNodes]: <default("EthernetSwitch")> like IEthernetNetworkNode {
        }
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        // Each node i (except root 0) connects to its parent (i-1)/k
        for i=1..totalNodes-1 {
            node[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node[int((i-1)/k)].ethg++;
        }
        for i=0..sizeof(host)-1 {
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node[groupIndex(numHosts,i)].ethg++;
        }
}

//
// Binary tree with depth 3: 15 nodes.
//
network BinaryTreeNetwork extends TreeNetwork
{
    parameters:
        k = default(2);
        depth = default(3);
}

//
// Ternary tree with depth 2: 13 nodes.
//
network TernaryTreeNetwork extends TreeNetwork
{
    parameters:
        k = default(3);
        depth = default(2);
}

//
// Quad tree with depth 2: 21 nodes.
//
network QuadTreeNetwork extends TreeNetwork
{
    parameters:
        k = default(4);
        depth = default(2);
}

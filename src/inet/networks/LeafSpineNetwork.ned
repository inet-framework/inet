//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.common.ILink;

//
// A parameterizable leaf-spine datacenter network topology.
//
// The topology consists of three tiers:
//  - Spine switches: form the backbone of the fabric
//  - Leaf switches: each connects to every spine (full bipartite graph)
//  - Hosts: distributed evenly across leaf switches
//
// Every leaf switch has an uplink to every spine switch, and each leaf
// switch has a configurable number of hosts attached to its downlink ports.
//
// <pre>
//   [spine0]  [spine1]  ...  [spine[N-1]]        <- numSpines
//      |  \  /  |    \  /  |
//      |   \/   |     \/   |
//      |   /\   |     /\   |                     <- full bipartite mesh
//      |  /  \  |    /  \  |
//   [leaf0]  [leaf1]  ...  [leaf[M-1]]            <- numLeaves
//    / \      / \           / \
//  [h0][h1] [h2][h3]     [h..][h..]              <- numHostsPerLeaf per leaf
// </pre>
//
network LeafSpineNetwork extends WiredNetworkBase
{
    parameters:
        int numSpines; // number of spine switches
        int numLeaves; // number of leaf (ToR) switches
        volatile int numHostsPerLeaf; // number of hosts connected to each leaf switch
        object numHosts = default(repeatPar("numHostsPerLeaf", numLeaves));
    submodules:
        spine[numSpines]: <default("EthernetSwitch")> like IEthernetNetworkNode {
            @display("p=400,100,row,200");
        }
        leaf[numLeaves]: <default("EthernetSwitch")> like IEthernetNetworkNode {
            @display("p=300,300,row,150");
        }
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {
            @display("p=250,500,row,100");
        }
    connections:
        // Full mesh: every leaf connects to every spine
        for i=0..numLeaves-1, for j=0..numSpines-1 {
            leaf[i].ethg++ <--> <default("EthernetLink")> like ILink <--> spine[j].ethg++;
        }
        // Each leaf connects to its hosts
        for i=0..sizeof(host)-1 {
            leaf[groupIndex(numHosts,i)].ethg++ <--> <default("EthernetLink")> like ILink <--> host[i].ethg++;
        }
}

//
// Small leaf-spine: 2 spines, 2 leaves, 3 hosts/leaf = 10 nodes.
//
network SmallLeafSpineNetwork extends LeafSpineNetwork
{
    parameters:
        numSpines = default(2);
        numLeaves = default(2);
        numHostsPerLeaf = default(3);
}

//
// Medium leaf-spine: 4 spines, 8 leaves, 11 hosts/leaf = 100 nodes.
//
network MediumLeafSpineNetwork extends LeafSpineNetwork
{
    parameters:
        numSpines = default(4);
        numLeaves = default(8);
        numHostsPerLeaf = default(11);
}

//
// Large leaf-spine: 8 spines, 32 leaves, 30 hosts/leaf = 1000 nodes.
//
network LargeLeafSpineNetwork extends LeafSpineNetwork
{
    parameters:
        numSpines = default(8);
        numLeaves = default(32);
        numHostsPerLeaf = default(30);
}

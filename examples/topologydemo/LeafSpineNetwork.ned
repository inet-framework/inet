//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.topologydemo;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.node.ethernet.EthernetLink;

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
        int numHostsPerLeaf; // number of hosts connected to each leaf switch
    submodules:
        spine[numSpines]: <default("EthernetSwitch")> like IEthernetNetworkNode {
            @display("p=400,100,row,200");
        }
        leaf[numLeaves]: <default("EthernetSwitch")> like IEthernetNetworkNode {
            @display("p=300,300,row,150");
        }
        host[numLeaves * numHostsPerLeaf]: <default("StandardHost")> like IEthernetNetworkNode {
            @display("p=250,500,row,100");
        }
    connections:
        // Full mesh: every leaf connects to every spine
        for i=0..numLeaves-1, for j=0..numSpines-1 {
            leaf[i].ethg++ <--> EthernetLink <--> spine[j].ethg++;
        }
        // Each leaf connects to its hosts
        for i=0..numLeaves-1, for j=0..numHostsPerLeaf-1 {
            leaf[i].ethg++ <--> EthernetLink <--> host[i * numHostsPerLeaf + j].ethg++;
        }
}

//
// Small leaf-spine: 2 spines, 2 leaves, 2 hosts/leaf = 4 hosts.
// Suitable for basic testing and tutorials.
//
network SmallLeafSpineNetwork extends LeafSpineNetwork
{
    parameters:
        numSpines = default(2);
        numLeaves = default(2);
        numHostsPerLeaf = default(2);
}

//
// Medium leaf-spine: 2 spines, 4 leaves, 4 hosts/leaf = 16 hosts.
// Represents a small datacenter pod or lab deployment.
//
network MediumLeafSpineNetwork extends LeafSpineNetwork
{
    parameters:
        numSpines = default(2);
        numLeaves = default(4);
        numHostsPerLeaf = default(4);
}

//
// Large leaf-spine: 4 spines, 8 leaves, 8 hosts/leaf = 64 hosts.
// Represents a typical datacenter fabric.
//
network LargeLeafSpineNetwork extends LeafSpineNetwork
{
    parameters:
        numSpines = default(4);
        numLeaves = default(8);
        numHostsPerLeaf = default(8);
}

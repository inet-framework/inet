//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.common.ILink;

//
// Clustered regional mesh with weak inter-cluster bridges (18 nodes, 28 links).
//
// Three clusters of six nodes each, internally well-connected as
// near-meshes (hexagonal ring plus two diameters per cluster). The
// clusters are joined by only four inter-cluster bridge links through
// designated gateway nodes: one bridge between clusters A and B, one
// between A and C, and two between B and C.
//
// Typical of municipalities, multi-site industrial systems, sensor/IoT
// domains, or federated organizations.
//
// Properties: strong modularity, heterogeneous degree distribution,
// high path diversity within clusters, low across clusters,
// bridge-dominated bottlenecks, regional redundancy but weak global
// redundancy.
//
// <pre>
//        Cluster A              Cluster B
//       [a0]--[a1]           [b0]--[b1]
//       / \    / \           / \    / \
//     [a5] \  / [a2]------[b5] \  / [b2]
//       \ / \/ /             \ / \/ /
//     [a4]--[a3]           [b4]--[b3]
//             \              |    /
//              \             |   /
//               [c0]--[c1]--+--'
//               / \    / \
//             [c5] \  / [c2]
//               \ / \/ /
//             [c4]--[c3]
//              Cluster C
// </pre>
//
network ClusteredMeshNetwork extends WiredNetworkBase
{
    parameters:
        volatile int numHostsPerSwitch = default(0); // hosts attached to every switch
        object numHosts = default(repeatPar("numHostsPerSwitch", 18));
    submodules:
        // cluster A (left)
        a0: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=350,150"); }
        a1: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=450,150"); }
        a2: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=490,260"); }
        a3: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=450,370"); }
        a4: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=350,370"); }
        a5: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=310,260"); }
        // cluster B (top-right)
        b0: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=730,60"); }
        b1: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=830,60"); }
        b2: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=870,170"); }
        b3: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=830,280"); }
        b4: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=730,280"); }
        b5: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=690,170"); }
        // cluster C (bottom-right)
        c0: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=690,430"); }
        c1: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=790,430"); }
        c2: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=830,540"); }
        c3: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=790,650"); }
        c4: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=690,650"); }
        c5: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=650,540"); }
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        // ---- cluster A internal (8 links: hexagonal ring + 2 diameters) ----
        a0.ethg++ <--> <default("EthernetLink")> like ILink <--> a1.ethg++;
        a1.ethg++ <--> <default("EthernetLink")> like ILink <--> a2.ethg++;
        a2.ethg++ <--> <default("EthernetLink")> like ILink <--> a3.ethg++;
        a3.ethg++ <--> <default("EthernetLink")> like ILink <--> a4.ethg++;
        a4.ethg++ <--> <default("EthernetLink")> like ILink <--> a5.ethg++;
        a5.ethg++ <--> <default("EthernetLink")> like ILink <--> a0.ethg++;
        a0.ethg++ <--> <default("EthernetLink")> like ILink <--> a3.ethg++;  // diameter
        a1.ethg++ <--> <default("EthernetLink")> like ILink <--> a4.ethg++;  // diameter

        // ---- cluster B internal (8 links: hexagonal ring + 2 diameters) ----
        b0.ethg++ <--> <default("EthernetLink")> like ILink <--> b1.ethg++;
        b1.ethg++ <--> <default("EthernetLink")> like ILink <--> b2.ethg++;
        b2.ethg++ <--> <default("EthernetLink")> like ILink <--> b3.ethg++;
        b3.ethg++ <--> <default("EthernetLink")> like ILink <--> b4.ethg++;
        b4.ethg++ <--> <default("EthernetLink")> like ILink <--> b5.ethg++;
        b5.ethg++ <--> <default("EthernetLink")> like ILink <--> b0.ethg++;
        b0.ethg++ <--> <default("EthernetLink")> like ILink <--> b3.ethg++;  // diameter
        b1.ethg++ <--> <default("EthernetLink")> like ILink <--> b4.ethg++;  // diameter

        // ---- cluster C internal (8 links: hexagonal ring + 2 diameters) ----
        c0.ethg++ <--> <default("EthernetLink")> like ILink <--> c1.ethg++;
        c1.ethg++ <--> <default("EthernetLink")> like ILink <--> c2.ethg++;
        c2.ethg++ <--> <default("EthernetLink")> like ILink <--> c3.ethg++;
        c3.ethg++ <--> <default("EthernetLink")> like ILink <--> c4.ethg++;
        c4.ethg++ <--> <default("EthernetLink")> like ILink <--> c5.ethg++;
        c5.ethg++ <--> <default("EthernetLink")> like ILink <--> c0.ethg++;
        c0.ethg++ <--> <default("EthernetLink")> like ILink <--> c3.ethg++;  // diameter
        c2.ethg++ <--> <default("EthernetLink")> like ILink <--> c5.ethg++;  // diameter

        // ---- inter-cluster bridges (4 links) ----
        a2.ethg++ <--> <default("EthernetLink")> like ILink <--> b5.ethg++;  // A ↔ B
        a3.ethg++ <--> <default("EthernetLink")> like ILink <--> c0.ethg++;  // A ↔ C
        b3.ethg++ <--> <default("EthernetLink")> like ILink <--> c2.ethg++;  // B ↔ C
        b4.ethg++ <--> <default("EthernetLink")> like ILink <--> c1.ethg++;  // B ↔ C (second bridge)
        // hosts
        for i=0..sizeof(host)-1 {
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> a0.ethg++ if groupIndex(numHosts,i) == 0;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> a1.ethg++ if groupIndex(numHosts,i) == 1;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> a2.ethg++ if groupIndex(numHosts,i) == 2;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> a3.ethg++ if groupIndex(numHosts,i) == 3;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> a4.ethg++ if groupIndex(numHosts,i) == 4;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> a5.ethg++ if groupIndex(numHosts,i) == 5;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b0.ethg++ if groupIndex(numHosts,i) == 6;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b1.ethg++ if groupIndex(numHosts,i) == 7;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b2.ethg++ if groupIndex(numHosts,i) == 8;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b3.ethg++ if groupIndex(numHosts,i) == 9;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b4.ethg++ if groupIndex(numHosts,i) == 10;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b5.ethg++ if groupIndex(numHosts,i) == 11;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> c0.ethg++ if groupIndex(numHosts,i) == 12;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> c1.ethg++ if groupIndex(numHosts,i) == 13;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> c2.ethg++ if groupIndex(numHosts,i) == 14;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> c3.ethg++ if groupIndex(numHosts,i) == 15;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> c4.ethg++ if groupIndex(numHosts,i) == 16;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> c5.ethg++ if groupIndex(numHosts,i) == 17;
        }
}

//
// A larger clustered regional mesh with 36 nodes and 56 links.
//
// Six clusters of six nodes each, arranged in a ring of clusters.
// Each cluster is internally connected as a hexagonal ring plus two
// diameters (8 internal links). Adjacent clusters share one bridge
// link each, plus two long-range bridges across the ring, for a
// total of 8 inter-cluster bridges.
//
// <pre>
//         Cluster A              Cluster B
//        [a0]--[a1]            [b0]--[b1]
//        / \    / \            / \    / \
//      [a5] \  / [a2]-------[b5] \  / [b2]
//        \ / \/ /              \ / \/ /
//      [a4]--[a3]            [b4]--[b3]
//       |                             |
//     Cluster F              Cluster C
//      [f0]--[f1]            [c0]--[c1]
//      / \    / \            / \    / \
//    [f5] \  / [f2]--------[c5] \  / [c2]
//      \ / \/ /              \ / \/ /
//    [f4]--[f3]            [c4]--[c3]
//       |                             |
//     Cluster E              Cluster D
//      [e0]--[e1]            [d0]--[d1]
//      / \    / \            / \    / \
//    [e5] \  / [e2]--------[d5] \  / [d2]
//      \ / \/ /              \ / \/ /
//    [e4]--[e3]            [d4]--[d3]
//
//   Long-range: a3-d0, b4-e1
// </pre>
//
network LargeClusteredMeshNetwork extends WiredNetworkBase
{
    parameters:
        volatile int numHostsPerSwitch = default(0); // hosts attached to every switch
        object numHosts = default(repeatPar("numHostsPerSwitch", 36));
    submodules:
        // cluster A (top-left)
        a0: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=400,50"); }
        a1: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=500,50"); }
        a2: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=540,160"); }
        a3: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=500,270"); }
        a4: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=400,270"); }
        a5: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=360,160"); }
        // cluster B (top-right)
        b0: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=800,50"); }
        b1: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=900,50"); }
        b2: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=940,160"); }
        b3: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=900,270"); }
        b4: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=800,270"); }
        b5: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=760,160"); }
        // cluster C (middle-right)
        c0: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=800,370"); }
        c1: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=900,370"); }
        c2: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=940,480"); }
        c3: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=900,590"); }
        c4: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=800,590"); }
        c5: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=760,480"); }
        // cluster D (bottom-right)
        d0: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=800,690"); }
        d1: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=900,690"); }
        d2: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=940,800"); }
        d3: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=900,910"); }
        d4: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=800,910"); }
        d5: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=760,800"); }
        // cluster E (bottom-left)
        e0: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=400,690"); }
        e1: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=500,690"); }
        e2: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=540,800"); }
        e3: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=500,910"); }
        e4: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=400,910"); }
        e5: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=360,800"); }
        // cluster F (middle-left)
        f0: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=400,370"); }
        f1: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=500,370"); }
        f2: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=540,480"); }
        f3: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=500,590"); }
        f4: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=400,590"); }
        f5: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=360,480"); }
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        // ---- cluster A internal (hex ring + 2 diameters = 8 links) ----
        a0.ethg++ <--> <default("EthernetLink")> like ILink <--> a1.ethg++;
        a1.ethg++ <--> <default("EthernetLink")> like ILink <--> a2.ethg++;
        a2.ethg++ <--> <default("EthernetLink")> like ILink <--> a3.ethg++;
        a3.ethg++ <--> <default("EthernetLink")> like ILink <--> a4.ethg++;
        a4.ethg++ <--> <default("EthernetLink")> like ILink <--> a5.ethg++;
        a5.ethg++ <--> <default("EthernetLink")> like ILink <--> a0.ethg++;
        a0.ethg++ <--> <default("EthernetLink")> like ILink <--> a3.ethg++;
        a1.ethg++ <--> <default("EthernetLink")> like ILink <--> a4.ethg++;
        // ---- cluster B internal ----
        b0.ethg++ <--> <default("EthernetLink")> like ILink <--> b1.ethg++;
        b1.ethg++ <--> <default("EthernetLink")> like ILink <--> b2.ethg++;
        b2.ethg++ <--> <default("EthernetLink")> like ILink <--> b3.ethg++;
        b3.ethg++ <--> <default("EthernetLink")> like ILink <--> b4.ethg++;
        b4.ethg++ <--> <default("EthernetLink")> like ILink <--> b5.ethg++;
        b5.ethg++ <--> <default("EthernetLink")> like ILink <--> b0.ethg++;
        b0.ethg++ <--> <default("EthernetLink")> like ILink <--> b3.ethg++;
        b1.ethg++ <--> <default("EthernetLink")> like ILink <--> b4.ethg++;
        // ---- cluster C internal ----
        c0.ethg++ <--> <default("EthernetLink")> like ILink <--> c1.ethg++;
        c1.ethg++ <--> <default("EthernetLink")> like ILink <--> c2.ethg++;
        c2.ethg++ <--> <default("EthernetLink")> like ILink <--> c3.ethg++;
        c3.ethg++ <--> <default("EthernetLink")> like ILink <--> c4.ethg++;
        c4.ethg++ <--> <default("EthernetLink")> like ILink <--> c5.ethg++;
        c5.ethg++ <--> <default("EthernetLink")> like ILink <--> c0.ethg++;
        c0.ethg++ <--> <default("EthernetLink")> like ILink <--> c3.ethg++;
        c2.ethg++ <--> <default("EthernetLink")> like ILink <--> c5.ethg++;
        // ---- cluster D internal ----
        d0.ethg++ <--> <default("EthernetLink")> like ILink <--> d1.ethg++;
        d1.ethg++ <--> <default("EthernetLink")> like ILink <--> d2.ethg++;
        d2.ethg++ <--> <default("EthernetLink")> like ILink <--> d3.ethg++;
        d3.ethg++ <--> <default("EthernetLink")> like ILink <--> d4.ethg++;
        d4.ethg++ <--> <default("EthernetLink")> like ILink <--> d5.ethg++;
        d5.ethg++ <--> <default("EthernetLink")> like ILink <--> d0.ethg++;
        d0.ethg++ <--> <default("EthernetLink")> like ILink <--> d3.ethg++;
        d1.ethg++ <--> <default("EthernetLink")> like ILink <--> d4.ethg++;
        // ---- cluster E internal ----
        e0.ethg++ <--> <default("EthernetLink")> like ILink <--> e1.ethg++;
        e1.ethg++ <--> <default("EthernetLink")> like ILink <--> e2.ethg++;
        e2.ethg++ <--> <default("EthernetLink")> like ILink <--> e3.ethg++;
        e3.ethg++ <--> <default("EthernetLink")> like ILink <--> e4.ethg++;
        e4.ethg++ <--> <default("EthernetLink")> like ILink <--> e5.ethg++;
        e5.ethg++ <--> <default("EthernetLink")> like ILink <--> e0.ethg++;
        e0.ethg++ <--> <default("EthernetLink")> like ILink <--> e3.ethg++;
        e1.ethg++ <--> <default("EthernetLink")> like ILink <--> e4.ethg++;
        // ---- cluster F internal ----
        f0.ethg++ <--> <default("EthernetLink")> like ILink <--> f1.ethg++;
        f1.ethg++ <--> <default("EthernetLink")> like ILink <--> f2.ethg++;
        f2.ethg++ <--> <default("EthernetLink")> like ILink <--> f3.ethg++;
        f3.ethg++ <--> <default("EthernetLink")> like ILink <--> f4.ethg++;
        f4.ethg++ <--> <default("EthernetLink")> like ILink <--> f5.ethg++;
        f5.ethg++ <--> <default("EthernetLink")> like ILink <--> f0.ethg++;
        f0.ethg++ <--> <default("EthernetLink")> like ILink <--> f3.ethg++;
        f2.ethg++ <--> <default("EthernetLink")> like ILink <--> f5.ethg++;
        // ---- inter-cluster bridges (ring + 2 long-range = 8 links) ----
        a2.ethg++ <--> <default("EthernetLink")> like ILink <--> b5.ethg++;  // A ↔ B
        b3.ethg++ <--> <default("EthernetLink")> like ILink <--> c0.ethg++;  // B ↔ C
        c4.ethg++ <--> <default("EthernetLink")> like ILink <--> d1.ethg++;  // C ↔ D
        d5.ethg++ <--> <default("EthernetLink")> like ILink <--> e2.ethg++;  // D ↔ E
        e4.ethg++ <--> <default("EthernetLink")> like ILink <--> f3.ethg++;  // E ↔ F
        f1.ethg++ <--> <default("EthernetLink")> like ILink <--> a4.ethg++;  // F ↔ A
        a3.ethg++ <--> <default("EthernetLink")> like ILink <--> d0.ethg++;  // long-range A ↔ D
        b4.ethg++ <--> <default("EthernetLink")> like ILink <--> e1.ethg++;  // long-range B ↔ E
        // hosts
        for i=0..sizeof(host)-1 {
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> a0.ethg++ if groupIndex(numHosts,i) == 0;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> a1.ethg++ if groupIndex(numHosts,i) == 1;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> a2.ethg++ if groupIndex(numHosts,i) == 2;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> a3.ethg++ if groupIndex(numHosts,i) == 3;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> a4.ethg++ if groupIndex(numHosts,i) == 4;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> a5.ethg++ if groupIndex(numHosts,i) == 5;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b0.ethg++ if groupIndex(numHosts,i) == 6;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b1.ethg++ if groupIndex(numHosts,i) == 7;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b2.ethg++ if groupIndex(numHosts,i) == 8;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b3.ethg++ if groupIndex(numHosts,i) == 9;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b4.ethg++ if groupIndex(numHosts,i) == 10;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b5.ethg++ if groupIndex(numHosts,i) == 11;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> c0.ethg++ if groupIndex(numHosts,i) == 12;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> c1.ethg++ if groupIndex(numHosts,i) == 13;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> c2.ethg++ if groupIndex(numHosts,i) == 14;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> c3.ethg++ if groupIndex(numHosts,i) == 15;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> c4.ethg++ if groupIndex(numHosts,i) == 16;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> c5.ethg++ if groupIndex(numHosts,i) == 17;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> d0.ethg++ if groupIndex(numHosts,i) == 18;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> d1.ethg++ if groupIndex(numHosts,i) == 19;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> d2.ethg++ if groupIndex(numHosts,i) == 20;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> d3.ethg++ if groupIndex(numHosts,i) == 21;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> d4.ethg++ if groupIndex(numHosts,i) == 22;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> d5.ethg++ if groupIndex(numHosts,i) == 23;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> e0.ethg++ if groupIndex(numHosts,i) == 24;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> e1.ethg++ if groupIndex(numHosts,i) == 25;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> e2.ethg++ if groupIndex(numHosts,i) == 26;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> e3.ethg++ if groupIndex(numHosts,i) == 27;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> e4.ethg++ if groupIndex(numHosts,i) == 28;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> e5.ethg++ if groupIndex(numHosts,i) == 29;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> f0.ethg++ if groupIndex(numHosts,i) == 30;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> f1.ethg++ if groupIndex(numHosts,i) == 31;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> f2.ethg++ if groupIndex(numHosts,i) == 32;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> f3.ethg++ if groupIndex(numHosts,i) == 33;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> f4.ethg++ if groupIndex(numHosts,i) == 34;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> f5.ethg++ if groupIndex(numHosts,i) == 35;
        }
}

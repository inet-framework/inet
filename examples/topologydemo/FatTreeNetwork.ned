//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.topologydemo;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.node.ethernet.EthernetLink;

//
// A parameterizable k-ary fat-tree datacenter network topology.
//
// Implements the canonical 3-tier fat-tree topology (Al-Fares et al., 2008),
// fully determined by a single parameter k (must be even):
//
//  - (k/2)^2 core switches
//  - k pods, each containing k/2 aggregation and k/2 edge switches
//  - k/2 hosts per edge switch, for a total of k^3/4 hosts
//
// Connectivity:
//  - Each edge switch connects to all k/2 aggregation switches in its pod
//  - Each aggregation switch connects to k/2 core switches
//  - Every edge switch connects to k/2 hosts
//
// <pre>
//            (k/2)^2 core switches
//     [core0] [core1] ... [core(k/2)^2-1]
//        \       |    ...    |       /
//         '------+----...----|------'       each aggr connects to k/2 cores
//      ,------ pod (x k) -------.
//      [aggr0] ... [aggr k/2-1]             k/2 aggregation switches per pod
//       | \   ...   / |
//      [edge0] ... [edge k/2-1]             k/2 edge switches per pod
//       / \          / \
//     [h] [h]      [h] [h]                  k/2 hosts per edge switch
// </pre>
//
// For k=4: 4 cores, 4 pods x (2 aggr + 2 edge) = 16 switches, 16 hosts.
//
network FatTreeNetwork extends WiredNetworkBase
{
    parameters:
        int k; // number of pods (must be even)
    submodules:
        core[int(k/2)*int(k/2)]: <default("EthernetSwitch")> like IEthernetNetworkNode {
            @display("p=400,100,row,150");
        }
        aggr[k*int(k/2)]: <default("EthernetSwitch")> like IEthernetNetworkNode {
            @display("p=300,300,row,120");
        }
        edge[k*int(k/2)]: <default("EthernetSwitch")> like IEthernetNetworkNode {
            @display("p=300,500,row,120");
        }
        host[k*int(k/2)*int(k/2)]: <default("StandardHost")> like IEthernetNetworkNode {
            @display("p=250,700,row,80");
        }
    connections:
        // Aggregation to core: aggr j in pod p connects to core[j*(k/2) + m]
        for p=0..k-1, for j=0..int(k/2)-1, for m=0..int(k/2)-1 {
            aggr[int(p*int(k/2)+j)].ethg++ <--> EthernetLink <--> core[int(j*int(k/2)+m)].ethg++;
        }
        // Edge to aggregation: full mesh within each pod
        for p=0..k-1, for e=0..int(k/2)-1, for a=0..int(k/2)-1 {
            edge[int(p*int(k/2)+e)].ethg++ <--> EthernetLink <--> aggr[int(p*int(k/2)+a)].ethg++;
        }
        // Hosts to edge switches
        for p=0..k-1, for e=0..int(k/2)-1, for h=0..int(k/2)-1 {
            edge[int(p*int(k/2)+e)].ethg++ <--> EthernetLink <--> host[int(p*int(k/2)*int(k/2)+e*int(k/2)+h)].ethg++;
        }
}

//
// Small fat-tree: k=4 -> 4 core, 8 aggr, 8 edge switches, 16 hosts.
// Suitable for basic testing and tutorials.
//
network SmallFatTreeNetwork extends FatTreeNetwork
{
    parameters:
        k = default(4);
}

//
// Medium fat-tree: k=6 -> 9 core, 18 aggr, 18 edge switches, 54 hosts.
// Represents a mid-size datacenter fabric.
//
network MediumFatTreeNetwork extends FatTreeNetwork
{
    parameters:
        k = default(6);
}

//
// Large fat-tree: k=8 -> 16 core, 32 aggr, 32 edge switches, 128 hosts.
// Represents a large datacenter fabric.
//
network LargeFatTreeNetwork extends FatTreeNetwork
{
    parameters:
        k = default(8);
}

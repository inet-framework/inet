//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.common.ILink;

//
// A parameterizable complete bipartite (client-server) network topology.
//
// Two groups of nodes: every node in group A connects to every node in
// group B (and vice versa), but no connections within the same group.
// Models client-server traffic patterns and datacenter east-west traffic.
//
// <pre>
//   [A0]  [A1]  [A2]
//    |\ \/ /\ \/ /|
//    | \/\/  \/\/ |    (all A-to-B links)
//    | /\/\  /\/\ |
//    |/ /\ \/ /\ \|
//   [B0]  [B1]  [B2]
// </pre>
//
network BipartiteNetwork extends WiredNetworkBase
{
    parameters:
        int numGroupA; // nodes in group A (e.g., clients)
        int numGroupB; // nodes in group B (e.g., servers)
        volatile int numHostsPerSwitch = default(0); // number of hosts connected to each switch
        object numHostsA = default(repeatPar("numHostsPerSwitch", numGroupA));
        object numHostsB = default(repeatPar("numHostsPerSwitch", numGroupB));
    submodules:
        groupA[numGroupA]: <default("EthernetSwitch")> like IEthernetNetworkNode {
            @display("p=300,100,row,120");
        }
        groupB[numGroupB]: <default("EthernetSwitch")> like IEthernetNetworkNode {
            @display("p=300,350,row,120");
        }
        host[sum(numHostsA) + sum(numHostsB)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        // Every A connects to every B
        for i=0..numGroupA-1, for j=0..numGroupB-1 {
            groupA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> groupB[j].ethg++;
        }
        // Hosts on group A
        for i=0..sum(numHostsA)-1 {
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> groupA[groupIndex(numHostsA,i)].ethg++;
        }
        // Hosts on group B
        for i=0..sum(numHostsB)-1 {
            host[sum(numHostsA)+i].ethg++ <--> <default("EthernetLink")> like ILink <--> groupB[groupIndex(numHostsB,i)].ethg++;
        }
}

//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.common.ILink;

//
// A parameterizable ladder (dual-rail) network topology.
//
// Two parallel chains of nodes with cross-links (rungs) at each position.
// Common in industrial fieldbus, HPC dual-rail InfiniBand, and redundancy studies.
//
// <pre>
//   [a0]---[a1]---[a2]--- ... ---[aN]    rail A
//    |       |       |              |
//   [b0]---[b1]---[b2]--- ... ---[bN]    rail B
// </pre>
//
network LadderNetwork extends WiredNetworkBase
{
    parameters:
        int numRungs; // number of rung positions (nodes per rail)
        volatile int numHostsPerSwitch = default(0); // number of hosts connected to each switch
        object numHostsA = default(repeatPar("numHostsPerSwitch", numRungs));
        object numHostsB = default(repeatPar("numHostsPerSwitch", numRungs));
    submodules:
        railA[numRungs]: <default("EthernetSwitch")> like IEthernetNetworkNode {
            @display("p=300,100,row,120");
        }
        railB[numRungs]: <default("EthernetSwitch")> like IEthernetNetworkNode {
            @display("p=300,300,row,120");
        }
        host[sum(numHostsA) + sum(numHostsB)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        // Rail A chain
        for i=0..numRungs-2 {
            railA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> railA[i+1].ethg++;
        }
        // Rail B chain
        for i=0..numRungs-2 {
            railB[i].ethg++ <--> <default("EthernetLink")> like ILink <--> railB[i+1].ethg++;
        }
        // Cross-links (rungs)
        for i=0..numRungs-1 {
            railA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> railB[i].ethg++;
        }
        // Hosts on rail A
        for i=0..sum(numHostsA)-1 {
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> railA[groupIndex(numHostsA,i)].ethg++;
        }
        // Hosts on rail B
        for i=0..sum(numHostsB)-1 {
            host[sum(numHostsA)+i].ethg++ <--> <default("EthernetLink")> like ILink <--> railB[groupIndex(numHostsB,i)].ethg++;
        }
}

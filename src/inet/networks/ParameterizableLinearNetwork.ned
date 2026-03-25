//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.common.ILink;

//
// A parameterizable linear (chain) network topology.
//
// Nodes are connected in a simple line, end to end.
// Commonly used for multi-hop delay studies and serial forwarding analysis.
//
// <pre>
//   [0]---[1]---[2]--- ... ---[n-1]
// </pre>
//
network LinearNetwork extends WiredNetworkBase
{
    parameters:
        int numNodes; // number of nodes in the chain
        volatile int numHostsPerSwitch = default(0); // number of hosts connected to each switch
        object numHosts = default(repeatPar("numHostsPerSwitch", numNodes));
    submodules:
        node[numNodes]: <default("EthernetSwitch")> like IEthernetNetworkNode {
            @display("p=200,200,row,120");
        }
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        for i=0..numNodes-2 {
            node[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node[i+1].ethg++;
        }
        for i=0..sizeof(host)-1 {
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node[groupIndex(numHosts,i)].ethg++;
        }
}

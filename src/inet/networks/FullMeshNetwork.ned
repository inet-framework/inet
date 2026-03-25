//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.common.ILink;

//
// A parameterizable full-mesh (complete graph) network topology.
//
// Every node has a direct link to every other node.
// Commonly used in backbone/WAN core networks and small switch fabrics.
//
// <pre>
//   [0]----[1]
//   |\ \  / /|
//   | \ \/ / |
//   |  \/\/  |
//   |  /\/\  |
//   | / /\ \ |
//   |/ /  \ \|
//   [3]----[2]
// </pre>
//
network FullMeshNetwork extends WiredNetworkBase
{
    parameters:
        int numNodes; // number of nodes (total links = numNodes*(numNodes-1)/2)
        volatile int numHostsPerSwitch = default(0); // number of hosts connected to each switch
        object numHosts = default(repeatPar("numHostsPerSwitch", numNodes));
    submodules:
        node[numNodes]: <default("EthernetSwitch")> like IEthernetNetworkNode {
            @display("p=200,100,ring,200");
        }
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        for i=0..numNodes-2, for j=i+1..numNodes-1 {
            node[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node[j].ethg++;
        }
        for i=0..sizeof(host)-1 {
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node[groupIndex(numHosts,i)].ethg++;
        }
}

//
// Small full mesh: 10 nodes (45 links).
//
network SmallFullMeshNetwork extends FullMeshNetwork
{
    parameters:
        numNodes = default(10);
}

//
// Medium full mesh: 100 nodes (4950 links).
//
network MediumFullMeshNetwork extends FullMeshNetwork
{
    parameters:
        numNodes = default(100);
}

//
// Large full mesh: 1000 nodes (499500 links).
//
network LargeFullMeshNetwork extends FullMeshNetwork
{
    parameters:
        numNodes = default(1000);
}

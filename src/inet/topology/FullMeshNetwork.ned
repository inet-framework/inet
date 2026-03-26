//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.topology;

import inet.common.ILink;
import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;

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
    submodules:
        node[numNodes]: <default("EthernetSwitch")> like IEthernetNetworkNode {
            @display("p=300,300,ring,200");
        }
    connections:
        for i=0..numNodes-2, for j=i+1..numNodes-1 {
            node[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node[j].ethg++;
        }
}

//
// Small full mesh: 4 nodes (6 links).
//
network SmallFullMeshNetwork extends FullMeshNetwork
{
    parameters:
        numNodes = default(4);
}

//
// Medium full mesh: 6 nodes (15 links).
//
network MediumFullMeshNetwork extends FullMeshNetwork
{
    parameters:
        numNodes = default(6);
}

//
// Large full mesh: 8 nodes (28 links).
//
network LargeFullMeshNetwork extends FullMeshNetwork
{
    parameters:
        numNodes = default(8);
}

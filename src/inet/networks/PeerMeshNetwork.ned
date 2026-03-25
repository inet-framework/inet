//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.common.ILink;

//
// Flat irregular peer mesh (15 nodes, ~24 links).
//
// A mostly non-hierarchical network with uneven local density.
// No clear backbone or root. Some local hot spots exist with
// slightly higher degree, but no dominant hubs. Good for
// distributed protocols, gossip/epidemic dissemination, and
// routing without strong hierarchy.
//
network PeerMeshNetwork extends WiredNetworkBase
{
    parameters:
        volatile int numHostsPerSwitch = default(0);
        object numHosts = default(repeatPar("numHostsPerSwitch", 15));
    submodules:
        node0:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        node1:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        node2:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        node3:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        node4:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        node5:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        node6:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        node7:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        node8:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        node9:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        node10: <default("EthernetSwitch")> like IEthernetNetworkNode;
        node11: <default("EthernetSwitch")> like IEthernetNetworkNode;
        node12: <default("EthernetSwitch")> like IEthernetNetworkNode;
        node13: <default("EthernetSwitch")> like IEthernetNetworkNode;
        node14: <default("EthernetSwitch")> like IEthernetNetworkNode;
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {}
    connections:
        // Dense local cluster A (nodes 0-4)
        node0.ethg++ <--> <default("EthernetLink")> like ILink <--> node1.ethg++;
        node0.ethg++ <--> <default("EthernetLink")> like ILink <--> node2.ethg++;
        node1.ethg++ <--> <default("EthernetLink")> like ILink <--> node3.ethg++;
        node2.ethg++ <--> <default("EthernetLink")> like ILink <--> node3.ethg++;
        node2.ethg++ <--> <default("EthernetLink")> like ILink <--> node4.ethg++;
        node3.ethg++ <--> <default("EthernetLink")> like ILink <--> node4.ethg++;
        // Sparse bridge region (nodes 5-7)
        node1.ethg++ <--> <default("EthernetLink")> like ILink <--> node5.ethg++;
        node4.ethg++ <--> <default("EthernetLink")> like ILink <--> node6.ethg++;
        node5.ethg++ <--> <default("EthernetLink")> like ILink <--> node7.ethg++;
        node5.ethg++ <--> <default("EthernetLink")> like ILink <--> node6.ethg++;
        node6.ethg++ <--> <default("EthernetLink")> like ILink <--> node7.ethg++;
        // Moderate cluster B (nodes 8-11)
        node7.ethg++ <--> <default("EthernetLink")> like ILink <--> node8.ethg++;
        node7.ethg++ <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node8.ethg++ <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node8.ethg++ <--> <default("EthernetLink")> like ILink <--> node10.ethg++;
        node9.ethg++ <--> <default("EthernetLink")> like ILink <--> node11.ethg++;
        node10.ethg++ <--> <default("EthernetLink")> like ILink <--> node11.ethg++;
        // Outliers (nodes 12-14) with sparse connections
        node11.ethg++ <--> <default("EthernetLink")> like ILink <--> node12.ethg++;
        node10.ethg++ <--> <default("EthernetLink")> like ILink <--> node13.ethg++;
        node12.ethg++ <--> <default("EthernetLink")> like ILink <--> node14.ethg++;
        node13.ethg++ <--> <default("EthernetLink")> like ILink <--> node14.ethg++;
        // Cross-links for moderate path diversity
        node0.ethg++ <--> <default("EthernetLink")> like ILink <--> node5.ethg++;
        node3.ethg++ <--> <default("EthernetLink")> like ILink <--> node7.ethg++;
        node6.ethg++ <--> <default("EthernetLink")> like ILink <--> node10.ethg++;
        // hosts
        for i=0..sizeof(host)-1 {
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node0.ethg++  if groupIndex(numHosts,i) == 0;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node1.ethg++  if groupIndex(numHosts,i) == 1;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node2.ethg++  if groupIndex(numHosts,i) == 2;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node3.ethg++  if groupIndex(numHosts,i) == 3;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node4.ethg++  if groupIndex(numHosts,i) == 4;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node5.ethg++  if groupIndex(numHosts,i) == 5;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node6.ethg++  if groupIndex(numHosts,i) == 6;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node7.ethg++  if groupIndex(numHosts,i) == 7;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node8.ethg++  if groupIndex(numHosts,i) == 8;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node9.ethg++  if groupIndex(numHosts,i) == 9;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node10.ethg++ if groupIndex(numHosts,i) == 10;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node11.ethg++ if groupIndex(numHosts,i) == 11;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node12.ethg++ if groupIndex(numHosts,i) == 12;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node13.ethg++ if groupIndex(numHosts,i) == 13;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node14.ethg++ if groupIndex(numHosts,i) == 14;
        }
}

//
// Large flat irregular peer mesh (30 nodes, ~50 links).
//
// Doubled version of PeerMeshNetwork. No hierarchy, uneven local
// density, multiple overlapping clusters connected by sparse bridges.
// Degree ranges from 2 to 5. Good for distributed protocols,
// gossip dissemination, and resilience without centralized control.
//
network LargePeerMeshNetwork extends WiredNetworkBase
{
    parameters:
        volatile int numHostsPerSwitch = default(0);
        object numHosts = default(repeatPar("numHostsPerSwitch", 30));
    submodules:
        node0:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        node1:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        node2:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        node3:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        node4:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        node5:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        node6:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        node7:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        node8:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        node9:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        node10: <default("EthernetSwitch")> like IEthernetNetworkNode;
        node11: <default("EthernetSwitch")> like IEthernetNetworkNode;
        node12: <default("EthernetSwitch")> like IEthernetNetworkNode;
        node13: <default("EthernetSwitch")> like IEthernetNetworkNode;
        node14: <default("EthernetSwitch")> like IEthernetNetworkNode;
        node15: <default("EthernetSwitch")> like IEthernetNetworkNode;
        node16: <default("EthernetSwitch")> like IEthernetNetworkNode;
        node17: <default("EthernetSwitch")> like IEthernetNetworkNode;
        node18: <default("EthernetSwitch")> like IEthernetNetworkNode;
        node19: <default("EthernetSwitch")> like IEthernetNetworkNode;
        node20: <default("EthernetSwitch")> like IEthernetNetworkNode;
        node21: <default("EthernetSwitch")> like IEthernetNetworkNode;
        node22: <default("EthernetSwitch")> like IEthernetNetworkNode;
        node23: <default("EthernetSwitch")> like IEthernetNetworkNode;
        node24: <default("EthernetSwitch")> like IEthernetNetworkNode;
        node25: <default("EthernetSwitch")> like IEthernetNetworkNode;
        node26: <default("EthernetSwitch")> like IEthernetNetworkNode;
        node27: <default("EthernetSwitch")> like IEthernetNetworkNode;
        node28: <default("EthernetSwitch")> like IEthernetNetworkNode;
        node29: <default("EthernetSwitch")> like IEthernetNetworkNode;
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {}
    connections:
        // Cluster A (nodes 0-5, dense)
        node0.ethg++ <--> <default("EthernetLink")> like ILink <--> node1.ethg++;
        node0.ethg++ <--> <default("EthernetLink")> like ILink <--> node2.ethg++;
        node1.ethg++ <--> <default("EthernetLink")> like ILink <--> node3.ethg++;
        node2.ethg++ <--> <default("EthernetLink")> like ILink <--> node3.ethg++;
        node2.ethg++ <--> <default("EthernetLink")> like ILink <--> node4.ethg++;
        node3.ethg++ <--> <default("EthernetLink")> like ILink <--> node5.ethg++;
        node4.ethg++ <--> <default("EthernetLink")> like ILink <--> node5.ethg++;
        node0.ethg++ <--> <default("EthernetLink")> like ILink <--> node4.ethg++;
        // Sparse bridge A→B
        node1.ethg++ <--> <default("EthernetLink")> like ILink <--> node6.ethg++;
        node5.ethg++ <--> <default("EthernetLink")> like ILink <--> node8.ethg++;
        // Cluster B (nodes 6-10, moderate)
        node6.ethg++ <--> <default("EthernetLink")> like ILink <--> node7.ethg++;
        node6.ethg++ <--> <default("EthernetLink")> like ILink <--> node8.ethg++;
        node7.ethg++ <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node8.ethg++ <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node8.ethg++ <--> <default("EthernetLink")> like ILink <--> node10.ethg++;
        node9.ethg++ <--> <default("EthernetLink")> like ILink <--> node10.ethg++;
        // Bridge B→C
        node7.ethg++  <--> <default("EthernetLink")> like ILink <--> node11.ethg++;
        node10.ethg++ <--> <default("EthernetLink")> like ILink <--> node14.ethg++;
        // Cluster C (nodes 11-15, irregular)
        node11.ethg++ <--> <default("EthernetLink")> like ILink <--> node12.ethg++;
        node11.ethg++ <--> <default("EthernetLink")> like ILink <--> node13.ethg++;
        node12.ethg++ <--> <default("EthernetLink")> like ILink <--> node14.ethg++;
        node13.ethg++ <--> <default("EthernetLink")> like ILink <--> node15.ethg++;
        node14.ethg++ <--> <default("EthernetLink")> like ILink <--> node15.ethg++;
        node12.ethg++ <--> <default("EthernetLink")> like ILink <--> node15.ethg++;
        // Bridge C→D
        node13.ethg++ <--> <default("EthernetLink")> like ILink <--> node16.ethg++;
        node15.ethg++ <--> <default("EthernetLink")> like ILink <--> node18.ethg++;
        // Cluster D (nodes 16-20, dense)
        node16.ethg++ <--> <default("EthernetLink")> like ILink <--> node17.ethg++;
        node16.ethg++ <--> <default("EthernetLink")> like ILink <--> node18.ethg++;
        node17.ethg++ <--> <default("EthernetLink")> like ILink <--> node19.ethg++;
        node18.ethg++ <--> <default("EthernetLink")> like ILink <--> node19.ethg++;
        node18.ethg++ <--> <default("EthernetLink")> like ILink <--> node20.ethg++;
        node19.ethg++ <--> <default("EthernetLink")> like ILink <--> node20.ethg++;
        node17.ethg++ <--> <default("EthernetLink")> like ILink <--> node20.ethg++;
        // Bridge D→E
        node20.ethg++ <--> <default("EthernetLink")> like ILink <--> node21.ethg++;
        node19.ethg++ <--> <default("EthernetLink")> like ILink <--> node23.ethg++;
        // Cluster E (nodes 21-25, moderate)
        node21.ethg++ <--> <default("EthernetLink")> like ILink <--> node22.ethg++;
        node21.ethg++ <--> <default("EthernetLink")> like ILink <--> node23.ethg++;
        node22.ethg++ <--> <default("EthernetLink")> like ILink <--> node24.ethg++;
        node23.ethg++ <--> <default("EthernetLink")> like ILink <--> node24.ethg++;
        node24.ethg++ <--> <default("EthernetLink")> like ILink <--> node25.ethg++;
        node22.ethg++ <--> <default("EthernetLink")> like ILink <--> node25.ethg++;
        // Outliers (nodes 26-29)
        node25.ethg++ <--> <default("EthernetLink")> like ILink <--> node26.ethg++;
        node26.ethg++ <--> <default("EthernetLink")> like ILink <--> node27.ethg++;
        node24.ethg++ <--> <default("EthernetLink")> like ILink <--> node28.ethg++;
        node27.ethg++ <--> <default("EthernetLink")> like ILink <--> node29.ethg++;
        node28.ethg++ <--> <default("EthernetLink")> like ILink <--> node29.ethg++;
        // Cross-links for path diversity (no hierarchy)
        node3.ethg++  <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node4.ethg++  <--> <default("EthernetLink")> like ILink <--> node14.ethg++;
        node10.ethg++ <--> <default("EthernetLink")> like ILink <--> node16.ethg++;
        node12.ethg++ <--> <default("EthernetLink")> like ILink <--> node20.ethg++;
        node26.ethg++ <--> <default("EthernetLink")> like ILink <--> node0.ethg++;
        // hosts
        for i=0..sizeof(host)-1 {
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node0.ethg++  if groupIndex(numHosts,i) == 0;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node1.ethg++  if groupIndex(numHosts,i) == 1;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node2.ethg++  if groupIndex(numHosts,i) == 2;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node3.ethg++  if groupIndex(numHosts,i) == 3;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node4.ethg++  if groupIndex(numHosts,i) == 4;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node5.ethg++  if groupIndex(numHosts,i) == 5;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node6.ethg++  if groupIndex(numHosts,i) == 6;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node7.ethg++  if groupIndex(numHosts,i) == 7;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node8.ethg++  if groupIndex(numHosts,i) == 8;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node9.ethg++  if groupIndex(numHosts,i) == 9;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node10.ethg++ if groupIndex(numHosts,i) == 10;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node11.ethg++ if groupIndex(numHosts,i) == 11;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node12.ethg++ if groupIndex(numHosts,i) == 12;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node13.ethg++ if groupIndex(numHosts,i) == 13;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node14.ethg++ if groupIndex(numHosts,i) == 14;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node15.ethg++ if groupIndex(numHosts,i) == 15;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node16.ethg++ if groupIndex(numHosts,i) == 16;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node17.ethg++ if groupIndex(numHosts,i) == 17;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node18.ethg++ if groupIndex(numHosts,i) == 18;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node19.ethg++ if groupIndex(numHosts,i) == 19;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node20.ethg++ if groupIndex(numHosts,i) == 20;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node21.ethg++ if groupIndex(numHosts,i) == 21;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node22.ethg++ if groupIndex(numHosts,i) == 22;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node23.ethg++ if groupIndex(numHosts,i) == 23;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node24.ethg++ if groupIndex(numHosts,i) == 24;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node25.ethg++ if groupIndex(numHosts,i) == 25;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node26.ethg++ if groupIndex(numHosts,i) == 26;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node27.ethg++ if groupIndex(numHosts,i) == 27;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node28.ethg++ if groupIndex(numHosts,i) == 28;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node29.ethg++ if groupIndex(numHosts,i) == 29;
        }
}

//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.common.ILink;

//
// Weakly connected, regional redundancy – 15 nodes, 20 links.
//
// Three clusters, each internally meshed so that any single
// intra-cluster failure is survivable. The clusters are joined
// by lone bridge links, making the inter-cluster connections
// fragile single points of failure.
//
// <pre>
//   Cluster A             Cluster B             Cluster C
//
//  [0]---[1]              [6]---[7]            [11]--[12]
//   |\ /  |               |\ /  |              |\ /  |
//  [2]-[3]               [8]-[9]              [13]-[14]
//       |                 |
//      [4]----[5]--------[10]
// </pre>
//
network WcRegionalNetwork extends WiredNetworkBase
{
    parameters:
        volatile int numHostsPerSwitch = default(0); // hosts attached to every switch
        object numHosts = default(repeatPar("numHostsPerSwitch", 15));
    submodules:
        node0:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=250,80"); }
        node1:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=390,80"); }
        node2:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=250,210"); }
        node3:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=390,210"); }
        node4:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=390,340"); }
        node5:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=550,340"); }
        node6:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=710,80"); }
        node7:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=850,80"); }
        node8:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=710,210"); }
        node9:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=850,210"); }
        node10: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=710,340"); }
        node11: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=970,80"); }
        node12: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1110,80"); }
        node13: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=970,210"); }
        node14: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1110,210"); }
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        // Cluster A – internal mesh (5 nodes, 6 links)
        node0.ethg++ <--> <default("EthernetLink")> like ILink <--> node1.ethg++;
        node0.ethg++ <--> <default("EthernetLink")> like ILink <--> node2.ethg++;
        node0.ethg++ <--> <default("EthernetLink")> like ILink <--> node3.ethg++;
        node1.ethg++ <--> <default("EthernetLink")> like ILink <--> node3.ethg++;
        node2.ethg++ <--> <default("EthernetLink")> like ILink <--> node3.ethg++;
        node3.ethg++ <--> <default("EthernetLink")> like ILink <--> node4.ethg++;
        // Bridge A → B (single link)
        node4.ethg++ <--> <default("EthernetLink")> like ILink <--> node5.ethg++;
        node5.ethg++ <--> <default("EthernetLink")> like ILink <--> node10.ethg++;
        // Cluster B – internal mesh (5 nodes, 6 links)
        node6.ethg++  <--> <default("EthernetLink")> like ILink <--> node7.ethg++;
        node6.ethg++  <--> <default("EthernetLink")> like ILink <--> node8.ethg++;
        node6.ethg++  <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node7.ethg++  <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node8.ethg++  <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node8.ethg++  <--> <default("EthernetLink")> like ILink <--> node10.ethg++;
        // Bridge B → C (single link)
        node9.ethg++ <--> <default("EthernetLink")> like ILink <--> node11.ethg++;
        // Cluster C – internal mesh (4 nodes, 5 links)
        node11.ethg++ <--> <default("EthernetLink")> like ILink <--> node12.ethg++;
        node11.ethg++ <--> <default("EthernetLink")> like ILink <--> node13.ethg++;
        node12.ethg++ <--> <default("EthernetLink")> like ILink <--> node14.ethg++;
        node13.ethg++ <--> <default("EthernetLink")> like ILink <--> node14.ethg++;
        node11.ethg++ <--> <default("EthernetLink")> like ILink <--> node14.ethg++;
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
// Weakly connected, regional redundancy (large) – 30 nodes, 40 links.
//
// Six internally meshed clusters joined by lone bridge links.
// Each cluster survives a single intra-cluster failure, but
// the inter-cluster connections remain fragile single points
// of failure. A scaled-up version of WcRegionalNetwork.
//
// <pre>
//   Cluster A             Cluster B             Cluster C
//  [0]---[1]              [6]---[7]            [11]--[12]
//   |\ /  |               |\ /  |              |\ /  |
//  [2]-[3]               [8]-[9]              [13]-[14]
//       |                 |
//      [4]----[5]--------[10]
//
//   Cluster D             Cluster E             Cluster F
// [15]--[16]             [21]--[22]            [26]--[27]
//   |\ /  |               |\ /  |              |\ /  |
// [17]-[18]             [23]-[24]             [28]-[29]
//       |
//     [19]---[20]-------[25]
// </pre>
//
// Bridge: node9--node15, node14--node21
//
network LargeWcRegionalNetwork extends WiredNetworkBase
{
    parameters:
        volatile int numHostsPerSwitch = default(0);
        object numHosts = default(repeatPar("numHostsPerSwitch", 30));
    submodules:
        // Cluster A
        node0:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=300,80"); }
        node1:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=440,80"); }
        node2:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=300,210"); }
        node3:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=440,210"); }
        node4:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=440,340"); }
        node5:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=600,340"); }
        // Cluster B
        node6:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=760,80"); }
        node7:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=900,80"); }
        node8:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=760,210"); }
        node9:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=900,210"); }
        node10: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=760,340"); }
        // Cluster C
        node11: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1020,80"); }
        node12: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1160,80"); }
        node13: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1020,210"); }
        node14: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1160,210"); }
        // Cluster D
        node15: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=300,500"); }
        node16: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=440,500"); }
        node17: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=300,630"); }
        node18: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=440,630"); }
        node19: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=440,760"); }
        node20: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=600,760"); }
        // Cluster E
        node21: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=760,500"); }
        node22: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=900,500"); }
        node23: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=760,630"); }
        node24: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=900,630"); }
        node25: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=760,760"); }
        // Cluster F
        node26: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1020,500"); }
        node27: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1160,500"); }
        node28: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1020,630"); }
        node29: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1160,630"); }
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        // Cluster A – internal mesh (5 nodes, 6 links)
        node0.ethg++ <--> <default("EthernetLink")> like ILink <--> node1.ethg++;
        node0.ethg++ <--> <default("EthernetLink")> like ILink <--> node2.ethg++;
        node0.ethg++ <--> <default("EthernetLink")> like ILink <--> node3.ethg++;
        node1.ethg++ <--> <default("EthernetLink")> like ILink <--> node3.ethg++;
        node2.ethg++ <--> <default("EthernetLink")> like ILink <--> node3.ethg++;
        node3.ethg++ <--> <default("EthernetLink")> like ILink <--> node4.ethg++;
        // Bridge A → B
        node4.ethg++ <--> <default("EthernetLink")> like ILink <--> node5.ethg++;
        node5.ethg++ <--> <default("EthernetLink")> like ILink <--> node10.ethg++;
        // Cluster B – internal mesh (5 nodes, 6 links)
        node6.ethg++  <--> <default("EthernetLink")> like ILink <--> node7.ethg++;
        node6.ethg++  <--> <default("EthernetLink")> like ILink <--> node8.ethg++;
        node6.ethg++  <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node7.ethg++  <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node8.ethg++  <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node8.ethg++  <--> <default("EthernetLink")> like ILink <--> node10.ethg++;
        // Bridge B → C
        node9.ethg++ <--> <default("EthernetLink")> like ILink <--> node11.ethg++;
        // Cluster C – internal mesh (4 nodes, 5 links)
        node11.ethg++ <--> <default("EthernetLink")> like ILink <--> node12.ethg++;
        node11.ethg++ <--> <default("EthernetLink")> like ILink <--> node13.ethg++;
        node12.ethg++ <--> <default("EthernetLink")> like ILink <--> node14.ethg++;
        node13.ethg++ <--> <default("EthernetLink")> like ILink <--> node14.ethg++;
        node11.ethg++ <--> <default("EthernetLink")> like ILink <--> node14.ethg++;
        // Bridge C → D (single link)
        node9.ethg++  <--> <default("EthernetLink")> like ILink <--> node15.ethg++;
        // Cluster D – internal mesh (5 nodes, 6 links)
        node15.ethg++ <--> <default("EthernetLink")> like ILink <--> node16.ethg++;
        node15.ethg++ <--> <default("EthernetLink")> like ILink <--> node17.ethg++;
        node15.ethg++ <--> <default("EthernetLink")> like ILink <--> node18.ethg++;
        node16.ethg++ <--> <default("EthernetLink")> like ILink <--> node18.ethg++;
        node17.ethg++ <--> <default("EthernetLink")> like ILink <--> node18.ethg++;
        node18.ethg++ <--> <default("EthernetLink")> like ILink <--> node19.ethg++;
        // Bridge D → E
        node19.ethg++ <--> <default("EthernetLink")> like ILink <--> node20.ethg++;
        node20.ethg++ <--> <default("EthernetLink")> like ILink <--> node25.ethg++;
        // Cluster E – internal mesh (5 nodes, 6 links)
        node21.ethg++ <--> <default("EthernetLink")> like ILink <--> node22.ethg++;
        node21.ethg++ <--> <default("EthernetLink")> like ILink <--> node23.ethg++;
        node21.ethg++ <--> <default("EthernetLink")> like ILink <--> node24.ethg++;
        node22.ethg++ <--> <default("EthernetLink")> like ILink <--> node24.ethg++;
        node23.ethg++ <--> <default("EthernetLink")> like ILink <--> node24.ethg++;
        node23.ethg++ <--> <default("EthernetLink")> like ILink <--> node25.ethg++;
        // Bridge E → F
        node24.ethg++ <--> <default("EthernetLink")> like ILink <--> node26.ethg++;
        // Cluster F – internal mesh (4 nodes, 5 links)
        node26.ethg++ <--> <default("EthernetLink")> like ILink <--> node27.ethg++;
        node26.ethg++ <--> <default("EthernetLink")> like ILink <--> node28.ethg++;
        node27.ethg++ <--> <default("EthernetLink")> like ILink <--> node29.ethg++;
        node28.ethg++ <--> <default("EthernetLink")> like ILink <--> node29.ethg++;
        node26.ethg++ <--> <default("EthernetLink")> like ILink <--> node29.ethg++;
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

//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.common.ILink;

//
// Weakly connected, no redundancy – 15 nodes, 14 links.
//
// Three small clusters joined by single bridge links.
// Every bridge node is an articulation point; losing any one
// of them partitions the network.
//
// <pre>
//   Cluster A          Cluster B            Cluster C
//
//  [0]---[1]            [6]                [11]--[12]
//   |     |              |                  |      |
//  [2]   [3]--[5]------[7]---[8]--[10]---[13]   [14]
//         |              |    |
//        [4]            [9]
// </pre>
//
network WcNoRedundancyNetwork extends WiredNetworkBase
{
    parameters:
        volatile int numHostsPerSwitch = default(0); // hosts attached to every switch
        object numHosts = default(repeatPar("numHostsPerSwitch", 15));
    submodules:
        node0:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=250,80"); }
        node1:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=370,80"); }
        node2:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=250,200"); }
        node3:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=370,200"); }
        node4:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=370,320"); }
        node5:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=510,200"); }
        node6:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=660,80"); }
        node7:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=660,200"); }
        node8:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=780,200"); }
        node9:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=660,320"); }
        node10: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=900,200"); }
        node11: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1030,80"); }
        node12: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1160,80"); }
        node13: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1030,200"); }
        node14: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1160,200"); }
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        // Cluster A (tree)
        node0.ethg++ <--> <default("EthernetLink")> like ILink <--> node1.ethg++;
        node0.ethg++ <--> <default("EthernetLink")> like ILink <--> node2.ethg++;
        node1.ethg++ <--> <default("EthernetLink")> like ILink <--> node3.ethg++;
        node3.ethg++ <--> <default("EthernetLink")> like ILink <--> node4.ethg++;
        // Bridge A-B
        node3.ethg++ <--> <default("EthernetLink")> like ILink <--> node5.ethg++;
        node5.ethg++ <--> <default("EthernetLink")> like ILink <--> node7.ethg++;
        // Cluster B (tree)
        node6.ethg++ <--> <default("EthernetLink")> like ILink <--> node7.ethg++;
        node7.ethg++ <--> <default("EthernetLink")> like ILink <--> node8.ethg++;
        node7.ethg++ <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node8.ethg++ <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        // Bridge B-C
        node8.ethg++ <--> <default("EthernetLink")> like ILink <--> node10.ethg++;
        node10.ethg++ <--> <default("EthernetLink")> like ILink <--> node13.ethg++;
        // Cluster C (tree)
        node11.ethg++ <--> <default("EthernetLink")> like ILink <--> node13.ethg++;
        node12.ethg++ <--> <default("EthernetLink")> like ILink <--> node14.ethg++;
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
// Weakly connected, no redundancy (large) – 30 nodes, 29 links.
//
// Six small clusters joined by single bridge links. Every bridge
// node is an articulation point; losing any one partitions the
// network. A scaled-up version of WcNoRedundancyNetwork.
//
// <pre>
//   Cluster A        Cluster B          Cluster C
//  [0]---[1]          [6]              [11]--[12]
//   |     |            |                |      |
//  [2]   [3]--[5]----[7]---[8]--[10]--[13]   [14]
//         |            |    |
//        [4]          [9]
//
//   Cluster D        Cluster E          Cluster F
// [15]--[16]         [21]              [26]--[27]
//   |     |            |                |      |
// [17]  [18]-[20]---[22]--[23]-[25]--[28]   [29]
//         |            |    |
//       [19]          [24]
//
//   Bridge: node14--node15
// </pre>
//
network LargeWcNoRedundancyNetwork extends WiredNetworkBase
{
    parameters:
        volatile int numHostsPerSwitch = default(0);
        object numHosts = default(repeatPar("numHostsPerSwitch", 30));
    submodules:
        // Cluster A
        node0:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=300,80"); }
        node1:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=420,80"); }
        node2:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=300,200"); }
        node3:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=420,200"); }
        node4:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=420,320"); }
        node5:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=560,200"); }
        // Cluster B
        node6:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=710,80"); }
        node7:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=710,200"); }
        node8:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=830,200"); }
        node9:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=710,320"); }
        node10: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=950,200"); }
        // Cluster C
        node11: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1080,80"); }
        node12: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1210,80"); }
        node13: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1080,200"); }
        node14: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1210,200"); }
        // Cluster D
        node15: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=300,480"); }
        node16: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=420,480"); }
        node17: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=300,600"); }
        node18: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=420,600"); }
        node19: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=420,720"); }
        node20: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=560,600"); }
        // Cluster E
        node21: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=710,480"); }
        node22: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=710,600"); }
        node23: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=830,600"); }
        node24: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=710,720"); }
        node25: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=950,600"); }
        // Cluster F
        node26: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1080,480"); }
        node27: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1210,480"); }
        node28: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1080,600"); }
        node29: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1210,600"); }
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        // Cluster A (tree)
        node0.ethg++ <--> <default("EthernetLink")> like ILink <--> node1.ethg++;
        node0.ethg++ <--> <default("EthernetLink")> like ILink <--> node2.ethg++;
        node1.ethg++ <--> <default("EthernetLink")> like ILink <--> node3.ethg++;
        node3.ethg++ <--> <default("EthernetLink")> like ILink <--> node4.ethg++;
        // Bridge A-B
        node3.ethg++ <--> <default("EthernetLink")> like ILink <--> node5.ethg++;
        node5.ethg++ <--> <default("EthernetLink")> like ILink <--> node7.ethg++;
        // Cluster B (tree)
        node6.ethg++ <--> <default("EthernetLink")> like ILink <--> node7.ethg++;
        node7.ethg++ <--> <default("EthernetLink")> like ILink <--> node8.ethg++;
        node7.ethg++ <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node8.ethg++ <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        // Bridge B-C
        node8.ethg++  <--> <default("EthernetLink")> like ILink <--> node10.ethg++;
        node10.ethg++ <--> <default("EthernetLink")> like ILink <--> node13.ethg++;
        // Cluster C (tree)
        node11.ethg++ <--> <default("EthernetLink")> like ILink <--> node13.ethg++;
        node12.ethg++ <--> <default("EthernetLink")> like ILink <--> node14.ethg++;
        // Bridge C-D (single link between upper and lower halves)
        node14.ethg++ <--> <default("EthernetLink")> like ILink <--> node15.ethg++;
        // Cluster D (tree)
        node15.ethg++ <--> <default("EthernetLink")> like ILink <--> node16.ethg++;
        node15.ethg++ <--> <default("EthernetLink")> like ILink <--> node17.ethg++;
        node16.ethg++ <--> <default("EthernetLink")> like ILink <--> node18.ethg++;
        node18.ethg++ <--> <default("EthernetLink")> like ILink <--> node19.ethg++;
        // Bridge D-E
        node18.ethg++ <--> <default("EthernetLink")> like ILink <--> node20.ethg++;
        node20.ethg++ <--> <default("EthernetLink")> like ILink <--> node22.ethg++;
        // Cluster E (tree)
        node21.ethg++ <--> <default("EthernetLink")> like ILink <--> node22.ethg++;
        node22.ethg++ <--> <default("EthernetLink")> like ILink <--> node23.ethg++;
        node22.ethg++ <--> <default("EthernetLink")> like ILink <--> node24.ethg++;
        node23.ethg++ <--> <default("EthernetLink")> like ILink <--> node24.ethg++;
        // Bridge E-F
        node23.ethg++ <--> <default("EthernetLink")> like ILink <--> node25.ethg++;
        node25.ethg++ <--> <default("EthernetLink")> like ILink <--> node28.ethg++;
        // Cluster F (tree)
        node26.ethg++ <--> <default("EthernetLink")> like ILink <--> node28.ethg++;
        node27.ethg++ <--> <default("EthernetLink")> like ILink <--> node29.ethg++;
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

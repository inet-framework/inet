//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.common.ILink;

//
// Highly connected, end-to-end redundancy – 14 nodes, 30 links.
//
// A dense irregular mesh where every node has degree 3-5.
// Multiple node-disjoint paths exist between any pair of nodes,
// so several simultaneous failures can be tolerated without
// partitioning the network. No single cluster dominates;
// redundancy is distributed across the whole topology.
//
// <pre>
//        [0]------[1]------[2]
//       / | \    / | \    / |
//     [3] |  [4]   |  [5]  |
//      |\ | / | \  | / |\  |
//     [6]-[7]  [8]-[9]  [10]-[11]
//       \  |  / \  |  /   |
//        [12]----[13]
// </pre>
//
network HcEndToEndNetwork extends WiredNetworkBase
{
    parameters:
        volatile int numHostsPerSwitch = default(0); // hosts attached to every switch
        object numHosts = default(repeatPar("numHostsPerSwitch", 14));
    submodules:
        node0:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=300,50"); }
        node1:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=570,50"); }
        node2:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=840,50"); }
        node3:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=250,200"); }
        node4:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=440,200"); }
        node5:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=700,200"); }
        node6:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=250,370"); }
        node7:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=390,370"); }
        node8:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=550,370"); }
        node9:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=700,370"); }
        node10: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=840,200"); }
        node11: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=940,370"); }
        node12: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=440,520"); }
        node13: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=700,520"); }
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        // Top row connections
        node0.ethg++  <--> <default("EthernetLink")> like ILink <--> node1.ethg++;
        node1.ethg++  <--> <default("EthernetLink")> like ILink <--> node2.ethg++;
        // Top → middle
        node0.ethg++  <--> <default("EthernetLink")> like ILink <--> node3.ethg++;
        node0.ethg++  <--> <default("EthernetLink")> like ILink <--> node4.ethg++;
        node1.ethg++  <--> <default("EthernetLink")> like ILink <--> node4.ethg++;
        node1.ethg++  <--> <default("EthernetLink")> like ILink <--> node5.ethg++;
        node2.ethg++  <--> <default("EthernetLink")> like ILink <--> node5.ethg++;
        node2.ethg++  <--> <default("EthernetLink")> like ILink <--> node10.ethg++;
        // Middle row connections
        node3.ethg++  <--> <default("EthernetLink")> like ILink <--> node6.ethg++;
        node3.ethg++  <--> <default("EthernetLink")> like ILink <--> node7.ethg++;
        node4.ethg++  <--> <default("EthernetLink")> like ILink <--> node7.ethg++;
        node4.ethg++  <--> <default("EthernetLink")> like ILink <--> node8.ethg++;
        node5.ethg++  <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node5.ethg++  <--> <default("EthernetLink")> like ILink <--> node10.ethg++;
        // Middle → bottom row
        node0.ethg++  <--> <default("EthernetLink")> like ILink <--> node7.ethg++;
        node1.ethg++  <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node6.ethg++  <--> <default("EthernetLink")> like ILink <--> node7.ethg++;
        node6.ethg++  <--> <default("EthernetLink")> like ILink <--> node12.ethg++;
        node7.ethg++  <--> <default("EthernetLink")> like ILink <--> node12.ethg++;
        node8.ethg++  <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node8.ethg++  <--> <default("EthernetLink")> like ILink <--> node12.ethg++;
        node9.ethg++  <--> <default("EthernetLink")> like ILink <--> node13.ethg++;
        node10.ethg++ <--> <default("EthernetLink")> like ILink <--> node11.ethg++;
        node10.ethg++ <--> <default("EthernetLink")> like ILink <--> node13.ethg++;
        node11.ethg++ <--> <default("EthernetLink")> like ILink <--> node13.ethg++;
        // Bottom row
        node12.ethg++ <--> <default("EthernetLink")> like ILink <--> node13.ethg++;
        // Extra cross-links for end-to-end diversity
        node2.ethg++  <--> <default("EthernetLink")> like ILink <--> node11.ethg++;
        node4.ethg++  <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node8.ethg++  <--> <default("EthernetLink")> like ILink <--> node13.ethg++;
        node7.ethg++  <--> <default("EthernetLink")> like ILink <--> node8.ethg++;
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
        }
}

//
// Highly connected, end-to-end redundancy (large) – 28 nodes, 60 links.
//
// A dense irregular mesh where every node has degree 3-6. Multiple
// node-disjoint paths exist between any pair of nodes, so several
// simultaneous failures can be tolerated. Redundancy is distributed
// across the whole topology. A doubled version of HcEndToEndNetwork.
//
// <pre>
//        [0]------[1]------[2]------[3]
//       / | \    / | \    / | \    / |
//     [4] |  [5]   |  [6]   |  [7]  |
//      |\ | / | \  | / |\  | / |\  |
//     [8]-[9]  [10]-[11] [12]-[13] [14]-[15]
//       \  |  / \   |  /  \  |  /   |
//        [16]-----[17]-----[18]----[19]
//       / | \    / | \    / | \    / |
//    [20] |  [21]   | [22]   | [23]  |
//      |\ | / | \  | / |\  | / |\  |
//    [24]-[25] [26]-[27]
// </pre>
//
network LargeHcEndToEndNetwork extends WiredNetworkBase
{
    parameters:
        volatile int numHostsPerSwitch = default(0);
        object numHosts = default(repeatPar("numHostsPerSwitch", 28));
    submodules:
        // Row 0 (top)
        node0:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=300,50"); }
        node1:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=550,50"); }
        node2:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=800,50"); }
        node3:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1050,50"); }
        // Row 1
        node4:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=250,190"); }
        node5:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=430,190"); }
        node6:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=670,190"); }
        node7:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=920,190"); }
        // Row 2
        node8:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=250,340"); }
        node9:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=380,340"); }
        node10: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=520,340"); }
        node11: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=670,340"); }
        node12: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=800,340"); }
        node13: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=920,340"); }
        node14: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1050,340"); }
        node15: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1160,340"); }
        // Row 3
        node16: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=380,490"); }
        node17: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=670,490"); }
        node18: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=920,490"); }
        node19: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1160,490"); }
        // Row 4
        node20: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=250,630"); }
        node21: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=430,630"); }
        node22: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=670,630"); }
        node23: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=920,630"); }
        // Row 5 (bottom)
        node24: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=250,770"); }
        node25: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=380,770"); }
        node26: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=670,770"); }
        node27: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=920,770"); }
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        // Row 0 chain
        node0.ethg++  <--> <default("EthernetLink")> like ILink <--> node1.ethg++;
        node1.ethg++  <--> <default("EthernetLink")> like ILink <--> node2.ethg++;
        node2.ethg++  <--> <default("EthernetLink")> like ILink <--> node3.ethg++;
        // Row 0 → Row 1
        node0.ethg++  <--> <default("EthernetLink")> like ILink <--> node4.ethg++;
        node0.ethg++  <--> <default("EthernetLink")> like ILink <--> node5.ethg++;
        node1.ethg++  <--> <default("EthernetLink")> like ILink <--> node5.ethg++;
        node1.ethg++  <--> <default("EthernetLink")> like ILink <--> node6.ethg++;
        node2.ethg++  <--> <default("EthernetLink")> like ILink <--> node6.ethg++;
        node2.ethg++  <--> <default("EthernetLink")> like ILink <--> node7.ethg++;
        node3.ethg++  <--> <default("EthernetLink")> like ILink <--> node7.ethg++;
        // Row 1 → Row 2
        node4.ethg++  <--> <default("EthernetLink")> like ILink <--> node8.ethg++;
        node4.ethg++  <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node5.ethg++  <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node5.ethg++  <--> <default("EthernetLink")> like ILink <--> node10.ethg++;
        node6.ethg++  <--> <default("EthernetLink")> like ILink <--> node11.ethg++;
        node6.ethg++  <--> <default("EthernetLink")> like ILink <--> node12.ethg++;
        node7.ethg++  <--> <default("EthernetLink")> like ILink <--> node13.ethg++;
        node7.ethg++  <--> <default("EthernetLink")> like ILink <--> node14.ethg++;
        // Row 2 internal + vertical ties
        node8.ethg++  <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node9.ethg++  <--> <default("EthernetLink")> like ILink <--> node10.ethg++;
        node10.ethg++ <--> <default("EthernetLink")> like ILink <--> node11.ethg++;
        node11.ethg++ <--> <default("EthernetLink")> like ILink <--> node12.ethg++;
        node12.ethg++ <--> <default("EthernetLink")> like ILink <--> node13.ethg++;
        node13.ethg++ <--> <default("EthernetLink")> like ILink <--> node14.ethg++;
        node14.ethg++ <--> <default("EthernetLink")> like ILink <--> node15.ethg++;
        node3.ethg++  <--> <default("EthernetLink")> like ILink <--> node15.ethg++;
        // Cross-links row 0-2
        node0.ethg++  <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node1.ethg++  <--> <default("EthernetLink")> like ILink <--> node11.ethg++;
        node2.ethg++  <--> <default("EthernetLink")> like ILink <--> node13.ethg++;
        // Row 2 → Row 3
        node8.ethg++  <--> <default("EthernetLink")> like ILink <--> node16.ethg++;
        node9.ethg++  <--> <default("EthernetLink")> like ILink <--> node16.ethg++;
        node10.ethg++ <--> <default("EthernetLink")> like ILink <--> node17.ethg++;
        node11.ethg++ <--> <default("EthernetLink")> like ILink <--> node17.ethg++;
        node12.ethg++ <--> <default("EthernetLink")> like ILink <--> node18.ethg++;
        node13.ethg++ <--> <default("EthernetLink")> like ILink <--> node18.ethg++;
        node14.ethg++ <--> <default("EthernetLink")> like ILink <--> node19.ethg++;
        node15.ethg++ <--> <default("EthernetLink")> like ILink <--> node19.ethg++;
        // Row 3 chain
        node16.ethg++ <--> <default("EthernetLink")> like ILink <--> node17.ethg++;
        node17.ethg++ <--> <default("EthernetLink")> like ILink <--> node18.ethg++;
        node18.ethg++ <--> <default("EthernetLink")> like ILink <--> node19.ethg++;
        // Row 3 → Row 4
        node16.ethg++ <--> <default("EthernetLink")> like ILink <--> node20.ethg++;
        node16.ethg++ <--> <default("EthernetLink")> like ILink <--> node21.ethg++;
        node17.ethg++ <--> <default("EthernetLink")> like ILink <--> node21.ethg++;
        node17.ethg++ <--> <default("EthernetLink")> like ILink <--> node22.ethg++;
        node18.ethg++ <--> <default("EthernetLink")> like ILink <--> node22.ethg++;
        node18.ethg++ <--> <default("EthernetLink")> like ILink <--> node23.ethg++;
        node19.ethg++ <--> <default("EthernetLink")> like ILink <--> node23.ethg++;
        // Row 4 → Row 5
        node20.ethg++ <--> <default("EthernetLink")> like ILink <--> node24.ethg++;
        node20.ethg++ <--> <default("EthernetLink")> like ILink <--> node25.ethg++;
        node21.ethg++ <--> <default("EthernetLink")> like ILink <--> node25.ethg++;
        node22.ethg++ <--> <default("EthernetLink")> like ILink <--> node26.ethg++;
        node23.ethg++ <--> <default("EthernetLink")> like ILink <--> node27.ethg++;
        // Row 5 chain
        node24.ethg++ <--> <default("EthernetLink")> like ILink <--> node25.ethg++;
        node25.ethg++ <--> <default("EthernetLink")> like ILink <--> node26.ethg++;
        node26.ethg++ <--> <default("EthernetLink")> like ILink <--> node27.ethg++;
        // Extra cross-links for end-to-end diversity
        node21.ethg++ <--> <default("EthernetLink")> like ILink <--> node26.ethg++;
        node22.ethg++ <--> <default("EthernetLink")> like ILink <--> node27.ethg++;
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
        }
}

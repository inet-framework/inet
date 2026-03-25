//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.common.ILink;

//
// Weakly connected, end-to-end redundancy – 14 nodes, 17 links.
//
// A sparse topology where most node pairs have only 2-3 neighbors,
// yet at least two node-disjoint paths exist between every pair of
// far-apart nodes. Achieved through a pair of long, loosely
// interleaved chains with periodic cross-links.
//
// <pre>
//  [0]---[1]---[2]---[3]---[4]---[5]---[6]
//   |         X           X           |
//  [7]---[8]---[9]--[10]--[11]--[12]-[13]
// </pre>
//
network WcEndToEndNetwork extends WiredNetworkBase
{
    parameters:
        volatile int numHostsPerSwitch = default(0); // hosts attached to every switch
        object numHosts = default(repeatPar("numHostsPerSwitch", 14));
    submodules:
        node0:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=250,80"); }
        node1:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=400,80"); }
        node2:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=550,80"); }
        node3:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=700,80"); }
        node4:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=850,80"); }
        node5:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1000,80"); }
        node6:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1150,80"); }
        node7:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=250,250"); }
        node8:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=400,250"); }
        node9:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=550,250"); }
        node10: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=700,250"); }
        node11: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=850,250"); }
        node12: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1000,250"); }
        node13: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1150,250"); }
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        // Upper chain
        node0.ethg++  <--> <default("EthernetLink")> like ILink <--> node1.ethg++;
        node1.ethg++  <--> <default("EthernetLink")> like ILink <--> node2.ethg++;
        node2.ethg++  <--> <default("EthernetLink")> like ILink <--> node3.ethg++;
        node3.ethg++  <--> <default("EthernetLink")> like ILink <--> node4.ethg++;
        node4.ethg++  <--> <default("EthernetLink")> like ILink <--> node5.ethg++;
        node5.ethg++  <--> <default("EthernetLink")> like ILink <--> node6.ethg++;
        // Lower chain
        node7.ethg++  <--> <default("EthernetLink")> like ILink <--> node8.ethg++;
        node8.ethg++  <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node9.ethg++  <--> <default("EthernetLink")> like ILink <--> node10.ethg++;
        node10.ethg++ <--> <default("EthernetLink")> like ILink <--> node11.ethg++;
        node11.ethg++ <--> <default("EthernetLink")> like ILink <--> node12.ethg++;
        node12.ethg++ <--> <default("EthernetLink")> like ILink <--> node13.ethg++;
        // Vertical ties at the ends
        node0.ethg++  <--> <default("EthernetLink")> like ILink <--> node7.ethg++;
        node6.ethg++  <--> <default("EthernetLink")> like ILink <--> node13.ethg++;
        // Cross-links (staggered, giving independent paths)
        node1.ethg++  <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node3.ethg++  <--> <default("EthernetLink")> like ILink <--> node11.ethg++;
        node4.ethg++  <--> <default("EthernetLink")> like ILink <--> node10.ethg++;
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
// Weakly connected, end-to-end redundancy (large) – 28 nodes, 34 links.
//
// A doubled version of WcEndToEndNetwork. Four loosely interleaved
// chains with periodic cross-links provide two node-disjoint paths
// between far-apart nodes while keeping the sparse, weak-connectivity
// character.
//
// <pre>
//  [0]---[1]---[2]---[3]---[4]---[5]---[6]
//   |         X           X           |
//  [7]---[8]---[9]--[10]--[11]--[12]-[13]
//   |         X           X           |
// [14]--[15]--[16]--[17]--[18]--[19]-[20]
//   |         X           X           |
// [21]--[22]--[23]--[24]--[25]--[26]-[27]
// </pre>
//
network LargeWcEndToEndNetwork extends WiredNetworkBase
{
    parameters:
        volatile int numHostsPerSwitch = default(0);
        object numHosts = default(repeatPar("numHostsPerSwitch", 28));
    submodules:
        // Row 0
        node0:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=250,80"); }
        node1:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=400,80"); }
        node2:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=550,80"); }
        node3:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=700,80"); }
        node4:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=850,80"); }
        node5:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1000,80"); }
        node6:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1150,80"); }
        // Row 1
        node7:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=250,230"); }
        node8:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=400,230"); }
        node9:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=550,230"); }
        node10: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=700,230"); }
        node11: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=850,230"); }
        node12: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1000,230"); }
        node13: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1150,230"); }
        // Row 2
        node14: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=250,380"); }
        node15: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=400,380"); }
        node16: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=550,380"); }
        node17: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=700,380"); }
        node18: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=850,380"); }
        node19: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1000,380"); }
        node20: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1150,380"); }
        // Row 3
        node21: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=250,530"); }
        node22: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=400,530"); }
        node23: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=550,530"); }
        node24: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=700,530"); }
        node25: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=850,530"); }
        node26: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1000,530"); }
        node27: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1150,530"); }
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        // Row 0 chain
        node0.ethg++  <--> <default("EthernetLink")> like ILink <--> node1.ethg++;
        node1.ethg++  <--> <default("EthernetLink")> like ILink <--> node2.ethg++;
        node2.ethg++  <--> <default("EthernetLink")> like ILink <--> node3.ethg++;
        node3.ethg++  <--> <default("EthernetLink")> like ILink <--> node4.ethg++;
        node4.ethg++  <--> <default("EthernetLink")> like ILink <--> node5.ethg++;
        node5.ethg++  <--> <default("EthernetLink")> like ILink <--> node6.ethg++;
        // Row 1 chain
        node7.ethg++  <--> <default("EthernetLink")> like ILink <--> node8.ethg++;
        node8.ethg++  <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node9.ethg++  <--> <default("EthernetLink")> like ILink <--> node10.ethg++;
        node10.ethg++ <--> <default("EthernetLink")> like ILink <--> node11.ethg++;
        node11.ethg++ <--> <default("EthernetLink")> like ILink <--> node12.ethg++;
        node12.ethg++ <--> <default("EthernetLink")> like ILink <--> node13.ethg++;
        // Row 2 chain
        node14.ethg++ <--> <default("EthernetLink")> like ILink <--> node15.ethg++;
        node15.ethg++ <--> <default("EthernetLink")> like ILink <--> node16.ethg++;
        node16.ethg++ <--> <default("EthernetLink")> like ILink <--> node17.ethg++;
        node17.ethg++ <--> <default("EthernetLink")> like ILink <--> node18.ethg++;
        node18.ethg++ <--> <default("EthernetLink")> like ILink <--> node19.ethg++;
        node19.ethg++ <--> <default("EthernetLink")> like ILink <--> node20.ethg++;
        // Row 3 chain
        node21.ethg++ <--> <default("EthernetLink")> like ILink <--> node22.ethg++;
        node22.ethg++ <--> <default("EthernetLink")> like ILink <--> node23.ethg++;
        node23.ethg++ <--> <default("EthernetLink")> like ILink <--> node24.ethg++;
        node24.ethg++ <--> <default("EthernetLink")> like ILink <--> node25.ethg++;
        node25.ethg++ <--> <default("EthernetLink")> like ILink <--> node26.ethg++;
        node26.ethg++ <--> <default("EthernetLink")> like ILink <--> node27.ethg++;
        // Vertical ties at the ends (rows 0-1)
        node0.ethg++  <--> <default("EthernetLink")> like ILink <--> node7.ethg++;
        node6.ethg++  <--> <default("EthernetLink")> like ILink <--> node13.ethg++;
        // Cross-links rows 0-1
        node1.ethg++  <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node3.ethg++  <--> <default("EthernetLink")> like ILink <--> node11.ethg++;
        node4.ethg++  <--> <default("EthernetLink")> like ILink <--> node10.ethg++;
        // Vertical ties (rows 1-2)
        node7.ethg++  <--> <default("EthernetLink")> like ILink <--> node14.ethg++;
        node13.ethg++ <--> <default("EthernetLink")> like ILink <--> node20.ethg++;
        // Cross-links rows 1-2
        node8.ethg++  <--> <default("EthernetLink")> like ILink <--> node16.ethg++;
        node10.ethg++ <--> <default("EthernetLink")> like ILink <--> node18.ethg++;
        node11.ethg++ <--> <default("EthernetLink")> like ILink <--> node17.ethg++;
        // Vertical ties (rows 2-3)
        node14.ethg++ <--> <default("EthernetLink")> like ILink <--> node21.ethg++;
        node20.ethg++ <--> <default("EthernetLink")> like ILink <--> node27.ethg++;
        // Cross-links rows 2-3
        node15.ethg++ <--> <default("EthernetLink")> like ILink <--> node23.ethg++;
        node17.ethg++ <--> <default("EthernetLink")> like ILink <--> node25.ethg++;
        node18.ethg++ <--> <default("EthernetLink")> like ILink <--> node24.ethg++;
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

//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.common.ILink;

//
// Highly connected, regional redundancy – 16 nodes, 30 links.
//
// Three dense clusters (near-cliques), each internally resilient
// to multiple failures. Adjacent clusters share two inter-cluster
// links, so regional failures are survivable. However, no direct
// path connects Cluster A to Cluster C without transiting
// Cluster B, so a full Cluster B outage partitions the network.
//
// <pre>
//   Cluster A           Cluster B            Cluster C
//  [0]--[1]           [6]--[7]            [11]--[12]
//   |\ /| \         / |\ /| \          /  |\ /|
//  [2]-[3] [4]==[5]  [8]-[9] [10]==[13] [14]-[15]
// </pre>
//
network HcRegionalNetwork extends WiredNetworkBase
{
    parameters:
        volatile int numHostsPerSwitch = default(0); // hosts attached to every switch
        object numHosts = default(repeatPar("numHostsPerSwitch", 16));
    submodules:
        // Cluster A
        node0:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=250,70"); }
        node1:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=380,70"); }
        node2:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=250,270"); }
        node3:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=380,130"); }
        node4:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=490,200"); }
        // Cluster B
        node5:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=560,200"); }
        node6:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=630,70"); }
        node7:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=760,70"); }
        node8:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=630,270"); }
        node9:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=760,200"); }
        node10: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=870,270"); }
        // Cluster C
        node11: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1010,70"); }
        node12: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1140,70"); }
        node13: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=980,200"); }
        node14: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1070,200"); }
        node15: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1140,200"); }
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        // Cluster A – dense (5 nodes, 8 links)
        node0.ethg++ <--> <default("EthernetLink")> like ILink <--> node1.ethg++;
        node0.ethg++ <--> <default("EthernetLink")> like ILink <--> node2.ethg++;
        node0.ethg++ <--> <default("EthernetLink")> like ILink <--> node3.ethg++;
        node1.ethg++ <--> <default("EthernetLink")> like ILink <--> node3.ethg++;
        node1.ethg++ <--> <default("EthernetLink")> like ILink <--> node4.ethg++;
        node2.ethg++ <--> <default("EthernetLink")> like ILink <--> node3.ethg++;
        node3.ethg++ <--> <default("EthernetLink")> like ILink <--> node4.ethg++;
        node2.ethg++ <--> <default("EthernetLink")> like ILink <--> node4.ethg++;
        // A ↔ B inter-cluster (2 links)
        node4.ethg++ <--> <default("EthernetLink")> like ILink <--> node5.ethg++;
        node4.ethg++ <--> <default("EthernetLink")> like ILink <--> node6.ethg++;
        // Cluster B – dense (5 nodes, 8 links)
        node5.ethg++ <--> <default("EthernetLink")> like ILink <--> node8.ethg++;
        node6.ethg++ <--> <default("EthernetLink")> like ILink <--> node7.ethg++;
        node6.ethg++ <--> <default("EthernetLink")> like ILink <--> node8.ethg++;
        node6.ethg++ <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node7.ethg++ <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node8.ethg++ <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node9.ethg++ <--> <default("EthernetLink")> like ILink <--> node10.ethg++;
        node8.ethg++ <--> <default("EthernetLink")> like ILink <--> node10.ethg++;
        // B ↔ C inter-cluster (2 links)
        node10.ethg++ <--> <default("EthernetLink")> like ILink <--> node13.ethg++;
        node10.ethg++ <--> <default("EthernetLink")> like ILink <--> node11.ethg++;
        // Cluster C – dense (5 nodes, 8 links)
        node11.ethg++ <--> <default("EthernetLink")> like ILink <--> node12.ethg++;
        node11.ethg++ <--> <default("EthernetLink")> like ILink <--> node14.ethg++;
        node12.ethg++ <--> <default("EthernetLink")> like ILink <--> node15.ethg++;
        node13.ethg++ <--> <default("EthernetLink")> like ILink <--> node14.ethg++;
        node14.ethg++ <--> <default("EthernetLink")> like ILink <--> node15.ethg++;
        node13.ethg++ <--> <default("EthernetLink")> like ILink <--> node15.ethg++;
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
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node15.ethg++ if groupIndex(numHosts,i) == 15;
        }
}

//
// Highly connected, regional redundancy (large) – 32 nodes, 60 links.
//
// Six dense clusters (near-cliques), each internally resilient to
// multiple failures. Adjacent clusters share two inter-cluster
// links, so regional failures are survivable. However, no direct
// path connects non-adjacent clusters without transiting the ones
// in between. A doubled version of HcRegionalNetwork.
//
// <pre>
//   Cluster A            Cluster B            Cluster C
//  [0]--[1]            [6]--[7]            [11]--[12]
//   |\ /| \          / |\ /| \          /  |\ /|
//  [2]-[3] [4]==[5]  [8]-[9] [10]==[13] [14]-[15]
//
//   Cluster D            Cluster E            Cluster F
// [16]-[17]            [22]-[23]            [27]-[28]
//   |\ /| \          / |\ /| \          /  |\ /|
// [18]-[19] [20]=[21] [24]-[25] [26]=[29] [30]-[31]
// </pre>
//
// Bridges: node15--node16, node13--node22
//
network LargeHcRegionalNetwork extends WiredNetworkBase
{
    parameters:
        volatile int numHostsPerSwitch = default(0);
        object numHosts = default(repeatPar("numHostsPerSwitch", 32));
    submodules:
        // Cluster A
        node0:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=300,70"); }
        node1:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=430,70"); }
        node2:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=300,200"); }
        node3:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=430,130"); }
        node4:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=540,200"); }
        // Cluster B
        node5:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=610,200"); }
        node6:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=680,70"); }
        node7:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=810,70"); }
        node8:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=680,250"); }
        node9:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=810,200"); }
        node10: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=920,200"); }
        // Cluster C
        node11: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1060,70"); }
        node12: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1190,70"); }
        node13: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=990,200"); }
        node14: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1120,150"); }
        node15: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1190,200"); }
        // Cluster D
        node16: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=300,400"); }
        node17: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=430,400"); }
        node18: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=300,530"); }
        node19: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=430,460"); }
        node20: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=540,530"); }
        // Cluster E
        node21: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=610,530"); }
        node22: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=680,400"); }
        node23: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=810,400"); }
        node24: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=680,580"); }
        node25: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=810,530"); }
        node26: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=920,530"); }
        // Cluster F
        node27: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1060,400"); }
        node28: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1190,400"); }
        node29: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=990,530"); }
        node30: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1120,480"); }
        node31: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1190,530"); }
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        // Cluster A – dense (5 nodes, 8 links)
        node0.ethg++ <--> <default("EthernetLink")> like ILink <--> node1.ethg++;
        node0.ethg++ <--> <default("EthernetLink")> like ILink <--> node2.ethg++;
        node0.ethg++ <--> <default("EthernetLink")> like ILink <--> node3.ethg++;
        node1.ethg++ <--> <default("EthernetLink")> like ILink <--> node3.ethg++;
        node1.ethg++ <--> <default("EthernetLink")> like ILink <--> node4.ethg++;
        node2.ethg++ <--> <default("EthernetLink")> like ILink <--> node3.ethg++;
        node3.ethg++ <--> <default("EthernetLink")> like ILink <--> node4.ethg++;
        node2.ethg++ <--> <default("EthernetLink")> like ILink <--> node4.ethg++;
        // A ↔ B inter-cluster (2 links)
        node4.ethg++ <--> <default("EthernetLink")> like ILink <--> node5.ethg++;
        node4.ethg++ <--> <default("EthernetLink")> like ILink <--> node6.ethg++;
        // Cluster B – dense (5 nodes, 8 links)
        node5.ethg++ <--> <default("EthernetLink")> like ILink <--> node8.ethg++;
        node6.ethg++ <--> <default("EthernetLink")> like ILink <--> node7.ethg++;
        node6.ethg++ <--> <default("EthernetLink")> like ILink <--> node8.ethg++;
        node6.ethg++ <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node7.ethg++ <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node8.ethg++ <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node9.ethg++ <--> <default("EthernetLink")> like ILink <--> node10.ethg++;
        node8.ethg++ <--> <default("EthernetLink")> like ILink <--> node10.ethg++;
        // B ↔ C inter-cluster (2 links)
        node10.ethg++ <--> <default("EthernetLink")> like ILink <--> node13.ethg++;
        node10.ethg++ <--> <default("EthernetLink")> like ILink <--> node11.ethg++;
        // Cluster C – dense (5 nodes, 8 links)
        node11.ethg++ <--> <default("EthernetLink")> like ILink <--> node12.ethg++;
        node11.ethg++ <--> <default("EthernetLink")> like ILink <--> node14.ethg++;
        node12.ethg++ <--> <default("EthernetLink")> like ILink <--> node15.ethg++;
        node13.ethg++ <--> <default("EthernetLink")> like ILink <--> node14.ethg++;
        node14.ethg++ <--> <default("EthernetLink")> like ILink <--> node15.ethg++;
        node13.ethg++ <--> <default("EthernetLink")> like ILink <--> node15.ethg++;
        node11.ethg++ <--> <default("EthernetLink")> like ILink <--> node13.ethg++;
        node12.ethg++ <--> <default("EthernetLink")> like ILink <--> node14.ethg++;
        // C ↔ D bridge (single link)
        node15.ethg++ <--> <default("EthernetLink")> like ILink <--> node16.ethg++;
        // Cluster D – dense (5 nodes, 8 links)
        node16.ethg++ <--> <default("EthernetLink")> like ILink <--> node17.ethg++;
        node16.ethg++ <--> <default("EthernetLink")> like ILink <--> node18.ethg++;
        node16.ethg++ <--> <default("EthernetLink")> like ILink <--> node19.ethg++;
        node17.ethg++ <--> <default("EthernetLink")> like ILink <--> node19.ethg++;
        node17.ethg++ <--> <default("EthernetLink")> like ILink <--> node20.ethg++;
        node18.ethg++ <--> <default("EthernetLink")> like ILink <--> node19.ethg++;
        node19.ethg++ <--> <default("EthernetLink")> like ILink <--> node20.ethg++;
        node18.ethg++ <--> <default("EthernetLink")> like ILink <--> node20.ethg++;
        // D ↔ E inter-cluster (2 links)
        node20.ethg++ <--> <default("EthernetLink")> like ILink <--> node21.ethg++;
        node20.ethg++ <--> <default("EthernetLink")> like ILink <--> node22.ethg++;
        // Cluster E – dense (5 nodes, 8 links)
        node21.ethg++ <--> <default("EthernetLink")> like ILink <--> node24.ethg++;
        node22.ethg++ <--> <default("EthernetLink")> like ILink <--> node23.ethg++;
        node22.ethg++ <--> <default("EthernetLink")> like ILink <--> node24.ethg++;
        node22.ethg++ <--> <default("EthernetLink")> like ILink <--> node25.ethg++;
        node23.ethg++ <--> <default("EthernetLink")> like ILink <--> node25.ethg++;
        node24.ethg++ <--> <default("EthernetLink")> like ILink <--> node25.ethg++;
        node25.ethg++ <--> <default("EthernetLink")> like ILink <--> node26.ethg++;
        node24.ethg++ <--> <default("EthernetLink")> like ILink <--> node26.ethg++;
        // E ↔ F inter-cluster (2 links)
        node26.ethg++ <--> <default("EthernetLink")> like ILink <--> node29.ethg++;
        node26.ethg++ <--> <default("EthernetLink")> like ILink <--> node27.ethg++;
        // Cluster F – dense (5 nodes, 8 links)
        node27.ethg++ <--> <default("EthernetLink")> like ILink <--> node28.ethg++;
        node27.ethg++ <--> <default("EthernetLink")> like ILink <--> node30.ethg++;
        node28.ethg++ <--> <default("EthernetLink")> like ILink <--> node31.ethg++;
        node29.ethg++ <--> <default("EthernetLink")> like ILink <--> node30.ethg++;
        node30.ethg++ <--> <default("EthernetLink")> like ILink <--> node31.ethg++;
        node29.ethg++ <--> <default("EthernetLink")> like ILink <--> node31.ethg++;
        node27.ethg++ <--> <default("EthernetLink")> like ILink <--> node29.ethg++;
        node28.ethg++ <--> <default("EthernetLink")> like ILink <--> node30.ethg++;
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
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node30.ethg++ if groupIndex(numHosts,i) == 30;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node31.ethg++ if groupIndex(numHosts,i) == 31;
        }
}

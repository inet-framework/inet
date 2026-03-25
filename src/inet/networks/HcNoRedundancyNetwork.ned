//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.common.ILink;

//
// Highly connected, no redundancy – 13 nodes, 24 links.
//
// Two dense sub-graphs (near-cliques) joined through a single
// articulation node (node6). Most node pairs have many neighbors,
// giving high overall connectivity, yet the entire network
// partitions if node6 fails.
//
// <pre>
//    [0]--[1]--[2]            [7]--[8]
//     |\  |\ / |              |\ / |\
//     | [3]-[4]              [9]-[10]|
//     | / \ |                 |\ /  /
//    [5]----[6]--------------[11]-[12]
// </pre>
//
network HcNoRedundancyNetwork extends WiredNetworkBase
{
    parameters:
        volatile int numHostsPerSwitch = default(0); // hosts attached to every switch
        object numHosts = default(repeatPar("numHostsPerSwitch", 13));
    submodules:
        // Left dense cluster
        node0:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=260,60"); }
        node1:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=400,60"); }
        node2:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=540,60"); }
        node3:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=330,185"); }
        node4:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=480,185"); }
        node5:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=260,310"); }
        node6:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=540,310"); }
        // Right dense cluster
        node7:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=760,60"); }
        node8:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=900,60"); }
        node9:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=760,185"); }
        node10: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=900,185"); }
        node11: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=760,310"); }
        node12: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=900,310"); }
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        // Left cluster – near-clique on {0,1,2,3,4,5,6}
        node0.ethg++ <--> <default("EthernetLink")> like ILink <--> node1.ethg++;
        node0.ethg++ <--> <default("EthernetLink")> like ILink <--> node3.ethg++;
        node0.ethg++ <--> <default("EthernetLink")> like ILink <--> node5.ethg++;
        node1.ethg++ <--> <default("EthernetLink")> like ILink <--> node2.ethg++;
        node1.ethg++ <--> <default("EthernetLink")> like ILink <--> node3.ethg++;
        node1.ethg++ <--> <default("EthernetLink")> like ILink <--> node4.ethg++;
        node2.ethg++ <--> <default("EthernetLink")> like ILink <--> node4.ethg++;
        node3.ethg++ <--> <default("EthernetLink")> like ILink <--> node4.ethg++;
        node3.ethg++ <--> <default("EthernetLink")> like ILink <--> node5.ethg++;
        node4.ethg++ <--> <default("EthernetLink")> like ILink <--> node6.ethg++;
        node5.ethg++ <--> <default("EthernetLink")> like ILink <--> node6.ethg++;
        node5.ethg++ <--> <default("EthernetLink")> like ILink <--> node3.ethg++;
        // Single bridge
        node6.ethg++ <--> <default("EthernetLink")> like ILink <--> node11.ethg++;
        // Right cluster – near-clique on {7,8,9,10,11,12}
        node7.ethg++  <--> <default("EthernetLink")> like ILink <--> node8.ethg++;
        node7.ethg++  <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node7.ethg++  <--> <default("EthernetLink")> like ILink <--> node10.ethg++;
        node8.ethg++  <--> <default("EthernetLink")> like ILink <--> node10.ethg++;
        node8.ethg++  <--> <default("EthernetLink")> like ILink <--> node12.ethg++;
        node9.ethg++  <--> <default("EthernetLink")> like ILink <--> node10.ethg++;
        node9.ethg++  <--> <default("EthernetLink")> like ILink <--> node11.ethg++;
        node10.ethg++ <--> <default("EthernetLink")> like ILink <--> node12.ethg++;
        node11.ethg++ <--> <default("EthernetLink")> like ILink <--> node12.ethg++;
        node11.ethg++ <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node9.ethg++  <--> <default("EthernetLink")> like ILink <--> node12.ethg++;
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
        }
}

//
// Highly connected, no redundancy (large) – 26 nodes, 48 links.
//
// Two dense sub-graphs (near-cliques of 13 nodes each) joined
// through a single articulation node (node12). Most node pairs
// have many neighbors, giving high overall connectivity, yet the
// entire network partitions if node12 fails. A doubled version
// of HcNoRedundancyNetwork.
//
// <pre>
//    [0]--[1]--[2]            [13]--[14]
//     |\  |\ / |              |\  / |\
//     | [3]-[4]              [15]-[16]|
//     | / \ |                 |\ /  /
//    [5]----[6]              [17]-[18]
//     |\  |\ / |              |\  / |
//     | [7]-[8]              [19]-[20]
//     | / \ |                 | /   |
//    [9]---[10]             [21]---[22]
//      \   / |               |   /
//      [11]-[12]============[23]-[24]
//             |                   |
//            [25]                 .
// </pre>
//
network LargeHcNoRedundancyNetwork extends WiredNetworkBase
{
    parameters:
        volatile int numHostsPerSwitch = default(0);
        object numHosts = default(repeatPar("numHostsPerSwitch", 26));
    submodules:
        // Left dense cluster
        node0:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=260,60"); }
        node1:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=400,60"); }
        node2:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=540,60"); }
        node3:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=330,160"); }
        node4:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=480,160"); }
        node5:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=260,260"); }
        node6:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=540,260"); }
        node7:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=330,360"); }
        node8:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=480,360"); }
        node9:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=260,460"); }
        node10: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=540,460"); }
        node11: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=330,560"); }
        node12: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=540,560"); }
        // Right dense cluster
        node13: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=760,60"); }
        node14: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=900,60"); }
        node15: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=760,160"); }
        node16: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=900,160"); }
        node17: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=760,260"); }
        node18: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=900,260"); }
        node19: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=760,360"); }
        node20: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=900,360"); }
        node21: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=760,460"); }
        node22: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=900,460"); }
        node23: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=760,560"); }
        node24: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=900,560"); }
        node25: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=540,660"); }
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        // Left cluster – near-clique on {0..12}
        node0.ethg++  <--> <default("EthernetLink")> like ILink <--> node1.ethg++;
        node0.ethg++  <--> <default("EthernetLink")> like ILink <--> node3.ethg++;
        node0.ethg++  <--> <default("EthernetLink")> like ILink <--> node5.ethg++;
        node1.ethg++  <--> <default("EthernetLink")> like ILink <--> node2.ethg++;
        node1.ethg++  <--> <default("EthernetLink")> like ILink <--> node3.ethg++;
        node1.ethg++  <--> <default("EthernetLink")> like ILink <--> node4.ethg++;
        node2.ethg++  <--> <default("EthernetLink")> like ILink <--> node4.ethg++;
        node3.ethg++  <--> <default("EthernetLink")> like ILink <--> node4.ethg++;
        node3.ethg++  <--> <default("EthernetLink")> like ILink <--> node5.ethg++;
        node4.ethg++  <--> <default("EthernetLink")> like ILink <--> node6.ethg++;
        node5.ethg++  <--> <default("EthernetLink")> like ILink <--> node6.ethg++;
        node5.ethg++  <--> <default("EthernetLink")> like ILink <--> node7.ethg++;
        node5.ethg++  <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node6.ethg++  <--> <default("EthernetLink")> like ILink <--> node8.ethg++;
        node6.ethg++  <--> <default("EthernetLink")> like ILink <--> node10.ethg++;
        node7.ethg++  <--> <default("EthernetLink")> like ILink <--> node8.ethg++;
        node7.ethg++  <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node8.ethg++  <--> <default("EthernetLink")> like ILink <--> node10.ethg++;
        node9.ethg++  <--> <default("EthernetLink")> like ILink <--> node10.ethg++;
        node9.ethg++  <--> <default("EthernetLink")> like ILink <--> node11.ethg++;
        node10.ethg++ <--> <default("EthernetLink")> like ILink <--> node11.ethg++;
        node10.ethg++ <--> <default("EthernetLink")> like ILink <--> node12.ethg++;
        node11.ethg++ <--> <default("EthernetLink")> like ILink <--> node12.ethg++;
        // Single bridge
        node12.ethg++ <--> <default("EthernetLink")> like ILink <--> node23.ethg++;
        // Right cluster – near-clique on {13..24}
        node13.ethg++ <--> <default("EthernetLink")> like ILink <--> node14.ethg++;
        node13.ethg++ <--> <default("EthernetLink")> like ILink <--> node15.ethg++;
        node13.ethg++ <--> <default("EthernetLink")> like ILink <--> node16.ethg++;
        node14.ethg++ <--> <default("EthernetLink")> like ILink <--> node16.ethg++;
        node15.ethg++ <--> <default("EthernetLink")> like ILink <--> node16.ethg++;
        node15.ethg++ <--> <default("EthernetLink")> like ILink <--> node17.ethg++;
        node16.ethg++ <--> <default("EthernetLink")> like ILink <--> node18.ethg++;
        node17.ethg++ <--> <default("EthernetLink")> like ILink <--> node18.ethg++;
        node17.ethg++ <--> <default("EthernetLink")> like ILink <--> node19.ethg++;
        node17.ethg++ <--> <default("EthernetLink")> like ILink <--> node21.ethg++;
        node18.ethg++ <--> <default("EthernetLink")> like ILink <--> node20.ethg++;
        node18.ethg++ <--> <default("EthernetLink")> like ILink <--> node22.ethg++;
        node19.ethg++ <--> <default("EthernetLink")> like ILink <--> node20.ethg++;
        node19.ethg++ <--> <default("EthernetLink")> like ILink <--> node21.ethg++;
        node20.ethg++ <--> <default("EthernetLink")> like ILink <--> node22.ethg++;
        node21.ethg++ <--> <default("EthernetLink")> like ILink <--> node22.ethg++;
        node21.ethg++ <--> <default("EthernetLink")> like ILink <--> node23.ethg++;
        node22.ethg++ <--> <default("EthernetLink")> like ILink <--> node23.ethg++;
        node22.ethg++ <--> <default("EthernetLink")> like ILink <--> node24.ethg++;
        node23.ethg++ <--> <default("EthernetLink")> like ILink <--> node24.ethg++;
        // Dangling leaf off bridge
        node12.ethg++ <--> <default("EthernetLink")> like ILink <--> node25.ethg++;
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
        }
}

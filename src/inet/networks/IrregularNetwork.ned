//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.common.ILink;

//
// An irregular backbone topology with 10 nodes and 14 links.
//
// An organic, asymmetric layout with no crossing links.
// Node degrees range from 2 to 4, with two hub nodes (node4, node7)
// providing higher connectivity. Suitable as a small ISP backbone
// or campus core topology for routing, resilience, and traffic
// engineering studies.
//
// <pre>
//          [0]
//         /   \
//       [1]    [2]
//       / \   / \
//     [3]--[4]  [5]
//      |    |   / \
//     [6]  [7]--[8]
//       \  /
//        [9]
// </pre>
//
network Irregular10Network extends WiredNetworkBase
{
    parameters:
        volatile int numHostsPerSwitch = default(0); // number of hosts connected to each switch
        object numHosts = default(repeatPar("numHostsPerSwitch", 10));
    submodules:
        node0: <default("Router")> like IEthernetNetworkNode { @display("p=400,50"); }
        node1: <default("Router")> like IEthernetNetworkNode { @display("p=275,170"); }
        node2: <default("Router")> like IEthernetNetworkNode { @display("p=545,135"); }
        node3: <default("Router")> like IEthernetNetworkNode { @display("p=240,325"); }
        node4: <default("Router")> like IEthernetNetworkNode { @display("p=415,265"); }
        node5: <default("Router")> like IEthernetNetworkNode { @display("p=595,280"); }
        node6: <default("Router")> like IEthernetNetworkNode { @display("p=305,460"); }
        node7: <default("Router")> like IEthernetNetworkNode { @display("p=485,415"); }
        node8: <default("Router")> like IEthernetNetworkNode { @display("p=630,445"); }
        node9: <default("Router")> like IEthernetNetworkNode { @display("p=390,565"); }
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        node0.ethg++ <--> <default("EthernetLink")> like ILink <--> node1.ethg++;
        node0.ethg++ <--> <default("EthernetLink")> like ILink <--> node2.ethg++;
        node1.ethg++ <--> <default("EthernetLink")> like ILink <--> node3.ethg++;
        node1.ethg++ <--> <default("EthernetLink")> like ILink <--> node4.ethg++;
        node2.ethg++ <--> <default("EthernetLink")> like ILink <--> node4.ethg++;
        node2.ethg++ <--> <default("EthernetLink")> like ILink <--> node5.ethg++;
        node3.ethg++ <--> <default("EthernetLink")> like ILink <--> node4.ethg++;
        node3.ethg++ <--> <default("EthernetLink")> like ILink <--> node6.ethg++;
        node4.ethg++ <--> <default("EthernetLink")> like ILink <--> node7.ethg++;
        node5.ethg++ <--> <default("EthernetLink")> like ILink <--> node7.ethg++;
        node5.ethg++ <--> <default("EthernetLink")> like ILink <--> node8.ethg++;
        node6.ethg++ <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node7.ethg++ <--> <default("EthernetLink")> like ILink <--> node8.ethg++;
        node7.ethg++ <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        // hosts
        for i=0..sizeof(host)-1 {
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node0.ethg++ if groupIndex(numHosts,i) == 0;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node1.ethg++ if groupIndex(numHosts,i) == 1;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node2.ethg++ if groupIndex(numHosts,i) == 2;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node3.ethg++ if groupIndex(numHosts,i) == 3;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node4.ethg++ if groupIndex(numHosts,i) == 4;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node5.ethg++ if groupIndex(numHosts,i) == 5;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node6.ethg++ if groupIndex(numHosts,i) == 6;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node7.ethg++ if groupIndex(numHosts,i) == 7;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node8.ethg++ if groupIndex(numHosts,i) == 8;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node9.ethg++ if groupIndex(numHosts,i) == 9;
        }
}

//
// An irregular backbone topology with 20 nodes and 30 links.
//
// A larger organic, asymmetric layout with no crossing links.
// Nodes are arranged in staggered rows flowing top to bottom.
// Node degrees range from 2 to 4, with several hub nodes providing
// higher connectivity. Suitable as a medium-sized ISP backbone
// for routing, resilience, and traffic engineering studies.
//
// <pre>
//                [0]
//               /   \
//            [1]     [2]
//           / \     / \
//         [3] [4]     [5]
//          |  / \   / \
//        [6][7]--[8]   [9]
//          |  |    \   / \
//       [10]-[11] [12]  [13]
//          \  / \  / \    |
//         [14] [15]  [16]
//            \  / \   /
//            [17] [18]
//              \  /
//              [19]
// </pre>
//
network Irregular20Network extends WiredNetworkBase
{
    parameters:
        volatile int numHostsPerSwitch = default(0); // number of hosts connected to each switch
        object numHosts = default(repeatPar("numHostsPerSwitch", 20));
    submodules:
        node0:  <default("Router")> like IEthernetNetworkNode { @display("p=550,50"); }
        node1:  <default("Router")> like IEthernetNetworkNode { @display("p=410,160"); }
        node2:  <default("Router")> like IEthernetNetworkNode { @display("p=700,150"); }
        node3:  <default("Router")> like IEthernetNetworkNode { @display("p=345,285"); }
        node4:  <default("Router")> like IEthernetNetworkNode { @display("p=525,275"); }
        node5:  <default("Router")> like IEthernetNetworkNode { @display("p=740,295"); }
        node6:  <default("Router")> like IEthernetNetworkNode { @display("p=330,410"); }
        node7:  <default("Router")> like IEthernetNetworkNode { @display("p=475,405"); }
        node8:  <default("Router")> like IEthernetNetworkNode { @display("p=650,395"); }
        node9:  <default("Router")> like IEthernetNetworkNode { @display("p=820,415"); }
        node10: <default("Router")> like IEthernetNetworkNode { @display("p=355,530"); }
        node11: <default("Router")> like IEthernetNetworkNode { @display("p=505,525"); }
        node12: <default("Router")> like IEthernetNetworkNode { @display("p=680,535"); }
        node13: <default("Router")> like IEthernetNetworkNode { @display("p=830,520"); }
        node14: <default("Router")> like IEthernetNetworkNode { @display("p=395,650"); }
        node15: <default("Router")> like IEthernetNetworkNode { @display("p=570,645"); }
        node16: <default("Router")> like IEthernetNetworkNode { @display("p=760,655"); }
        node17: <default("Router")> like IEthernetNetworkNode { @display("p=455,760"); }
        node18: <default("Router")> like IEthernetNetworkNode { @display("p=695,755"); }
        node19: <default("Router")> like IEthernetNetworkNode { @display("p=565,865"); }
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        // row 1 -> row 2
        node0.ethg++  <--> <default("EthernetLink")> like ILink <--> node1.ethg++;
        node0.ethg++  <--> <default("EthernetLink")> like ILink <--> node2.ethg++;
        // row 2 -> row 3
        node1.ethg++  <--> <default("EthernetLink")> like ILink <--> node3.ethg++;
        node1.ethg++  <--> <default("EthernetLink")> like ILink <--> node4.ethg++;
        node2.ethg++  <--> <default("EthernetLink")> like ILink <--> node4.ethg++;
        node2.ethg++  <--> <default("EthernetLink")> like ILink <--> node5.ethg++;
        // row 3 -> row 4
        node3.ethg++  <--> <default("EthernetLink")> like ILink <--> node6.ethg++;
        node4.ethg++  <--> <default("EthernetLink")> like ILink <--> node7.ethg++;
        node4.ethg++  <--> <default("EthernetLink")> like ILink <--> node8.ethg++;
        node5.ethg++  <--> <default("EthernetLink")> like ILink <--> node8.ethg++;
        node5.ethg++  <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        // row 4 (within)
        node7.ethg++  <--> <default("EthernetLink")> like ILink <--> node8.ethg++;
        // row 4 -> row 5
        node6.ethg++  <--> <default("EthernetLink")> like ILink <--> node10.ethg++;
        node7.ethg++  <--> <default("EthernetLink")> like ILink <--> node11.ethg++;
        node8.ethg++  <--> <default("EthernetLink")> like ILink <--> node12.ethg++;
        node9.ethg++  <--> <default("EthernetLink")> like ILink <--> node12.ethg++;
        node9.ethg++  <--> <default("EthernetLink")> like ILink <--> node13.ethg++;
        // row 5 (within)
        node10.ethg++ <--> <default("EthernetLink")> like ILink <--> node11.ethg++;
        // row 5 -> row 6
        node10.ethg++ <--> <default("EthernetLink")> like ILink <--> node14.ethg++;
        node11.ethg++ <--> <default("EthernetLink")> like ILink <--> node14.ethg++;
        node11.ethg++ <--> <default("EthernetLink")> like ILink <--> node15.ethg++;
        node12.ethg++ <--> <default("EthernetLink")> like ILink <--> node15.ethg++;
        node12.ethg++ <--> <default("EthernetLink")> like ILink <--> node16.ethg++;
        node13.ethg++ <--> <default("EthernetLink")> like ILink <--> node16.ethg++;
        // row 6 -> row 7
        node14.ethg++ <--> <default("EthernetLink")> like ILink <--> node17.ethg++;
        node15.ethg++ <--> <default("EthernetLink")> like ILink <--> node17.ethg++;
        node15.ethg++ <--> <default("EthernetLink")> like ILink <--> node18.ethg++;
        node16.ethg++ <--> <default("EthernetLink")> like ILink <--> node18.ethg++;
        // row 7 -> row 8
        node17.ethg++ <--> <default("EthernetLink")> like ILink <--> node19.ethg++;
        node18.ethg++ <--> <default("EthernetLink")> like ILink <--> node19.ethg++;
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
        }
}

//
// A larger irregular backbone topology with 40 nodes and 60 links.
//
// A scaled-up staggered-row layout preserving the top-to-bottom flow
// of Irregular20Network. Rows widen towards the center then narrow
// again, with hub nodes providing higher connectivity. Node degrees
// range from 2 to 5.
//
// <pre>
//            [0]   [1]   [2]
//           / \   / \   / \
//         [3] [4]  [5]  [6]
//         / \ |   |\ / \
//       [7] [8]--[9]-[10] [11]
//        |   | / \  / \  / \
//      [12][13]--[14] [15]--[16]
//       / \  |  / \   |   / \
//    [17][18][19]--[20][21][22]
//       |   \ / \  / \ / \
//     [23] [24][25]--[26][27]
//       |   \ / \  / \  /
//     [28] [29][30]--[31][32]
//            \ / \  / \ /
//           [33][34]--[35][36]
//              \ / \ / \ /
//             [37] [38] [39]
// </pre>
//
network Irregular40Network extends WiredNetworkBase
{
    parameters:
        volatile int numHostsPerSwitch = default(0); // number of hosts connected to each switch
        object numHosts = default(repeatPar("numHostsPerSwitch", 40));
    submodules:
        // row 0
        node0:  <default("Router")> like IEthernetNetworkNode { @display("p=500,50"); }
        node1:  <default("Router")> like IEthernetNetworkNode { @display("p=700,50"); }
        node2:  <default("Router")> like IEthernetNetworkNode { @display("p=900,50"); }
        // row 1
        node3:  <default("Router")> like IEthernetNetworkNode { @display("p=400,160"); }
        node4:  <default("Router")> like IEthernetNetworkNode { @display("p=580,160"); }
        node5:  <default("Router")> like IEthernetNetworkNode { @display("p=750,160"); }
        node6:  <default("Router")> like IEthernetNetworkNode { @display("p=940,160"); }
        // row 2
        node7:  <default("Router")> like IEthernetNetworkNode { @display("p=340,270"); }
        node8:  <default("Router")> like IEthernetNetworkNode { @display("p=490,270"); }
        node9:  <default("Router")> like IEthernetNetworkNode { @display("p=640,270"); }
        node10: <default("Router")> like IEthernetNetworkNode { @display("p=800,270"); }
        node11: <default("Router")> like IEthernetNetworkNode { @display("p=980,270"); }
        // row 3
        node12: <default("Router")> like IEthernetNetworkNode { @display("p=300,380"); }
        node13: <default("Router")> like IEthernetNetworkNode { @display("p=460,380"); }
        node14: <default("Router")> like IEthernetNetworkNode { @display("p=630,380"); }
        node15: <default("Router")> like IEthernetNetworkNode { @display("p=810,380"); }
        node16: <default("Router")> like IEthernetNetworkNode { @display("p=1000,380"); }
        // row 4
        node17: <default("Router")> like IEthernetNetworkNode { @display("p=260,490"); }
        node18: <default("Router")> like IEthernetNetworkNode { @display("p=400,490"); }
        node19: <default("Router")> like IEthernetNetworkNode { @display("p=540,490"); }
        node20: <default("Router")> like IEthernetNetworkNode { @display("p=700,490"); }
        node21: <default("Router")> like IEthernetNetworkNode { @display("p=860,490"); }
        node22: <default("Router")> like IEthernetNetworkNode { @display("p=1040,490"); }
        // row 5
        node23: <default("Router")> like IEthernetNetworkNode { @display("p=300,600"); }
        node24: <default("Router")> like IEthernetNetworkNode { @display("p=460,600"); }
        node25: <default("Router")> like IEthernetNetworkNode { @display("p=620,600"); }
        node26: <default("Router")> like IEthernetNetworkNode { @display("p=800,600"); }
        node27: <default("Router")> like IEthernetNetworkNode { @display("p=980,600"); }
        // row 6
        node28: <default("Router")> like IEthernetNetworkNode { @display("p=300,710"); }
        node29: <default("Router")> like IEthernetNetworkNode { @display("p=460,710"); }
        node30: <default("Router")> like IEthernetNetworkNode { @display("p=620,710"); }
        node31: <default("Router")> like IEthernetNetworkNode { @display("p=800,710"); }
        node32: <default("Router")> like IEthernetNetworkNode { @display("p=980,710"); }
        // row 7
        node33: <default("Router")> like IEthernetNetworkNode { @display("p=420,820"); }
        node34: <default("Router")> like IEthernetNetworkNode { @display("p=590,820"); }
        node35: <default("Router")> like IEthernetNetworkNode { @display("p=760,820"); }
        node36: <default("Router")> like IEthernetNetworkNode { @display("p=930,820"); }
        // row 8
        node37: <default("Router")> like IEthernetNetworkNode { @display("p=500,930"); }
        node38: <default("Router")> like IEthernetNetworkNode { @display("p=680,930"); }
        node39: <default("Router")> like IEthernetNetworkNode { @display("p=860,930"); }
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        // row 0 -> row 1
        node0.ethg++  <--> <default("EthernetLink")> like ILink <--> node3.ethg++;
        node0.ethg++  <--> <default("EthernetLink")> like ILink <--> node4.ethg++;
        node1.ethg++  <--> <default("EthernetLink")> like ILink <--> node4.ethg++;
        node1.ethg++  <--> <default("EthernetLink")> like ILink <--> node5.ethg++;
        node2.ethg++  <--> <default("EthernetLink")> like ILink <--> node5.ethg++;
        node2.ethg++  <--> <default("EthernetLink")> like ILink <--> node6.ethg++;
        // row 1 -> row 2
        node3.ethg++  <--> <default("EthernetLink")> like ILink <--> node7.ethg++;
        node3.ethg++  <--> <default("EthernetLink")> like ILink <--> node8.ethg++;
        node4.ethg++  <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node5.ethg++  <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        node5.ethg++  <--> <default("EthernetLink")> like ILink <--> node10.ethg++;
        node6.ethg++  <--> <default("EthernetLink")> like ILink <--> node10.ethg++;
        node6.ethg++  <--> <default("EthernetLink")> like ILink <--> node11.ethg++;
        // row 2 (within)
        node8.ethg++  <--> <default("EthernetLink")> like ILink <--> node9.ethg++;
        // row 2 -> row 3
        node7.ethg++  <--> <default("EthernetLink")> like ILink <--> node12.ethg++;
        node8.ethg++  <--> <default("EthernetLink")> like ILink <--> node13.ethg++;
        node9.ethg++  <--> <default("EthernetLink")> like ILink <--> node13.ethg++;
        node9.ethg++  <--> <default("EthernetLink")> like ILink <--> node14.ethg++;
        node10.ethg++ <--> <default("EthernetLink")> like ILink <--> node15.ethg++;
        node11.ethg++ <--> <default("EthernetLink")> like ILink <--> node15.ethg++;
        node11.ethg++ <--> <default("EthernetLink")> like ILink <--> node16.ethg++;
        // row 3 (within)
        node13.ethg++ <--> <default("EthernetLink")> like ILink <--> node14.ethg++;
        // row 3 -> row 4
        node12.ethg++ <--> <default("EthernetLink")> like ILink <--> node17.ethg++;
        node12.ethg++ <--> <default("EthernetLink")> like ILink <--> node18.ethg++;
        node13.ethg++ <--> <default("EthernetLink")> like ILink <--> node19.ethg++;
        node14.ethg++ <--> <default("EthernetLink")> like ILink <--> node19.ethg++;
        node14.ethg++ <--> <default("EthernetLink")> like ILink <--> node20.ethg++;
        node15.ethg++ <--> <default("EthernetLink")> like ILink <--> node21.ethg++;
        node16.ethg++ <--> <default("EthernetLink")> like ILink <--> node21.ethg++;
        node16.ethg++ <--> <default("EthernetLink")> like ILink <--> node22.ethg++;
        // row 4 (within)
        node19.ethg++ <--> <default("EthernetLink")> like ILink <--> node20.ethg++;
        // row 4 -> row 5
        node17.ethg++ <--> <default("EthernetLink")> like ILink <--> node23.ethg++;
        node18.ethg++ <--> <default("EthernetLink")> like ILink <--> node24.ethg++;
        node19.ethg++ <--> <default("EthernetLink")> like ILink <--> node24.ethg++;
        node20.ethg++ <--> <default("EthernetLink")> like ILink <--> node25.ethg++;
        node21.ethg++ <--> <default("EthernetLink")> like ILink <--> node26.ethg++;
        node22.ethg++ <--> <default("EthernetLink")> like ILink <--> node26.ethg++;
        node22.ethg++ <--> <default("EthernetLink")> like ILink <--> node27.ethg++;
        // row 5 (within)
        node25.ethg++ <--> <default("EthernetLink")> like ILink <--> node26.ethg++;
        // row 5 -> row 6
        node23.ethg++ <--> <default("EthernetLink")> like ILink <--> node28.ethg++;
        node24.ethg++ <--> <default("EthernetLink")> like ILink <--> node29.ethg++;
        node25.ethg++ <--> <default("EthernetLink")> like ILink <--> node29.ethg++;
        node25.ethg++ <--> <default("EthernetLink")> like ILink <--> node30.ethg++;
        node26.ethg++ <--> <default("EthernetLink")> like ILink <--> node31.ethg++;
        node27.ethg++ <--> <default("EthernetLink")> like ILink <--> node31.ethg++;
        node27.ethg++ <--> <default("EthernetLink")> like ILink <--> node32.ethg++;
        // row 6 (within)
        node30.ethg++ <--> <default("EthernetLink")> like ILink <--> node31.ethg++;
        // row 6 -> row 7
        node28.ethg++ <--> <default("EthernetLink")> like ILink <--> node33.ethg++;
        node29.ethg++ <--> <default("EthernetLink")> like ILink <--> node34.ethg++;
        node30.ethg++ <--> <default("EthernetLink")> like ILink <--> node34.ethg++;
        node30.ethg++ <--> <default("EthernetLink")> like ILink <--> node35.ethg++;
        node31.ethg++ <--> <default("EthernetLink")> like ILink <--> node36.ethg++;
        node32.ethg++ <--> <default("EthernetLink")> like ILink <--> node36.ethg++;
        // row 7 (within)
        node34.ethg++ <--> <default("EthernetLink")> like ILink <--> node35.ethg++;
        // row 7 -> row 8
        node33.ethg++ <--> <default("EthernetLink")> like ILink <--> node37.ethg++;
        node34.ethg++ <--> <default("EthernetLink")> like ILink <--> node37.ethg++;
        node34.ethg++ <--> <default("EthernetLink")> like ILink <--> node38.ethg++;
        node35.ethg++ <--> <default("EthernetLink")> like ILink <--> node38.ethg++;
        node35.ethg++ <--> <default("EthernetLink")> like ILink <--> node39.ethg++;
        node36.ethg++ <--> <default("EthernetLink")> like ILink <--> node39.ethg++;
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
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node32.ethg++ if groupIndex(numHosts,i) == 32;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node33.ethg++ if groupIndex(numHosts,i) == 33;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node34.ethg++ if groupIndex(numHosts,i) == 34;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node35.ethg++ if groupIndex(numHosts,i) == 35;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node36.ethg++ if groupIndex(numHosts,i) == 36;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node37.ethg++ if groupIndex(numHosts,i) == 37;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node38.ethg++ if groupIndex(numHosts,i) == 38;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node39.ethg++ if groupIndex(numHosts,i) == 39;
        }
}

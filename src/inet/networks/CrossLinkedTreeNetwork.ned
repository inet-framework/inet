//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.common.ILink;

//
// A sparse tree with a few accidental cross-links (15 nodes, 17 links).
//
// A mostly hierarchical network that grew incrementally: still tree-like
// but no longer a pure tree. Three lateral cross-links between sibling
// branches at depths 2 and 3 add a little path diversity without
// eliminating the single points of failure near the root.
//
// Typical of small enterprise, industrial control, legacy campus, or
// utility monitoring networks.
//
// Properties: strong hierarchy, sparse, fragile connectivity, high
// bottleneck potential near the root, low path diversity, mildly
// heterogeneous degree distribution.
//
// <pre>
//                      [root]
//                    /   |   \
//                [n1]  [n2]  [n3]
//               / \    / \    / \
//            [n4][n5--n6][n7--n8][n9]
//            / \       |     / \
//        [n10][n11---n12] [n13][n14]
// </pre>
//
network CrossLinkedTreeNetwork extends WiredNetworkBase
{
    parameters:
        volatile int numHostsPerSwitch = default(0); // hosts attached to every switch
        object numHosts = default(repeatPar("numHostsPerSwitch", 15));
    submodules:
        // depth 0 – root
        root: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=650,50"); }
        // depth 1
        n1:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=400,180"); }
        n2:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=650,180"); }
        n3:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=900,180"); }
        // depth 2
        n4:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=330,320"); }
        n5:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=470,320"); }
        n6:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=590,320"); }
        n7:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=710,320"); }
        n8:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=830,320"); }
        n9:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=970,320"); }
        // depth 3 – leaves
        n10: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=290,460"); }
        n11: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=410,460"); }
        n12: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=590,460"); }
        n13: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=790,460"); }
        n14: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=890,460"); }
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        // tree edges – depth 0 to 1
        root.ethg++ <--> <default("EthernetLink")> like ILink <--> n1.ethg++;
        root.ethg++ <--> <default("EthernetLink")> like ILink <--> n2.ethg++;
        root.ethg++ <--> <default("EthernetLink")> like ILink <--> n3.ethg++;
        // tree edges – depth 1 to 2
        n1.ethg++   <--> <default("EthernetLink")> like ILink <--> n4.ethg++;
        n1.ethg++   <--> <default("EthernetLink")> like ILink <--> n5.ethg++;
        n2.ethg++   <--> <default("EthernetLink")> like ILink <--> n6.ethg++;
        n2.ethg++   <--> <default("EthernetLink")> like ILink <--> n7.ethg++;
        n3.ethg++   <--> <default("EthernetLink")> like ILink <--> n8.ethg++;
        n3.ethg++   <--> <default("EthernetLink")> like ILink <--> n9.ethg++;
        // tree edges – depth 2 to 3
        n4.ethg++   <--> <default("EthernetLink")> like ILink <--> n10.ethg++;
        n4.ethg++   <--> <default("EthernetLink")> like ILink <--> n11.ethg++;
        n6.ethg++   <--> <default("EthernetLink")> like ILink <--> n12.ethg++;
        n8.ethg++   <--> <default("EthernetLink")> like ILink <--> n13.ethg++;
        n8.ethg++   <--> <default("EthernetLink")> like ILink <--> n14.ethg++;
        // cross-links between sibling branches
        n5.ethg++   <--> <default("EthernetLink")> like ILink <--> n6.ethg++;   // n1-subtree ↔ n2-subtree
        n7.ethg++   <--> <default("EthernetLink")> like ILink <--> n8.ethg++;   // n2-subtree ↔ n3-subtree
        n11.ethg++  <--> <default("EthernetLink")> like ILink <--> n12.ethg++;  // leaf cross-link n1 ↔ n2
        // hosts
        for i=0..sizeof(host)-1 {
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> root.ethg++ if groupIndex(numHosts,i) == 0;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n1.ethg++  if groupIndex(numHosts,i) == 1;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n2.ethg++  if groupIndex(numHosts,i) == 2;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n3.ethg++  if groupIndex(numHosts,i) == 3;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n4.ethg++  if groupIndex(numHosts,i) == 4;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n5.ethg++  if groupIndex(numHosts,i) == 5;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n6.ethg++  if groupIndex(numHosts,i) == 6;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n7.ethg++  if groupIndex(numHosts,i) == 7;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n8.ethg++  if groupIndex(numHosts,i) == 8;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n9.ethg++  if groupIndex(numHosts,i) == 9;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n10.ethg++ if groupIndex(numHosts,i) == 10;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n11.ethg++ if groupIndex(numHosts,i) == 11;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n12.ethg++ if groupIndex(numHosts,i) == 12;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n13.ethg++ if groupIndex(numHosts,i) == 13;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n14.ethg++ if groupIndex(numHosts,i) == 14;
        }
}

//
// A larger cross-linked tree with 30 nodes and 35 links.
//
// A deeper and wider version of CrossLinkedTreeNetwork. Four subtrees
// hang from the root, each 3-4 levels deep. Six lateral cross-links
// between sibling branches add path diversity without eliminating
// the hierarchical bottlenecks near the root.
//
// <pre>
//                                  [root]
//                         /      /      \      \
//                      [n1]   [n2]     [n3]    [n4]
//                      / \    / \      / \      / \
//                   [n5][n6--n7][n8--n9][n10--n11][n12]
//                   / \    |  / \   |   / \   / \    |
//               [n13][n14-n15][n16][n17-n18][n19][n20-n21][n22][n23]
//                 |           |           |         |   / \         |
//               [n24]       [n25]-------[n26]     [n27][n28]     [n29]
// </pre>
//
network LargeCrossLinkedTreeNetwork extends WiredNetworkBase
{
    parameters:
        volatile int numHostsPerSwitch = default(0); // hosts attached to every switch
        object numHosts = default(repeatPar("numHostsPerSwitch", 30));
    submodules:
        // depth 0
        root: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=900,50"); }
        // depth 1
        n1:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=450,180"); }
        n2:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=700,180"); }
        n3:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1000,180"); }
        n4:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1300,180"); }
        // depth 2
        n5:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=370,320"); }
        n6:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=530,320"); }
        n7:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=640,320"); }
        n8:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=780,320"); }
        n9:  <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=920,320"); }
        n10: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1070,320"); }
        n11: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1220,320"); }
        n12: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1380,320"); }
        // depth 3
        n13: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=320,460"); }
        n14: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=430,460"); }
        n15: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=590,460"); }
        n16: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=730,460"); }
        n17: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=870,460"); }
        n18: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1010,460"); }
        n19: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1120,460"); }
        n20: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1220,460"); }
        n21: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1320,460"); }
        n22: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1420,460"); }
        n23: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1520,460"); }
        // depth 4
        n24: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=320,600"); }
        n25: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=730,600"); }
        n26: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1010,600"); }
        n27: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1220,600"); }
        n28: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1320,600"); }
        n29: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=1520,600"); }
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        // tree edges – depth 0 to 1
        root.ethg++ <--> <default("EthernetLink")> like ILink <--> n1.ethg++;
        root.ethg++ <--> <default("EthernetLink")> like ILink <--> n2.ethg++;
        root.ethg++ <--> <default("EthernetLink")> like ILink <--> n3.ethg++;
        root.ethg++ <--> <default("EthernetLink")> like ILink <--> n4.ethg++;
        // tree edges – depth 1 to 2
        n1.ethg++   <--> <default("EthernetLink")> like ILink <--> n5.ethg++;
        n1.ethg++   <--> <default("EthernetLink")> like ILink <--> n6.ethg++;
        n2.ethg++   <--> <default("EthernetLink")> like ILink <--> n7.ethg++;
        n2.ethg++   <--> <default("EthernetLink")> like ILink <--> n8.ethg++;
        n3.ethg++   <--> <default("EthernetLink")> like ILink <--> n9.ethg++;
        n3.ethg++   <--> <default("EthernetLink")> like ILink <--> n10.ethg++;
        n4.ethg++   <--> <default("EthernetLink")> like ILink <--> n11.ethg++;
        n4.ethg++   <--> <default("EthernetLink")> like ILink <--> n12.ethg++;
        // tree edges – depth 2 to 3
        n5.ethg++   <--> <default("EthernetLink")> like ILink <--> n13.ethg++;
        n5.ethg++   <--> <default("EthernetLink")> like ILink <--> n14.ethg++;
        n7.ethg++   <--> <default("EthernetLink")> like ILink <--> n15.ethg++;
        n8.ethg++   <--> <default("EthernetLink")> like ILink <--> n16.ethg++;
        n8.ethg++   <--> <default("EthernetLink")> like ILink <--> n17.ethg++;
        n9.ethg++   <--> <default("EthernetLink")> like ILink <--> n18.ethg++;
        n10.ethg++  <--> <default("EthernetLink")> like ILink <--> n19.ethg++;
        n10.ethg++  <--> <default("EthernetLink")> like ILink <--> n20.ethg++;
        n11.ethg++  <--> <default("EthernetLink")> like ILink <--> n21.ethg++;
        n11.ethg++  <--> <default("EthernetLink")> like ILink <--> n22.ethg++;
        n12.ethg++  <--> <default("EthernetLink")> like ILink <--> n23.ethg++;
        // tree edges – depth 3 to 4
        n13.ethg++  <--> <default("EthernetLink")> like ILink <--> n24.ethg++;
        n16.ethg++  <--> <default("EthernetLink")> like ILink <--> n25.ethg++;
        n18.ethg++  <--> <default("EthernetLink")> like ILink <--> n26.ethg++;
        n21.ethg++  <--> <default("EthernetLink")> like ILink <--> n27.ethg++;
        n21.ethg++  <--> <default("EthernetLink")> like ILink <--> n28.ethg++;
        n23.ethg++  <--> <default("EthernetLink")> like ILink <--> n29.ethg++;
        // cross-links between sibling branches
        n6.ethg++   <--> <default("EthernetLink")> like ILink <--> n7.ethg++;    // n1 ↔ n2 subtrees
        n8.ethg++   <--> <default("EthernetLink")> like ILink <--> n9.ethg++;    // n2 ↔ n3 subtrees
        n10.ethg++  <--> <default("EthernetLink")> like ILink <--> n11.ethg++;   // n3 ↔ n4 subtrees
        n14.ethg++  <--> <default("EthernetLink")> like ILink <--> n15.ethg++;   // deep cross n1 ↔ n2
        n17.ethg++  <--> <default("EthernetLink")> like ILink <--> n18.ethg++;   // deep cross n2 ↔ n3
        n25.ethg++  <--> <default("EthernetLink")> like ILink <--> n26.ethg++;   // leaf cross n2 ↔ n3
        // hosts
        for i=0..sizeof(host)-1 {
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> root.ethg++ if groupIndex(numHosts,i) == 0;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n1.ethg++   if groupIndex(numHosts,i) == 1;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n2.ethg++   if groupIndex(numHosts,i) == 2;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n3.ethg++   if groupIndex(numHosts,i) == 3;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n4.ethg++   if groupIndex(numHosts,i) == 4;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n5.ethg++   if groupIndex(numHosts,i) == 5;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n6.ethg++   if groupIndex(numHosts,i) == 6;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n7.ethg++   if groupIndex(numHosts,i) == 7;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n8.ethg++   if groupIndex(numHosts,i) == 8;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n9.ethg++   if groupIndex(numHosts,i) == 9;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n10.ethg++  if groupIndex(numHosts,i) == 10;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n11.ethg++  if groupIndex(numHosts,i) == 11;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n12.ethg++  if groupIndex(numHosts,i) == 12;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n13.ethg++  if groupIndex(numHosts,i) == 13;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n14.ethg++  if groupIndex(numHosts,i) == 14;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n15.ethg++  if groupIndex(numHosts,i) == 15;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n16.ethg++  if groupIndex(numHosts,i) == 16;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n17.ethg++  if groupIndex(numHosts,i) == 17;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n18.ethg++  if groupIndex(numHosts,i) == 18;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n19.ethg++  if groupIndex(numHosts,i) == 19;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n20.ethg++  if groupIndex(numHosts,i) == 20;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n21.ethg++  if groupIndex(numHosts,i) == 21;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n22.ethg++  if groupIndex(numHosts,i) == 22;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n23.ethg++  if groupIndex(numHosts,i) == 23;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n24.ethg++  if groupIndex(numHosts,i) == 24;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n25.ethg++  if groupIndex(numHosts,i) == 25;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n26.ethg++  if groupIndex(numHosts,i) == 26;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n27.ethg++  if groupIndex(numHosts,i) == 27;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n28.ethg++  if groupIndex(numHosts,i) == 28;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> n29.ethg++  if groupIndex(numHosts,i) == 29;
        }
}

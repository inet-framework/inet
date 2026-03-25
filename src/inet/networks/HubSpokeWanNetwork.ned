//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.common.ILink;

//
// A hub-and-spoke WAN with backup rings among hubs (18 nodes, 22 links).
//
// A common WAN-style topology: one central data-center connects to three
// regional hubs that form a ring among themselves. Each hub serves four
// branch/leaf nodes. Two of the branches also have a sub-branch hanging
// off them (sb0 off b3, sb1 off b7). Two backup cross-links (b3–hub1
// and b11–hub0) give selected edge nodes an alternate path through a
// neighboring hub.
//
// Typical of enterprise WAN, ISP access aggregation, or branch-office
// networks.
//
// Properties: highly heterogeneous degree distribution, strong hierarchy,
// hub-dominated bottlenecks, edge fragility paired with core resilience,
// low path diversity at the edge, moderate in the core.
//
// <pre>
//                        [dc]
//                      /  |  \
//      [b0]          /    |    \          [b8]
//      [b1]--[hub0]--     |     --[hub2]--[b9]
//      [b2]   /  \   \    |    /   /  \   [b10]
//           [b3]  '---[hub1]---' [b11]
//            |       / /    \ \
//          [sb0]  [b4][b5][b6][b7]
//                               |
//                             [sb1]
//
//   Backup links: b3--hub1, b11--hub0
// </pre>
//
network HubSpokeWanNetwork extends WiredNetworkBase
{
    parameters:
        volatile int numHostsPerSwitch = default(0); // hosts attached to every switch
        object numHosts = default(repeatPar("numHostsPerSwitch", 18));
    submodules:
        // central data-center
        dc:   <default("Router")> like IEthernetNetworkNode { @display("p=700,50"); }
        // regional hubs
        hub0: <default("Router")> like IEthernetNetworkNode { @display("p=400,250"); }
        hub1: <default("Router")> like IEthernetNetworkNode { @display("p=700,400"); }
        hub2: <default("Router")> like IEthernetNetworkNode { @display("p=1000,250"); }
        // hub0 branches (left)
        b0:   <default("Router")> like IEthernetNetworkNode { @display("p=280,100"); }
        b1:   <default("Router")> like IEthernetNetworkNode { @display("p=280,200"); }
        b2:   <default("Router")> like IEthernetNetworkNode { @display("p=280,300"); }
        b3:   <default("Router")> like IEthernetNetworkNode { @display("p=470,400"); }
        // sub-branch off b3
        sb0:  <default("Router")> like IEthernetNetworkNode { @display("p=470,540"); }
        // hub1 branches (bottom)
        b4:   <default("Router")> like IEthernetNetworkNode { @display("p=550,550"); }
        b5:   <default("Router")> like IEthernetNetworkNode { @display("p=650,580"); }
        b6:   <default("Router")> like IEthernetNetworkNode { @display("p=750,580"); }
        b7:   <default("Router")> like IEthernetNetworkNode { @display("p=850,550"); }
        // sub-branch off b7
        sb1:  <default("Router")> like IEthernetNetworkNode { @display("p=910,670"); }
        // hub2 branches (right)
        b8:   <default("Router")> like IEthernetNetworkNode { @display("p=1120,100"); }
        b9:   <default("Router")> like IEthernetNetworkNode { @display("p=1120,200"); }
        b10:  <default("Router")> like IEthernetNetworkNode { @display("p=1120,300"); }
        b11:  <default("Router")> like IEthernetNetworkNode { @display("p=930,400"); }
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        // data-center to hubs
        dc.ethg++   <--> <default("EthernetLink")> like ILink <--> hub0.ethg++;
        dc.ethg++   <--> <default("EthernetLink")> like ILink <--> hub1.ethg++;
        dc.ethg++   <--> <default("EthernetLink")> like ILink <--> hub2.ethg++;
        // hub ring
        hub0.ethg++ <--> <default("EthernetLink")> like ILink <--> hub1.ethg++;
        hub1.ethg++ <--> <default("EthernetLink")> like ILink <--> hub2.ethg++;
        hub2.ethg++ <--> <default("EthernetLink")> like ILink <--> hub0.ethg++;
        // hub0 spokes
        hub0.ethg++ <--> <default("EthernetLink")> like ILink <--> b0.ethg++;
        hub0.ethg++ <--> <default("EthernetLink")> like ILink <--> b1.ethg++;
        hub0.ethg++ <--> <default("EthernetLink")> like ILink <--> b2.ethg++;
        hub0.ethg++ <--> <default("EthernetLink")> like ILink <--> b3.ethg++;
        // hub1 spokes
        hub1.ethg++ <--> <default("EthernetLink")> like ILink <--> b4.ethg++;
        hub1.ethg++ <--> <default("EthernetLink")> like ILink <--> b5.ethg++;
        hub1.ethg++ <--> <default("EthernetLink")> like ILink <--> b6.ethg++;
        hub1.ethg++ <--> <default("EthernetLink")> like ILink <--> b7.ethg++;
        // hub2 spokes
        hub2.ethg++ <--> <default("EthernetLink")> like ILink <--> b8.ethg++;
        hub2.ethg++ <--> <default("EthernetLink")> like ILink <--> b9.ethg++;
        hub2.ethg++ <--> <default("EthernetLink")> like ILink <--> b10.ethg++;
        hub2.ethg++ <--> <default("EthernetLink")> like ILink <--> b11.ethg++;
        // sub-branches
        b3.ethg++   <--> <default("EthernetLink")> like ILink <--> sb0.ethg++;
        b7.ethg++   <--> <default("EthernetLink")> like ILink <--> sb1.ethg++;
        // backup cross-links (edge node to neighboring hub)
        b3.ethg++   <--> <default("EthernetLink")> like ILink <--> hub1.ethg++;  // hub0 branch reachable via hub1
        b11.ethg++  <--> <default("EthernetLink")> like ILink <--> hub0.ethg++;  // hub2 branch reachable via hub0
        // hosts
        for i=0..sizeof(host)-1 {
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> dc.ethg++   if groupIndex(numHosts,i) == 0;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> hub0.ethg++ if groupIndex(numHosts,i) == 1;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> hub1.ethg++ if groupIndex(numHosts,i) == 2;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> hub2.ethg++ if groupIndex(numHosts,i) == 3;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b0.ethg++   if groupIndex(numHosts,i) == 4;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b1.ethg++   if groupIndex(numHosts,i) == 5;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b2.ethg++   if groupIndex(numHosts,i) == 6;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b3.ethg++   if groupIndex(numHosts,i) == 7;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> sb0.ethg++  if groupIndex(numHosts,i) == 8;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b4.ethg++   if groupIndex(numHosts,i) == 9;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b5.ethg++   if groupIndex(numHosts,i) == 10;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b6.ethg++   if groupIndex(numHosts,i) == 11;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b7.ethg++   if groupIndex(numHosts,i) == 12;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> sb1.ethg++  if groupIndex(numHosts,i) == 13;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b8.ethg++   if groupIndex(numHosts,i) == 14;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b9.ethg++   if groupIndex(numHosts,i) == 15;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b10.ethg++  if groupIndex(numHosts,i) == 16;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b11.ethg++  if groupIndex(numHosts,i) == 17;
        }
}

//
// A larger hub-and-spoke WAN with 36 nodes and 45 links.
//
// Scales up HubSpokeWanNetwork from 3 to 5 regional hubs. Each hub
// serves four branches, and two of those branches carry a sub-branch.
// The five hubs form a ring among themselves, plus a star connection
// to the central data-center. Five backup cross-links give selected
// edge nodes an alternate path through a neighboring hub.
//
// <pre>
//                           [dc]
//                    /    /  |  \    \
//                 [hub0] [hub1] [hub2] [hub3] [hub4]
//                  ring: hub0-hub1-hub2-hub3-hub4-hub0
//
//   hub0: b0,b1,b2,b3   sb0(off b1) sb1(off b3)
//   hub1: b4,b5,b6,b7   sb2(off b5) sb3(off b7)
//   hub2: b8,b9,b10,b11  sb4(off b9) sb5(off b11)
//   hub3: b12,b13,b14,b15 sb6(off b13) sb7(off b15)
//   hub4: b16,b17,b18,b19 sb8(off b17) sb9(off b19)
//
//   Backup: b3-hub1, b7-hub2, b11-hub3, b15-hub4, b19-hub0
// </pre>
//
network LargeHubSpokeWanNetwork extends WiredNetworkBase
{
    parameters:
        volatile int numHostsPerSwitch = default(0); // hosts attached to every switch
        object numHosts = default(repeatPar("numHostsPerSwitch", 36));
    submodules:
        // central data-center
        dc:   <default("Router")> like IEthernetNetworkNode { @display("p=800,50"); }
        // regional hubs (pentagon layout)
        hub0: <default("Router")> like IEthernetNetworkNode { @display("p=400,250"); }
        hub1: <default("Router")> like IEthernetNetworkNode { @display("p=600,500"); }
        hub2: <default("Router")> like IEthernetNetworkNode { @display("p=800,600"); }
        hub3: <default("Router")> like IEthernetNetworkNode { @display("p=1000,500"); }
        hub4: <default("Router")> like IEthernetNetworkNode { @display("p=1200,250"); }
        // hub0 branches
        b0:   <default("Router")> like IEthernetNetworkNode { @display("p=260,120"); }
        b1:   <default("Router")> like IEthernetNetworkNode { @display("p=260,220"); }
        b2:   <default("Router")> like IEthernetNetworkNode { @display("p=260,320"); }
        b3:   <default("Router")> like IEthernetNetworkNode { @display("p=400,420"); }
        sb0:  <default("Router")> like IEthernetNetworkNode { @display("p=220,270"); }
        sb1:  <default("Router")> like IEthernetNetworkNode { @display("p=400,560"); }
        // hub1 branches
        b4:   <default("Router")> like IEthernetNetworkNode { @display("p=460,580"); }
        b5:   <default("Router")> like IEthernetNetworkNode { @display("p=500,670"); }
        b6:   <default("Router")> like IEthernetNetworkNode { @display("p=620,670"); }
        b7:   <default("Router")> like IEthernetNetworkNode { @display("p=680,620"); }
        sb2:  <default("Router")> like IEthernetNetworkNode { @display("p=430,770"); }
        sb3:  <default("Router")> like IEthernetNetworkNode { @display("p=740,720"); }
        // hub2 branches
        b8:   <default("Router")> like IEthernetNetworkNode { @display("p=700,730"); }
        b9:   <default("Router")> like IEthernetNetworkNode { @display("p=760,790"); }
        b10:  <default("Router")> like IEthernetNetworkNode { @display("p=860,790"); }
        b11:  <default("Router")> like IEthernetNetworkNode { @display("p=920,730"); }
        sb4:  <default("Router")> like IEthernetNetworkNode { @display("p=720,890"); }
        sb5:  <default("Router")> like IEthernetNetworkNode { @display("p=960,830"); }
        // hub3 branches
        b12:  <default("Router")> like IEthernetNetworkNode { @display("p=950,620"); }
        b13:  <default("Router")> like IEthernetNetworkNode { @display("p=1020,670"); }
        b14:  <default("Router")> like IEthernetNetworkNode { @display("p=1120,670"); }
        b15:  <default("Router")> like IEthernetNetworkNode { @display("p=1160,580"); }
        sb6:  <default("Router")> like IEthernetNetworkNode { @display("p=1080,770"); }
        sb7:  <default("Router")> like IEthernetNetworkNode { @display("p=1240,680"); }
        // hub4 branches
        b16:  <default("Router")> like IEthernetNetworkNode { @display("p=1340,120"); }
        b17:  <default("Router")> like IEthernetNetworkNode { @display("p=1340,220"); }
        b18:  <default("Router")> like IEthernetNetworkNode { @display("p=1340,320"); }
        b19:  <default("Router")> like IEthernetNetworkNode { @display("p=1200,420"); }
        sb8:  <default("Router")> like IEthernetNetworkNode { @display("p=1380,270"); }
        sb9:  <default("Router")> like IEthernetNetworkNode { @display("p=1200,560"); }
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        // data-center to hubs
        dc.ethg++   <--> <default("EthernetLink")> like ILink <--> hub0.ethg++;
        dc.ethg++   <--> <default("EthernetLink")> like ILink <--> hub1.ethg++;
        dc.ethg++   <--> <default("EthernetLink")> like ILink <--> hub2.ethg++;
        dc.ethg++   <--> <default("EthernetLink")> like ILink <--> hub3.ethg++;
        dc.ethg++   <--> <default("EthernetLink")> like ILink <--> hub4.ethg++;
        // hub ring
        hub0.ethg++ <--> <default("EthernetLink")> like ILink <--> hub1.ethg++;
        hub1.ethg++ <--> <default("EthernetLink")> like ILink <--> hub2.ethg++;
        hub2.ethg++ <--> <default("EthernetLink")> like ILink <--> hub3.ethg++;
        hub3.ethg++ <--> <default("EthernetLink")> like ILink <--> hub4.ethg++;
        hub4.ethg++ <--> <default("EthernetLink")> like ILink <--> hub0.ethg++;
        // hub0 spokes
        hub0.ethg++ <--> <default("EthernetLink")> like ILink <--> b0.ethg++;
        hub0.ethg++ <--> <default("EthernetLink")> like ILink <--> b1.ethg++;
        hub0.ethg++ <--> <default("EthernetLink")> like ILink <--> b2.ethg++;
        hub0.ethg++ <--> <default("EthernetLink")> like ILink <--> b3.ethg++;
        // hub1 spokes
        hub1.ethg++ <--> <default("EthernetLink")> like ILink <--> b4.ethg++;
        hub1.ethg++ <--> <default("EthernetLink")> like ILink <--> b5.ethg++;
        hub1.ethg++ <--> <default("EthernetLink")> like ILink <--> b6.ethg++;
        hub1.ethg++ <--> <default("EthernetLink")> like ILink <--> b7.ethg++;
        // hub2 spokes
        hub2.ethg++ <--> <default("EthernetLink")> like ILink <--> b8.ethg++;
        hub2.ethg++ <--> <default("EthernetLink")> like ILink <--> b9.ethg++;
        hub2.ethg++ <--> <default("EthernetLink")> like ILink <--> b10.ethg++;
        hub2.ethg++ <--> <default("EthernetLink")> like ILink <--> b11.ethg++;
        // hub3 spokes
        hub3.ethg++ <--> <default("EthernetLink")> like ILink <--> b12.ethg++;
        hub3.ethg++ <--> <default("EthernetLink")> like ILink <--> b13.ethg++;
        hub3.ethg++ <--> <default("EthernetLink")> like ILink <--> b14.ethg++;
        hub3.ethg++ <--> <default("EthernetLink")> like ILink <--> b15.ethg++;
        // hub4 spokes
        hub4.ethg++ <--> <default("EthernetLink")> like ILink <--> b16.ethg++;
        hub4.ethg++ <--> <default("EthernetLink")> like ILink <--> b17.ethg++;
        hub4.ethg++ <--> <default("EthernetLink")> like ILink <--> b18.ethg++;
        hub4.ethg++ <--> <default("EthernetLink")> like ILink <--> b19.ethg++;
        // sub-branches
        b1.ethg++   <--> <default("EthernetLink")> like ILink <--> sb0.ethg++;
        b3.ethg++   <--> <default("EthernetLink")> like ILink <--> sb1.ethg++;
        b5.ethg++   <--> <default("EthernetLink")> like ILink <--> sb2.ethg++;
        b7.ethg++   <--> <default("EthernetLink")> like ILink <--> sb3.ethg++;
        b9.ethg++   <--> <default("EthernetLink")> like ILink <--> sb4.ethg++;
        b11.ethg++  <--> <default("EthernetLink")> like ILink <--> sb5.ethg++;
        b13.ethg++  <--> <default("EthernetLink")> like ILink <--> sb6.ethg++;
        b15.ethg++  <--> <default("EthernetLink")> like ILink <--> sb7.ethg++;
        b17.ethg++  <--> <default("EthernetLink")> like ILink <--> sb8.ethg++;
        b19.ethg++  <--> <default("EthernetLink")> like ILink <--> sb9.ethg++;
        // backup cross-links (edge node to neighboring hub)
        b3.ethg++   <--> <default("EthernetLink")> like ILink <--> hub1.ethg++;
        b7.ethg++   <--> <default("EthernetLink")> like ILink <--> hub2.ethg++;
        b11.ethg++  <--> <default("EthernetLink")> like ILink <--> hub3.ethg++;
        b15.ethg++  <--> <default("EthernetLink")> like ILink <--> hub4.ethg++;
        b19.ethg++  <--> <default("EthernetLink")> like ILink <--> hub0.ethg++;
        // hosts
        for i=0..sizeof(host)-1 {
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> dc.ethg++   if groupIndex(numHosts,i) == 0;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> hub0.ethg++ if groupIndex(numHosts,i) == 1;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> hub1.ethg++ if groupIndex(numHosts,i) == 2;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> hub2.ethg++ if groupIndex(numHosts,i) == 3;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> hub3.ethg++ if groupIndex(numHosts,i) == 4;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> hub4.ethg++ if groupIndex(numHosts,i) == 5;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b0.ethg++   if groupIndex(numHosts,i) == 6;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b1.ethg++   if groupIndex(numHosts,i) == 7;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b2.ethg++   if groupIndex(numHosts,i) == 8;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b3.ethg++   if groupIndex(numHosts,i) == 9;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> sb0.ethg++  if groupIndex(numHosts,i) == 10;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> sb1.ethg++  if groupIndex(numHosts,i) == 11;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b4.ethg++   if groupIndex(numHosts,i) == 12;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b5.ethg++   if groupIndex(numHosts,i) == 13;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b6.ethg++   if groupIndex(numHosts,i) == 14;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b7.ethg++   if groupIndex(numHosts,i) == 15;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> sb2.ethg++  if groupIndex(numHosts,i) == 16;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> sb3.ethg++  if groupIndex(numHosts,i) == 17;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b8.ethg++   if groupIndex(numHosts,i) == 18;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b9.ethg++   if groupIndex(numHosts,i) == 19;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b10.ethg++  if groupIndex(numHosts,i) == 20;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b11.ethg++  if groupIndex(numHosts,i) == 21;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> sb4.ethg++  if groupIndex(numHosts,i) == 22;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> sb5.ethg++  if groupIndex(numHosts,i) == 23;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b12.ethg++  if groupIndex(numHosts,i) == 24;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b13.ethg++  if groupIndex(numHosts,i) == 25;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b14.ethg++  if groupIndex(numHosts,i) == 26;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b15.ethg++  if groupIndex(numHosts,i) == 27;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> sb6.ethg++  if groupIndex(numHosts,i) == 28;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> sb7.ethg++  if groupIndex(numHosts,i) == 29;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b16.ethg++  if groupIndex(numHosts,i) == 30;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b17.ethg++  if groupIndex(numHosts,i) == 31;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b18.ethg++  if groupIndex(numHosts,i) == 32;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> b19.ethg++  if groupIndex(numHosts,i) == 33;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> sb8.ethg++  if groupIndex(numHosts,i) == 34;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> sb9.ethg++  if groupIndex(numHosts,i) == 35;
        }
}

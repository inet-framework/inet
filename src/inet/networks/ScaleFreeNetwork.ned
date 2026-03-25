//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.common.ILink;

//
// Scale-free / hub-dominated organic growth network (20 nodes, ~28 links).
//
// A network shaped by preferential attachment: many low-degree leaf
// nodes, a few massive hubs, and medium-degree connectors. Not a
// clean hub-and-spoke; it is more organic and uneven. Useful for
// studying robustness to random failures versus targeted attacks,
// hub congestion, and epidemic spread.
//
// Hub nodes have fixed positions; connectors and leaves auto-layout.
//
network ScaleFreeNetwork extends WiredNetworkBase
{
    parameters:
        volatile int numHostsPerHub = default(0);
        volatile int numHostsPerConnector = default(0);
        volatile int numHostsPerLeaf = default(0);
        object numHostsH = default(repeatPar("numHostsPerHub", 3));
        object numHostsN = default(repeatPar("numHostsPerConnector", 5));
        object numHostsL = default(repeatPar("numHostsPerLeaf", 12));
    submodules:
        // Hubs (high degree, positioned)
        hub0: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=400,250"); }
        hub1: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=700,200"); }
        hub2: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=550,400"); }
        // Connectors (medium degree, auto-layout)
        conn0: <default("EthernetSwitch")> like IEthernetNetworkNode;
        conn1: <default("EthernetSwitch")> like IEthernetNetworkNode;
        conn2: <default("EthernetSwitch")> like IEthernetNetworkNode;
        conn3: <default("EthernetSwitch")> like IEthernetNetworkNode;
        conn4: <default("EthernetSwitch")> like IEthernetNetworkNode;
        // Leaves (low degree, auto-layout)
        leaf0:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf1:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf2:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf3:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf4:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf5:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf6:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf7:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf8:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf9:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf10: <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf11: <default("EthernetSwitch")> like IEthernetNetworkNode;
        // Dangling hosts
        hostH[sum(numHostsH)]: <default("StandardHost")> like IEthernetNetworkNode {}
        hostN[sum(numHostsN)]: <default("StandardHost")> like IEthernetNetworkNode {}
        hostL[sum(numHostsL)]: <default("StandardHost")> like IEthernetNetworkNode {}
    connections:
        // Hub interconnections (sparse, not full mesh)
        hub0.ethg++ <--> <default("EthernetLink")> like ILink <--> hub1.ethg++;
        hub0.ethg++ <--> <default("EthernetLink")> like ILink <--> hub2.ethg++;
        hub1.ethg++ <--> <default("EthernetLink")> like ILink <--> hub2.ethg++;
        // Hub0 spokes (degree ~7)
        hub0.ethg++ <--> <default("EthernetLink")> like ILink <--> conn0.ethg++;
        hub0.ethg++ <--> <default("EthernetLink")> like ILink <--> conn1.ethg++;
        hub0.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf0.ethg++;
        hub0.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf1.ethg++;
        // Hub1 spokes (degree ~8)
        hub1.ethg++ <--> <default("EthernetLink")> like ILink <--> conn2.ethg++;
        hub1.ethg++ <--> <default("EthernetLink")> like ILink <--> conn3.ethg++;
        hub1.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf2.ethg++;
        hub1.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf3.ethg++;
        hub1.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf4.ethg++;
        // Hub2 spokes (degree ~6)
        hub2.ethg++ <--> <default("EthernetLink")> like ILink <--> conn4.ethg++;
        hub2.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf5.ethg++;
        hub2.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf6.ethg++;
        // Connector chains and branches
        conn0.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf7.ethg++;
        conn0.ethg++ <--> <default("EthernetLink")> like ILink <--> conn2.ethg++;
        conn1.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf8.ethg++;
        conn1.ethg++ <--> <default("EthernetLink")> like ILink <--> conn4.ethg++;
        conn2.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf9.ethg++;
        conn3.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf10.ethg++;
        conn3.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf11.ethg++;
        conn4.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf11.ethg++;
        // Hosts on hubs
        for i=0..sizeof(hostH)-1 {
            hostH[i].ethg++ <--> <default("EthernetLink")> like ILink <--> hub0.ethg++ if groupIndex(numHostsH,i) == 0;
            hostH[i].ethg++ <--> <default("EthernetLink")> like ILink <--> hub1.ethg++ if groupIndex(numHostsH,i) == 1;
            hostH[i].ethg++ <--> <default("EthernetLink")> like ILink <--> hub2.ethg++ if groupIndex(numHostsH,i) == 2;
        }
        // Hosts on connectors
        for i=0..sizeof(hostN)-1 {
            hostN[i].ethg++ <--> <default("EthernetLink")> like ILink <--> conn0.ethg++ if groupIndex(numHostsN,i) == 0;
            hostN[i].ethg++ <--> <default("EthernetLink")> like ILink <--> conn1.ethg++ if groupIndex(numHostsN,i) == 1;
            hostN[i].ethg++ <--> <default("EthernetLink")> like ILink <--> conn2.ethg++ if groupIndex(numHostsN,i) == 2;
            hostN[i].ethg++ <--> <default("EthernetLink")> like ILink <--> conn3.ethg++ if groupIndex(numHostsN,i) == 3;
            hostN[i].ethg++ <--> <default("EthernetLink")> like ILink <--> conn4.ethg++ if groupIndex(numHostsN,i) == 4;
        }
        // Hosts on leaves
        for i=0..sizeof(hostL)-1 {
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf0.ethg++  if groupIndex(numHostsL,i) == 0;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf1.ethg++  if groupIndex(numHostsL,i) == 1;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf2.ethg++  if groupIndex(numHostsL,i) == 2;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf3.ethg++  if groupIndex(numHostsL,i) == 3;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf4.ethg++  if groupIndex(numHostsL,i) == 4;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf5.ethg++  if groupIndex(numHostsL,i) == 5;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf6.ethg++  if groupIndex(numHostsL,i) == 6;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf7.ethg++  if groupIndex(numHostsL,i) == 7;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf8.ethg++  if groupIndex(numHostsL,i) == 8;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf9.ethg++  if groupIndex(numHostsL,i) == 9;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf10.ethg++ if groupIndex(numHostsL,i) == 10;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf11.ethg++ if groupIndex(numHostsL,i) == 11;
        }
}

//
// Large scale-free / hub-dominated organic growth network (50 nodes, ~70 links).
//
// A larger version with 4 massive hubs, 10 medium-degree connectors,
// and 36 low-degree leaves. Heavy-tailed degree distribution: hubs
// have degree 10-14, connectors 3-5, leaves 1-2. Useful for studying
// robustness, hub congestion, epidemic spread, and uneven centrality.
//
network LargeScaleFreeNetwork extends WiredNetworkBase
{
    parameters:
        volatile int numHostsPerHub = default(0);
        volatile int numHostsPerConnector = default(0);
        volatile int numHostsPerLeaf = default(0);
        object numHostsH = default(repeatPar("numHostsPerHub", 4));
        object numHostsN = default(repeatPar("numHostsPerConnector", 10));
        object numHostsL = default(repeatPar("numHostsPerLeaf", 36));
    submodules:
        // Hubs (4, high degree, positioned)
        hub0: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=400,200"); }
        hub1: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=800,200"); }
        hub2: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=500,450"); }
        hub3: <default("EthernetSwitch")> like IEthernetNetworkNode { @display("p=700,450"); }
        // Connectors (10, medium degree, auto-layout)
        conn0:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        conn1:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        conn2:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        conn3:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        conn4:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        conn5:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        conn6:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        conn7:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        conn8:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        conn9:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        // Leaves (36, low degree, auto-layout)
        leaf0:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf1:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf2:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf3:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf4:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf5:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf6:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf7:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf8:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf9:  <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf10: <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf11: <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf12: <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf13: <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf14: <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf15: <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf16: <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf17: <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf18: <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf19: <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf20: <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf21: <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf22: <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf23: <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf24: <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf25: <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf26: <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf27: <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf28: <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf29: <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf30: <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf31: <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf32: <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf33: <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf34: <default("EthernetSwitch")> like IEthernetNetworkNode;
        leaf35: <default("EthernetSwitch")> like IEthernetNetworkNode;
        // Dangling hosts
        hostH[sum(numHostsH)]: <default("StandardHost")> like IEthernetNetworkNode {}
        hostN[sum(numHostsN)]: <default("StandardHost")> like IEthernetNetworkNode {}
        hostL[sum(numHostsL)]: <default("StandardHost")> like IEthernetNetworkNode {}
    connections:
        // Hub mesh (not full — organic)
        hub0.ethg++ <--> <default("EthernetLink")> like ILink <--> hub1.ethg++;
        hub0.ethg++ <--> <default("EthernetLink")> like ILink <--> hub2.ethg++;
        hub1.ethg++ <--> <default("EthernetLink")> like ILink <--> hub3.ethg++;
        hub2.ethg++ <--> <default("EthernetLink")> like ILink <--> hub3.ethg++;
        hub0.ethg++ <--> <default("EthernetLink")> like ILink <--> hub3.ethg++;
        // Hub0 spokes (degree ~12)
        hub0.ethg++ <--> <default("EthernetLink")> like ILink <--> conn0.ethg++;
        hub0.ethg++ <--> <default("EthernetLink")> like ILink <--> conn1.ethg++;
        hub0.ethg++ <--> <default("EthernetLink")> like ILink <--> conn2.ethg++;
        hub0.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf0.ethg++;
        hub0.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf1.ethg++;
        hub0.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf2.ethg++;
        hub0.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf3.ethg++;
        // Hub1 spokes (degree ~14)
        hub1.ethg++ <--> <default("EthernetLink")> like ILink <--> conn3.ethg++;
        hub1.ethg++ <--> <default("EthernetLink")> like ILink <--> conn4.ethg++;
        hub1.ethg++ <--> <default("EthernetLink")> like ILink <--> conn5.ethg++;
        hub1.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf4.ethg++;
        hub1.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf5.ethg++;
        hub1.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf6.ethg++;
        hub1.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf7.ethg++;
        hub1.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf8.ethg++;
        hub1.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf9.ethg++;
        // Hub2 spokes (degree ~10)
        hub2.ethg++ <--> <default("EthernetLink")> like ILink <--> conn6.ethg++;
        hub2.ethg++ <--> <default("EthernetLink")> like ILink <--> conn7.ethg++;
        hub2.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf10.ethg++;
        hub2.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf11.ethg++;
        hub2.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf12.ethg++;
        hub2.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf13.ethg++;
        // Hub3 spokes (degree ~11)
        hub3.ethg++ <--> <default("EthernetLink")> like ILink <--> conn8.ethg++;
        hub3.ethg++ <--> <default("EthernetLink")> like ILink <--> conn9.ethg++;
        hub3.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf14.ethg++;
        hub3.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf15.ethg++;
        hub3.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf16.ethg++;
        hub3.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf17.ethg++;
        // Connector branches (each degree 3-5)
        conn0.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf18.ethg++;
        conn0.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf19.ethg++;
        conn1.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf20.ethg++;
        conn1.ethg++ <--> <default("EthernetLink")> like ILink <--> conn6.ethg++;
        conn2.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf21.ethg++;
        conn2.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf22.ethg++;
        conn2.ethg++ <--> <default("EthernetLink")> like ILink <--> conn3.ethg++;
        conn3.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf23.ethg++;
        conn4.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf24.ethg++;
        conn4.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf25.ethg++;
        conn4.ethg++ <--> <default("EthernetLink")> like ILink <--> conn8.ethg++;
        conn5.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf26.ethg++;
        conn5.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf27.ethg++;
        conn6.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf28.ethg++;
        conn7.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf29.ethg++;
        conn7.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf30.ethg++;
        conn7.ethg++ <--> <default("EthernetLink")> like ILink <--> conn9.ethg++;
        conn8.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf31.ethg++;
        conn8.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf32.ethg++;
        conn9.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf33.ethg++;
        conn9.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf34.ethg++;
        conn9.ethg++ <--> <default("EthernetLink")> like ILink <--> leaf35.ethg++;
        // Hosts on hubs
        for i=0..sizeof(hostH)-1 {
            hostH[i].ethg++ <--> <default("EthernetLink")> like ILink <--> hub0.ethg++ if groupIndex(numHostsH,i) == 0;
            hostH[i].ethg++ <--> <default("EthernetLink")> like ILink <--> hub1.ethg++ if groupIndex(numHostsH,i) == 1;
            hostH[i].ethg++ <--> <default("EthernetLink")> like ILink <--> hub2.ethg++ if groupIndex(numHostsH,i) == 2;
            hostH[i].ethg++ <--> <default("EthernetLink")> like ILink <--> hub3.ethg++ if groupIndex(numHostsH,i) == 3;
        }
        // Hosts on connectors
        for i=0..sizeof(hostN)-1 {
            hostN[i].ethg++ <--> <default("EthernetLink")> like ILink <--> conn0.ethg++ if groupIndex(numHostsN,i) == 0;
            hostN[i].ethg++ <--> <default("EthernetLink")> like ILink <--> conn1.ethg++ if groupIndex(numHostsN,i) == 1;
            hostN[i].ethg++ <--> <default("EthernetLink")> like ILink <--> conn2.ethg++ if groupIndex(numHostsN,i) == 2;
            hostN[i].ethg++ <--> <default("EthernetLink")> like ILink <--> conn3.ethg++ if groupIndex(numHostsN,i) == 3;
            hostN[i].ethg++ <--> <default("EthernetLink")> like ILink <--> conn4.ethg++ if groupIndex(numHostsN,i) == 4;
            hostN[i].ethg++ <--> <default("EthernetLink")> like ILink <--> conn5.ethg++ if groupIndex(numHostsN,i) == 5;
            hostN[i].ethg++ <--> <default("EthernetLink")> like ILink <--> conn6.ethg++ if groupIndex(numHostsN,i) == 6;
            hostN[i].ethg++ <--> <default("EthernetLink")> like ILink <--> conn7.ethg++ if groupIndex(numHostsN,i) == 7;
            hostN[i].ethg++ <--> <default("EthernetLink")> like ILink <--> conn8.ethg++ if groupIndex(numHostsN,i) == 8;
            hostN[i].ethg++ <--> <default("EthernetLink")> like ILink <--> conn9.ethg++ if groupIndex(numHostsN,i) == 9;
        }
        // Hosts on leaves
        for i=0..sizeof(hostL)-1 {
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf0.ethg++  if groupIndex(numHostsL,i) == 0;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf1.ethg++  if groupIndex(numHostsL,i) == 1;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf2.ethg++  if groupIndex(numHostsL,i) == 2;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf3.ethg++  if groupIndex(numHostsL,i) == 3;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf4.ethg++  if groupIndex(numHostsL,i) == 4;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf5.ethg++  if groupIndex(numHostsL,i) == 5;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf6.ethg++  if groupIndex(numHostsL,i) == 6;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf7.ethg++  if groupIndex(numHostsL,i) == 7;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf8.ethg++  if groupIndex(numHostsL,i) == 8;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf9.ethg++  if groupIndex(numHostsL,i) == 9;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf10.ethg++ if groupIndex(numHostsL,i) == 10;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf11.ethg++ if groupIndex(numHostsL,i) == 11;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf12.ethg++ if groupIndex(numHostsL,i) == 12;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf13.ethg++ if groupIndex(numHostsL,i) == 13;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf14.ethg++ if groupIndex(numHostsL,i) == 14;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf15.ethg++ if groupIndex(numHostsL,i) == 15;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf16.ethg++ if groupIndex(numHostsL,i) == 16;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf17.ethg++ if groupIndex(numHostsL,i) == 17;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf18.ethg++ if groupIndex(numHostsL,i) == 18;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf19.ethg++ if groupIndex(numHostsL,i) == 19;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf20.ethg++ if groupIndex(numHostsL,i) == 20;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf21.ethg++ if groupIndex(numHostsL,i) == 21;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf22.ethg++ if groupIndex(numHostsL,i) == 22;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf23.ethg++ if groupIndex(numHostsL,i) == 23;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf24.ethg++ if groupIndex(numHostsL,i) == 24;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf25.ethg++ if groupIndex(numHostsL,i) == 25;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf26.ethg++ if groupIndex(numHostsL,i) == 26;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf27.ethg++ if groupIndex(numHostsL,i) == 27;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf28.ethg++ if groupIndex(numHostsL,i) == 28;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf29.ethg++ if groupIndex(numHostsL,i) == 29;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf30.ethg++ if groupIndex(numHostsL,i) == 30;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf31.ethg++ if groupIndex(numHostsL,i) == 31;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf32.ethg++ if groupIndex(numHostsL,i) == 32;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf33.ethg++ if groupIndex(numHostsL,i) == 33;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf34.ethg++ if groupIndex(numHostsL,i) == 34;
            hostL[i].ethg++ <--> <default("EthernetLink")> like ILink <--> leaf35.ethg++ if groupIndex(numHostsL,i) == 35;
        }
}

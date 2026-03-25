//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.common.ILink;

//
// Partial core mesh with irregular access trees (20 nodes).
//
// A realistic "carrier backbone plus messy edge" topology.
// A small, richly connected core (4 nodes), an aggregation ring (4 nodes),
// and irregular access branches (12 nodes) including dangling trees,
// dual-homed sites, and odd one-off links.
//
// Core nodes have fixed positions; access nodes use auto-layout.
//
network CoreAccessNetwork extends WiredNetworkBase
{
    parameters:
        volatile int numHostsPerCore = default(0);
        volatile int numHostsPerAggregation = default(0);
        volatile int numHostsPerAccess = default(0);
        object numHostsC = default(repeatPar("numHostsPerCore", 4));
        object numHostsG = default(repeatPar("numHostsPerAggregation", 4));
        object numHostsA = default(repeatPar("numHostsPerAccess", 12));
    submodules:
        // Core (fully meshed)
        core0: <default("Router")> like IEthernetNetworkNode { @display("p=600,200"); }
        core1: <default("Router")> like IEthernetNetworkNode { @display("p=800,200"); }
        core2: <default("Router")> like IEthernetNetworkNode { @display("p=600,350"); }
        core3: <default("Router")> like IEthernetNetworkNode { @display("p=800,350"); }
        // Aggregation
        agg0:  <default("Router")> like IEthernetNetworkNode { @display("p=450,120"); }
        agg1:  <default("Router")> like IEthernetNetworkNode { @display("p=950,120"); }
        agg2:  <default("Router")> like IEthernetNetworkNode { @display("p=450,430"); }
        agg3:  <default("Router")> like IEthernetNetworkNode { @display("p=950,430"); }
        // Access (auto-layout)
        acc0:  <default("Router")> like IEthernetNetworkNode;
        acc1:  <default("Router")> like IEthernetNetworkNode;
        acc2:  <default("Router")> like IEthernetNetworkNode;
        acc3:  <default("Router")> like IEthernetNetworkNode;
        acc4:  <default("Router")> like IEthernetNetworkNode;
        acc5:  <default("Router")> like IEthernetNetworkNode;
        acc6:  <default("Router")> like IEthernetNetworkNode;
        acc7:  <default("Router")> like IEthernetNetworkNode;
        acc8:  <default("Router")> like IEthernetNetworkNode;
        acc9:  <default("Router")> like IEthernetNetworkNode;
        acc10: <default("Router")> like IEthernetNetworkNode;
        acc11: <default("Router")> like IEthernetNetworkNode;
        // Dangling hosts
        hostC[sum(numHostsC)]: <default("StandardHost")> like IEthernetNetworkNode {}
        hostG[sum(numHostsG)]: <default("StandardHost")> like IEthernetNetworkNode {}
        hostA[sum(numHostsA)]: <default("StandardHost")> like IEthernetNetworkNode {}
    connections:
        // Core full mesh (6 links)
        core0.ethg++ <--> <default("EthernetLink")> like ILink <--> core1.ethg++;
        core0.ethg++ <--> <default("EthernetLink")> like ILink <--> core2.ethg++;
        core0.ethg++ <--> <default("EthernetLink")> like ILink <--> core3.ethg++;
        core1.ethg++ <--> <default("EthernetLink")> like ILink <--> core2.ethg++;
        core1.ethg++ <--> <default("EthernetLink")> like ILink <--> core3.ethg++;
        core2.ethg++ <--> <default("EthernetLink")> like ILink <--> core3.ethg++;
        // Core ↔ aggregation (dual-homed agg nodes)
        agg0.ethg++ <--> <default("EthernetLink")> like ILink <--> core0.ethg++;
        agg0.ethg++ <--> <default("EthernetLink")> like ILink <--> core1.ethg++;
        agg1.ethg++ <--> <default("EthernetLink")> like ILink <--> core1.ethg++;
        agg1.ethg++ <--> <default("EthernetLink")> like ILink <--> core3.ethg++;
        agg2.ethg++ <--> <default("EthernetLink")> like ILink <--> core0.ethg++;
        agg2.ethg++ <--> <default("EthernetLink")> like ILink <--> core2.ethg++;
        agg3.ethg++ <--> <default("EthernetLink")> like ILink <--> core2.ethg++;
        agg3.ethg++ <--> <default("EthernetLink")> like ILink <--> core3.ethg++;
        // Aggregation ↔ access (irregular trees + dual-homed sites)
        // agg0 branch: tree + dual-home
        agg0.ethg++ <--> <default("EthernetLink")> like ILink <--> acc0.ethg++;
        agg0.ethg++ <--> <default("EthernetLink")> like ILink <--> acc1.ethg++;
        acc0.ethg++ <--> <default("EthernetLink")> like ILink <--> acc2.ethg++;
        // agg1 branch: chain
        agg1.ethg++ <--> <default("EthernetLink")> like ILink <--> acc3.ethg++;
        acc3.ethg++ <--> <default("EthernetLink")> like ILink <--> acc4.ethg++;
        acc4.ethg++ <--> <default("EthernetLink")> like ILink <--> acc5.ethg++;
        // agg2 branch: dual-homed site
        agg2.ethg++ <--> <default("EthernetLink")> like ILink <--> acc6.ethg++;
        agg2.ethg++ <--> <default("EthernetLink")> like ILink <--> acc7.ethg++;
        acc6.ethg++ <--> <default("EthernetLink")> like ILink <--> acc7.ethg++;
        acc7.ethg++ <--> <default("EthernetLink")> like ILink <--> acc8.ethg++;
        // agg3 branch: star + odd link
        agg3.ethg++ <--> <default("EthernetLink")> like ILink <--> acc9.ethg++;
        agg3.ethg++ <--> <default("EthernetLink")> like ILink <--> acc10.ethg++;
        agg3.ethg++ <--> <default("EthernetLink")> like ILink <--> acc11.ethg++;
        acc10.ethg++ <--> <default("EthernetLink")> like ILink <--> acc11.ethg++;
        // Cross-link: acc1 dual-homed to agg2 as well
        acc1.ethg++ <--> <default("EthernetLink")> like ILink <--> agg2.ethg++;
        // Hosts on core
        for i=0..sizeof(hostC)-1 {
            hostC[i].ethg++ <--> <default("EthernetLink")> like ILink <--> core0.ethg++ if groupIndex(numHostsC,i) == 0;
            hostC[i].ethg++ <--> <default("EthernetLink")> like ILink <--> core1.ethg++ if groupIndex(numHostsC,i) == 1;
            hostC[i].ethg++ <--> <default("EthernetLink")> like ILink <--> core2.ethg++ if groupIndex(numHostsC,i) == 2;
            hostC[i].ethg++ <--> <default("EthernetLink")> like ILink <--> core3.ethg++ if groupIndex(numHostsC,i) == 3;
        }
        // Hosts on aggregation
        for i=0..sizeof(hostG)-1 {
            hostG[i].ethg++ <--> <default("EthernetLink")> like ILink <--> agg0.ethg++ if groupIndex(numHostsG,i) == 0;
            hostG[i].ethg++ <--> <default("EthernetLink")> like ILink <--> agg1.ethg++ if groupIndex(numHostsG,i) == 1;
            hostG[i].ethg++ <--> <default("EthernetLink")> like ILink <--> agg2.ethg++ if groupIndex(numHostsG,i) == 2;
            hostG[i].ethg++ <--> <default("EthernetLink")> like ILink <--> agg3.ethg++ if groupIndex(numHostsG,i) == 3;
        }
        // Hosts on access
        for i=0..sizeof(hostA)-1 {
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc0.ethg++  if groupIndex(numHostsA,i) == 0;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc1.ethg++  if groupIndex(numHostsA,i) == 1;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc2.ethg++  if groupIndex(numHostsA,i) == 2;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc3.ethg++  if groupIndex(numHostsA,i) == 3;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc4.ethg++  if groupIndex(numHostsA,i) == 4;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc5.ethg++  if groupIndex(numHostsA,i) == 5;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc6.ethg++  if groupIndex(numHostsA,i) == 6;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc7.ethg++  if groupIndex(numHostsA,i) == 7;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc8.ethg++  if groupIndex(numHostsA,i) == 8;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc9.ethg++  if groupIndex(numHostsA,i) == 9;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc10.ethg++ if groupIndex(numHostsA,i) == 10;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc11.ethg++ if groupIndex(numHostsA,i) == 11;
        }
}

//
// Large partial core mesh with irregular access trees (40 nodes).
//
// Doubled version of CoreAccessNetwork: 6 core nodes (full mesh),
// 6 aggregation nodes (each dual-homed to 2 core), and 28 access
// nodes in irregular branches — trees, chains, dual-homed pairs,
// and odd cross-links. Core nodes have fixed positions; access
// nodes use auto-layout.
//
network LargeCoreAccessNetwork extends WiredNetworkBase
{
    parameters:
        volatile int numHostsPerCore = default(0);
        volatile int numHostsPerAggregation = default(0);
        volatile int numHostsPerAccess = default(0);
        object numHostsC = default(repeatPar("numHostsPerCore", 6));
        object numHostsG = default(repeatPar("numHostsPerAggregation", 6));
        object numHostsA = default(repeatPar("numHostsPerAccess", 28));
    submodules:
        // Core (fully meshed, 6 nodes)
        core0: <default("Router")> like IEthernetNetworkNode { @display("p=550,200"); }
        core1: <default("Router")> like IEthernetNetworkNode { @display("p=700,140"); }
        core2: <default("Router")> like IEthernetNetworkNode { @display("p=850,200"); }
        core3: <default("Router")> like IEthernetNetworkNode { @display("p=550,360"); }
        core4: <default("Router")> like IEthernetNetworkNode { @display("p=700,420"); }
        core5: <default("Router")> like IEthernetNetworkNode { @display("p=850,360"); }
        // Aggregation (6 nodes, each dual-homed)
        agg0: <default("Router")> like IEthernetNetworkNode { @display("p=400,100"); }
        agg1: <default("Router")> like IEthernetNetworkNode { @display("p=700,60"); }
        agg2: <default("Router")> like IEthernetNetworkNode { @display("p=1000,100"); }
        agg3: <default("Router")> like IEthernetNetworkNode { @display("p=400,460"); }
        agg4: <default("Router")> like IEthernetNetworkNode { @display("p=700,520"); }
        agg5: <default("Router")> like IEthernetNetworkNode { @display("p=1000,460"); }
        // Access (28 nodes, auto-layout)
        acc0:  <default("Router")> like IEthernetNetworkNode;
        acc1:  <default("Router")> like IEthernetNetworkNode;
        acc2:  <default("Router")> like IEthernetNetworkNode;
        acc3:  <default("Router")> like IEthernetNetworkNode;
        acc4:  <default("Router")> like IEthernetNetworkNode;
        acc5:  <default("Router")> like IEthernetNetworkNode;
        acc6:  <default("Router")> like IEthernetNetworkNode;
        acc7:  <default("Router")> like IEthernetNetworkNode;
        acc8:  <default("Router")> like IEthernetNetworkNode;
        acc9:  <default("Router")> like IEthernetNetworkNode;
        acc10: <default("Router")> like IEthernetNetworkNode;
        acc11: <default("Router")> like IEthernetNetworkNode;
        acc12: <default("Router")> like IEthernetNetworkNode;
        acc13: <default("Router")> like IEthernetNetworkNode;
        acc14: <default("Router")> like IEthernetNetworkNode;
        acc15: <default("Router")> like IEthernetNetworkNode;
        acc16: <default("Router")> like IEthernetNetworkNode;
        acc17: <default("Router")> like IEthernetNetworkNode;
        acc18: <default("Router")> like IEthernetNetworkNode;
        acc19: <default("Router")> like IEthernetNetworkNode;
        acc20: <default("Router")> like IEthernetNetworkNode;
        acc21: <default("Router")> like IEthernetNetworkNode;
        acc22: <default("Router")> like IEthernetNetworkNode;
        acc23: <default("Router")> like IEthernetNetworkNode;
        acc24: <default("Router")> like IEthernetNetworkNode;
        acc25: <default("Router")> like IEthernetNetworkNode;
        acc26: <default("Router")> like IEthernetNetworkNode;
        acc27: <default("Router")> like IEthernetNetworkNode;
        // Dangling hosts
        hostC[sum(numHostsC)]: <default("StandardHost")> like IEthernetNetworkNode {}
        hostG[sum(numHostsG)]: <default("StandardHost")> like IEthernetNetworkNode {}
        hostA[sum(numHostsA)]: <default("StandardHost")> like IEthernetNetworkNode {}
    connections:
        // Core full mesh (15 links for 6 nodes)
        core0.ethg++ <--> <default("EthernetLink")> like ILink <--> core1.ethg++;
        core0.ethg++ <--> <default("EthernetLink")> like ILink <--> core2.ethg++;
        core0.ethg++ <--> <default("EthernetLink")> like ILink <--> core3.ethg++;
        core0.ethg++ <--> <default("EthernetLink")> like ILink <--> core4.ethg++;
        core0.ethg++ <--> <default("EthernetLink")> like ILink <--> core5.ethg++;
        core1.ethg++ <--> <default("EthernetLink")> like ILink <--> core2.ethg++;
        core1.ethg++ <--> <default("EthernetLink")> like ILink <--> core3.ethg++;
        core1.ethg++ <--> <default("EthernetLink")> like ILink <--> core4.ethg++;
        core1.ethg++ <--> <default("EthernetLink")> like ILink <--> core5.ethg++;
        core2.ethg++ <--> <default("EthernetLink")> like ILink <--> core3.ethg++;
        core2.ethg++ <--> <default("EthernetLink")> like ILink <--> core4.ethg++;
        core2.ethg++ <--> <default("EthernetLink")> like ILink <--> core5.ethg++;
        core3.ethg++ <--> <default("EthernetLink")> like ILink <--> core4.ethg++;
        core3.ethg++ <--> <default("EthernetLink")> like ILink <--> core5.ethg++;
        core4.ethg++ <--> <default("EthernetLink")> like ILink <--> core5.ethg++;
        // Core ↔ aggregation (each agg dual-homed)
        agg0.ethg++ <--> <default("EthernetLink")> like ILink <--> core0.ethg++;
        agg0.ethg++ <--> <default("EthernetLink")> like ILink <--> core1.ethg++;
        agg1.ethg++ <--> <default("EthernetLink")> like ILink <--> core1.ethg++;
        agg1.ethg++ <--> <default("EthernetLink")> like ILink <--> core2.ethg++;
        agg2.ethg++ <--> <default("EthernetLink")> like ILink <--> core2.ethg++;
        agg2.ethg++ <--> <default("EthernetLink")> like ILink <--> core5.ethg++;
        agg3.ethg++ <--> <default("EthernetLink")> like ILink <--> core0.ethg++;
        agg3.ethg++ <--> <default("EthernetLink")> like ILink <--> core3.ethg++;
        agg4.ethg++ <--> <default("EthernetLink")> like ILink <--> core3.ethg++;
        agg4.ethg++ <--> <default("EthernetLink")> like ILink <--> core4.ethg++;
        agg5.ethg++ <--> <default("EthernetLink")> like ILink <--> core4.ethg++;
        agg5.ethg++ <--> <default("EthernetLink")> like ILink <--> core5.ethg++;
        // agg0 branch: tree with sub-tree
        agg0.ethg++ <--> <default("EthernetLink")> like ILink <--> acc0.ethg++;
        agg0.ethg++ <--> <default("EthernetLink")> like ILink <--> acc1.ethg++;
        acc0.ethg++ <--> <default("EthernetLink")> like ILink <--> acc2.ethg++;
        acc0.ethg++ <--> <default("EthernetLink")> like ILink <--> acc3.ethg++;
        acc1.ethg++ <--> <default("EthernetLink")> like ILink <--> acc4.ethg++;
        // agg1 branch: chain + spur
        agg1.ethg++ <--> <default("EthernetLink")> like ILink <--> acc5.ethg++;
        acc5.ethg++ <--> <default("EthernetLink")> like ILink <--> acc6.ethg++;
        acc6.ethg++ <--> <default("EthernetLink")> like ILink <--> acc7.ethg++;
        acc5.ethg++ <--> <default("EthernetLink")> like ILink <--> acc8.ethg++;
        // agg2 branch: dual-homed pair + dangling
        agg2.ethg++ <--> <default("EthernetLink")> like ILink <--> acc9.ethg++;
        agg2.ethg++ <--> <default("EthernetLink")> like ILink <--> acc10.ethg++;
        acc9.ethg++ <--> <default("EthernetLink")> like ILink <--> acc10.ethg++;
        acc9.ethg++ <--> <default("EthernetLink")> like ILink <--> acc11.ethg++;
        acc10.ethg++ <--> <default("EthernetLink")> like ILink <--> acc12.ethg++;
        // agg3 branch: star + chain
        agg3.ethg++ <--> <default("EthernetLink")> like ILink <--> acc13.ethg++;
        agg3.ethg++ <--> <default("EthernetLink")> like ILink <--> acc14.ethg++;
        acc13.ethg++ <--> <default("EthernetLink")> like ILink <--> acc15.ethg++;
        acc14.ethg++ <--> <default("EthernetLink")> like ILink <--> acc16.ethg++;
        acc16.ethg++ <--> <default("EthernetLink")> like ILink <--> acc17.ethg++;
        // agg4 branch: dual-homed + tree
        agg4.ethg++ <--> <default("EthernetLink")> like ILink <--> acc18.ethg++;
        agg4.ethg++ <--> <default("EthernetLink")> like ILink <--> acc19.ethg++;
        acc18.ethg++ <--> <default("EthernetLink")> like ILink <--> acc19.ethg++;
        acc19.ethg++ <--> <default("EthernetLink")> like ILink <--> acc20.ethg++;
        acc18.ethg++ <--> <default("EthernetLink")> like ILink <--> acc21.ethg++;
        acc21.ethg++ <--> <default("EthernetLink")> like ILink <--> acc22.ethg++;
        // agg5 branch: odd tree
        agg5.ethg++ <--> <default("EthernetLink")> like ILink <--> acc23.ethg++;
        agg5.ethg++ <--> <default("EthernetLink")> like ILink <--> acc24.ethg++;
        acc23.ethg++ <--> <default("EthernetLink")> like ILink <--> acc25.ethg++;
        acc24.ethg++ <--> <default("EthernetLink")> like ILink <--> acc26.ethg++;
        acc24.ethg++ <--> <default("EthernetLink")> like ILink <--> acc27.ethg++;
        // Cross-links between access trees
        acc4.ethg++ <--> <default("EthernetLink")> like ILink <--> agg1.ethg++;
        acc14.ethg++ <--> <default("EthernetLink")> like ILink <--> agg4.ethg++;
        // Hosts on core
        for i=0..sizeof(hostC)-1 {
            hostC[i].ethg++ <--> <default("EthernetLink")> like ILink <--> core0.ethg++ if groupIndex(numHostsC,i) == 0;
            hostC[i].ethg++ <--> <default("EthernetLink")> like ILink <--> core1.ethg++ if groupIndex(numHostsC,i) == 1;
            hostC[i].ethg++ <--> <default("EthernetLink")> like ILink <--> core2.ethg++ if groupIndex(numHostsC,i) == 2;
            hostC[i].ethg++ <--> <default("EthernetLink")> like ILink <--> core3.ethg++ if groupIndex(numHostsC,i) == 3;
            hostC[i].ethg++ <--> <default("EthernetLink")> like ILink <--> core4.ethg++ if groupIndex(numHostsC,i) == 4;
            hostC[i].ethg++ <--> <default("EthernetLink")> like ILink <--> core5.ethg++ if groupIndex(numHostsC,i) == 5;
        }
        // Hosts on aggregation
        for i=0..sizeof(hostG)-1 {
            hostG[i].ethg++ <--> <default("EthernetLink")> like ILink <--> agg0.ethg++ if groupIndex(numHostsG,i) == 0;
            hostG[i].ethg++ <--> <default("EthernetLink")> like ILink <--> agg1.ethg++ if groupIndex(numHostsG,i) == 1;
            hostG[i].ethg++ <--> <default("EthernetLink")> like ILink <--> agg2.ethg++ if groupIndex(numHostsG,i) == 2;
            hostG[i].ethg++ <--> <default("EthernetLink")> like ILink <--> agg3.ethg++ if groupIndex(numHostsG,i) == 3;
            hostG[i].ethg++ <--> <default("EthernetLink")> like ILink <--> agg4.ethg++ if groupIndex(numHostsG,i) == 4;
            hostG[i].ethg++ <--> <default("EthernetLink")> like ILink <--> agg5.ethg++ if groupIndex(numHostsG,i) == 5;
        }
        // Hosts on access
        for i=0..sizeof(hostA)-1 {
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc0.ethg++  if groupIndex(numHostsA,i) == 0;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc1.ethg++  if groupIndex(numHostsA,i) == 1;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc2.ethg++  if groupIndex(numHostsA,i) == 2;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc3.ethg++  if groupIndex(numHostsA,i) == 3;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc4.ethg++  if groupIndex(numHostsA,i) == 4;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc5.ethg++  if groupIndex(numHostsA,i) == 5;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc6.ethg++  if groupIndex(numHostsA,i) == 6;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc7.ethg++  if groupIndex(numHostsA,i) == 7;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc8.ethg++  if groupIndex(numHostsA,i) == 8;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc9.ethg++  if groupIndex(numHostsA,i) == 9;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc10.ethg++ if groupIndex(numHostsA,i) == 10;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc11.ethg++ if groupIndex(numHostsA,i) == 11;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc12.ethg++ if groupIndex(numHostsA,i) == 12;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc13.ethg++ if groupIndex(numHostsA,i) == 13;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc14.ethg++ if groupIndex(numHostsA,i) == 14;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc15.ethg++ if groupIndex(numHostsA,i) == 15;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc16.ethg++ if groupIndex(numHostsA,i) == 16;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc17.ethg++ if groupIndex(numHostsA,i) == 17;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc18.ethg++ if groupIndex(numHostsA,i) == 18;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc19.ethg++ if groupIndex(numHostsA,i) == 19;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc20.ethg++ if groupIndex(numHostsA,i) == 20;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc21.ethg++ if groupIndex(numHostsA,i) == 21;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc22.ethg++ if groupIndex(numHostsA,i) == 22;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc23.ethg++ if groupIndex(numHostsA,i) == 23;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc24.ethg++ if groupIndex(numHostsA,i) == 24;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc25.ethg++ if groupIndex(numHostsA,i) == 25;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc26.ethg++ if groupIndex(numHostsA,i) == 26;
            hostA[i].ethg++ <--> <default("EthernetLink")> like ILink <--> acc27.ethg++ if groupIndex(numHostsA,i) == 27;
        }
}

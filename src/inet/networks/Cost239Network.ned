//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.common.ILink;

//
// The COST 239 European optical network topology (11 nodes, 26 links).
//
// A dense but irregular reference network used extensively in optical
// networking and survivability research. Node degrees range from 3 to 6,
// making it a compact yet realistic testbed for routing, protection,
// and restoration studies.
//
network Cost239Network extends WiredNetworkBase
{
    parameters:
        volatile int numHostsPerSwitch = default(0); // number of hosts connected to each switch
        object numHosts = default(repeatPar("numHostsPerSwitch", 11));
    submodules:
        london:     <default("Router")> like IEthernetNetworkNode { @display("p=280,250"); }
        amsterdam:  <default("Router")> like IEthernetNetworkNode { @display("p=400,200"); }
        paris:      <default("Router")> like IEthernetNetworkNode { @display("p=340,340"); }
        brussels:   <default("Router")> like IEthernetNetworkNode { @display("p=390,270"); }
        luxembourg: <default("Router")> like IEthernetNetworkNode { @display("p=420,310"); }
        berlin:     <default("Router")> like IEthernetNetworkNode { @display("p=500,210"); }
        prague:     <default("Router")> like IEthernetNetworkNode { @display("p=520,290"); }
        copenhagen: <default("Router")> like IEthernetNetworkNode { @display("p=460,130"); }
        vienna:     <default("Router")> like IEthernetNetworkNode { @display("p=540,340"); }
        milan:      <default("Router")> like IEthernetNetworkNode { @display("p=430,420"); }
        zurich:     <default("Router")> like IEthernetNetworkNode { @display("p=430,360"); }
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        london.ethg++     <--> <default("EthernetLink")> like ILink <--> amsterdam.ethg++;
        london.ethg++     <--> <default("EthernetLink")> like ILink <--> paris.ethg++;
        london.ethg++     <--> <default("EthernetLink")> like ILink <--> luxembourg.ethg++;
        london.ethg++     <--> <default("EthernetLink")> like ILink <--> prague.ethg++;
        amsterdam.ethg++  <--> <default("EthernetLink")> like ILink <--> paris.ethg++;
        amsterdam.ethg++  <--> <default("EthernetLink")> like ILink <--> brussels.ethg++;
        amsterdam.ethg++  <--> <default("EthernetLink")> like ILink <--> copenhagen.ethg++;
        amsterdam.ethg++  <--> <default("EthernetLink")> like ILink <--> vienna.ethg++;
        paris.ethg++      <--> <default("EthernetLink")> like ILink <--> berlin.ethg++;
        paris.ethg++      <--> <default("EthernetLink")> like ILink <--> vienna.ethg++;
        brussels.ethg++   <--> <default("EthernetLink")> like ILink <--> luxembourg.ethg++;
        brussels.ethg++   <--> <default("EthernetLink")> like ILink <--> berlin.ethg++;
        brussels.ethg++   <--> <default("EthernetLink")> like ILink <--> zurich.ethg++;
        luxembourg.ethg++ <--> <default("EthernetLink")> like ILink <--> prague.ethg++;
        luxembourg.ethg++ <--> <default("EthernetLink")> like ILink <--> milan.ethg++;
        berlin.ethg++     <--> <default("EthernetLink")> like ILink <--> prague.ethg++;
        berlin.ethg++     <--> <default("EthernetLink")> like ILink <--> copenhagen.ethg++;
        berlin.ethg++     <--> <default("EthernetLink")> like ILink <--> milan.ethg++;
        prague.ethg++     <--> <default("EthernetLink")> like ILink <--> copenhagen.ethg++;
        prague.ethg++     <--> <default("EthernetLink")> like ILink <--> zurich.ethg++;
        copenhagen.ethg++ <--> <default("EthernetLink")> like ILink <--> vienna.ethg++;
        copenhagen.ethg++ <--> <default("EthernetLink")> like ILink <--> milan.ethg++;
        copenhagen.ethg++ <--> <default("EthernetLink")> like ILink <--> zurich.ethg++;
        vienna.ethg++     <--> <default("EthernetLink")> like ILink <--> milan.ethg++;
        vienna.ethg++     <--> <default("EthernetLink")> like ILink <--> zurich.ethg++;
        milan.ethg++      <--> <default("EthernetLink")> like ILink <--> zurich.ethg++;
        for i=0..sizeof(host)-1 {
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> london.ethg++     if groupIndex(numHosts,i) == 0;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> amsterdam.ethg++  if groupIndex(numHosts,i) == 1;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> paris.ethg++      if groupIndex(numHosts,i) == 2;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> brussels.ethg++   if groupIndex(numHosts,i) == 3;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> luxembourg.ethg++ if groupIndex(numHosts,i) == 4;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> berlin.ethg++     if groupIndex(numHosts,i) == 5;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> prague.ethg++     if groupIndex(numHosts,i) == 6;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> copenhagen.ethg++ if groupIndex(numHosts,i) == 7;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> vienna.ethg++     if groupIndex(numHosts,i) == 8;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> milan.ethg++      if groupIndex(numHosts,i) == 9;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> zurich.ethg++     if groupIndex(numHosts,i) == 10;
        }
}

//
// Large COST 239 European optical network topology (22 nodes, ~52 links).
//
// A doubled version of the COST 239 dense irregular reference network.
// Node degrees range from 3 to 7, preserving the compact yet realistic
// character of the original. Organic geographic-style node placement
// with high average degree suitable for routing, protection, and
// restoration studies.
//
network LargeCost239Network extends WiredNetworkBase
{
    parameters:
        volatile int numHostsPerSwitch = default(0);
        object numHosts = default(repeatPar("numHostsPerSwitch", 22));
    submodules:
        // Northern tier
        oslo:       <default("Router")> like IEthernetNetworkNode { @display("p=340,100"); }
        stockholm:  <default("Router")> like IEthernetNetworkNode { @display("p=440,110"); }
        copenhagen: <default("Router")> like IEthernetNetworkNode { @display("p=380,170"); }
        dublin:     <default("Router")> like IEthernetNetworkNode { @display("p=220,200"); }
        london:     <default("Router")> like IEthernetNetworkNode { @display("p=290,260"); }
        // Upper middle
        amsterdam:  <default("Router")> like IEthernetNetworkNode { @display("p=360,240"); }
        brussels:   <default("Router")> like IEthernetNetworkNode { @display("p=340,300"); }
        berlin:     <default("Router")> like IEthernetNetworkNode { @display("p=440,230"); }
        warsaw:     <default("Router")> like IEthernetNetworkNode { @display("p=530,210"); }
        // Central band
        paris:      <default("Router")> like IEthernetNetworkNode { @display("p=290,370"); }
        luxembourg: <default("Router")> like IEthernetNetworkNode { @display("p=360,350"); }
        zurich:     <default("Router")> like IEthernetNetworkNode { @display("p=400,390"); }
        prague:     <default("Router")> like IEthernetNetworkNode { @display("p=470,310"); }
        vienna:     <default("Router")> like IEthernetNetworkNode { @display("p=510,350"); }
        // Lower middle
        lisbon:     <default("Router")> like IEthernetNetworkNode { @display("p=180,490"); }
        milan:      <default("Router")> like IEthernetNetworkNode { @display("p=390,460"); }
        budapest:   <default("Router")> like IEthernetNetworkNode { @display("p=510,410"); }
        bucharest:  <default("Router")> like IEthernetNetworkNode { @display("p=580,420"); }
        // Southern tier
        madrid:     <default("Router")> like IEthernetNetworkNode { @display("p=230,530"); }
        rome:       <default("Router")> like IEthernetNetworkNode { @display("p=420,530"); }
        athens:     <default("Router")> like IEthernetNetworkNode { @display("p=540,530"); }
        istanbul:   <default("Router")> like IEthernetNetworkNode { @display("p=620,490"); }
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        // Northern tier chain + cross-links
        oslo.ethg++       <--> <default("EthernetLink")> like ILink <--> stockholm.ethg++;
        stockholm.ethg++  <--> <default("EthernetLink")> like ILink <--> copenhagen.ethg++;
        copenhagen.ethg++ <--> <default("EthernetLink")> like ILink <--> dublin.ethg++;
        dublin.ethg++     <--> <default("EthernetLink")> like ILink <--> london.ethg++;
        oslo.ethg++       <--> <default("EthernetLink")> like ILink <--> copenhagen.ethg++;
        stockholm.ethg++  <--> <default("EthernetLink")> like ILink <--> dublin.ethg++;
        // Northern → upper middle
        oslo.ethg++       <--> <default("EthernetLink")> like ILink <--> amsterdam.ethg++;
        stockholm.ethg++  <--> <default("EthernetLink")> like ILink <--> brussels.ethg++;
        copenhagen.ethg++ <--> <default("EthernetLink")> like ILink <--> berlin.ethg++;
        dublin.ethg++     <--> <default("EthernetLink")> like ILink <--> warsaw.ethg++;
        london.ethg++     <--> <default("EthernetLink")> like ILink <--> warsaw.ethg++;
        // Upper middle mesh
        amsterdam.ethg++  <--> <default("EthernetLink")> like ILink <--> brussels.ethg++;
        brussels.ethg++   <--> <default("EthernetLink")> like ILink <--> berlin.ethg++;
        berlin.ethg++     <--> <default("EthernetLink")> like ILink <--> warsaw.ethg++;
        amsterdam.ethg++  <--> <default("EthernetLink")> like ILink <--> berlin.ethg++;
        brussels.ethg++   <--> <default("EthernetLink")> like ILink <--> warsaw.ethg++;
        // Upper middle → central
        amsterdam.ethg++  <--> <default("EthernetLink")> like ILink <--> paris.ethg++;
        amsterdam.ethg++  <--> <default("EthernetLink")> like ILink <--> luxembourg.ethg++;
        brussels.ethg++   <--> <default("EthernetLink")> like ILink <--> luxembourg.ethg++;
        brussels.ethg++   <--> <default("EthernetLink")> like ILink <--> zurich.ethg++;
        berlin.ethg++     <--> <default("EthernetLink")> like ILink <--> zurich.ethg++;
        berlin.ethg++     <--> <default("EthernetLink")> like ILink <--> prague.ethg++;
        warsaw.ethg++     <--> <default("EthernetLink")> like ILink <--> prague.ethg++;
        warsaw.ethg++     <--> <default("EthernetLink")> like ILink <--> vienna.ethg++;
        // Central band mesh
        paris.ethg++      <--> <default("EthernetLink")> like ILink <--> luxembourg.ethg++;
        luxembourg.ethg++ <--> <default("EthernetLink")> like ILink <--> zurich.ethg++;
        zurich.ethg++     <--> <default("EthernetLink")> like ILink <--> prague.ethg++;
        prague.ethg++     <--> <default("EthernetLink")> like ILink <--> vienna.ethg++;
        paris.ethg++      <--> <default("EthernetLink")> like ILink <--> zurich.ethg++;
        luxembourg.ethg++ <--> <default("EthernetLink")> like ILink <--> prague.ethg++;
        // Central → lower middle
        paris.ethg++      <--> <default("EthernetLink")> like ILink <--> lisbon.ethg++;
        luxembourg.ethg++ <--> <default("EthernetLink")> like ILink <--> milan.ethg++;
        zurich.ethg++     <--> <default("EthernetLink")> like ILink <--> milan.ethg++;
        zurich.ethg++     <--> <default("EthernetLink")> like ILink <--> budapest.ethg++;
        prague.ethg++     <--> <default("EthernetLink")> like ILink <--> budapest.ethg++;
        prague.ethg++     <--> <default("EthernetLink")> like ILink <--> bucharest.ethg++;
        vienna.ethg++     <--> <default("EthernetLink")> like ILink <--> bucharest.ethg++;
        // Lower middle mesh
        lisbon.ethg++     <--> <default("EthernetLink")> like ILink <--> milan.ethg++;
        milan.ethg++      <--> <default("EthernetLink")> like ILink <--> budapest.ethg++;
        budapest.ethg++   <--> <default("EthernetLink")> like ILink <--> bucharest.ethg++;
        lisbon.ethg++     <--> <default("EthernetLink")> like ILink <--> budapest.ethg++;
        // Lower middle → southern
        lisbon.ethg++     <--> <default("EthernetLink")> like ILink <--> madrid.ethg++;
        lisbon.ethg++     <--> <default("EthernetLink")> like ILink <--> rome.ethg++;
        milan.ethg++      <--> <default("EthernetLink")> like ILink <--> rome.ethg++;
        budapest.ethg++   <--> <default("EthernetLink")> like ILink <--> athens.ethg++;
        bucharest.ethg++  <--> <default("EthernetLink")> like ILink <--> istanbul.ethg++;
        // Southern tier
        madrid.ethg++     <--> <default("EthernetLink")> like ILink <--> rome.ethg++;
        rome.ethg++       <--> <default("EthernetLink")> like ILink <--> athens.ethg++;
        athens.ethg++     <--> <default("EthernetLink")> like ILink <--> istanbul.ethg++;
        // hosts
        for i=0..sizeof(host)-1 {
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> oslo.ethg++       if groupIndex(numHosts,i) == 0;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> stockholm.ethg++  if groupIndex(numHosts,i) == 1;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> copenhagen.ethg++ if groupIndex(numHosts,i) == 2;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> dublin.ethg++     if groupIndex(numHosts,i) == 3;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> london.ethg++     if groupIndex(numHosts,i) == 4;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> amsterdam.ethg++  if groupIndex(numHosts,i) == 5;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> brussels.ethg++   if groupIndex(numHosts,i) == 6;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> berlin.ethg++     if groupIndex(numHosts,i) == 7;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> warsaw.ethg++     if groupIndex(numHosts,i) == 8;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> paris.ethg++      if groupIndex(numHosts,i) == 9;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> luxembourg.ethg++ if groupIndex(numHosts,i) == 10;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> zurich.ethg++     if groupIndex(numHosts,i) == 11;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> prague.ethg++     if groupIndex(numHosts,i) == 12;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> vienna.ethg++     if groupIndex(numHosts,i) == 13;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> lisbon.ethg++     if groupIndex(numHosts,i) == 14;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> milan.ethg++      if groupIndex(numHosts,i) == 15;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> budapest.ethg++   if groupIndex(numHosts,i) == 16;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> bucharest.ethg++  if groupIndex(numHosts,i) == 17;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> madrid.ethg++     if groupIndex(numHosts,i) == 18;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> rome.ethg++       if groupIndex(numHosts,i) == 19;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> athens.ethg++     if groupIndex(numHosts,i) == 20;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> istanbul.ethg++   if groupIndex(numHosts,i) == 21;
        }
}

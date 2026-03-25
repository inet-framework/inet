//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.common.ILink;

//
// The classic 1991 NSFNET T3 backbone topology (14 nodes, 21 links).
//
// Nodes represent the real supercomputer centers and regional network
// attachment points of the historical NSFNET backbone:
//
//   seattle      – NorthWestNet
//   paloAlto     – BARRNet (Bay Area)
//   sanDiego     – SDSC
//   saltLakeCity – Westnet
//   boulder      – NCAR
//   houston      – SESQUINET
//   lincoln      – MidNet
//   champaign    – NCSA
//   annArbor     – MERIT
//   atlanta      – SURAnet
//   pittsburgh   – PSC
//   ithaca       – Cornell Theory Center
//   princeton    – JvNCnet
//   collegePark  – UMD / SURAnet-East
//
// Node degrees range from 2 to 4, making it representative of
// real-world ISP backbone topologies. The most widely cited
// research network topology.
//
network NsfnetNetwork extends WiredNetworkBase
{
    parameters:
        volatile int numHostsPerNode = default(0);
        object numHosts = default(repeatPar("numHostsPerNode", 14));
    submodules:
        // Pacific Northwest & West Coast
        seattle:      <default("Router")> like IEthernetNetworkNode { @display("p=280,80"); }
        paloAlto:     <default("Router")> like IEthernetNetworkNode { @display("p=260,250"); }
        sanDiego:     <default("Router")> like IEthernetNetworkNode { @display("p=320,360"); }
        // Mountain West
        saltLakeCity: <default("Router")> like IEthernetNetworkNode { @display("p=390,180"); }
        boulder:      <default("Router")> like IEthernetNetworkNode { @display("p=470,240"); }
        // Central
        lincoln:      <default("Router")> like IEthernetNetworkNode { @display("p=530,160"); }
        houston:      <default("Router")> like IEthernetNetworkNode { @display("p=580,420"); }
        // Great Lakes & Midwest
        champaign:    <default("Router")> like IEthernetNetworkNode { @display("p=650,200"); }
        annArbor:     <default("Router")> like IEthernetNetworkNode { @display("p=720,150"); }
        // Southeast
        atlanta:      <default("Router")> like IEthernetNetworkNode { @display("p=740,360"); }
        // Mid-Atlantic & Northeast
        pittsburgh:   <default("Router")> like IEthernetNetworkNode { @display("p=800,180"); }
        ithaca:       <default("Router")> like IEthernetNetworkNode { @display("p=840,110"); }
        princeton:    <default("Router")> like IEthernetNetworkNode { @display("p=880,190"); }
        collegePark:  <default("Router")> like IEthernetNetworkNode { @display("p=870,270"); }
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {}
    connections:
        seattle.ethg++      <--> <default("EthernetLink")> like ILink <--> paloAlto.ethg++;
        seattle.ethg++      <--> <default("EthernetLink")> like ILink <--> sanDiego.ethg++;
        seattle.ethg++      <--> <default("EthernetLink")> like ILink <--> champaign.ethg++;
        paloAlto.ethg++     <--> <default("EthernetLink")> like ILink <--> sanDiego.ethg++;
        paloAlto.ethg++     <--> <default("EthernetLink")> like ILink <--> saltLakeCity.ethg++;
        sanDiego.ethg++     <--> <default("EthernetLink")> like ILink <--> houston.ethg++;
        saltLakeCity.ethg++ <--> <default("EthernetLink")> like ILink <--> boulder.ethg++;
        saltLakeCity.ethg++ <--> <default("EthernetLink")> like ILink <--> pittsburgh.ethg++;
        boulder.ethg++      <--> <default("EthernetLink")> like ILink <--> houston.ethg++;
        boulder.ethg++      <--> <default("EthernetLink")> like ILink <--> lincoln.ethg++;
        houston.ethg++      <--> <default("EthernetLink")> like ILink <--> lincoln.ethg++;
        houston.ethg++      <--> <default("EthernetLink")> like ILink <--> atlanta.ethg++;
        lincoln.ethg++      <--> <default("EthernetLink")> like ILink <--> champaign.ethg++;
        champaign.ethg++    <--> <default("EthernetLink")> like ILink <--> annArbor.ethg++;
        champaign.ethg++    <--> <default("EthernetLink")> like ILink <--> atlanta.ethg++;
        annArbor.ethg++     <--> <default("EthernetLink")> like ILink <--> atlanta.ethg++;
        annArbor.ethg++     <--> <default("EthernetLink")> like ILink <--> ithaca.ethg++;
        annArbor.ethg++     <--> <default("EthernetLink")> like ILink <--> princeton.ethg++;
        atlanta.ethg++      <--> <default("EthernetLink")> like ILink <--> collegePark.ethg++;
        pittsburgh.ethg++   <--> <default("EthernetLink")> like ILink <--> ithaca.ethg++;
        ithaca.ethg++       <--> <default("EthernetLink")> like ILink <--> princeton.ethg++;
        princeton.ethg++    <--> <default("EthernetLink")> like ILink <--> collegePark.ethg++;
        for i=0..sizeof(host)-1 {
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> seattle.ethg++      if groupIndex(numHosts,i) == 0;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> paloAlto.ethg++     if groupIndex(numHosts,i) == 1;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> sanDiego.ethg++     if groupIndex(numHosts,i) == 2;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> saltLakeCity.ethg++ if groupIndex(numHosts,i) == 3;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> boulder.ethg++      if groupIndex(numHosts,i) == 4;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> lincoln.ethg++      if groupIndex(numHosts,i) == 5;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> houston.ethg++      if groupIndex(numHosts,i) == 6;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> champaign.ethg++    if groupIndex(numHosts,i) == 7;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> annArbor.ethg++     if groupIndex(numHosts,i) == 8;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> atlanta.ethg++      if groupIndex(numHosts,i) == 9;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> pittsburgh.ethg++   if groupIndex(numHosts,i) == 10;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> ithaca.ethg++       if groupIndex(numHosts,i) == 11;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> princeton.ethg++    if groupIndex(numHosts,i) == 12;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> collegePark.ethg++  if groupIndex(numHosts,i) == 13;
        }
}

//
// A parameterizable NSFNET-style irregular partial mesh.
//
// Generates a topology with non-uniform node degrees using deterministic rules:
//  - Ring backbone: every node connects to its neighbor
//  - Skip-2 links: every 3rd node also connects 2 hops ahead
//  - Long-range links: every 5th node also connects N/3 hops ahead
//
// This produces hub-like nodes (degree 4-5) and leaf-like nodes (degree 2),
// mimicking the degree distribution of real ISP backbone topologies.
//
network NsfnetStyleNetwork extends WiredNetworkBase
{
    parameters:
        int numNodes;
    submodules:
        node[numNodes]: <default("Router")> like IEthernetNetworkNode;
    connections:
        // Ring backbone
        for i=0..numNodes-1 {
            node[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node[(i+1) % numNodes].ethg++;
        }
        // Skip-2 links for every 3rd node
        for i=0..numNodes-1 {
            node[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node[(i+2) % numNodes].ethg++ if i % 3 == 0 && numNodes > 4;
        }
        // Long-range links for every 5th node
        for i=0..numNodes-1 {
            node[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node[(i+int(numNodes/3)) % numNodes].ethg++ if i % 5 == 0 && numNodes >= 15;
        }
}

//
// Medium NSFNET-style irregular mesh: 100 nodes.
//
network MediumNsfnetNetwork extends NsfnetStyleNetwork
{
    parameters:
        numNodes = default(100);
}

//
// Large NSFNET-style irregular mesh: 1000 nodes.
//
network LargeNsfnetNetwork extends NsfnetStyleNetwork
{
    parameters:
        numNodes = default(1000);
}

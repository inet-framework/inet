//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.common.ILink;

//
// The GÉANT European research network topology (22 nodes, 36 links).
//
// An irregular mesh representing the pan-European research backbone.
// Node degrees range from 1 (peripheral) to 7 (central hubs),
// with dense connectivity in Western/Central Europe and sparser
// links to the periphery. Widely used alongside US backbones in
// networking research.
//
network GeantNetwork extends WiredNetworkBase
{
    parameters:
        volatile int numHostsPerSwitch = default(0); // number of hosts connected to each switch
        object numHosts = default(repeatPar("numHostsPerSwitch", 22));
    submodules:
        // Western Europe
        london:     <default("Router")> like IEthernetNetworkNode { @display("p=280,250"); }
        amsterdam:  <default("Router")> like IEthernetNetworkNode { @display("p=390,200"); }
        paris:      <default("Router")> like IEthernetNetworkNode { @display("p=320,340"); }
        brussels:   <default("Router")> like IEthernetNetworkNode { @display("p=380,280"); }
        // Northern / Central Europe
        copenhagen: <default("Router")> like IEthernetNetworkNode { @display("p=460,140"); }
        hamburg:    <default("Router")> like IEthernetNetworkNode { @display("p=440,200"); }
        madrid:     <default("Router")> like IEthernetNetworkNode { @display("p=240,450"); }
        frankfurt:  <default("Router")> like IEthernetNetworkNode { @display("p=420,300"); }
        berlin:     <default("Router")> like IEthernetNetworkNode { @display("p=490,220"); }
        warsaw:     <default("Router")> like IEthernetNetworkNode { @display("p=560,200"); }
        // Iberian periphery
        lisbon:     <default("Router")> like IEthernetNetworkNode { @display("p=180,490"); }
        // Central Europe
        zurich:     <default("Router")> like IEthernetNetworkNode { @display("p=420,370"); }
        milan:      <default("Router")> like IEthernetNetworkNode { @display("p=420,430"); }
        vienna:     <default("Router")> like IEthernetNetworkNode { @display("p=530,310"); }
        prague:     <default("Router")> like IEthernetNetworkNode { @display("p=510,270"); }
        porto:      <default("Router")> like IEthernetNetworkNode { @display("p=190,440"); }
        // Scandinavia
        lyon:       <default("Router")> like IEthernetNetworkNode { @display("p=360,400"); }
        rome:       <default("Router")> like IEthernetNetworkNode { @display("p=450,500"); }
        // Southeast Europe
        budapest:   <default("Router")> like IEthernetNetworkNode { @display("p=540,380"); }
        bucharest:  <default("Router")> like IEthernetNetworkNode { @display("p=600,400"); }
        sofia:      <default("Router")> like IEthernetNetworkNode { @display("p=570,450"); }
        athens:     <default("Router")> like IEthernetNetworkNode { @display("p=560,520"); }
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        london.ethg++    <--> <default("EthernetLink")> like ILink <--> amsterdam.ethg++;
        london.ethg++    <--> <default("EthernetLink")> like ILink <--> paris.ethg++;
        london.ethg++    <--> <default("EthernetLink")> like ILink <--> brussels.ethg++;
        amsterdam.ethg++ <--> <default("EthernetLink")> like ILink <--> copenhagen.ethg++;
        amsterdam.ethg++ <--> <default("EthernetLink")> like ILink <--> hamburg.ethg++;
        paris.ethg++     <--> <default("EthernetLink")> like ILink <--> brussels.ethg++;
        paris.ethg++     <--> <default("EthernetLink")> like ILink <--> madrid.ethg++;
        brussels.ethg++  <--> <default("EthernetLink")> like ILink <--> madrid.ethg++;
        brussels.ethg++  <--> <default("EthernetLink")> like ILink <--> frankfurt.ethg++;
        copenhagen.ethg++ <--> <default("EthernetLink")> like ILink <--> hamburg.ethg++;
        copenhagen.ethg++ <--> <default("EthernetLink")> like ILink <--> frankfurt.ethg++;
        copenhagen.ethg++ <--> <default("EthernetLink")> like ILink <--> berlin.ethg++;
        hamburg.ethg++   <--> <default("EthernetLink")> like ILink <--> warsaw.ethg++;
        madrid.ethg++    <--> <default("EthernetLink")> like ILink <--> lisbon.ethg++;
        madrid.ethg++    <--> <default("EthernetLink")> like ILink <--> zurich.ethg++;
        frankfurt.ethg++ <--> <default("EthernetLink")> like ILink <--> berlin.ethg++;
        frankfurt.ethg++ <--> <default("EthernetLink")> like ILink <--> zurich.ethg++;
        frankfurt.ethg++ <--> <default("EthernetLink")> like ILink <--> milan.ethg++;
        berlin.ethg++    <--> <default("EthernetLink")> like ILink <--> warsaw.ethg++;
        berlin.ethg++    <--> <default("EthernetLink")> like ILink <--> milan.ethg++;
        berlin.ethg++    <--> <default("EthernetLink")> like ILink <--> vienna.ethg++;
        warsaw.ethg++    <--> <default("EthernetLink")> like ILink <--> prague.ethg++;
        lisbon.ethg++    <--> <default("EthernetLink")> like ILink <--> porto.ethg++;
        lisbon.ethg++    <--> <default("EthernetLink")> like ILink <--> lyon.ethg++;
        zurich.ethg++    <--> <default("EthernetLink")> like ILink <--> milan.ethg++;
        zurich.ethg++    <--> <default("EthernetLink")> like ILink <--> lyon.ethg++;
        milan.ethg++     <--> <default("EthernetLink")> like ILink <--> vienna.ethg++;
        milan.ethg++     <--> <default("EthernetLink")> like ILink <--> rome.ethg++;
        vienna.ethg++    <--> <default("EthernetLink")> like ILink <--> prague.ethg++;
        vienna.ethg++    <--> <default("EthernetLink")> like ILink <--> budapest.ethg++;
        prague.ethg++    <--> <default("EthernetLink")> like ILink <--> bucharest.ethg++;
        prague.ethg++    <--> <default("EthernetLink")> like ILink <--> sofia.ethg++;
        lyon.ethg++      <--> <default("EthernetLink")> like ILink <--> rome.ethg++;
        rome.ethg++      <--> <default("EthernetLink")> like ILink <--> budapest.ethg++;
        budapest.ethg++  <--> <default("EthernetLink")> like ILink <--> bucharest.ethg++;
        bucharest.ethg++ <--> <default("EthernetLink")> like ILink <--> sofia.ethg++;
        bucharest.ethg++ <--> <default("EthernetLink")> like ILink <--> athens.ethg++;
        athens.ethg++    <--> <default("EthernetLink")> like ILink <--> budapest.ethg++;
        for i=0..sizeof(host)-1 {
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> london.ethg++    if groupIndex(numHosts,i) == 0;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> amsterdam.ethg++ if groupIndex(numHosts,i) == 1;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> paris.ethg++     if groupIndex(numHosts,i) == 2;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> brussels.ethg++  if groupIndex(numHosts,i) == 3;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> copenhagen.ethg++ if groupIndex(numHosts,i) == 4;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> hamburg.ethg++   if groupIndex(numHosts,i) == 5;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> madrid.ethg++    if groupIndex(numHosts,i) == 6;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> frankfurt.ethg++ if groupIndex(numHosts,i) == 7;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> berlin.ethg++    if groupIndex(numHosts,i) == 8;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> warsaw.ethg++    if groupIndex(numHosts,i) == 9;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> lisbon.ethg++    if groupIndex(numHosts,i) == 10;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> zurich.ethg++    if groupIndex(numHosts,i) == 11;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> milan.ethg++     if groupIndex(numHosts,i) == 12;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> vienna.ethg++    if groupIndex(numHosts,i) == 13;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> prague.ethg++    if groupIndex(numHosts,i) == 14;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> porto.ethg++     if groupIndex(numHosts,i) == 15;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> lyon.ethg++      if groupIndex(numHosts,i) == 16;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> rome.ethg++      if groupIndex(numHosts,i) == 17;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> budapest.ethg++  if groupIndex(numHosts,i) == 18;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> bucharest.ethg++ if groupIndex(numHosts,i) == 19;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> sofia.ethg++     if groupIndex(numHosts,i) == 20;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> athens.ethg++    if groupIndex(numHosts,i) == 21;
        }
}

//
// Large GÉANT European research network topology (44 nodes, ~72 links).
//
// A doubled version of the GÉANT backbone preserving the original's
// character: dense connectivity in Western/Central Europe, sparser
// links to the periphery, node degrees from 1 (peripheral) to 7+
// (central hubs), and organic geographic-style node placement.
//
network LargeGeantNetwork extends WiredNetworkBase
{
    parameters:
        volatile int numHostsPerSwitch = default(0);
        object numHosts = default(repeatPar("numHostsPerSwitch", 44));
    submodules:
        // Iberian peninsula
        lisbon:     <default("Router")> like IEthernetNetworkNode { @display("p=320,520"); }
        madrid:     <default("Router")> like IEthernetNetworkNode { @display("p=400,460"); }
        porto:      <default("Router")> like IEthernetNetworkNode { @display("p=360,600"); }
        // France
        paris:      <default("Router")> like IEthernetNetworkNode { @display("p=480,380"); }
        lyon:       <default("Router")> like IEthernetNetworkNode { @display("p=540,300"); }
        strasbourg: <default("Router")> like IEthernetNetworkNode { @display("p=460,280"); }
        // British Isles
        london:     <default("Router")> like IEthernetNetworkNode { @display("p=420,160"); }
        dublin:     <default("Router")> like IEthernetNetworkNode { @display("p=360,100"); }
        edinburgh:  <default("Router")> like IEthernetNetworkNode { @display("p=480,100"); }
        // Benelux
        amsterdam:  <default("Router")> like IEthernetNetworkNode { @display("p=580,180"); }
        brussels:   <default("Router")> like IEthernetNetworkNode { @display("p=600,240"); }
        // Germany
        hamburg:    <default("Router")> like IEthernetNetworkNode { @display("p=660,160"); }
        frankfurt:  <default("Router")> like IEthernetNetworkNode { @display("p=700,260"); }
        munich:     <default("Router")> like IEthernetNetworkNode { @display("p=640,320"); }
        // Switzerland & Austria
        zurich:     <default("Router")> like IEthernetNetworkNode { @display("p=600,400"); }
        vienna:     <default("Router")> like IEthernetNetworkNode { @display("p=720,380"); }
        // Italy
        milan:      <default("Router")> like IEthernetNetworkNode { @display("p=640,500"); }
        rome:       <default("Router")> like IEthernetNetworkNode { @display("p=700,580"); }
        naples:     <default("Router")> like IEthernetNetworkNode { @display("p=580,620"); }
        // Scandinavia
        copenhagen: <default("Router")> like IEthernetNetworkNode { @display("p=620,60"); }
        stockholm:  <default("Router")> like IEthernetNetworkNode { @display("p=720,80"); }
        oslo:       <default("Router")> like IEthernetNetworkNode { @display("p=760,140"); }
        helsinki:   <default("Router")> like IEthernetNetworkNode { @display("p=820,60"); }
        // Poland & Baltics
        warsaw:     <default("Router")> like IEthernetNetworkNode { @display("p=800,200"); }
        vilnius:    <default("Router")> like IEthernetNetworkNode { @display("p=860,140"); }
        riga:       <default("Router")> like IEthernetNetworkNode { @display("p=900,220"); }
        // Czech & Slovakia
        prague:     <default("Router")> like IEthernetNetworkNode { @display("p=780,300"); }
        bratislava: <default("Router")> like IEthernetNetworkNode { @display("p=840,340"); }
        // Hungary & Romania
        budapest:   <default("Router")> like IEthernetNetworkNode { @display("p=820,420"); }
        bucharest:  <default("Router")> like IEthernetNetworkNode { @display("p=920,380"); }
        sofia:      <default("Router")> like IEthernetNetworkNode { @display("p=980,440"); }
        // Balkans
        zagreb:     <default("Router")> like IEthernetNetworkNode { @display("p=780,500"); }
        belgrade:   <default("Router")> like IEthernetNetworkNode { @display("p=860,540"); }
        sarajevo:   <default("Router")> like IEthernetNetworkNode { @display("p=920,500"); }
        // Greece & Turkey
        athens:     <default("Router")> like IEthernetNetworkNode { @display("p=840,640"); }
        istanbul:   <default("Router")> like IEthernetNetworkNode { @display("p=940,600"); }
        ankara:     <default("Router")> like IEthernetNetworkNode { @display("p=1020,560"); }
        // Ukraine & Eastern Europe
        kiev:       <default("Router")> like IEthernetNetworkNode { @display("p=960,300"); }
        moscow:     <default("Router")> like IEthernetNetworkNode { @display("p=1040,260"); }
        // Nordic periphery
        tallinn:    <default("Router")> like IEthernetNetworkNode { @display("p=900,80"); }
        minsk:      <default("Router")> like IEthernetNetworkNode { @display("p=980,120"); }
        // Mediterranean islands
        palermo:    <default("Router")> like IEthernetNetworkNode { @display("p=500,680"); }
        heraklion:  <default("Router")> like IEthernetNetworkNode { @display("p=740,660"); }
        // Iceland
        reykjavik:  <default("Router")> like IEthernetNetworkNode { @display("p=280,60"); }
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        // Iberian peninsula
        lisbon.ethg++     <--> <default("EthernetLink")> like ILink <--> madrid.ethg++;
        lisbon.ethg++     <--> <default("EthernetLink")> like ILink <--> porto.ethg++;
        madrid.ethg++     <--> <default("EthernetLink")> like ILink <--> paris.ethg++;
        porto.ethg++      <--> <default("EthernetLink")> like ILink <--> palermo.ethg++;
        // France hub
        paris.ethg++      <--> <default("EthernetLink")> like ILink <--> lyon.ethg++;
        paris.ethg++      <--> <default("EthernetLink")> like ILink <--> strasbourg.ethg++;
        paris.ethg++      <--> <default("EthernetLink")> like ILink <--> zurich.ethg++;
        lyon.ethg++       <--> <default("EthernetLink")> like ILink <--> strasbourg.ethg++;
        lyon.ethg++       <--> <default("EthernetLink")> like ILink <--> brussels.ethg++;
        lyon.ethg++       <--> <default("EthernetLink")> like ILink <--> munich.ethg++;
        strasbourg.ethg++ <--> <default("EthernetLink")> like ILink <--> london.ethg++;
        // British Isles
        london.ethg++     <--> <default("EthernetLink")> like ILink <--> dublin.ethg++;
        london.ethg++     <--> <default("EthernetLink")> like ILink <--> edinburgh.ethg++;
        london.ethg++     <--> <default("EthernetLink")> like ILink <--> amsterdam.ethg++;
        dublin.ethg++     <--> <default("EthernetLink")> like ILink <--> reykjavik.ethg++;
        edinburgh.ethg++  <--> <default("EthernetLink")> like ILink <--> amsterdam.ethg++;
        // Benelux
        amsterdam.ethg++  <--> <default("EthernetLink")> like ILink <--> brussels.ethg++;
        amsterdam.ethg++  <--> <default("EthernetLink")> like ILink <--> hamburg.ethg++;
        brussels.ethg++   <--> <default("EthernetLink")> like ILink <--> frankfurt.ethg++;
        // Germany hub
        hamburg.ethg++    <--> <default("EthernetLink")> like ILink <--> frankfurt.ethg++;
        hamburg.ethg++    <--> <default("EthernetLink")> like ILink <--> copenhagen.ethg++;
        hamburg.ethg++    <--> <default("EthernetLink")> like ILink <--> oslo.ethg++;
        frankfurt.ethg++  <--> <default("EthernetLink")> like ILink <--> munich.ethg++;
        frankfurt.ethg++  <--> <default("EthernetLink")> like ILink <--> prague.ethg++;
        munich.ethg++     <--> <default("EthernetLink")> like ILink <--> zurich.ethg++;
        munich.ethg++     <--> <default("EthernetLink")> like ILink <--> vienna.ethg++;
        // Switzerland & Austria
        zurich.ethg++     <--> <default("EthernetLink")> like ILink <--> milan.ethg++;
        vienna.ethg++     <--> <default("EthernetLink")> like ILink <--> prague.ethg++;
        vienna.ethg++     <--> <default("EthernetLink")> like ILink <--> budapest.ethg++;
        // Italy
        milan.ethg++      <--> <default("EthernetLink")> like ILink <--> rome.ethg++;
        milan.ethg++      <--> <default("EthernetLink")> like ILink <--> zagreb.ethg++;
        rome.ethg++       <--> <default("EthernetLink")> like ILink <--> naples.ethg++;
        rome.ethg++       <--> <default("EthernetLink")> like ILink <--> heraklion.ethg++;
        naples.ethg++     <--> <default("EthernetLink")> like ILink <--> palermo.ethg++;
        // Scandinavia
        copenhagen.ethg++ <--> <default("EthernetLink")> like ILink <--> stockholm.ethg++;
        stockholm.ethg++  <--> <default("EthernetLink")> like ILink <--> oslo.ethg++;
        stockholm.ethg++  <--> <default("EthernetLink")> like ILink <--> helsinki.ethg++;
        oslo.ethg++       <--> <default("EthernetLink")> like ILink <--> warsaw.ethg++;
        helsinki.ethg++   <--> <default("EthernetLink")> like ILink <--> vilnius.ethg++;
        helsinki.ethg++   <--> <default("EthernetLink")> like ILink <--> tallinn.ethg++;
        // Poland & Baltics
        warsaw.ethg++     <--> <default("EthernetLink")> like ILink <--> vilnius.ethg++;
        warsaw.ethg++     <--> <default("EthernetLink")> like ILink <--> prague.ethg++;
        vilnius.ethg++    <--> <default("EthernetLink")> like ILink <--> riga.ethg++;
        riga.ethg++       <--> <default("EthernetLink")> like ILink <--> kiev.ethg++;
        // Czech & Slovakia
        prague.ethg++     <--> <default("EthernetLink")> like ILink <--> bratislava.ethg++;
        bratislava.ethg++ <--> <default("EthernetLink")> like ILink <--> budapest.ethg++;
        bratislava.ethg++ <--> <default("EthernetLink")> like ILink <--> bucharest.ethg++;
        // Hungary & Romania
        budapest.ethg++   <--> <default("EthernetLink")> like ILink <--> zagreb.ethg++;
        bucharest.ethg++  <--> <default("EthernetLink")> like ILink <--> sofia.ethg++;
        bucharest.ethg++  <--> <default("EthernetLink")> like ILink <--> sarajevo.ethg++;
        sofia.ethg++      <--> <default("EthernetLink")> like ILink <--> kiev.ethg++;
        sofia.ethg++      <--> <default("EthernetLink")> like ILink <--> sarajevo.ethg++;
        // Balkans
        zagreb.ethg++     <--> <default("EthernetLink")> like ILink <--> belgrade.ethg++;
        belgrade.ethg++   <--> <default("EthernetLink")> like ILink <--> sarajevo.ethg++;
        belgrade.ethg++   <--> <default("EthernetLink")> like ILink <--> athens.ethg++;
        // Greece & Turkey
        athens.ethg++     <--> <default("EthernetLink")> like ILink <--> istanbul.ethg++;
        athens.ethg++     <--> <default("EthernetLink")> like ILink <--> heraklion.ethg++;
        istanbul.ethg++   <--> <default("EthernetLink")> like ILink <--> ankara.ethg++;
        ankara.ethg++     <--> <default("EthernetLink")> like ILink <--> sarajevo.ethg++;
        // Ukraine & Eastern Europe
        kiev.ethg++       <--> <default("EthernetLink")> like ILink <--> moscow.ethg++;
        moscow.ethg++     <--> <default("EthernetLink")> like ILink <--> minsk.ethg++;
        // Nordic periphery
        tallinn.ethg++    <--> <default("EthernetLink")> like ILink <--> minsk.ethg++;
        // hosts
        for i=0..sizeof(host)-1 {
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> lisbon.ethg++     if groupIndex(numHosts,i) == 0;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> madrid.ethg++     if groupIndex(numHosts,i) == 1;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> porto.ethg++      if groupIndex(numHosts,i) == 2;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> paris.ethg++      if groupIndex(numHosts,i) == 3;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> lyon.ethg++       if groupIndex(numHosts,i) == 4;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> strasbourg.ethg++ if groupIndex(numHosts,i) == 5;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> london.ethg++     if groupIndex(numHosts,i) == 6;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> dublin.ethg++     if groupIndex(numHosts,i) == 7;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> edinburgh.ethg++  if groupIndex(numHosts,i) == 8;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> amsterdam.ethg++  if groupIndex(numHosts,i) == 9;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> brussels.ethg++   if groupIndex(numHosts,i) == 10;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> hamburg.ethg++    if groupIndex(numHosts,i) == 11;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> frankfurt.ethg++  if groupIndex(numHosts,i) == 12;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> munich.ethg++     if groupIndex(numHosts,i) == 13;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> zurich.ethg++     if groupIndex(numHosts,i) == 14;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> vienna.ethg++     if groupIndex(numHosts,i) == 15;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> milan.ethg++      if groupIndex(numHosts,i) == 16;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> rome.ethg++       if groupIndex(numHosts,i) == 17;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> naples.ethg++     if groupIndex(numHosts,i) == 18;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> copenhagen.ethg++ if groupIndex(numHosts,i) == 19;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> stockholm.ethg++  if groupIndex(numHosts,i) == 20;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> oslo.ethg++       if groupIndex(numHosts,i) == 21;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> helsinki.ethg++   if groupIndex(numHosts,i) == 22;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> warsaw.ethg++     if groupIndex(numHosts,i) == 23;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> vilnius.ethg++    if groupIndex(numHosts,i) == 24;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> riga.ethg++       if groupIndex(numHosts,i) == 25;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> prague.ethg++     if groupIndex(numHosts,i) == 26;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> bratislava.ethg++ if groupIndex(numHosts,i) == 27;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> budapest.ethg++   if groupIndex(numHosts,i) == 28;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> bucharest.ethg++  if groupIndex(numHosts,i) == 29;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> sofia.ethg++      if groupIndex(numHosts,i) == 30;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> zagreb.ethg++     if groupIndex(numHosts,i) == 31;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> belgrade.ethg++   if groupIndex(numHosts,i) == 32;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> sarajevo.ethg++   if groupIndex(numHosts,i) == 33;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> athens.ethg++     if groupIndex(numHosts,i) == 34;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> istanbul.ethg++   if groupIndex(numHosts,i) == 35;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> ankara.ethg++     if groupIndex(numHosts,i) == 36;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> kiev.ethg++       if groupIndex(numHosts,i) == 37;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> moscow.ethg++     if groupIndex(numHosts,i) == 38;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> tallinn.ethg++    if groupIndex(numHosts,i) == 39;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> minsk.ethg++      if groupIndex(numHosts,i) == 40;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> palermo.ethg++    if groupIndex(numHosts,i) == 41;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> heraklion.ethg++  if groupIndex(numHosts,i) == 42;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> reykjavik.ethg++  if groupIndex(numHosts,i) == 43;
        }
}

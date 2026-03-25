//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.common.ILink;

//
// The US backbone network topology (39 nodes, 63 links).
//
// A well-known irregular mesh used extensively in networking research.
// Node degrees range from 1 to 5, with hub nodes in the central and
// eastern regions and sparser connectivity on the periphery.
//
network UsBackboneNetwork extends WiredNetworkBase
{
    parameters:
        volatile int numHostsPerSwitch = default(0); // number of hosts connected to each switch
        object numHosts = default(repeatPar("numHostsPerSwitch", 39));
    submodules:
        // Pacific Northwest & Western Canada
        vancouver:     <default("Router")> like IEthernetNetworkNode { @display("p=295,105"); }
        seattle:       <default("Router")> like IEthernetNetworkNode { @display("p=290,145"); }
        portland:      <default("Router")> like IEthernetNetworkNode { @display("p=270,190"); }
        calgary:       <default("Router")> like IEthernetNetworkNode { @display("p=445,65"); }
        // California
        sacramento:    <default("Router")> like IEthernetNetworkNode { @display("p=285,285"); }
        sanFrancisco:  <default("Router")> like IEthernetNetworkNode { @display("p=270,325"); }
        losAngeles:    <default("Router")> like IEthernetNetworkNode { @display("p=325,395"); }
        sanDiego:      <default("Router")> like IEthernetNetworkNode { @display("p=375,435"); }
        // Mountain West & Southwest
        lasVegas:      <default("Router")> like IEthernetNetworkNode { @display("p=415,350"); }
        phoenix:       <default("Router")> like IEthernetNetworkNode { @display("p=460,415"); }
        saltLakeCity:  <default("Router")> like IEthernetNetworkNode { @display("p=495,260"); }
        denver:        <default("Router")> like IEthernetNetworkNode { @display("p=550,300"); }
        elPaso:        <default("Router")> like IEthernetNetworkNode { @display("p=515,445"); }
        // Central Canada
        winnipeg:      <default("Router")> like IEthernetNetworkNode { @display("p=640,100"); }
        // Central plains
        minneapolis:   <default("Router")> like IEthernetNetworkNode { @display("p=645,200"); }
        kansasCity:    <default("Router")> like IEthernetNetworkNode { @display("p=660,275"); }
        oklahomaCity:  <default("Router")> like IEthernetNetworkNode { @display("p=635,340"); }
        stLouis:       <default("Router")> like IEthernetNetworkNode { @display("p=710,270"); }
        // South Central
        dallas:        <default("Router")> like IEthernetNetworkNode { @display("p=620,400"); }
        houston:       <default("Router")> like IEthernetNetworkNode { @display("p=660,445"); }
        memphis:       <default("Router")> like IEthernetNetworkNode { @display("p=730,375"); }
        nashville:     <default("Router")> like IEthernetNetworkNode { @display("p=795,330"); }
        newOrleans:    <default("Router")> like IEthernetNetworkNode { @display("p=745,445"); }
        // Great Lakes
        chicago:       <default("Router")> like IEthernetNetworkNode { @display("p=760,220"); }
        indianapolis:  <default("Router")> like IEthernetNetworkNode { @display("p=775,270"); }
        cincinnati:    <default("Router")> like IEthernetNetworkNode { @display("p=825,305"); }
        detroit:       <default("Router")> like IEthernetNetworkNode { @display("p=830,205"); }
        cleveland:     <default("Router")> like IEthernetNetworkNode { @display("p=870,225"); }
        toronto:       <default("Router")> like IEthernetNetworkNode { @display("p=890,165"); }
        // Southeast
        atlanta:       <default("Router")> like IEthernetNetworkNode { @display("p=810,380"); }
        charlotte:     <default("Router")> like IEthernetNetworkNode { @display("p=880,335"); }
        tampa:         <default("Router")> like IEthernetNetworkNode { @display("p=885,430"); }
        miami:         <default("Router")> like IEthernetNetworkNode { @display("p=910,490"); }
        // Mid-Atlantic & Northeast
        pittsburgh:    <default("Router")> like IEthernetNetworkNode { @display("p=905,250"); }
        washingtonDC:  <default("Router")> like IEthernetNetworkNode { @display("p=965,305"); }
        philadelphia:  <default("Router")> like IEthernetNetworkNode { @display("p=955,270"); }
        newYork:       <default("Router")> like IEthernetNetworkNode { @display("p=985,225"); }
        montreal:      <default("Router")> like IEthernetNetworkNode { @display("p=970,125"); }
        boston:         <default("Router")> like IEthernetNetworkNode { @display("p=1010,195"); }
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        // West coast & Western Canada
        vancouver.ethg++     <--> <default("EthernetLink")> like ILink <--> seattle.ethg++;
        vancouver.ethg++     <--> <default("EthernetLink")> like ILink <--> calgary.ethg++;
        seattle.ethg++       <--> <default("EthernetLink")> like ILink <--> portland.ethg++;
        seattle.ethg++       <--> <default("EthernetLink")> like ILink <--> calgary.ethg++;
        portland.ethg++      <--> <default("EthernetLink")> like ILink <--> sacramento.ethg++;
        portland.ethg++      <--> <default("EthernetLink")> like ILink <--> saltLakeCity.ethg++;
        sacramento.ethg++    <--> <default("EthernetLink")> like ILink <--> sanFrancisco.ethg++;
        sacramento.ethg++    <--> <default("EthernetLink")> like ILink <--> saltLakeCity.ethg++;
        sanFrancisco.ethg++  <--> <default("EthernetLink")> like ILink <--> losAngeles.ethg++;
        sanFrancisco.ethg++  <--> <default("EthernetLink")> like ILink <--> lasVegas.ethg++;
        losAngeles.ethg++    <--> <default("EthernetLink")> like ILink <--> sanDiego.ethg++;
        losAngeles.ethg++    <--> <default("EthernetLink")> like ILink <--> lasVegas.ethg++;
        sanDiego.ethg++      <--> <default("EthernetLink")> like ILink <--> phoenix.ethg++;
        // Mountain West
        phoenix.ethg++       <--> <default("EthernetLink")> like ILink <--> lasVegas.ethg++;
        phoenix.ethg++       <--> <default("EthernetLink")> like ILink <--> elPaso.ethg++;
        phoenix.ethg++       <--> <default("EthernetLink")> like ILink <--> denver.ethg++;
        lasVegas.ethg++      <--> <default("EthernetLink")> like ILink <--> saltLakeCity.ethg++;
        saltLakeCity.ethg++  <--> <default("EthernetLink")> like ILink <--> calgary.ethg++;
        saltLakeCity.ethg++  <--> <default("EthernetLink")> like ILink <--> denver.ethg++;
        calgary.ethg++       <--> <default("EthernetLink")> like ILink <--> winnipeg.ethg++;
        // Southwest & South Central
        elPaso.ethg++        <--> <default("EthernetLink")> like ILink <--> dallas.ethg++;
        elPaso.ethg++        <--> <default("EthernetLink")> like ILink <--> houston.ethg++;
        denver.ethg++        <--> <default("EthernetLink")> like ILink <--> kansasCity.ethg++;
        denver.ethg++        <--> <default("EthernetLink")> like ILink <--> oklahomaCity.ethg++;
        // Central plains
        winnipeg.ethg++      <--> <default("EthernetLink")> like ILink <--> minneapolis.ethg++;
        minneapolis.ethg++   <--> <default("EthernetLink")> like ILink <--> kansasCity.ethg++;
        minneapolis.ethg++   <--> <default("EthernetLink")> like ILink <--> chicago.ethg++;
        kansasCity.ethg++    <--> <default("EthernetLink")> like ILink <--> stLouis.ethg++;
        kansasCity.ethg++    <--> <default("EthernetLink")> like ILink <--> oklahomaCity.ethg++;
        oklahomaCity.ethg++  <--> <default("EthernetLink")> like ILink <--> dallas.ethg++;
        oklahomaCity.ethg++  <--> <default("EthernetLink")> like ILink <--> memphis.ethg++;
        dallas.ethg++        <--> <default("EthernetLink")> like ILink <--> houston.ethg++;
        dallas.ethg++        <--> <default("EthernetLink")> like ILink <--> memphis.ethg++;
        houston.ethg++       <--> <default("EthernetLink")> like ILink <--> newOrleans.ethg++;
        memphis.ethg++       <--> <default("EthernetLink")> like ILink <--> nashville.ethg++;
        memphis.ethg++       <--> <default("EthernetLink")> like ILink <--> stLouis.ethg++;
        nashville.ethg++     <--> <default("EthernetLink")> like ILink <--> stLouis.ethg++;
        nashville.ethg++     <--> <default("EthernetLink")> like ILink <--> atlanta.ethg++;
        nashville.ethg++     <--> <default("EthernetLink")> like ILink <--> cincinnati.ethg++;
        newOrleans.ethg++    <--> <default("EthernetLink")> like ILink <--> atlanta.ethg++;
        newOrleans.ethg++    <--> <default("EthernetLink")> like ILink <--> tampa.ethg++;
        // Great Lakes & Midwest
        stLouis.ethg++       <--> <default("EthernetLink")> like ILink <--> chicago.ethg++;
        stLouis.ethg++       <--> <default("EthernetLink")> like ILink <--> indianapolis.ethg++;
        chicago.ethg++       <--> <default("EthernetLink")> like ILink <--> indianapolis.ethg++;
        chicago.ethg++       <--> <default("EthernetLink")> like ILink <--> detroit.ethg++;
        indianapolis.ethg++  <--> <default("EthernetLink")> like ILink <--> cincinnati.ethg++;
        cincinnati.ethg++    <--> <default("EthernetLink")> like ILink <--> cleveland.ethg++;
        cincinnati.ethg++    <--> <default("EthernetLink")> like ILink <--> pittsburgh.ethg++;
        detroit.ethg++       <--> <default("EthernetLink")> like ILink <--> cleveland.ethg++;
        detroit.ethg++       <--> <default("EthernetLink")> like ILink <--> toronto.ethg++;
        cleveland.ethg++     <--> <default("EthernetLink")> like ILink <--> pittsburgh.ethg++;
        cleveland.ethg++     <--> <default("EthernetLink")> like ILink <--> toronto.ethg++;
        toronto.ethg++       <--> <default("EthernetLink")> like ILink <--> montreal.ethg++;
        // Southeast
        atlanta.ethg++       <--> <default("EthernetLink")> like ILink <--> charlotte.ethg++;
        atlanta.ethg++       <--> <default("EthernetLink")> like ILink <--> tampa.ethg++;
        charlotte.ethg++     <--> <default("EthernetLink")> like ILink <--> washingtonDC.ethg++;
        tampa.ethg++         <--> <default("EthernetLink")> like ILink <--> miami.ethg++;
        // Mid-Atlantic & Northeast
        pittsburgh.ethg++    <--> <default("EthernetLink")> like ILink <--> philadelphia.ethg++;
        pittsburgh.ethg++    <--> <default("EthernetLink")> like ILink <--> newYork.ethg++;
        montreal.ethg++      <--> <default("EthernetLink")> like ILink <--> boston.ethg++;
        boston.ethg++         <--> <default("EthernetLink")> like ILink <--> newYork.ethg++;
        newYork.ethg++       <--> <default("EthernetLink")> like ILink <--> philadelphia.ethg++;
        philadelphia.ethg++  <--> <default("EthernetLink")> like ILink <--> washingtonDC.ethg++;
        // hosts
        for i=0..sizeof(host)-1 {
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> vancouver.ethg++     if groupIndex(numHosts,i) == 0;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> seattle.ethg++       if groupIndex(numHosts,i) == 1;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> portland.ethg++      if groupIndex(numHosts,i) == 2;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> calgary.ethg++       if groupIndex(numHosts,i) == 3;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> sacramento.ethg++    if groupIndex(numHosts,i) == 4;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> sanFrancisco.ethg++  if groupIndex(numHosts,i) == 5;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> losAngeles.ethg++    if groupIndex(numHosts,i) == 6;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> sanDiego.ethg++      if groupIndex(numHosts,i) == 7;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> lasVegas.ethg++      if groupIndex(numHosts,i) == 8;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> phoenix.ethg++       if groupIndex(numHosts,i) == 9;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> saltLakeCity.ethg++  if groupIndex(numHosts,i) == 10;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> denver.ethg++        if groupIndex(numHosts,i) == 11;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> elPaso.ethg++        if groupIndex(numHosts,i) == 12;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> winnipeg.ethg++      if groupIndex(numHosts,i) == 13;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> minneapolis.ethg++   if groupIndex(numHosts,i) == 14;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> kansasCity.ethg++    if groupIndex(numHosts,i) == 15;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> oklahomaCity.ethg++  if groupIndex(numHosts,i) == 16;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> stLouis.ethg++       if groupIndex(numHosts,i) == 17;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> dallas.ethg++        if groupIndex(numHosts,i) == 18;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> houston.ethg++       if groupIndex(numHosts,i) == 19;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> memphis.ethg++       if groupIndex(numHosts,i) == 20;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> nashville.ethg++     if groupIndex(numHosts,i) == 21;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> newOrleans.ethg++    if groupIndex(numHosts,i) == 22;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> chicago.ethg++       if groupIndex(numHosts,i) == 23;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> indianapolis.ethg++  if groupIndex(numHosts,i) == 24;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> cincinnati.ethg++    if groupIndex(numHosts,i) == 25;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> detroit.ethg++       if groupIndex(numHosts,i) == 26;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> cleveland.ethg++     if groupIndex(numHosts,i) == 27;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> toronto.ethg++       if groupIndex(numHosts,i) == 28;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> atlanta.ethg++       if groupIndex(numHosts,i) == 29;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> charlotte.ethg++     if groupIndex(numHosts,i) == 30;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> tampa.ethg++         if groupIndex(numHosts,i) == 31;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> miami.ethg++         if groupIndex(numHosts,i) == 32;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> pittsburgh.ethg++    if groupIndex(numHosts,i) == 33;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> washingtonDC.ethg++  if groupIndex(numHosts,i) == 34;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> philadelphia.ethg++  if groupIndex(numHosts,i) == 35;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> newYork.ethg++       if groupIndex(numHosts,i) == 36;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> montreal.ethg++      if groupIndex(numHosts,i) == 37;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> boston.ethg++         if groupIndex(numHosts,i) == 38;
        }
}

//
// Large US backbone network topology (56 nodes, ~90 links).
//
// A larger version of the US backbone irregular mesh preserving the
// original's character: hub nodes in the central/eastern regions with
// higher degree, sparser connectivity on the periphery, and organic
// geographic-style node placement.
//
network LargeUsBackboneNetwork extends WiredNetworkBase
{
    parameters:
        volatile int numHostsPerSwitch = default(0);
        object numHosts = default(repeatPar("numHostsPerSwitch", 56));
    submodules:
        // West coast & Pacific Northwest
        seattle:       <default("Router")> like IEthernetNetworkNode { @display("p=280,120"); }
        portland:      <default("Router")> like IEthernetNetworkNode { @display("p=280,240"); }
        sanFrancisco:  <default("Router")> like IEthernetNetworkNode { @display("p=320,360"); }
        sacramento:    <default("Router")> like IEthernetNetworkNode { @display("p=360,180"); }
        oakland:       <default("Router")> like IEthernetNetworkNode { @display("p=400,300"); }
        sanJose:       <default("Router")> like IEthernetNetworkNode { @display("p=300,460"); }
        // Mountain West
        boise:         <default("Router")> like IEthernetNetworkNode { @display("p=460,140"); }
        saltLakeCity:  <default("Router")> like IEthernetNetworkNode { @display("p=500,260"); }
        denver:        <default("Router")> like IEthernetNetworkNode { @display("p=540,380"); }
        albuquerque:   <default("Router")> like IEthernetNetworkNode { @display("p=480,440"); }
        tucson:        <default("Router")> like IEthernetNetworkNode { @display("p=420,520"); }
        reno:          <default("Router")> like IEthernetNetworkNode { @display("p=550,200"); }
        // Southwest
        elPaso:        <default("Router")> like IEthernetNetworkNode { @display("p=400,600"); }
        phoenix:       <default("Router")> like IEthernetNetworkNode { @display("p=520,560"); }
        lubbock:       <default("Router")> like IEthernetNetworkNode { @display("p=600,500"); }
        // Central plains
        omaha:         <default("Router")> like IEthernetNetworkNode { @display("p=640,160"); }
        kansasCity:    <default("Router")> like IEthernetNetworkNode { @display("p=680,280"); }
        wichita:       <default("Router")> like IEthernetNetworkNode { @display("p=660,380"); }
        oklahomacity:  <default("Router")> like IEthernetNetworkNode { @display("p=720,440"); }
        lincoln:       <default("Router")> like IEthernetNetworkNode { @display("p=620,100"); }
        tulsa:         <default("Router")> like IEthernetNetworkNode { @display("p=740,340"); }
        // Upper Midwest
        minneapolis:   <default("Router")> like IEthernetNetworkNode { @display("p=760,120"); }
        madison:       <default("Router")> like IEthernetNetworkNode { @display("p=820,200"); }
        fargo:         <default("Router")> like IEthernetNetworkNode { @display("p=800,80"); }
        duluth:        <default("Router")> like IEthernetNetworkNode { @display("p=880,140"); }
        // Great Lakes
        chicago:       <default("Router")> like IEthernetNetworkNode { @display("p=900,240"); }
        milwaukee:     <default("Router")> like IEthernetNetworkNode { @display("p=940,160"); }
        detroit:       <default("Router")> like IEthernetNetworkNode { @display("p=980,280"); }
        indianapolis:  <default("Router")> like IEthernetNetworkNode { @display("p=860,320"); }
        // South Central
        dallas:        <default("Router")> like IEthernetNetworkNode { @display("p=700,540"); }
        houston:       <default("Router")> like IEthernetNetworkNode { @display("p=760,600"); }
        sanAntonio:    <default("Router")> like IEthernetNetworkNode { @display("p=820,540"); }
        memphis:       <default("Router")> like IEthernetNetworkNode { @display("p=840,440"); }
        nashville:     <default("Router")> like IEthernetNetworkNode { @display("p=780,480"); }
        // Mid-Atlantic
        pittsburgh:    <default("Router")> like IEthernetNetworkNode { @display("p=1040,200"); }
        buffalo:       <default("Router")> like IEthernetNetworkNode { @display("p=1080,120"); }
        philadelphia:  <default("Router")> like IEthernetNetworkNode { @display("p=1100,280"); }
        baltimore:     <default("Router")> like IEthernetNetworkNode { @display("p=1060,360"); }
        richmond:      <default("Router")> like IEthernetNetworkNode { @display("p=1020,440"); }
        // Northeast
        newYork:       <default("Router")> like IEthernetNetworkNode { @display("p=1140,80"); }
        newark:        <default("Router")> like IEthernetNetworkNode { @display("p=1180,160"); }
        hartford:      <default("Router")> like IEthernetNetworkNode { @display("p=1220,80"); }
        boston:         <default("Router")> like IEthernetNetworkNode { @display("p=1260,180"); }
        washington:    <default("Router")> like IEthernetNetworkNode { @display("p=1160,240"); }
        // Southeast
        atlanta:       <default("Router")> like IEthernetNetworkNode { @display("p=940,480"); }
        charlotte:     <default("Router")> like IEthernetNetworkNode { @display("p=980,560"); }
        raleigh:       <default("Router")> like IEthernetNetworkNode { @display("p=1040,520"); }
        norfolk:       <default("Router")> like IEthernetNetworkNode { @display("p=1100,460"); }
        jacksonville:  <default("Router")> like IEthernetNetworkNode { @display("p=900,600"); }
        savannah:      <default("Router")> like IEthernetNetworkNode { @display("p=1060,600"); }
        // Florida & Gulf periphery
        tampa:         <default("Router")> like IEthernetNetworkNode { @display("p=1120,560"); }
        miami:         <default("Router")> like IEthernetNetworkNode { @display("p=1160,640"); }
        newOrleans:    <default("Router")> like IEthernetNetworkNode { @display("p=840,660"); }
        batonRouge:    <default("Router")> like IEthernetNetworkNode { @display("p=700,660"); }
        // New England periphery
        providence:    <default("Router")> like IEthernetNetworkNode { @display("p=1300,100"); }
        albany:        <default("Router")> like IEthernetNetworkNode { @display("p=1260,260"); }
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        // West coast chain
        seattle.ethg++       <--> <default("EthernetLink")> like ILink <--> portland.ethg++;
        seattle.ethg++       <--> <default("EthernetLink")> like ILink <--> sacramento.ethg++;
        portland.ethg++      <--> <default("EthernetLink")> like ILink <--> sanFrancisco.ethg++;
        portland.ethg++      <--> <default("EthernetLink")> like ILink <--> oakland.ethg++;
        sanFrancisco.ethg++  <--> <default("EthernetLink")> like ILink <--> sanJose.ethg++;
        sanFrancisco.ethg++  <--> <default("EthernetLink")> like ILink <--> oakland.ethg++;
        sacramento.ethg++    <--> <default("EthernetLink")> like ILink <--> boise.ethg++;
        sacramento.ethg++    <--> <default("EthernetLink")> like ILink <--> saltLakeCity.ethg++;
        oakland.ethg++       <--> <default("EthernetLink")> like ILink <--> saltLakeCity.ethg++;
        oakland.ethg++       <--> <default("EthernetLink")> like ILink <--> denver.ethg++;
        sanJose.ethg++       <--> <default("EthernetLink")> like ILink <--> tucson.ethg++;
        sanJose.ethg++       <--> <default("EthernetLink")> like ILink <--> elPaso.ethg++;
        // Mountain West mesh
        boise.ethg++         <--> <default("EthernetLink")> like ILink <--> reno.ethg++;
        boise.ethg++         <--> <default("EthernetLink")> like ILink <--> omaha.ethg++;
        saltLakeCity.ethg++  <--> <default("EthernetLink")> like ILink <--> denver.ethg++;
        saltLakeCity.ethg++  <--> <default("EthernetLink")> like ILink <--> reno.ethg++;
        saltLakeCity.ethg++  <--> <default("EthernetLink")> like ILink <--> kansasCity.ethg++;
        denver.ethg++        <--> <default("EthernetLink")> like ILink <--> albuquerque.ethg++;
        denver.ethg++        <--> <default("EthernetLink")> like ILink <--> wichita.ethg++;
        albuquerque.ethg++   <--> <default("EthernetLink")> like ILink <--> tucson.ethg++;
        albuquerque.ethg++   <--> <default("EthernetLink")> like ILink <--> phoenix.ethg++;
        tucson.ethg++        <--> <default("EthernetLink")> like ILink <--> elPaso.ethg++;
        tucson.ethg++        <--> <default("EthernetLink")> like ILink <--> phoenix.ethg++;
        // Southwest
        elPaso.ethg++        <--> <default("EthernetLink")> like ILink <--> batonRouge.ethg++;
        phoenix.ethg++       <--> <default("EthernetLink")> like ILink <--> lubbock.ethg++;
        lubbock.ethg++       <--> <default("EthernetLink")> like ILink <--> wichita.ethg++;
        lubbock.ethg++       <--> <default("EthernetLink")> like ILink <--> oklahomacity.ethg++;
        // Central plains hub
        reno.ethg++          <--> <default("EthernetLink")> like ILink <--> omaha.ethg++;
        reno.ethg++          <--> <default("EthernetLink")> like ILink <--> kansasCity.ethg++;
        omaha.ethg++         <--> <default("EthernetLink")> like ILink <--> lincoln.ethg++;
        omaha.ethg++         <--> <default("EthernetLink")> like ILink <--> minneapolis.ethg++;
        kansasCity.ethg++    <--> <default("EthernetLink")> like ILink <--> wichita.ethg++;
        kansasCity.ethg++    <--> <default("EthernetLink")> like ILink <--> tulsa.ethg++;
        wichita.ethg++       <--> <default("EthernetLink")> like ILink <--> oklahomacity.ethg++;
        wichita.ethg++       <--> <default("EthernetLink")> like ILink <--> tulsa.ethg++;
        oklahomacity.ethg++  <--> <default("EthernetLink")> like ILink <--> dallas.ethg++;
        oklahomacity.ethg++  <--> <default("EthernetLink")> like ILink <--> nashville.ethg++;
        lincoln.ethg++       <--> <default("EthernetLink")> like ILink <--> minneapolis.ethg++;
        tulsa.ethg++         <--> <default("EthernetLink")> like ILink <--> indianapolis.ethg++;
        tulsa.ethg++         <--> <default("EthernetLink")> like ILink <--> nashville.ethg++;
        // Upper Midwest
        minneapolis.ethg++   <--> <default("EthernetLink")> like ILink <--> madison.ethg++;
        minneapolis.ethg++   <--> <default("EthernetLink")> like ILink <--> fargo.ethg++;
        madison.ethg++       <--> <default("EthernetLink")> like ILink <--> chicago.ethg++;
        fargo.ethg++         <--> <default("EthernetLink")> like ILink <--> duluth.ethg++;
        duluth.ethg++        <--> <default("EthernetLink")> like ILink <--> milwaukee.ethg++;
        duluth.ethg++        <--> <default("EthernetLink")> like ILink <--> chicago.ethg++;
        // Great Lakes hub
        chicago.ethg++       <--> <default("EthernetLink")> like ILink <--> detroit.ethg++;
        chicago.ethg++       <--> <default("EthernetLink")> like ILink <--> indianapolis.ethg++;
        milwaukee.ethg++     <--> <default("EthernetLink")> like ILink <--> detroit.ethg++;
        milwaukee.ethg++     <--> <default("EthernetLink")> like ILink <--> pittsburgh.ethg++;
        detroit.ethg++       <--> <default("EthernetLink")> like ILink <--> pittsburgh.ethg++;
        detroit.ethg++       <--> <default("EthernetLink")> like ILink <--> philadelphia.ethg++;
        indianapolis.ethg++  <--> <default("EthernetLink")> like ILink <--> memphis.ethg++;
        indianapolis.ethg++  <--> <default("EthernetLink")> like ILink <--> baltimore.ethg++;
        // South Central
        dallas.ethg++        <--> <default("EthernetLink")> like ILink <--> houston.ethg++;
        dallas.ethg++        <--> <default("EthernetLink")> like ILink <--> batonRouge.ethg++;
        houston.ethg++       <--> <default("EthernetLink")> like ILink <--> sanAntonio.ethg++;
        houston.ethg++       <--> <default("EthernetLink")> like ILink <--> jacksonville.ethg++;
        sanAntonio.ethg++    <--> <default("EthernetLink")> like ILink <--> memphis.ethg++;
        sanAntonio.ethg++    <--> <default("EthernetLink")> like ILink <--> atlanta.ethg++;
        memphis.ethg++       <--> <default("EthernetLink")> like ILink <--> nashville.ethg++;
        memphis.ethg++       <--> <default("EthernetLink")> like ILink <--> atlanta.ethg++;
        nashville.ethg++     <--> <default("EthernetLink")> like ILink <--> sanAntonio.ethg++;
        // Mid-Atlantic hub
        pittsburgh.ethg++    <--> <default("EthernetLink")> like ILink <--> buffalo.ethg++;
        pittsburgh.ethg++    <--> <default("EthernetLink")> like ILink <--> philadelphia.ethg++;
        buffalo.ethg++       <--> <default("EthernetLink")> like ILink <--> newYork.ethg++;
        buffalo.ethg++       <--> <default("EthernetLink")> like ILink <--> newark.ethg++;
        philadelphia.ethg++  <--> <default("EthernetLink")> like ILink <--> baltimore.ethg++;
        philadelphia.ethg++  <--> <default("EthernetLink")> like ILink <--> washington.ethg++;
        baltimore.ethg++     <--> <default("EthernetLink")> like ILink <--> richmond.ethg++;
        baltimore.ethg++     <--> <default("EthernetLink")> like ILink <--> norfolk.ethg++;
        richmond.ethg++      <--> <default("EthernetLink")> like ILink <--> raleigh.ethg++;
        richmond.ethg++      <--> <default("EthernetLink")> like ILink <--> norfolk.ethg++;
        // Northeast
        newYork.ethg++       <--> <default("EthernetLink")> like ILink <--> newark.ethg++;
        newYork.ethg++       <--> <default("EthernetLink")> like ILink <--> hartford.ethg++;
        newark.ethg++        <--> <default("EthernetLink")> like ILink <--> boston.ethg++;
        newark.ethg++        <--> <default("EthernetLink")> like ILink <--> washington.ethg++;
        hartford.ethg++      <--> <default("EthernetLink")> like ILink <--> boston.ethg++;
        hartford.ethg++      <--> <default("EthernetLink")> like ILink <--> providence.ethg++;
        boston.ethg++         <--> <default("EthernetLink")> like ILink <--> providence.ethg++;
        boston.ethg++         <--> <default("EthernetLink")> like ILink <--> albany.ethg++;
        washington.ethg++    <--> <default("EthernetLink")> like ILink <--> albany.ethg++;
        // Southeast
        atlanta.ethg++       <--> <default("EthernetLink")> like ILink <--> charlotte.ethg++;
        atlanta.ethg++       <--> <default("EthernetLink")> like ILink <--> raleigh.ethg++;
        charlotte.ethg++     <--> <default("EthernetLink")> like ILink <--> raleigh.ethg++;
        charlotte.ethg++     <--> <default("EthernetLink")> like ILink <--> jacksonville.ethg++;
        charlotte.ethg++     <--> <default("EthernetLink")> like ILink <--> savannah.ethg++;
        raleigh.ethg++       <--> <default("EthernetLink")> like ILink <--> norfolk.ethg++;
        raleigh.ethg++       <--> <default("EthernetLink")> like ILink <--> tampa.ethg++;
        norfolk.ethg++       <--> <default("EthernetLink")> like ILink <--> tampa.ethg++;
        // Florida & Gulf periphery
        jacksonville.ethg++  <--> <default("EthernetLink")> like ILink <--> newOrleans.ethg++;
        savannah.ethg++      <--> <default("EthernetLink")> like ILink <--> tampa.ethg++;
        tampa.ethg++         <--> <default("EthernetLink")> like ILink <--> miami.ethg++;
        newOrleans.ethg++    <--> <default("EthernetLink")> like ILink <--> batonRouge.ethg++;
        // hosts
        for i=0..sizeof(host)-1 {
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> seattle.ethg++       if groupIndex(numHosts,i) == 0;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> portland.ethg++      if groupIndex(numHosts,i) == 1;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> sanFrancisco.ethg++  if groupIndex(numHosts,i) == 2;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> sacramento.ethg++    if groupIndex(numHosts,i) == 3;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> oakland.ethg++       if groupIndex(numHosts,i) == 4;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> sanJose.ethg++       if groupIndex(numHosts,i) == 5;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> boise.ethg++         if groupIndex(numHosts,i) == 6;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> saltLakeCity.ethg++  if groupIndex(numHosts,i) == 7;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> denver.ethg++        if groupIndex(numHosts,i) == 8;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> albuquerque.ethg++   if groupIndex(numHosts,i) == 9;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> tucson.ethg++        if groupIndex(numHosts,i) == 10;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> reno.ethg++          if groupIndex(numHosts,i) == 11;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> elPaso.ethg++        if groupIndex(numHosts,i) == 12;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> phoenix.ethg++       if groupIndex(numHosts,i) == 13;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> lubbock.ethg++       if groupIndex(numHosts,i) == 14;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> omaha.ethg++         if groupIndex(numHosts,i) == 15;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> kansasCity.ethg++    if groupIndex(numHosts,i) == 16;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> wichita.ethg++       if groupIndex(numHosts,i) == 17;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> oklahomacity.ethg++  if groupIndex(numHosts,i) == 18;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> lincoln.ethg++       if groupIndex(numHosts,i) == 19;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> tulsa.ethg++         if groupIndex(numHosts,i) == 20;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> minneapolis.ethg++   if groupIndex(numHosts,i) == 21;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> madison.ethg++       if groupIndex(numHosts,i) == 22;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> fargo.ethg++         if groupIndex(numHosts,i) == 23;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> duluth.ethg++        if groupIndex(numHosts,i) == 24;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> chicago.ethg++       if groupIndex(numHosts,i) == 25;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> milwaukee.ethg++     if groupIndex(numHosts,i) == 26;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> detroit.ethg++       if groupIndex(numHosts,i) == 27;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> indianapolis.ethg++  if groupIndex(numHosts,i) == 28;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> dallas.ethg++        if groupIndex(numHosts,i) == 29;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> houston.ethg++       if groupIndex(numHosts,i) == 30;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> sanAntonio.ethg++    if groupIndex(numHosts,i) == 31;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> memphis.ethg++       if groupIndex(numHosts,i) == 32;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> nashville.ethg++     if groupIndex(numHosts,i) == 33;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> pittsburgh.ethg++    if groupIndex(numHosts,i) == 34;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> buffalo.ethg++       if groupIndex(numHosts,i) == 35;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> philadelphia.ethg++  if groupIndex(numHosts,i) == 36;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> baltimore.ethg++     if groupIndex(numHosts,i) == 37;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> richmond.ethg++      if groupIndex(numHosts,i) == 38;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> newYork.ethg++       if groupIndex(numHosts,i) == 39;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> newark.ethg++        if groupIndex(numHosts,i) == 40;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> hartford.ethg++      if groupIndex(numHosts,i) == 41;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> boston.ethg++         if groupIndex(numHosts,i) == 42;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> washington.ethg++    if groupIndex(numHosts,i) == 43;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> atlanta.ethg++       if groupIndex(numHosts,i) == 44;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> charlotte.ethg++     if groupIndex(numHosts,i) == 45;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> raleigh.ethg++       if groupIndex(numHosts,i) == 46;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> norfolk.ethg++       if groupIndex(numHosts,i) == 47;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> jacksonville.ethg++  if groupIndex(numHosts,i) == 48;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> savannah.ethg++      if groupIndex(numHosts,i) == 49;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> tampa.ethg++         if groupIndex(numHosts,i) == 50;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> miami.ethg++         if groupIndex(numHosts,i) == 51;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> newOrleans.ethg++    if groupIndex(numHosts,i) == 52;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> batonRouge.ethg++    if groupIndex(numHosts,i) == 53;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> providence.ethg++    if groupIndex(numHosts,i) == 54;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> albany.ethg++        if groupIndex(numHosts,i) == 55;
        }
}

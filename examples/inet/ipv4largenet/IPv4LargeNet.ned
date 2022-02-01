//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.inet.ipv4largenet;

import inet.node.ethernet.EthernetHub;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.physicallayer.wired.common.WireJunction;
import ned.DatarateChannel;


//
// A 100Mb/s Ethernet cable. Part of ~IPv4LargeNet.
//
channel cable extends DatarateChannel
{
    parameters:
        delay = 0.1us;
        datarate = 100Mbps;
}


//
// Several hosts on a router; part of ~IPv4LargeNet.
//
module IPv4SmallLAN
{
    parameters:
        int h; // number of hosts on the router
        @display("i=old/cloud");
    gates:
        inout ethg;
    submodules:
        router: Router;
        host[h]: StandardHost;
    connections:
        for i=0..h-1 {
            router.ethg++ <--> cable <--> host[i].ethg++;
        }
        router.ethg++ <--> ethg;
}


//
// Several hosts and a router on an Ethernet hub and a switch
//
module IPv4MediumLAN
{
    parameters:
        int n; // number of hosts on the switch
        int h; // number of hosts on the hub
        @display("i=old/cloud");
    gates:
        inout ethg;
    submodules:
        router: Router;
        switch: EthernetSwitch;
        host[n]: StandardHost;
        hub: EthernetHub;
        hhost[h]: StandardHost;
    connections:
        for i=0..n-1 {
            switch.ethg++ <--> cable <--> host[i].ethg++;
        }

        switch.ethg++ <--> cable <--> router.ethg++;
        router.ethg++ <--> ethg;

        for i=0..h-1 {
            hub.ethg++ <--> cable <--> hhost[i].ethg++;
        }

        router.ethg++ <--> cable <--> hub.ethg++;

}


//
// Several hosts and an Ethernet hub on a switch. One port of the hub
// connect to a 10Base2 segment.
//
module IPv4LargeLAN
{
    parameters:
        int n; // number of hosts on the switch
        int h; // number of hosts on the hub
        int b; // number of hosts on the bus
        @display("i=old/cloud");
    gates:
        inout ethg;
    types:
        channel cabletobus extends DatarateChannel
        {
            parameters:
                delay = 0s;
                datarate = 10Mbps;
        }
    submodules:
        router: Router;
        switch: EthernetSwitch;
        host[n]: StandardHost;
        hub: EthernetHub;
        hhost[h]: StandardHost;
        tap[b+1]: WireJunction {
            parameters:
                @display("p=424,6,r,50");
        }
        bhost[b]: StandardHost;
    connections:
        for i=0..n-1 {
            switch.ethg++ <--> cable <--> host[i].ethg++;
        }

        switch.ethg++ <--> cable <--> router.ethg++;
        router.ethg++ <--> ethg;

        for i=0..h-1 {
            hub.ethg++ <--> cabletobus <--> hhost[i].ethg++;
        }

        router.ethg++ <--> cabletobus <--> hub.ethg++;

        for i=0..b-1 {
            tap[i].port++ <--> cabletobus <--> bhost[i].ethg++;
        }

        tap[b].port++ <--> cabletobus <--> hub.ethg++;
        for i=0..b-1 {
            tap[i].port++ <--> cabletobus <--> tap[i+1].port++;
        }
}

//
// A large Ethernet LAN -- see model description
//
network IPv4LargeNet
{
    parameters:
        int n;   // length of the "backbone" (n>5!)
        int bbs; // number of small LANs on "backbone" switches
        int bbm; // number of medium LANs on "backbone" switches
        int bbl; // number of large LANs on "backbone" switches
        int as;  // number of small LANs on switch A
        int am;  // number of medium LANs on switch A
        int al;  // number of large LANs on switch A
        int bs;  // number of small LANs on switch B
        int bm;  // number of medium LANs on switch B
        int bl;  // number of large LANs on switch B
        int cs;  // number of small LANs on switch C
        int cm;  // number of medium LANs on switch C
        int cl;  // number of large LANs on switch C
        int ds;  // number of small LANs on switch D
        int dm;  // number of medium LANs on switch D
        int dl;  // number of large LANs on switch D
    types:
        channel cabletoserver extends DatarateChannel
        {
            parameters:
                delay = 0s;
                datarate = 1Gbps;
        }
    submodules:
        routerBB[n]: Router;
        slanBB[n*bbs]: IPv4SmallLAN;
        mlanBB[n*bbm]: IPv4MediumLAN;
        llanBB[n*bbl]: IPv4LargeLAN;
        routerA: Router {
            @display("p=1300,550");
        }
        serverA: StandardHost {
            @display("p=1450,570");
        }
        slanA[as]: IPv4SmallLAN {
            @display("p=1200,650");
        }
        mlanA[am]: IPv4MediumLAN {
            @display("p=1350,650");
        }
        llanA[al]: IPv4LargeLAN {
            @display("p=1450,470");
        }
        routerB: Router {
            @display("p=350,550");
        }
        serverB: StandardHost {
            @display("p=200,570");
        }
        slanB[bs]: IPv4SmallLAN {
            @display("p=450,650");
        }
        mlanB[bm]: IPv4MediumLAN {
            @display("p=300,650");
        }
        llanB[bl]: IPv4LargeLAN {
            @display("p=200,470");
        }
        routerC: Router {
            @display("p=350,170");
        }
        serverC: StandardHost {
            @display("p=200,150");
        }
        slanC[cs]: IPv4SmallLAN {
            @display("p=450,70");
        }
        mlanC[cm]: IPv4MediumLAN {
            @display("p=300,70");
        }
        llanC[cl]: IPv4LargeLAN {
            @display("p=200,250");
        }
        routerD: Router {
            @display("p=1300,170");
        }
        serverD: StandardHost {
            @display("p=1450,150");
        }
        slanD[ds]: IPv4SmallLAN {
            @display("p=1200,70");
        }
        mlanD[dm]: IPv4MediumLAN {
            @display("p=1350,70");
        }
        llanD[dl]: IPv4LargeLAN {
            @display("p=1450,250");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100;is=s");
        }
    connections:
        // "backbone" switches
        for k=0..n-1, for i=0..bbs-1 {
            routerBB[k].ethg++ <--> cable <--> slanBB[k*bbs+i].ethg;
        }
        for k=0..n-1, for i=0..bbm-1 {
            routerBB[k].ethg++ <--> cable <--> mlanBB[k*bbm+i].ethg;
        }
        for k=0..n-1, for i=0..bbl-1 {
            routerBB[k].ethg++ <--> cable <--> llanBB[k*bbl+i].ethg;
        }
        for k=0..n-2 {
            routerBB[k].ethg++ <--> cable <--> routerBB[k+1].ethg++;
        }

        // switch A
        for i=0..as-1 {
            routerA.ethg++ <--> cable <--> slanA[i].ethg;
        }
        for i=0..am-1 {
            routerA.ethg++ <--> cable <--> mlanA[i].ethg;
        }
        for i=0..al-1 {
            routerA.ethg++ <--> cable <--> llanA[i].ethg;
        }
        routerA.ethg++ <--> cabletoserver <--> serverA.ethg++;

        // switch B
        for i=0..bs-1 {
            routerB.ethg++ <--> cable <--> slanB[i].ethg;
        }
        for i=0..bm-1 {
            routerB.ethg++ <--> cable <--> mlanB[i].ethg;
        }
        for i=0..bl-1 {
            routerB.ethg++ <--> cable <--> llanB[i].ethg;
        }
        routerB.ethg++ <--> cabletoserver <--> serverB.ethg++;

        // switch C
        for i=0..cs-1 {
            routerC.ethg++ <--> cable <--> slanC[i].ethg;
        }
        for i=0..cm-1 {
            routerC.ethg++ <--> cable <--> mlanC[i].ethg;
        }
        for i=0..cl-1 {
            routerC.ethg++ <--> cable <--> llanC[i].ethg;
        }
        routerC.ethg++ <--> cabletoserver <--> serverC.ethg++;

        // switch D
        for i=0..ds-1 {
            routerD.ethg++ <--> cable <--> slanD[i].ethg;
        }
        for i=0..dm-1 {
            routerD.ethg++ <--> cable <--> mlanD[i].ethg;
        }
        for i=0..dl-1 {
            routerD.ethg++ <--> cable <--> llanD[i].ethg;
        }
        routerD.ethg++ <--> cabletoserver <--> serverD.ethg++;

        // connect switches
        routerA.ethg++ <--> cable <--> routerB.ethg++;
        routerB.ethg++ <--> cable <--> routerC.ethg++;
        routerC.ethg++ <--> cable <--> routerD.ethg++;
        routerD.ethg++ <--> cable <--> routerA.ethg++;
        routerB.ethg++ <--> cable <--> routerBB[0].ethg++;
        routerD.ethg++ <--> cable <--> routerBB[int(n/2)].ethg++;
        routerBB[0].ethg++ <--> cable <--> routerBB[n-1].ethg++;

}


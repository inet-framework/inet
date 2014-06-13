//
// Copyright (C) 2012 Opensim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

package inet.examples.inet.ipv4largenet;

import inet.linklayer.ethernet.EtherBus;
import inet.linklayer.ethernet.EtherHub;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
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
        @display("i=cloud_s");
    gates:
        inout ethg;
    submodules:
        router: Router {
            @display("is=s");
        }
        host[h]: StandardHost {
            @display("is=s");
        }
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
        @display("i=cloud_s");
    gates:
        inout ethg;
    submodules:
        router: Router {
            @display("is=s");
        }
        switch: EtherSwitch {
            @display("is=s");
        }
        host[n]: StandardHost {
            @display("is=s");
        }
        hub: EtherHub {
            @display("is=s");
        }
        hhost[h]: StandardHost {
            @display("is=s");
        }
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
        @display("i=cloud");
    gates:
        inout ethg;
    types:
        channel cabletobus extends DatarateChannel
        {
            parameters:
                delay = 0;
                datarate = 10Mbps;
        }
    submodules:
        router: Router {
            @display("is=s");
        }
        switch: EtherSwitch {
            @display("is=s");
        }
        host[n]: StandardHost {
            @display("is=s");
        }
        hub: EtherHub {
            @display("is=s");
        }
        hhost[h]: StandardHost {
            @display("is=s");
        }
        bus: EtherBus {
            parameters:
                positions = "5 10 15"; // every 5 meters
                propagationSpeed = 2e8 mps;  // 1us = 200m
                @display("b=424,6;o=#408060");
        }
        bhost[b]: StandardHost {
            parameters:
                @display("is=s;p=,r");
        }
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
            bus.ethg++ <--> cabletobus <--> bhost[i].ethg++;
        }

        bus.ethg++ <--> cabletobus <--> hub.ethg++;
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
                delay = 0;
                datarate = 1Gbps;
        }
    submodules:
        routerBB[n]: Router;
        slanBB[n*bbs]: IPv4SmallLAN;
        mlanBB[n*bbm]: IPv4MediumLAN;
        llanBB[n*bbl]: IPv4LargeLAN;
        routerA: Router;
        serverA: StandardHost;
        slanA[as]: IPv4SmallLAN;
        mlanA[am]: IPv4MediumLAN;
        llanA[al]: IPv4LargeLAN;
        routerB: Router;
        serverB: StandardHost;
        slanB[bs]: IPv4SmallLAN;
        mlanB[bm]: IPv4MediumLAN;
        llanB[bl]: IPv4LargeLAN;
        routerC: Router;
        serverC: StandardHost;
        slanC[cs]: IPv4SmallLAN;
        mlanC[cm]: IPv4MediumLAN;
        llanC[cl]: IPv4LargeLAN;
        routerD: Router;
        serverD: StandardHost;
        slanD[ds]: IPv4SmallLAN;
        mlanD[dm]: IPv4MediumLAN;
        llanD[dl]: IPv4LargeLAN;
        configurator: IPv4NetworkConfigurator {
            @display("p=121,37");
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
        routerD.ethg++ <--> cable <--> routerBB[n/2].ethg++;
        routerBB[0].ethg++ <--> cable <--> routerBB[n-1].ethg++;

}


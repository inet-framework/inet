//
// Copyright (C) 2003 CTIE, Monash University
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


package inet.examples.ethernet.lans;

import inet.linklayer.ethernet.EtherBus;
import inet.linklayer.ethernet.EtherHub;
import inet.node.ethernet.EtherHost;
import inet.node.ethernet.EtherSwitch;
import ned.DatarateChannel;


//
// A 100Mb/s Ethernet cable. Part of ~LargeNet.
//
channel cable extends DatarateChannel
{
    parameters:
        delay = 0.1us;
        datarate = 100Mbps;
}


//
// Several hosts on an Ethernet hub; part of ~LargeNet.
//
module SmallLAN
{
    parameters:
        int h; // number of hosts on the hub
        @display("i=old/cloud_s");
    gates:
        inout ethg;
    submodules:
        hub: EtherHub {
            @display("is=s");
        }
        host[h]: EtherHost {
            @display("is=s");
        }
    connections:
        for i=0..h-1 {
            hub.ethg++ <--> cable <--> host[i].ethg;
        }
        hub.ethg++ <--> ethg;
}


//
// Several hosts and an Ethernet hub on a switch; part of ~LargeNet.
//
module MediumLAN
{
    parameters:
        int n; // number of hosts on the switch
        int h; // number of hosts on the hub
        @display("i=old/cloud_s");
    gates:
        inout ethg;
    submodules:
        switch: EtherSwitch {
            @display("is=s");
        }
        host[n]: EtherHost {
            @display("is=s");
        }
        hub: EtherHub {
            @display("is=s");
        }
        hhost[h]: EtherHost {
            @display("is=s");
        }
    connections:
        for i=0..n-1 {
            switch.ethg++ <--> cable <--> host[i].ethg;
        }

        switch.ethg++ <--> ethg;

        for i=0..h-1 {
            hub.ethg++ <--> cable <--> hhost[i].ethg;
        }

        switch.ethg++ <--> cable <--> hub.ethg++;

}


//
// Several hosts and an Ethernet hub on a switch. One port of the hub
// connect to a 10Base2 segment. Part of ~LargeNet.
//
module LargeLAN
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
                delay = 0;
                datarate = 10Mbps;
        }
    submodules:
        switch: EtherSwitch {
            @display("is=s");
        }
        host[n]: EtherHost {
            @display("is=s");
        }
        hub: EtherHub {
            @display("is=s");
        }
        hhost[h]: EtherHost {
            @display("is=s");
        }
        bus: EtherBus {
            parameters:
                positions = "5 10 15"; // every 5 meters
                propagationSpeed = 2e8 mps;  // 1us = 200m
                @display("b=424,6;o=#408060");
        }
        bhost[b]: EtherHost {
            parameters:
                @display("is=s;p=,r");
        }
    connections:
        for i=0..n-1 {
            switch.ethg++ <--> cable <--> host[i].ethg;
        }

        switch.ethg++ <--> ethg;

        for i=0..h-1 {
            hub.ethg++ <--> cabletobus <--> hhost[i].ethg;
        }

        switch.ethg++ <--> cabletobus <--> hub.ethg++;

        for i=0..b-1 {
            bus.ethg++ <--> cabletobus <--> bhost[i].ethg;
        }

        bus.ethg++ <--> cabletobus <--> hub.ethg++;
}

//
// A large Ethernet LAN -- see model description
// <a href="largenet.html">here</a>.
//
network LargeNet
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
        switchBB[n]: EtherSwitch {
            @display("is=s");
        }
        slanBB[n*bbs]: SmallLAN;
        mlanBB[n*bbm]: MediumLAN;
        llanBB[n*bbl]: LargeLAN;
        switchA: EtherSwitch;
        serverA: EtherHost;
        slanA[as]: SmallLAN;
        mlanA[am]: MediumLAN;
        llanA[al]: LargeLAN;
        switchB: EtherSwitch;
        serverB: EtherHost;
        slanB[bs]: SmallLAN;
        mlanB[bm]: MediumLAN;
        llanB[bl]: LargeLAN;
        switchC: EtherSwitch;
        serverC: EtherHost;
        slanC[cs]: SmallLAN;
        mlanC[cm]: MediumLAN;
        llanC[cl]: LargeLAN;
        switchD: EtherSwitch;
        serverD: EtherHost;
        slanD[ds]: SmallLAN;
        mlanD[dm]: MediumLAN;
        llanD[dl]: LargeLAN;
    connections:
        // "backbone" switches
        for k=0..n-1, for i=0..bbs-1 {
            switchBB[k].ethg++ <--> cable <--> slanBB[k*bbs+i].ethg;
        }
        for k=0..n-1, for i=0..bbm-1 {
            switchBB[k].ethg++ <--> cable <--> mlanBB[k*bbm+i].ethg;
        }
        for k=0..n-1, for i=0..bbl-1 {
            switchBB[k].ethg++ <--> cable <--> llanBB[k*bbl+i].ethg;
        }
        for k=0..n-2 {
            switchBB[k].ethg++ <--> cable <--> switchBB[k+1].ethg++;
        }

        // switch A
        for i=0..as-1 {
            switchA.ethg++ <--> cable <--> slanA[i].ethg;
        }
        for i=0..am-1 {
            switchA.ethg++ <--> cable <--> mlanA[i].ethg;
        }
        for i=0..al-1 {
            switchA.ethg++ <--> cable <--> llanA[i].ethg;
        }
        switchA.ethg++ <--> cabletoserver <--> serverA.ethg;

        // switch B
        for i=0..bs-1 {
            switchB.ethg++ <--> cable <--> slanB[i].ethg;
        }
        for i=0..bm-1 {
            switchB.ethg++ <--> cable <--> mlanB[i].ethg;
        }
        for i=0..bl-1 {
            switchB.ethg++ <--> cable <--> llanB[i].ethg;
        }
        switchB.ethg++ <--> cabletoserver <--> serverB.ethg;

        // switch C
        for i=0..cs-1 {
            switchC.ethg++ <--> cable <--> slanC[i].ethg;
        }
        for i=0..cm-1 {
            switchC.ethg++ <--> cable <--> mlanC[i].ethg;
        }
        for i=0..cl-1 {
            switchC.ethg++ <--> cable <--> llanC[i].ethg;
        }
        switchC.ethg++ <--> cabletoserver <--> serverC.ethg;

        // switch D
        for i=0..ds-1 {
            switchD.ethg++ <--> cable <--> slanD[i].ethg;
        }
        for i=0..dm-1 {
            switchD.ethg++ <--> cable <--> mlanD[i].ethg;
        }
        for i=0..dl-1 {
            switchD.ethg++ <--> cable <--> llanD[i].ethg;
        }
        switchD.ethg++ <--> cabletoserver <--> serverD.ethg;

        // connect switches
        switchA.ethg++ <--> cable <--> switchB.ethg++;
        switchB.ethg++ <--> cable <--> switchC.ethg++;
        switchC.ethg++ <--> cable <--> switchD.ethg++;
        switchB.ethg++ <--> cable <--> switchBB[4].ethg++;

}



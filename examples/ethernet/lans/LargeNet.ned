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
import ned.TransmissionChannel;


//
// A 100Mb/s Ethernet cable. Part of ~LargeNet.
//
channel cable extends TransmissionChannel
{
    delay = 0.1us;
    double datarate @unit(bps) = 100Mbps;
}

//
// Several hosts on an Ethernet hub; part of ~LargeNet.
//
module SmallLAN
{
    parameters:
        int h; // number of hosts on the hub
        @display("i=old/cloud");
    gates:
        inout ethg;
    submodules:
        switch: EtherSwitch {
            parameters:
                numEthInterfaces = 2;
                eth[1].duplexMode = false;
                @display("p=625,100");
        }
        hub: EtherHub {
            @display("p=400,100");
        }
        host[h]: EtherHost {
            parameters:
                csmacdSupport = true;
                eth.duplexMode = false;
                @display("p=100,200,row,150");
        }
    connections:
        switch.ethg[0] <--> ethg;
        hub.ethg++ <--> cable <--> switch.ethg[1];
        for i=0..h-1 {
            hub.ethg++ <--> cable <--> host[i].ethg;
        }
}

//
// Several hosts and an Ethernet hub on a switch; part of ~LargeNet.
//
module MediumLAN
{
    parameters:
        int n; // number of hosts on the switch
        int h; // number of hosts on the hub
        @display("i=old/cloud");
    gates:
        inout ethg;
    submodules:
        switch: EtherSwitch {
            parameters:
                numEthInterfaces = n+2;
                eth[0].duplexMode = false;
                @display("p=625,100");
        }
        host[n]: EtherHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("p=100,200,row,150");
        }
        hub: EtherHub {
            parameters:
                @display("p=625,300");
        }
        hhost[h]: EtherHost {
            parameters:
                csmacdSupport = true;
                eth.duplexMode = false;
                @display("p=325,400,row,150");
        }
    connections:
        // half-duplex:
        switch.ethg[0] <--> cable <--> hub.ethg++;

        for i=0..h-1 {
            hub.ethg++ <--> cable <--> hhost[i].ethg;
        }

        // full-duplex:
        switch.ethg[1] <--> ethg;
        for i=0..n-1 {
            switch.ethg[i+2] <--> cable <--> host[i].ethg;
        }
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
        channel cabletobus extends TransmissionChannel
        {
            double datarate @unit(bps) = 10Mbps;
            delay = 0s;
        }
    submodules:
        switch: EtherSwitch {
            parameters:
                eth[0].duplexMode = false;
                @display("p=450,100");
        }
        host[n]: EtherHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("p=100,200,row,100");
        }
        hub: EtherHub {
            parameters:
                @display("p=900,100");
        }
        hhost[h]: EtherHost {
            parameters:
                csmacdSupport = true;
                eth.duplexMode = false;
                @display("p=1050,100,col,100");
        }
        bus: EtherBus {
            parameters:
                positions = "5 10 15"; // every 5 meters
                propagationSpeed = 2e8 mps;  // 1us = 200m
                @display("b=800,5;o=#408060;p=450,300");
        }
        bhost[b]: EtherHost {
            parameters:
                csmacdSupport = true;
                eth.duplexMode = false;
                @display("p=100,400,row,100");
        }
    connections:
        switch.ethg++ <--> cabletobus <--> hub.ethg++;

        for i=0..n-1 {
            switch.ethg++ <--> cable <--> host[i].ethg;
        }

        switch.ethg++ <--> ethg;

        for i=0..h-1 {
            hub.ethg++ <--> cabletobus <--> hhost[i].ethg;
        }

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
        channel cabletoserver extends TransmissionChannel
        {
            delay = 0s;
            double datarate @unit(bps) = 1Gbps;
        }
    submodules:
        switchBB[n]: EtherSwitch {
            @display("p=200,180,row,200");
        }
        slanBB[n*bbs]: SmallLAN {
            @display("p=100,130,row,200");
        }
        mlanBB[n*bbm]: MediumLAN {
            @display("p=200,80,row,200");
        }
        llanBB[n*bbl]: LargeLAN {
            @display("p=200,280,row,200");
        }
        switchA: EtherSwitch {
            @display("p=400,580");
        }
        serverA: EtherHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("p=550,580");
        }
        slanA[as]: SmallLAN {
            @display("p=250,580");
        }
        mlanA[am]: MediumLAN {
            @display("p=325,680");
        }
        llanA[al]: LargeLAN {
            @display("p=475,680");
        }
        switchB: EtherSwitch {
            @display("p=700,380");
        }
        serverB: EtherHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("p=850,380");
        }
        slanB[bs]: SmallLAN {
            @display("p=550,380");
        }
        mlanB[bm]: MediumLAN {
            @display("p=625,480");
        }
        llanB[bl]: LargeLAN {
            @display("p=775,480");
        }
        switchC: EtherSwitch {
            @display("p=1000,580");
        }
        serverC: EtherHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("p=1150,580");
        }
        slanC[cs]: SmallLAN {
            @display("p=850,580");
        }
        mlanC[cm]: MediumLAN {
            @display("p=925,680");
        }
        llanC[cl]: LargeLAN {
            @display("p=1075,680");
        }
        switchD: EtherSwitch {
            @display("p=1300,380");
        }
        serverD: EtherHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("p=1450,380");
        }
        slanD[ds]: SmallLAN {
            @display("p=1150,380");
        }
        mlanD[dm]: MediumLAN {
            @display("p=1225,480");
        }
        llanD[dl]: LargeLAN {
            @display("p=1375,480");
        }
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


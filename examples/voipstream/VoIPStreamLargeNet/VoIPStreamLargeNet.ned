//
// Copyright (C) 2003 CTIE, Monash University
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//


package inet.examples.voipstream.VoIPStreamLargeNet;

import inet.node.ethernet.EthernetHub;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.EthernetHost;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.StandardHost;
import inet.physicallayer.wired.common.WireJunction;
import ned.DatarateChannel;


channel cable extends DatarateChannel
{
    parameters:
        delay = 0.1us;
        datarate = 100Mbps;
}


//
// Several hosts on an Ethernet hub
//
module VoIPStreamSmallLAN
{
    parameters:
        int h; // number of hosts on the hub
        @display("i=old/cloud_s");
    gates:
        inout ethg;
    submodules:
        switch: EthernetSwitch {
            parameters:
                numEthInterfaces = 2;
                eth[1].duplexMode = false;
                @display("p=625,100");
        }
        hub: EthernetHub {
            @display("is=s");
        }
        host[h]: EthernetHost {
            parameters:
                eth.duplexMode = false;
                @display("is=s");
        }
    connections:
        switch.ethg[0] <--> ethg;
        hub.ethg++ <--> cable <--> switch.ethg[1];
        for i=0..h-1 {
            hub.ethg++ <--> cable <--> host[i].ethg;
        }
}


//
// Several hosts and an Ethernet hub on a switch
//
module VoIPStreamMediumLAN
{
    parameters:
        int n; // number of hosts on the switch
        int h; // number of hosts on the hub
        @display("i=old/cloud_s");
    gates:
        inout ethg;
    submodules:
        switch: EthernetSwitch {
            parameters:
                numEthInterfaces = parent.n+2;
                eth[0].duplexMode = false;
                @display("is=s");
        }
        host[n]: EthernetHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("is=s");
        }
        hub: EthernetHub {
            @display("is=s");
        }
        hhost[h]: EthernetHost {
            parameters:
                csmacdSupport = true;
                eth.duplexMode = false;
                @display("is=s");
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
// connect to a 10Base2 segment.
//
module VoIPStreamLargeLAN
{
    parameters:
        int n; // number of hosts on the switch
        int h; // number of hosts on the hub
        int b; // number of hosts on the bus
        @display("i=old/cloud");
    gates:
        inout ethg;
    submodules:
        switch: EthernetSwitch {
            parameters:
                eth[0].duplexMode = false;
                @display("is=s");
        }
        host[n]: EthernetHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("is=s");
        }
        hub: EthernetHub {
            parameters:
                @display("is=s");
        }
        hhost[h]: EthernetHost {
            parameters:
                csmacdSupport = true;
                eth.duplexMode = false;
                @display("is=s");
        }
        tap[b+1]: WireJunction {
            parameters:
                @display("p=,,r,50");
        }
        bhost[b]: EthernetHost {
            parameters:
                csmacdSupport = true;
                eth.duplexMode = false;
                @display("is=s;p=,,r");
        }
    connections:
        switch.ethg++ <--> cable <--> hub.ethg++;

        for i=0..n-1 {
            switch.ethg++ <--> cable <--> host[i].ethg;
        }

        switch.ethg++ <--> ethg;

        for i=0..h-1 {
            hub.ethg++ <--> cable <--> hhost[i].ethg;
        }

        for i=0..b-1 {
            tap[i].port++ <--> cable <--> bhost[i].ethg;
        }
        tap[b].port++ <--> cable <--> hub.ethg++;

        for i=0..b-1 {
            tap[i].port++ <--> cable <--> tap[i+1].port++;
        }
}

//
// This is a copy of the LargeNet Ethernet demo simulation in the INET Framework,
// modified to add a VoIP server and a VoIP client. It can be used to test VoIP
// transmission on a LAN with high background traffic.
//
network VoIPStreamLargeNet
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
    submodules:
        switchBB[n]: EthernetSwitch {
            @display("is=s");
        }
        slanBB[n*bbs]: VoIPStreamSmallLAN;
        mlanBB[n*bbm]: VoIPStreamMediumLAN;
        llanBB[n*bbl]: VoIPStreamLargeLAN;
        switchA: EthernetSwitch;
        serverA: EthernetHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
        }
        slanA[as]: VoIPStreamSmallLAN;
        mlanA[am]: VoIPStreamMediumLAN;
        llanA[al]: VoIPStreamLargeLAN;
        switchB: EthernetSwitch;
        serverB: EthernetHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
        }
        slanB[bs]: VoIPStreamSmallLAN;
        mlanB[bm]: VoIPStreamMediumLAN;
        llanB[bl]: VoIPStreamLargeLAN;
        switchC: EthernetSwitch;
        serverC: EthernetHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
        }
        slanC[cs]: VoIPStreamSmallLAN;
        mlanC[cm]: VoIPStreamMediumLAN;
        llanC[cl]: VoIPStreamLargeLAN;
        switchD: EthernetSwitch;
        serverD: EthernetHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
        }
        slanD[ds]: VoIPStreamSmallLAN;
        mlanD[dm]: VoIPStreamMediumLAN;
        llanD[dl]: VoIPStreamLargeLAN;
        voipClient: StandardHost;
        voipServer: StandardHost;
        configurator: Ipv4NetworkConfigurator {
            parameters:
                config = xml("<config><interface hosts='*' address='10.x.x.x' netmask='255.x.x.x'/></config>");
                @display("p=495,160;i=block/cogwheel_s");
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
        switchA.ethg++ <--> cable <--> serverA.ethg;

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
        switchB.ethg++ <--> cable <--> serverB.ethg;

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
        switchC.ethg++ <--> cable <--> serverC.ethg;

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
        switchD.ethg++ <--> cable <--> serverD.ethg;

        // connect switches
        switchA.ethg++ <--> cable <--> switchB.ethg++;
        switchB.ethg++ <--> cable <--> switchC.ethg++;
        switchC.ethg++ <--> cable <--> switchD.ethg++;
        switchB.ethg++ <--> cable <--> switchBB[4].ethg++;

        switchA.ethg++ <--> cable <--> voipClient.ethg++;
        switchD.ethg++ <--> cable <--> voipServer.ethg++;
}


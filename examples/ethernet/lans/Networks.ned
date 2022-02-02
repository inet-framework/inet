//
// Copyright (C) 2003 CTIE, Monash University
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

package inet.examples.ethernet.lans;

import inet.node.ethernet.EthernetHub;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.Eth10M;
import inet.node.ethernet.EthernetHost;
import inet.node.ethernet.EthernetSwitch;
import inet.physicallayer.wired.common.WireJunction;
import ned.DatarateChannel;


//
// Sample Ethernet LAN containing eight hosts, a switch and a bus.
//
network MixedLAN
{
    types:
        channel C extends Eth10M
        {
            length = 1cm;
        }
    submodules:
        tap1: WireJunction {
            parameters:
                @display("p=126,307");
        }
        tap2: WireJunction {
            parameters:
                @display("p=258,307");
        }
        tap3: WireJunction {
            parameters:
                @display("p=344,307");
        }
        tap4: WireJunction {
            parameters:
                @display("p=514,307");
        }
        tap5: WireJunction {
            parameters:
                @display("p=650,307");
        }
        tap6: WireJunction {
            parameters:
                @display("p=753,307");
        }
        busHostA: EthernetHost {
            parameters:
                eth.duplexMode = false;
                @display("p=126,200");
        }
        busHostB: EthernetHost {
            parameters:
                eth.duplexMode = false;
                @display("p=258,200");
        }
        busHostC: EthernetHost {
            parameters:
                eth.duplexMode = false;
                @display("p=514,400");
        }
        busHostD: EthernetHost {
            parameters:
                eth.duplexMode = false;
                @display("p=753,400");
        }
        switchHostA: EthernetHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("p=100,500");
        }
        switchHostB: EthernetHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("p=250,500");
        }
        switchHostC: EthernetHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("p=400,500");
        }
        switchHostD: EthernetHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("p=550,500");
        }
        switch: EthernetSwitch {
            parameters:
                eth[4].duplexMode = false;
                @display("p=344,400");
            gates:
                ethg[5];
        }
        hubHostA: EthernetHost {
            parameters:
                eth.duplexMode = false;
                @display("p=500,100");
        }
        hubHostB: EthernetHost {
            parameters:
                eth.duplexMode = false;
                @display("p=650,100");
        }
        hubHostC: EthernetHost {
            parameters:
                eth.duplexMode = false;
                @display("p=800,100");
        }
        hub: EthernetHub {
            parameters:
                @display("p=650,200");
        }
    connections:
        tap1.port++ <--> Eth10M {  length = 10m; } <--> tap2.port++;
        tap2.port++ <--> Eth10M {  length = 10m; } <--> tap3.port++;
        tap3.port++ <--> Eth10M {  length = 4m; } <--> tap4.port++;
        tap4.port++ <--> Eth10M {  length = 11m; } <--> tap5.port++;
        tap5.port++ <--> Eth10M {  length = 8m; } <--> tap6.port++;

        //half-duplex:
        tap1.port++ <--> C <--> busHostA.ethg;
        tap2.port++ <--> C <--> busHostB.ethg;
        tap3.port++ <--> C <--> switch.ethg[4];
        tap4.port++ <--> C <--> busHostC.ethg;
        tap5.port++ <--> C <--> hub.ethg++;
        tap6.port++ <--> C <--> busHostD.ethg;

        //full-duplex:
        switch.ethg[0] <--> Eth100M {  length = 3m; } <--> switchHostA.ethg;
        switch.ethg[1] <--> Eth100M {  length = 2m; } <--> switchHostB.ethg;
        switch.ethg[2] <--> Eth10M {  length = 4m; } <--> switchHostC.ethg;
        switch.ethg[3] <--> Eth100M {  length = 5m; } <--> switchHostD.ethg;

        //half-duplex:
        hub.ethg++ <--> Eth10M {  length = 3m; } <--> hubHostA.ethg;
        hub.ethg++ <--> Eth10M {  length = 4m; } <--> hubHostB.ethg;
        hub.ethg++ <--> Eth10M {  length = 5m; } <--> hubHostC.ethg;
}


//
// Sample Ethernet LAN: two hosts directly connected to each other
// via twisted pair.
//
network TwoHosts
{
    submodules:
        hostA: EthernetHost {
            parameters:
                @display("p=100,100");
        }
        hostB: EthernetHost {
            parameters:
                @display("p=300,100");
        }
    connections:
        hostA.ethg <--> {  delay = 0.5us; datarate = 100Mbps; } <--> hostB.ethg;
}


//
// Sample Ethernet LAN: four hosts connected to a switch.
//
network SwitchedLAN
{
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
        }
    submodules:
        hostA: EthernetHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("p=250,100");
        }
        hostB: EthernetHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("p=400,200");
        }
        hostC: EthernetHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("p=250,300");
        }
        hostD: EthernetHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("p=100,200");
        }
        switch: EthernetSwitch {
            parameters:
                @display("p=250,200");
            gates:
                ethg[4];
        }
    connections:
        switch.ethg[0] <--> C <--> hostA.ethg;
        switch.ethg[1] <--> C <--> hostB.ethg;
        switch.ethg[2] <--> C <--> hostC.ethg;
        switch.ethg[3] <--> C <--> hostD.ethg;
}


//
// Sample Ethernet LAN: four hosts connected by a hub.
//
network HubLAN
{
    submodules:
        hostA: EthernetHost {
            parameters:
                eth.duplexMode = false;
                @display("p=250,100");
        }
        hostB: EthernetHost {
            parameters:
                eth.duplexMode = false;
                @display("p=400,200");
        }
        hostC: EthernetHost {
            parameters:
                eth.duplexMode = false;
                @display("p=250,300");
        }
        hostD: EthernetHost {
            parameters:
                eth.duplexMode = false;
                @display("p=100,200");
        }
        hub: EthernetHub {
            parameters:
                @display("p=250,200");
            gates:
                ethg[4];
        }
    connections:
        hub.ethg[0] <--> {  delay = 0.1us; datarate = 100Mbps; } <--> hostA.ethg;
        hub.ethg[1] <--> {  delay = 0.3us; datarate = 100Mbps; } <--> hostB.ethg;
        hub.ethg[2] <--> {  delay = 0.4us; datarate = 100Mbps; } <--> hostC.ethg;
        hub.ethg[3] <--> {  delay = 0.2us; datarate = 100Mbps; } <--> hostD.ethg;
}


//
// Sample Ethernet LAN: four hosts on a bus.
//
network BusLAN
{
    types:
        channel C extends Eth10M
        {
            length = 1cm;
        }
    submodules:
        hostA: EthernetHost {
            parameters:
                eth.duplexMode = false;
                @display("p=100,111");
        }
        hostB: EthernetHost {
            parameters:
                eth.duplexMode = false;
                @display("p=250,111");
        }
        hostC: EthernetHost {
            parameters:
                eth.duplexMode = false;
                @display("p=400,111");
        }
        hostD: EthernetHost {
            parameters:
                eth.duplexMode = false;
                @display("p=550,111");
        }
        tap1: WireJunction {
            parameters:
                @display("p=100,180");
        }
        tap2: WireJunction {
            parameters:
                @display("p=250,180");
        }
        tap3: WireJunction {
            parameters:
                @display("p=400,180");
        }
        tap4: WireJunction {
            parameters:
                @display("p=550,180");
        }
    connections:
        tap1.port++ <--> Eth10M <--> tap2.port++;
        tap2.port++ <--> Eth10M <--> tap3.port++;
        tap3.port++ <--> Eth10M <--> tap4.port++;
        hostA.ethg <--> C <--> tap1.port++;
        hostB.ethg <--> C <--> tap2.port++;
        hostC.ethg <--> C <--> tap3.port++;
        hostD.ethg <--> C <--> tap4.port++;
}

network SwitchedDuplexLAN
{
    types:
        channel ethernetline extends DatarateChannel
        {
            parameters:
                delay = 0.1us;
                datarate = 10Mbps;
        }

    submodules:
        hostA: EthernetHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("p=250,100");
        }
        hostB: EthernetHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("p=400,200");
        }
        hostC: EthernetHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("p=250,300");
        }
        hostD: EthernetHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("p=100,200");
        }
        switch: EthernetSwitch {
            parameters:
                @display("p=250,200");
            gates:
                ethg[4];
        }
    connections:
        switch.ethg[0] <--> ethernetline <--> hostA.ethg;
        switch.ethg[1] <--> ethernetline <--> hostB.ethg;
        switch.ethg[2] <--> ethernetline <--> hostC.ethg;
        switch.ethg[3] <--> ethernetline <--> hostD.ethg;
}


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

import inet.common.misc.ThruputMeteringChannel;
import inet.linklayer.ethernet.EtherBus;
import inet.linklayer.ethernet.EtherHub;
import inet.node.ethernet.EtherHost;
import inet.node.ethernet.EtherSwitch;
import ned.TransmissionChannel;


//
// Sample Ethernet LAN containing eight hosts, a switch and a bus.
//
network MixedLAN
{
    parameters:
        *.eth.bitrate = 100 Mbps;
        *.eth[*].bitrate = 100 Mbps;
    types:
        channel C extends TransmissionChannel {
            delay = 01us;
        }
    submodules:
        bus: EtherBus {
            parameters:
                positions = "10 20 24 35 40 48";
                propagationSpeed = 2e8 mps; // 1us = 200m
                @display("p=450,300;b=800,5;o=#408060");
            gates:
                ethg[6];
        }
        busHostA: EtherHost {
            parameters:
                eth.duplexMode = false;
                @display("p=100,200");
        }
        busHostB: EtherHost {
            parameters:
                eth.duplexMode = false;
                @display("p=250,200");
        }
        busHostC: EtherHost {
            parameters:
                eth.duplexMode = false;
                @display("p=650,400");
        }
        busHostD: EtherHost {
            parameters:
                eth.duplexMode = false;
                @display("p=800,400");
        }
        switchHostA: EtherHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("p=100,500");
        }
        switchHostB: EtherHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("p=250,500");
        }
        switchHostC: EtherHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("p=400,500");
        }
        switchHostD: EtherHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("p=550,500");
        }
        switch: EtherSwitch {
            parameters:
                eth[4].duplexMode = false;
                @display("p=325,400");
            gates:
                ethg[5];
        }
        hubHostA: EtherHost {
            parameters:
                eth.duplexMode = false;
                @display("p=500,100");
        }
        hubHostB: EtherHost {
            parameters:
                eth.duplexMode = false;
                @display("p=650,100");
        }
        hubHostC: EtherHost {
            parameters:
                eth.duplexMode = false;
                @display("p=800,100");
        }
        hub: EtherHub {
            parameters:
                @display("p=650,200");
            gates:
                ethg[4];
        }
    connections:
        //half-duplex:
        bus.ethg[0] <--> C <--> busHostA.ethg;
        bus.ethg[1] <--> C <--> busHostB.ethg;
        bus.ethg[2] <--> C <--> switch.ethg[4];
        bus.ethg[3] <--> C <--> busHostC.ethg;
        bus.ethg[4] <--> C <--> hub.ethg[3];
        bus.ethg[5] <--> C <--> busHostD.ethg;

        //full-duplex:
        switch.ethg[0] <--> C <--> switchHostA.ethg;
        switch.ethg[1] <--> C <--> switchHostB.ethg;
        switch.ethg[2] <--> C <--> switchHostC.ethg;
        switch.ethg[3] <--> C <--> switchHostD.ethg;

        //half-duplex:
        hub.ethg[0] <--> TransmissionChannel { delay = 0.1us; } <--> hubHostA.ethg;
        hub.ethg[1] <--> TransmissionChannel { delay = 0.3us; } <--> hubHostB.ethg;
        hub.ethg[2] <--> TransmissionChannel { delay = 0.2us; } <--> hubHostC.ethg;
}


//
// Sample Ethernet LAN: two hosts directly connected to each other
// via twisted pair.
//
network TwoHosts
{
    parameters:
        *.eth.bitrate = 100 Mbps;
    types:
        channel C extends TransmissionChannel {
            delay = 0.5us;
        }
    submodules:
        hostA: EtherHost {
            parameters:
                @display("p=100,100");
        }
        hostB: EtherHost {
            parameters:
                @display("p=300,100");
        }
    connections:
        hostA.ethg <--> C <--> hostB.ethg;
}


//
// Sample Ethernet LAN: four hosts connected to a switch.
//
network SwitchedLAN
{
    parameters:
        *.eth.bitrate = 100 Mbps;
        *.eth[*].bitrate = 100 Mbps;
    types:
        channel C extends TransmissionChannel {
            delay = 0.1us;
        }
    submodules:
        hostA: EtherHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("p=250,100");
        }
        hostB: EtherHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("p=400,200");
        }
        hostC: EtherHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("p=250,300");
        }
        hostD: EtherHost {
            parameters:
                csmacdSupport = false;
                eth.duplexMode = true;
                @display("p=100,200");
        }
        switch: EtherSwitch {
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
    parameters:
        *.eth.bitrate = 100 Mbps;
    submodules:
        hostA: EtherHost {
            parameters:
                eth.duplexMode = false;
                @display("p=250,100");
        }
        hostB: EtherHost {
            parameters:
                eth.duplexMode = false;
                @display("p=400,200");
        }
        hostC: EtherHost {
            parameters:
                eth.duplexMode = false;
                @display("p=250,300");
        }
        hostD: EtherHost {
            parameters:
                eth.duplexMode = false;
                @display("p=100,200");
        }
        hub: EtherHub {
            parameters:
                @display("p=250,200");
            gates:
                ethg[4];
        }
    connections:
        hub.ethg[0] <--> TransmissionChannel { delay = 0.1us; } <--> hostA.ethg;
        hub.ethg[1] <--> TransmissionChannel { delay = 0.3us; } <--> hostB.ethg;
        hub.ethg[2] <--> TransmissionChannel { delay = 0.4us; } <--> hostC.ethg;
        hub.ethg[3] <--> TransmissionChannel { delay = 0.2us; } <--> hostD.ethg;
}


//
// Sample Ethernet LAN: four hosts on a bus.
//
network BusLAN
{
    parameters:
        *.eth.bitrate = 10 Mbps;
    types:
        channel C extends TransmissionChannel {
            delay = 0s;
        }
    submodules:
        hostA: EtherHost {
            parameters:
                eth.duplexMode = false;
                @display("p=100,200");
        }
        hostB: EtherHost {
            parameters:
                eth.duplexMode = false;
                @display("p=250,200");
        }
        hostC: EtherHost {
            parameters:
                eth.duplexMode = false;
                @display("p=400,200");
        }
        hostD: EtherHost {
            parameters:
                eth.duplexMode = false;
                @display("p=550,200");
        }
        bus: EtherBus {
            parameters:
                positions = "0 10 20 30";
                propagationSpeed = 2e8 mps;
                @display("p=325,100;b=500,6;o=#408060");
            gates:
                ethg[4];
        }
    connections:
        bus.ethg[0] <--> C <--> hostA.ethg;
        bus.ethg[1] <--> C <--> hostB.ethg;
        bus.ethg[2] <--> C <--> hostC.ethg;
        bus.ethg[3] <--> C <--> hostD.ethg;
}

network SwitchedDuplexLAN
{
    parameters:
        *.eth.bitrate = 10 Mbps;
        *.eth[*].bitrate = 10 Mbps;
    types:
        channel ethernetline extends TransmissionChannel {
            parameters:
                delay = 0.1us;
        }

    submodules:
        hostA: EtherHost {
            parameters:
                csmacdSupport = false;
                @display("p=250,100");
        }
        hostB: EtherHost {
            parameters:
                csmacdSupport = false;
                @display("p=400,200");
        }
        hostC: EtherHost {
            parameters:
                csmacdSupport = false;
                @display("p=250,300");
        }
        hostD: EtherHost {
            parameters:
                csmacdSupport = false;
                @display("p=100,200");
        }
        switch: EtherSwitch {
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


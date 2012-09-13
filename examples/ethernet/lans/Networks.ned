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

import ned.DatarateChannel;
import inet.util.ThruputMeteringChannel;
import inet.nodes.ethernet.EtherSwitch;
import inet.nodes.ethernet.EtherHost;
import inet.linklayer.ethernet.EtherHub;
import inet.linklayer.ethernet.EtherBus;


//
// Sample Ethernet LAN containing eight hosts, a switch and a bus.
//
network MixedLAN
{
    types:
        channel C extends ThruputMeteringChannel
        {
            delay = 01us;
            datarate = 100Mbps;
        }
    submodules:
        bus: EtherBus {
            parameters:
                positions = "10 20 24 35 40 48";
                propagationSpeed = 2e8 mps; // 1us = 200m
                @display("p=277,182;b=424,6;o=#408060");
            gates:
                ethg[6];
        }
        busHostA: EtherHost {
            parameters:
                @display("p=122,118");
        }
        busHostB: EtherHost {
            parameters:
                @display("p=210,118");
        }
        busHostC: EtherHost {
            parameters:
                @display("p=338,238");
        }
        busHostD: EtherHost {
            parameters:
                @display("p=426,238");
        }
        switchHostA: EtherHost {
            parameters:
                @display("p=82,300");
        }
        switchHostB: EtherHost {
            parameters:
                @display("p=162,300");
        }
        switchHostC: EtherHost {
            parameters:
                @display("p=234,300");
        }
        switchHostD: EtherHost {
            parameters:
                @display("p=306,300");
        }
        switch: EtherSwitch {
            parameters:
                @display("p=194,236");
            gates:
                ethg[5];
        }
        hubHostA: EtherHost {
            parameters:
                @display("p=297,55");
        }
        hubHostB: EtherHost {
            parameters:
                @display("p=365,55");
        }
        hubHostC: EtherHost {
            parameters:
                @display("p=430,55");
        }
        hub: EtherHub {
            parameters:
                @display("p=362,132");
            gates:
                ethg[4];
        }
    connections:
        bus.ethg[0] <--> C <--> busHostA.ethg;
        bus.ethg[1] <--> C <--> busHostB.ethg;
        bus.ethg[2] <--> C <--> switch.ethg[4];
        bus.ethg[3] <--> C <--> busHostC.ethg;
        bus.ethg[4] <--> C <--> hub.ethg[3];
        bus.ethg[5] <--> C <--> busHostD.ethg;

        switch.ethg[0] <--> C <--> switchHostA.ethg;
        switch.ethg[1] <--> C <--> switchHostB.ethg;
        switch.ethg[2] <--> C <--> switchHostC.ethg;
        switch.ethg[3] <--> C <--> switchHostD.ethg;

        hub.ethg[0] <--> {  delay = 0.1us; datarate = 100Mbps; } <--> hubHostA.ethg;
        hub.ethg[1] <--> {  delay = 0.3us; datarate = 100Mbps; } <--> hubHostB.ethg;
        hub.ethg[2] <--> {  delay = 0.2us; datarate = 100Mbps; } <--> hubHostC.ethg;
}


//
// Sample Ethernet LAN: two hosts directly connected to each other
// via twisted pair.
//
network TwoHosts
{
    submodules:
        hostA: EtherHost {
            parameters:
                @display("p=60,160");
        }
        hostB: EtherHost {
            parameters:
                @display("p=200,140");
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
        hostA: EtherHost {
            parameters:
                @display("p=161,55");
        }
        hostB: EtherHost {
            parameters:
                @display("p=253,125");
        }
        hostC: EtherHost {
            parameters:
                @display("p=158,199");
        }
        hostD: EtherHost {
            parameters:
                @display("p=59,127");
        }
        switch: EtherSwitch {
            parameters:
                @display("p=162,124");
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
        hostA: EtherHost {
            parameters:
                @display("p=161,55");
        }
        hostB: EtherHost {
            parameters:
                @display("p=253,125");
        }
        hostC: EtherHost {
            parameters:
                @display("p=158,199");
        }
        hostD: EtherHost {
            parameters:
                @display("p=59,127");
        }
        hub: EtherHub {
            parameters:
                @display("p=162,124");
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
        channel C extends DatarateChannel
        {
            delay = 0;
            datarate = 10Mbps;
        }
    submodules:
        hostA: EtherHost {
            parameters:
                @display("p=100,150");
        }
        hostB: EtherHost {
            parameters:
                @display("p=200,150");
        }
        hostC: EtherHost {
            parameters:
                @display("p=300,150");
        }
        hostD: EtherHost {
            parameters:
                @display("p=400,150");
        }
        bus: EtherBus {
            parameters:
                positions = "0 10 20 30";
                propagationSpeed = 2e8 mps;
                @display("p=250,100;b=400,6;o=#408060");
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
    types:
        channel ethernetline extends DatarateChannel
        {
            parameters:
                delay = 0.1us;
                datarate = 10Mbps;
        }

        module EtherHostFullDuplex extends EtherHost
        {
            parameters:
                csmacdSupport = false;
        }

        module EtherSwitchFullDuplex extends EtherSwitch
        {
            parameters:
                csmacdSupport = false;
        }
    submodules:
        hostA: EtherHostFullDuplex {
            parameters:
                @display("p=161,55");
        }
        hostB: EtherHostFullDuplex {
            parameters:
                @display("p=253,125");
        }
        hostC: EtherHostFullDuplex {
            parameters:
                @display("p=158,199");
        }
        hostD: EtherHostFullDuplex {
            parameters:
                @display("p=59,127");
        }
        switch: EtherSwitchFullDuplex {
            parameters:
                @display("p=162,124");
            gates:
                ethg[4];
        }
    connections:
        switch.ethg[0] <--> ethernetline <--> hostA.ethg;
        switch.ethg[1] <--> ethernetline <--> hostB.ethg;
        switch.ethg[2] <--> ethernetline <--> hostC.ethg;
        switch.ethg[3] <--> ethernetline <--> hostD.ethg;
}

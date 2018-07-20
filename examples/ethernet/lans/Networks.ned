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
import ned.DatarateChannel;


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
                @display("p=450,300;b=800,5;o=#408060");
            gates:
                ethg[6];
        }
        busHostA: EtherHost {
            parameters:
                @display("p=100,200");
        }
        busHostB: EtherHost {
            parameters:
                @display("p=250,200");
        }
        busHostC: EtherHost {
            parameters:
                @display("p=650,400");
        }
        busHostD: EtherHost {
            parameters:
                @display("p=800,400");
        }
        switchHostA: EtherHost {
            parameters:
                @display("p=100,500");
        }
        switchHostB: EtherHost {
            parameters:
                @display("p=250,500");
        }
        switchHostC: EtherHost {
            parameters:
                @display("p=400,500");
        }
        switchHostD: EtherHost {
            parameters:
                @display("p=550,500");
        }
        switch: EtherSwitch {
            parameters:
                @display("p=325,400");
            gates:
                ethg[5];
        }
        hubHostA: EtherHost {
            parameters:
                @display("p=500,100");
        }
        hubHostB: EtherHost {
            parameters:
                @display("p=650,100");
        }
        hubHostC: EtherHost {
            parameters:
                @display("p=800,100");
        }
        hub: EtherHub {
            parameters:
                @display("p=650,200");
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
                @display("p=100,100");
        }
        hostB: EtherHost {
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
        hostA: EtherHost {
            parameters:
                @display("p=250,100");
        }
        hostB: EtherHost {
            parameters:
                @display("p=400,200");
        }
        hostC: EtherHost {
            parameters:
                @display("p=250,300");
        }
        hostD: EtherHost {
            parameters:
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
    submodules:
        hostA: EtherHost {
            parameters:
                @display("p=250,100");
        }
        hostB: EtherHost {
            parameters:
                @display("p=400,200");
        }
        hostC: EtherHost {
            parameters:
                @display("p=250,300");
        }
        hostD: EtherHost {
            parameters:
                @display("p=100,200");
        }
        hub: EtherHub {
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
        channel C extends DatarateChannel
        {
            delay = 0s;
            datarate = 10Mbps;
        }
    submodules:
        hostA: EtherHost {
            parameters:
                @display("p=100,200");
        }
        hostB: EtherHost {
            parameters:
                @display("p=250,200");
        }
        hostC: EtherHost {
            parameters:
                @display("p=400,200");
        }
        hostD: EtherHost {
            parameters:
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
                @display("p=250,100");
        }
        hostB: EtherHostFullDuplex {
            parameters:
                @display("p=400,200");
        }
        hostC: EtherHostFullDuplex {
            parameters:
                @display("p=250,300");
        }
        hostD: EtherHostFullDuplex {
            parameters:
                @display("p=100,200");
        }
        switch: EtherSwitchFullDuplex {
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

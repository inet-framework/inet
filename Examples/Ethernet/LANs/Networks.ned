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

import
    "EtherBus",
    "EtherHub",
    "EtherHost",
    "EtherHost2",
    "EtherSwitch",
    "EtherDuplexSwitch",
    "ChannelInstaller";


//
// Sample Ethernet LAN containing eight hosts, a switch and a bus.
//
module MixedLAN
    submodules:
        channelInstaller: ChannelInstaller;
            parameters:
                channelClass = "ThruputMeteringChannel",
                channelAttrs = "format=#N";
            display: "p=75,52;i=block/cogwheel_s";
        bus: EtherBus;
            parameters:
                positions = "10 20 24 35 40 48",
                propagationSpeed = 200000000; // 1us = 200m
            gatesizes:
                in[6],
                out[6];
            display: "p=277,182;b=424,6;o=#408060";
        busHostA: EtherHost;
            display: "p=122,118;i=device/pc2";
        busHostB: EtherHost;
            display: "p=210,118;i=device/pc2";
        busHostC: EtherHost;
            display: "p=338,238;i=device/pc2";
        busHostD: EtherHost;
            display: "p=426,238;i=device/pc2";
        switchHostA: EtherHost;
            display: "p=82,300;i=device/server";
        switchHostB: EtherHost;
            display: "p=162,300;i=device/pc2";
        switchHostC: EtherHost;
            display: "p=234,300;i=device/pc2";
        switchHostD: EtherHost;
            display: "p=306,300;i=device/pc2";
        switch: EtherSwitch;
            gatesizes:
                in[5],
                out[5];
            display: "p=194,236;i=switch2";
        hubHostA: EtherHost;
            display: "p=297,55;i=device/pc2";
        hubHostB: EtherHost;
            display: "p=365,55;i=device/pc2";
        hubHostC: EtherHost;
            display: "p=430,55;i=device/pc2";
        hub: EtherHub;
            gatesizes:
                in[4],
                out[4];
            display: "p=362,132;i=device/switch";
    connections:
        bus.in[0] <-- busHostA.out;
        bus.out[0] --> busHostA.in;

        bus.in[1] <-- busHostB.out;
        bus.out[1] --> busHostB.in;

        bus.in[2] <-- switch.out[4];
        bus.out[2] --> switch.in[4];

        bus.in[3] <-- busHostC.out;
        bus.out[3] --> busHostC.in;

        bus.in[4] <-- hub.out[3];
        bus.out[4] --> hub.in[3];

        bus.in[5] <-- busHostD.out;
        bus.out[5] --> busHostD.in;

        switch.out[0] --> delay 0.1us --> switchHostA.in;
        switch.in[0] <-- delay 0.1us <-- switchHostA.out;
        switch.out[1] --> delay 0.1us --> switchHostB.in;
        switch.in[1] <-- delay 0.1us <-- switchHostB.out;
        switch.out[2] --> delay 0.1us --> switchHostC.in;
        switch.in[2] <-- delay 0.1us <-- switchHostC.out;
        switch.out[3] --> delay 0.1us --> switchHostD.in;
        switch.in[3] <-- delay 0.1us <-- switchHostD.out;

        hub.out[0] --> delay 0.1us --> hubHostA.in;
        hub.in[0] <-- delay 0.1us <-- hubHostA.out;
        hub.out[1] --> delay 0.3us --> hubHostB.in;
        hub.in[1] <-- delay 0.3us <-- hubHostB.out;
        hub.out[2] --> delay 0.2us --> hubHostC.in;
        hub.in[2] <-- delay 0.2us <-- hubHostC.out;
endmodule


//
// Sample Ethernet LAN: two hosts directly connected to each other
// via twisted pair.
//
module TwoHosts
    submodules:
        hostA: EtherHost;
            display: "p=60,160;i=device/pc2";
        hostB: EtherHost;
            display: "p=200,140;i=device/pc2";
    connections:
        hostA.out --> delay 0.5us --> hostB.in;
        hostB.out --> delay 0.5us --> hostA.in;
endmodule


//
// Sample Ethernet LAN: four hosts connected to a switch.
//
module SwitchedLAN
    submodules:
        hostA: EtherHost;
            display: "p=161,55;i=device/server";
        hostB: EtherHost;
            display: "p=253,125;i=device/pc2";
        hostC: EtherHost;
            display: "p=158,199;i=device/pc2";
        hostD: EtherHost;
            display: "p=59,127;i=device/pc2";
        switch: EtherSwitch;
            gatesizes:
                in[4],
                out[4];
            display: "p=162,124;i=switch2";
    connections:
        switch.out[0] --> delay 0.1us --> hostA.in;
        switch.in[0] <-- delay 0.1us <-- hostA.out;
        switch.out[1] --> delay 0.1us --> hostB.in;
        switch.in[1] <-- delay 0.1us <-- hostB.out;
        switch.out[2] --> delay 0.1us --> hostC.in;
        switch.in[2] <-- delay 0.1us <-- hostC.out;
        switch.out[3] --> delay 0.1us --> hostD.in;
        switch.in[3] <-- delay 0.1us <-- hostD.out;
endmodule


//
// Sample Ethernet LAN: four hosts connected by a hub.
//
module HubLAN
    submodules:
        hostA: EtherHost;
            display: "p=161,55;i=device/server";
        hostB: EtherHost;
            display: "p=253,125;i=device/pc2";
        hostC: EtherHost;
            display: "p=158,199;i=device/pc2";
        hostD: EtherHost;
            display: "p=59,127;i=device/pc2";
        hub: EtherHub;
            gatesizes:
                in[4],
                out[4];
            display: "p=162,124;i=device/switch";
    connections:
        hub.out[0] --> delay 0.1us --> hostA.in;
        hub.in[0] <-- delay 0.1us <-- hostA.out;
        hub.out[1] --> delay 0.3us --> hostB.in;
        hub.in[1] <-- delay 0.3us <-- hostB.out;
        hub.out[2] --> delay 0.4us --> hostC.in;
        hub.in[2] <-- delay 0.4us <-- hostC.out;
        hub.out[3] --> delay 0.2us --> hostD.in;
        hub.in[3] <-- delay 0.2us <-- hostD.out;
endmodule


//
// Sample Ethernet LAN: four hosts on a bus.
//
module BusLAN
    submodules:
        hostA: EtherHost;
            display: "p=100,150;i=device/server";
        hostB: EtherHost;
            display: "p=200,150;i=device/pc2";
        hostC: EtherHost;
            display: "p=300,150;i=device/pc2";
        hostD: EtherHost;
            display: "p=400,150;i=device/pc2";
        bus: EtherBus;
            parameters:
                positions = "0 10 20 30",
                propagationSpeed = 200000000;
            gatesizes:
                in[4],
                out[4];
            display: "p=250,100;b=400,6;o=#408060";
    connections:
        bus.in[0] <-- hostA.out;
        bus.out[0] --> hostA.in;
        bus.in[1] <-- hostB.out;
        bus.out[1] --> hostB.in;
        bus.in[2] <-- hostC.out;
        bus.out[2] --> hostC.in;
        bus.in[3] <-- hostD.out;
        bus.out[3] --> hostD.in;
endmodule

channel ethernetline
    delay 0.1us;
    datarate 10*1000000;
endchannel

module SwitchedDuplexLAN
    submodules:
        hostA: EtherHost2;
            display: "p=161,55;i=device/server";
        hostB: EtherHost2;
            display: "p=253,125;i=device/pc2";
        hostC: EtherHost2;
            display: "p=158,199;i=device/pc2";
        hostD: EtherHost2;
            display: "p=59,127;i=device/pc2";
        switch: EtherSwitch2;
            gatesizes:
                in[4],
                out[4];
            display: "p=162,124;i=switch2";
    connections:
        switch.out[0] --> ethernetline --> hostA.in;
        switch.in[0] <-- ethernetline <-- hostA.out;
        switch.out[1] --> ethernetline --> hostB.in;
        switch.in[1] <-- ethernetline <-- hostB.out;
        switch.out[2] --> ethernetline --> hostC.in;
        switch.in[2] <-- ethernetline <-- hostC.out;
        switch.out[3] --> ethernetline --> hostD.in;
        switch.in[3] <-- ethernetline <-- hostD.out;
endmodule

network mixedLAN : MixedLAN
endnetwork

network twoHosts : TwoHosts
endnetwork

network switchedLAN : SwitchedLAN
endnetwork

network switchedDuplexLAN : SwitchedDuplexLAN
endnetwork

network hubLAN : HubLAN
endnetwork

network busLAN : BusLAN
endnetwork



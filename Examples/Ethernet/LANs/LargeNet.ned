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
    "EtherSwitch",
    "ChannelInstaller";

channel cable
    delay 0.1us
endchannel


//
// Several hosts on an Ethernet hub
//
module SmallLAN
    parameters:
        h: numeric const; // number of hosts on the hub
    gates:
        in: in;
        out: out;
    submodules:
        hub: EtherHub;
            display: "i=device/switch";
        host: EtherHost[h];
            display: "i=device/pc2_s";
    connections:
        for i=0..h-1 do
            hub.out++ --> cable --> host[i].in;
            hub.in++ <-- cable <-- host[i].out;
        endfor
        hub.out++ --> out;
        hub.in++ <-- in;
endmodule


//
// Several hosts and an Ethernet hub on a switch
//
module MediumLAN
    parameters:
        n: numeric const, // number of hosts on the switch
        h: numeric const; // number of hosts on the hub
    gates:
        in: in;
        out: out;
    submodules:
        switch: EtherSwitch;
            display: "i=switch2";
        host: EtherHost[n];
            display: "i=device/pc2_s";
        hub: EtherHub;
            display: "i=device/switch";
        hhost: EtherHost[h];
            display: "i=device/pc2_s";
    connections:
        for i=0..n-1 do
            switch.out++ --> cable --> host[i].in;
            switch.in++ <-- cable <-- host[i].out;
        endfor

        switch.out++ --> out;
        switch.in++ <-- in;

        for i=0..h-1 do
            hub.out++ --> cable --> hhost[i].in;
            hub.in++ <-- cable <-- hhost[i].out;
        endfor

        switch.out++ --> cable --> hub.in++;
        switch.in++ <-- cable <-- hub.out++;

endmodule


//
// Several hosts and an Ethernet hub on a switch. One port of the hub
// connect to a 10Base2 segment.
//
module LargeLAN
    parameters:
        n: numeric const, // number of hosts on the switch
        h: numeric const, // number of hosts on the hub
        b: numeric const; // number of hosts on the bus
    gates:
        in: in;
        out: out;
    submodules:
        switch: EtherSwitch;
            display: "i=switch2";
        host: EtherHost[n];
            display: "i=device/pc2_s";
        hub: EtherHub;
            display: "i=device/switch";
        hhost: EtherHost[h];
            display: "i=device/pc2_s";
        bus: EtherBus;
            parameters:
                positions = "5 10 15", // every 5 meters
                propagationSpeed = 200000000;  // 1us = 200m
            display: "b=424,6;o=#408060";
        bhost: EtherHost[b];
            display: "p=,,r;i=device/pc2_s";

    connections:
        for i=0..n-1 do
            switch.out++ --> cable --> host[i].in;
            switch.in++ <-- cable <-- host[i].out;
        endfor

        switch.out++ --> out;
        switch.in++ <-- in;

        for i=0..h-1 do
            hub.out++ --> cable --> hhost[i].in;
            hub.in++ <-- cable <-- hhost[i].out;
        endfor

        switch.out++ --> cable --> hub.in++;
        switch.in++ <-- cable <-- hub.out++;

        for i=0..b-1 do
            bus.out++ --> bhost[i].in;
            bus.in++ <-- bhost[i].out;
        endfor

        bus.out++ --> hub.in++;
        bus.in++ <-- hub.out++;

endmodule

//
// A large Ethernet LAN -- see model description
// <a href="largenet.html">here</a>.
//
module LargeNet
    parameters:
        n: numeric const,          // length of the "backbone" (n>5!)
        bbs, bbm, bbl: numeric const, // number of small/medium/large LANs on "backbone" switches
        as, am, al: numeric const, // number of small/medium/large LANs on switch A
        bs, bm, bl: numeric const, // number of small/medium/large LANs on switch B
        cs, cm, cl: numeric const, // number of small/medium/large LANs on switch C
        ds, dm, dl: numeric const; // number of small/medium/large LANs on switch D
    submodules:
        switchBB: EtherSwitch[n];
            display: "i=ball2_s";
        slanBB: SmallLAN[n*bbs];
            display: "i=cloud_s";
        mlanBB: MediumLAN[n*bbm];
            display: "i=cloud_s";
        llanBB: LargeLAN[n*bbl];
            display: "i=cloud";

        switchA: EtherSwitch;
            display: "i=ball2_s";
        serverA: EtherHost;
            display: "i=device/pc2_s";
        slanA: SmallLAN[as];
            display: "i=cloud_s";
        mlanA: MediumLAN[am];
            display: "i=cloud_s";
        llanA: LargeLAN[al];
            display: "i=cloud";

        switchB: EtherSwitch;
            display: "i=ball2_s";
        serverB: EtherHost;
            display: "i=device/pc2_s";
        slanB: SmallLAN[bs];
            display: "i=cloud_s";
        mlanB: MediumLAN[bm];
            display: "i=cloud_s";
        llanB: LargeLAN[bl];
            display: "i=cloud";

        switchC: EtherSwitch;
            display: "i=ball2_s";
        serverC: EtherHost;
            display: "i=device/pc2_s";
        slanC: SmallLAN[cs];
            display: "i=cloud_s";
        mlanC: MediumLAN[cm];
            display: "i=cloud_s";
        llanC: LargeLAN[cl];
            display: "i=cloud";

        switchD: EtherSwitch;
            display: "i=ball2_s";
        serverD: EtherHost;
            display: "i=device/pc2_s";
        slanD: SmallLAN[ds];
            display: "i=cloud_s";
        mlanD: MediumLAN[dm];
            display: "i=cloud_s";
        llanD: LargeLAN[dl];
            display: "i=cloud";

    connections nocheck:
        // "backbone" switches
        for k=0..n-1, i=0..bbs-1 do
            switchBB[k].out++ --> cable --> slanBB[k*bbs+i].in;
            switchBB[k].in++ <-- cable <-- slanBB[k*bbs+i].out;
        endfor
        for k=0..n-1, i=0..bbm-1 do
            switchBB[k].out++ --> cable --> mlanBB[k*bbm+i].in;
            switchBB[k].in++ <-- cable <-- mlanBB[k*bbm+i].out;
        endfor
        for k=0..n-1, i=0..bbl-1 do
            switchBB[k].out++ --> cable --> llanBB[k*bbl+i].in;
            switchBB[k].in++ <-- cable <-- llanBB[k*bbl+i].out;
        endfor
        for k=0..n-2 do
            switchBB[k].out++ --> cable --> switchBB[k+1].in++;
            switchBB[k].in++ <-- cable <-- switchBB[k+1].out++;
        endfor

        // switch A
        for i=0..as-1 do
            switchA.out++ --> cable --> slanA[i].in;
            switchA.in++ <-- cable <-- slanA[i].out;
        endfor
        for i=0..am-1 do
            switchA.out++ --> cable --> mlanA[i].in;
            switchA.in++ <-- cable <-- mlanA[i].out;
        endfor
        for i=0..al-1 do
            switchA.out++ --> cable --> llanA[i].in;
            switchA.in++ <-- cable <-- llanA[i].out;
        endfor
        switchA.out++ --> cable --> serverA.in;
        switchA.in++ <-- cable <-- serverA.out;

        // switch B
        for i=0..bs-1 do
            switchB.out++ --> cable --> slanB[i].in;
            switchB.in++ <-- cable <-- slanB[i].out;
        endfor
        for i=0..bm-1 do
            switchB.out++ --> cable --> mlanB[i].in;
            switchB.in++ <-- cable <-- mlanB[i].out;
        endfor
        for i=0..bl-1 do
            switchB.out++ --> cable --> llanB[i].in;
            switchB.in++ <-- cable <-- llanB[i].out;
        endfor
        switchB.out++ --> cable --> serverB.in;
        switchB.in++ <-- cable <-- serverB.out;

        // switch C
        for i=0..cs-1 do
            switchC.out++ --> cable --> slanC[i].in;
            switchC.in++ <-- cable <-- slanC[i].out;
        endfor
        for i=0..cm-1 do
            switchC.out++ --> cable --> mlanC[i].in;
            switchC.in++ <-- cable <-- mlanC[i].out;
        endfor
        for i=0..cl-1 do
            switchC.out++ --> cable --> llanC[i].in;
            switchC.in++ <-- cable <-- llanC[i].out;
        endfor
        switchC.out++ --> cable --> serverC.in;
        switchC.in++ <-- cable <-- serverC.out;

        // switch D
        for i=0..ds-1 do
            switchD.out++ --> cable --> slanD[i].in;
            switchD.in++ <-- cable <-- slanD[i].out;
        endfor
        for i=0..dm-1 do
            switchD.out++ --> cable --> mlanD[i].in;
            switchD.in++ <-- cable <-- mlanD[i].out;
        endfor
        for i=0..dl-1 do
            switchD.out++ --> cable --> llanD[i].in;
            switchD.in++ <-- cable <-- llanD[i].out;
        endfor
        switchD.out++ --> cable --> serverD.in;
        switchD.in++ <-- cable <-- serverD.out;

        // connect switches
        switchA.out++ --> cable --> switchB.in++;
        switchA.in++ <-- cable <-- switchB.out++;

        switchB.out++ --> cable --> switchC.in++;
        switchB.in++ <-- cable <-- switchC.out++;

        switchC.out++ --> cable --> switchD.in++;
        switchC.in++ <-- cable <-- switchD.out++;

        switchB.out++ --> cable --> switchBB[4].in++;
        switchB.in++ <-- cable <-- switchBB[4].out++;

endmodule


network largeNet : LargeNet
endnetwork

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
    "EtherAppCli",
    "EtherAppSrv",
    "EtherMAC",
    "EtherLLC";


//
// Example host model with one Ethernet port and several traffic generators
// to create traffic in the test network.
//
module EtherHost
    gates:
        in: in;
        out: out;
    submodules:
        cli: EtherAppCli;
            parameters:
                registerSAP = true;
            display: "p=60,60,col";
        srv: EtherAppSrv;
            parameters:
                registerSAP = true;
            display: "p=250,60,col";
        llc: EtherLLC;
            gatesizes:
                upperLayerIn[2],
                upperLayerOut[2];
            display: "p=155,120;i=block/fork";
        mac: EtherMAC;
            parameters:
                txQueueLimit = 1000, // increase if needed
                queueModule = "";
            display: "p=155,200;i=block/queue;q=queue";
    connections:
        llc.lowerLayerIn <-- mac.upperLayerOut;
        llc.lowerLayerOut --> mac.upperLayerIn;

        mac.physIn <-- in;
        mac.physOut --> out;

        cli.out --> llc.upperLayerIn[0];
        cli.in <-- llc.upperLayerOut[0];

        srv.out --> llc.upperLayerIn[1];
        srv.in <-- llc.upperLayerOut[1];
endmodule


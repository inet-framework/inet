//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//



package inet.nodes.ethernet;

import inet.applications.ethernet.EtherAppCli;
import inet.applications.ethernet.EtherAppSrv;
import inet.linklayer.ethernet.EtherLLC;
import inet.linklayer.ethernet.EtherMAC2;


//
// Example host model with one Ethernet port and several traffic generators
// to create traffic in the test network.
//
module EtherHost2
{
    parameters:
        @node();
        @labels(node,ethernet-node);
        @display("b=271,262;i=device/pc2");
    gates:
        inout ethg @labels(EtherFrame-conn);

    submodules:
        cli: EtherAppCli {
            parameters:
                registerSAP = true;
                @display("p=60,60,col");
        }
        srv: EtherAppSrv {
            parameters:
                registerSAP = true;
                @display("p=250,60,col");
        }
        llc: EtherLLC {
            parameters:
                @display("p=155,120");
            gates:
                upperLayerIn[2];
                upperLayerOut[2];
        }
        mac: EtherMAC2 {
            parameters:
                queueModule = "";
                @display("p=155,200;q=queue");
        }
    connections:
        llc.lowerLayerIn <-- mac.upperLayerOut;
        llc.lowerLayerOut --> mac.upperLayerIn;

        mac.phys <--> ethg;

        cli.out --> llc.upperLayerIn[0];
        cli.in <-- llc.upperLayerOut[0];

        srv.out --> llc.upperLayerIn[1];
        srv.in <-- llc.upperLayerOut[1];
}


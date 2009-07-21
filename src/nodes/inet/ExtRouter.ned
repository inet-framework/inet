//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//               2005 Christian Dankbar, Irene Ruengeler, Michael Tuexen
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.nodes.inet;

import inet.base.NotificationBoard;
import inet.linklayer.ethernet.EthernetInterface;
import inet.linklayer.ppp.PPPInterface;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.RoutingTable;
import inet.util.NAMTraceWriter;
import inet.linklayer.ext.ExtInterface;
import inet.nodes.inet.NetworkLayer;


//
// External router.
//
module ExtRouter
{
    parameters:
        @node();
        @labels(node,ethernet-node);
        @display("i=abstract/router;i2=status/connect");
        string routingFile = default("");
        int numExtInterfaces = default(0);
    gates:
        inout pppg[] @labels(PPPFrame-conn);
        inout ethg[] @labels(EtherFrame-conn);
    submodules:
        namTrace: NAMTraceWriter {
            parameters:
                namid = -1; // auto
                @display("p=330,60;i=block/sink");
        }
        notificationBoard: NotificationBoard {
            @display("p=60,60;i=block/control");
        }
        interfaceTable: InterfaceTable {
            @display("p=150,60;i=block/table");
        }
        routingTable: RoutingTable {
            parameters:
                IPForward = true;
                routerId = "auto";
                routingFile = routingFile;
                @display("p=240,60;i=block/table");
        }
        networkLayer: NetworkLayer {
            @display("p=184,133;i=block/fork;q=queue");
            gates:
                //ifIn[sizeof(out)+sizeof(ethOut)+sizeof(extOut)],
                //ifOut[sizeof(out)+sizeof(ethOut)+sizeof(extOut)];
                ifIn[sizeof(pppg)+sizeof(ethg)+1];
                ifOut[sizeof(pppg)+sizeof(ethg)+1];
        }
        ppp[sizeof(pppg)]: PPPInterface {
            @display("p=178,265,row,110;q=l2queue;i=block/ifcard");
        }
        eth[sizeof(ethg)]: EthernetInterface {
            @display("p=297,265,row,110;q=l2queue;i=block/ifcard");
        }
        ext[numExtInterfaces]: ExtInterface {
            @display("p=50,265,row,110;i=block/ifcard");
        }
    connections allowunconnected:
        // connections to network outside
        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;
            ppp[i].netwOut --> networkLayer.ifIn[i];
            ppp[i].netwIn <-- networkLayer.ifOut[i];
        }

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> eth[i].phys;
            eth[i].netwOut --> networkLayer.ifIn[sizeof(pppg)+i];
            eth[i].netwIn <-- networkLayer.ifOut[sizeof(pppg)+i];
        }
        for i=0..numExtInterfaces-1 {
            ext[i].netwOut --> networkLayer.ifIn[sizeof(pppg)+sizeof(ethg)+i];
            ext[i].netwIn <-- networkLayer.ifOut[sizeof(pppg)+sizeof(ethg)+i];
        }
    //ext.netwOut --> networkLayer.ifIn[sizeof(pppg)+sizeof(ethg)];
        //ext.netwIn <-- networkLayer.ifOut[sizeof(pppg)+sizeof(ethg)];
}


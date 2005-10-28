//
// Copyright (C) 2005 Andras Babos
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
    "NotificationBoard",
    "InterfaceTable",
    "RoutingTable",
    "PPPRouterInterface",
    "EthernetRouterInterface",
    "NetworkLayer",
    "OSPFRouting";



//
// \IP router.
//
module OSPFRouter
    parameters:
        routingFile: string;
    gates:
        in: in[];
        out: out[];
        in: ethIn[];
        out: ethOut[];
    submodules:
        notificationBoard: NotificationBoard;
            display: "p=60,60;i=block/control";
        interfaceTable: InterfaceTable;
            display: "p=60,130;i=block/table";
        routingTable: RoutingTable;
            parameters:
                IPForward = true,
                routerId = "auto",
                routingFile = routingFile;
            display: "p=60,200;i=block/table";
        ospf: OSPFRouting;
            parameters:
            display: "p=300,140,row;i=block/network2";
        networkLayer: NetworkLayer;
            gatesizes:
                physIn[sizeof(out)+sizeof(ethOut)],
                physOut[sizeof(out)+sizeof(ethOut)];
            display: "p=200,140;i=block/fork;q=queue";
        ppp: PPPRouterInterface[sizeof(out)];
            display: "p=90,257,row,110;q=queue;i=block/ifcard";
        eth: EthernetRouterInterface[sizeof(ethOut)];
            display: "p=145,257,row,110;q=queue;i=block/ifcard";
    connections nocheck:
        ospf.to_ip --> networkLayer.OSPFIn;
        ospf.from_ip <-- networkLayer.OSPFOut;

        // connections to network outside
        for i=0..sizeof(out)-1 do
            in[i] --> ppp[i].physIn;
            out[i] <-- ppp[i].physOut;
            ppp[i].netwOut --> networkLayer.physIn[i];
            ppp[i].netwIn <-- networkLayer.physOut[i];
        endfor;

        for i=0..sizeof(ethOut)-1 do
            ethIn[i] --> eth[i].physIn;
            ethOut[i] <-- eth[i].physOut;
            eth[i].netwOut --> networkLayer.physIn[sizeof(out)+i];
            eth[i].netwIn <-- networkLayer.physOut[sizeof(out)+i];
        endfor;
endmodule


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
    "PPPInterface",
    "EthernetInterface",
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
            display: "p=60,140;i=block/table";
        routingTable: RoutingTable;
            parameters:
                IPForward = true,
                routerId = "auto",
                routingFile = routingFile;
            display: "p=60,220;i=block/table";
        ospf: OSPFRouting;
            parameters:
            display: "p=300,140,row;i=block/network2";
        networkLayer: NetworkLayer;
            gatesizes:
                ifIn[sizeof(out)+sizeof(ethOut)],
                ifOut[sizeof(out)+sizeof(ethOut)];
            display: "p=200,140;i=block/fork;q=queue";
        ppp: PPPInterface[sizeof(out)];
            display: "p=90,257,row,110;q=l2queue;i=block/ifcard";
        eth: EthernetInterface[sizeof(ethOut)];
            display: "p=145,257,row,110;q=l2queue;i=block/ifcard";
    connections nocheck:
        ospf.to_ip --> networkLayer.OSPFIn;
        ospf.from_ip <-- networkLayer.OSPFOut;

        // connections to network outside
        for i=0..sizeof(out)-1 do
            in[i] --> ppp[i].physIn;
            out[i] <-- ppp[i].physOut;
            ppp[i].netwOut --> networkLayer.ifIn[i];
            ppp[i].netwIn <-- networkLayer.ifOut[i];
        endfor;

        for i=0..sizeof(ethOut)-1 do
            ethIn[i] --> eth[i].physIn;
            ethOut[i] <-- eth[i].physOut;
            eth[i].netwOut --> networkLayer.ifIn[sizeof(out)+i];
            eth[i].netwIn <-- networkLayer.ifOut[sizeof(out)+i];
        endfor;
endmodule


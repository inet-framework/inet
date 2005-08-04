//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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
    "RoutingTable6",
    "PPPRouterInterface",
    "EthernetRouterInterface",
    "NetworkLayer6";


//
// \IPv6 router.
//
module Router6
    gates:
        in: in[];
        out: out[];
        in: ethIn[];
        out: ethOut[];
    submodules:
        notificationBoard: NotificationBoard;
            display: "p=60,60;i=block/control";
        interfaceTable: InterfaceTable;
            display: "p=150,60;i=block/table";
        routingTable6: RoutingTable6;
            parameters:
                isRouter = true;
            display: "p=240,60;i=block/table";
        networkLayer: NetworkLayer6;
            gatesizes:
                physIn[sizeof(out)+sizeof(ethOut)],
                physOut[sizeof(out)+sizeof(ethOut)];
            display: "p=200,141;i=block/fork;q=queue";
        ppp: PPPRouterInterface[sizeof(out)];
            display: "p=90,257,row,110;q=queue;i=block/ifcard";
        eth: EthernetRouterInterface[sizeof(ethOut)];
            display: "p=145,257,row,110;q=queue;i=block/ifcard";
    connections nocheck:
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


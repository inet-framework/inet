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
    "Blackboard",
    "NetworkLayer";


//
// \IP router.
//
module Router
    parameters:
        routingFile : string;
    gates:
        in: in[];
        out: out[];
        in: ethIn[];
        out: ethOut[];
    submodules:
        blackboard: Blackboard;
            display: "p=62,60;i=block/blackboard";
        routingTable: RoutingTable;
            parameters:
                IPForward = true,
                routerId = "auto",
                routingFile = routingFile;
            display: "p=62,140;i=block/table";
        networkLayer: NetworkLayer;
            gatesizes:
                physIn[sizeof(out)+sizeof(ethOut)],
                physOut[sizeof(out)+sizeof(ethOut)];
            display: "p=200,124;i=block/fork;q=queue";
        ppp: PPPInterface[sizeof(out)];
            display: "p=90,240,row,110;q=queue;i=block/ifcard";
        eth: EthernetInterface[sizeof(ethOut)];
            display: "p=145,240,row,110;q=queue;i=block/ifcard";
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


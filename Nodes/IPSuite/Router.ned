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
            display: "i=proc2;p=70,60";
        networkLayer: NetworkLayer;
            parameters:
                // other Network parameters
                IPForward = true,
                routerId = "auto",
                routingFile = routingFile;
            gatesizes:
                physIn[sizeof(out)+sizeof(ethOut)],
                physOut[sizeof(out)+sizeof(ethOut)];
            display: "p=160,60;i=fork";
        ppp : PPPInterface[sizeof(out)];
            display: "p=80,150,row,90;q=queue;i=iface";
        eth: EthernetInterface[sizeof(ethOut)];
            display: "p=125,150,row,90;q=queue;i=iface";
    connections:
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


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
    "NetworkLayer",
    "IPTrafficGenerator",
    "PPPInterface",
    "RoutingTable";



//
// Definition of an IPNode with a transport generator that connects
// to IP directly, without TCP or UDP.
//
module BurstHost
    parameters:
        trafGenType : string,
        IPForward : bool,
        routingFile : string;
    gates:
        in: in[];
        out: out[];
    submodules:
        blackboard: Blackboard;
            display: "p=70,60;i=proc2";
        routingTable: RoutingTable;
            parameters:
                IPForward = IPForward,
                routerId = "",
                routingFile = routingFile;
            display: "p=70,130;i=box3";
        trafGen: trafGenType like IPTrafficGenerator;
            display: "i=app;p=140,60";
        networkLayer: NetworkLayer;
            gatesizes:
                physIn[1],
                physOut[1];
            display: "p=140,120;i=fork;q=queue";
        ppp: PPPInterface[1];
            display: "p=140,200,row,90;q=queue;i=iface";
    connections nocheck:
        // transport connections
        networkLayer.UDPOut --> trafGen.from_ip;
        networkLayer.UDPIn <-- trafGen.to_ip;

        // connections to network outside
        for i=0..0 do
            in[i] --> ppp[i].physIn;
            out[i] <-- ppp[i].physOut;
            ppp[i].netwOut --> networkLayer.physIn[i];
            ppp[i].netwIn <-- networkLayer.physOut[i];
        endfor;

endmodule


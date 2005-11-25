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
    "RoutingTable",
    "NetworkLayer",
    "IPTrafficGenerator",
    "PPPInterface";



//
// Definition of an IP node with a transport generator that connects
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
        notificationBoard: NotificationBoard;
            display: "p=83,66;i=block/control";
        interfaceTable: InterfaceTable;
            display: "p=83,136;i=block/table";
        routingTable: RoutingTable;
            parameters:
                IPForward = IPForward,
                routerId = "",
                routingFile = routingFile;
            display: "p=83,206;i=block/table";
        trafGen: trafGenType like IPTrafficGenerator;
            display: "i=block/app;p=198,86";
        networkLayer: NetworkLayer;
            gatesizes:
                ifIn[1],
                ifOut[1];
            display: "p=198,173;i=block/fork;q=queue";
        ppp: PPPInterface[1];
            display: "p=198,262,row,90;q=txQueue;i=block/ifcard";
    connections nocheck:
        // transport connections
        networkLayer.UDPOut --> trafGen.from_ip;
        networkLayer.UDPIn <-- trafGen.to_ip;

        // connections to network outside
        for i=0..0 do
            in[i] --> ppp[i].physIn;
            out[i] <-- ppp[i].physOut;
            ppp[i].netwOut --> networkLayer.ifIn[i];
            ppp[i].netwIn <-- networkLayer.ifOut[i];
        endfor;

endmodule


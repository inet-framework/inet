//
// Copyright (C) 2005 Vojta Janota
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
    "UDP",
    "Daemon";


//
// Quagga-based \IP router.
//
module QuaggaRouter
    parameters:
        namid: numeric,
        routingFile: string,
        routingDaemon: string; // "ripd", "ospfd", etc.
    gates:
        in: in[];
        out: out[];
        in: ethIn[];
        out: ethOut[];
    submodules:
        namTrace: NAMTraceWriter;
            parameters:
                namid = namid;
            display: "p=330,60;i=block/sink";
        notificationBoard: NotificationBoard;
            display: "p=60,60;i=block/control";
        interfaceTable: InterfaceTable;
            display: "p=150,60;i=block/table";
        routingTable: RoutingTable;
            parameters:
                IPForward = true,
                routerId = "auto",
                routingFile = routingFile;
            display: "p=240,60;i=block/table";
        networkLayer: NetworkLayer;
            gatesizes:
                ifIn[sizeof(out)+sizeof(ethOut)],
                ifOut[sizeof(out)+sizeof(ethOut)];
            display: "p=200,341;i=block/fork;q=queue";
        ppp: PPPInterface[sizeof(out)];
            display: "p=90,457,row,110;q=l2queue;i=block/ifcard";
        eth: EthernetInterface[sizeof(ethOut)];
            display: "p=145,457,row,110;q=l2queue;i=block/ifcard";
        tcp: TCP;
            display: "p=200,250;i=block/wheelbarrow";
        udp: UDP;
            display: "p=300,250;i=block/wheelbarrow";
        zebra: Daemon;
            parameters:
                server = "zebra";
            display: "p=150, 150;i=block/cogwheel";
        ospfd: Daemon;
            parameters:
                server = routingDaemon;
            display: "p=250, 150;i=block/cogwheel";
    connections nocheck:
        // connections to network outside
        for i=0..sizeof(out)-1 do
            in[i] --> ppp[i].physIn;
            out[i] <-- ppp[i].physOut;
            ppp[i].netwOut --> networkLayer.ifIn[i];
            ppp[i].netwIn <-- networkLayer.ifOut[i];
        endfor;

        tcp.to_ip --> networkLayer.TCPIn;
        tcp.from_ip <-- networkLayer.TCPOut;

        udp.to_ip --> networkLayer.UDPIn;
        udp.from_ip <-- networkLayer.UDPOut;

        zebra.tcpIn <-- tcp.to_appl++;
        zebra.tcpOut --> tcp.from_appl++;

        ospfd.tcpIn <-- tcp.to_appl++;
        ospfd.tcpOut --> tcp.from_appl++;

        ospfd.udpIn <-- udp.to_app++;
        ospfd.udpOut --> udp.from_app++;

        ospfd.ipOut --> networkLayer.OSPFIn;
        ospfd.ipIn <-- networkLayer.OSPFOut;

        for i=0..sizeof(ethOut)-1 do
            ethIn[i] --> eth[i].physIn;
            ethOut[i] <-- eth[i].physOut;
            eth[i].netwOut --> networkLayer.ifIn[sizeof(out)+i];
            eth[i].netwIn <-- networkLayer.ifOut[sizeof(out)+i];
        endfor;
endmodule


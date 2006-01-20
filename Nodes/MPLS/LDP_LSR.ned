//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//


import
    "NotificationBoard",
    "RoutingTable",
    "InterfaceTable",
    "NetworkLayer",
    "LDP",
    "TCP",
    "UDP",
    "LinkStateRouting",
    "TED",
    "MPLS",
    "PPPInterface",
    "LIBTable",
    "NAMTraceWriter";


//
// An LDP-capable router.
//
// LDP Capable Routers are main building blocks for an LDP MPLS network.
//
module LDP_LSR
    parameters:
        holdTime: numeric,
        helloInterval: numeric,

        namid: numeric,

        peers: string,
        routerId: string,
        routingFile: string;
    gates:
        in: in[];
        out: out[];

    submodules:
        notificationBoard: NotificationBoard;
            display: "p=70,60;i=block/control";
        interfaceTable: InterfaceTable;
            display: "p=70,150;i=block/table";
        routingTable: RoutingTable;
            parameters:
                IPForward = true,
                routerId = routerId,
                routingFile = routingFile;
            display: "p=70,240;i=block/table";
        ldp: LDP;
            parameters:
                holdTime = holdTime,
                helloInterval = helloInterval;
            display: "p=222,67;i=block/control";
        ted: TED;
            display: "p=340,244;i=block/table";
        linkStateRouting: LinkStateRouting;
            parameters:
                peers = peers;
            display: "p=360,150;i=block/network2";
        tcp: TCP;
            display: "p=177,145;i=block/wheelbarrow";
        udp: UDP;
            display: "p=267,145;i=block/transport";
        networkLayer: NetworkLayer;
            gatesizes:
                ifIn[sizeof(out)],
                ifOut[sizeof(out)];
            display: "p=224,223;i=block/fork";
        ppp: PPPInterface[sizeof(out)];
            display: "p=105,388,row,90;q=l2queue;i=block/ifcard";
        mpls: MPLS;
            parameters:
                //peers = peers,
                classifier = "ldp";
            gatesizes:
                netwIn[sizeof(out)],
                netwOut[sizeof(out)],
                ifIn[sizeof(out)],
                ifOut[sizeof(out)];
            display: "p=224,302;i=block/switch";
        libTable: LIBTable;
            parameters:
            display: "p=340,305;i=block/table2";
        namTrace: NAMTraceWriter;
            parameters:
                namid = namid;
            display: "p=70,310;i=old/floppy1";

    connections nocheck:
        linkStateRouting.to_ip --> networkLayer.OSPFIn;
        linkStateRouting.from_ip <-- networkLayer.OSPFOut;

        ldp.tcpIn <-- tcp.to_appl++;
        ldp.tcpOut --> tcp.from_appl++;

        ldp.udpOut --> udp.from_app++;
        ldp.udpIn <-- udp.to_app++;

        udp.to_ip --> networkLayer.UDPIn;
        udp.from_ip <-- networkLayer.UDPOut;

        tcp.to_ip --> networkLayer.TCPIn;
        tcp.from_ip <-- networkLayer.TCPOut;

        for i=0..sizeof(out)-1 do
            in[i] --> ppp[i].physIn;
            out[i] <-- ppp[i].physOut;

            ppp[i].netwOut --> mpls.ifIn[i];
            ppp[i].netwIn <-- mpls.ifOut[i];

            mpls.netwOut[i] --> networkLayer.ifIn[i];
            mpls.netwIn[i] <-- networkLayer.ifOut[i];
        endfor;
endmodule




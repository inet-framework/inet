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
    "NewLDP",
    "TCP",
    "UDP",
    "MPLSModule",
    "PPPInterface",
    "LIBTable";


//
// An LDP-capable router.
//
// LDP Capable Routers are main building blocks for an LDP MPLS network.
//
module LDP_LSR
    parameters:
        routerId : string,
        ldpTimeout : numeric,
        ldpKeepAliveTime : numeric,
        helloTimeout : numeric,
        peerNo : numeric,
        isIR : bool,
        isER : bool,
        routingFile : string,
        LibTableFileName : string,
        PrtTableFileName : string;
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
        signal_module: NewLDP;
            parameters:
                routerId = routerId,
                helloTimeout = helloTimeout,
                isER = isER,
                isIR = isIR;
            display: "p=222,67;i=block/control";
        tcp: TCP;
            display: "p=177,145;i=block/wheelbarrow";
        udp: UDP;
            display: "p=267,145;i=block/transport";
        networkLayer: NetworkLayer;
            gatesizes:
                physIn[sizeof(out)],
                physOut[sizeof(out)];
            display: "p=224,223;i=block/fork";
        ppp: PPPInterface[sizeof(out)];
            display: "p=105,388,row,90;q=queue;i=block/ifcard";
        mplsModule: MPLSModule;
            parameters:
                isIR = isIR,
                isER = isER;
            gatesizes:
                fromL3[sizeof(out)],
                toL3[sizeof(out)],
                fromL2[sizeof(out)],
                toL2[sizeof(out)];
            display: "p=224,302;i=block/switch";
        libTable: LIBTable;
            parameters:
                LibTableFileName = LibTableFileName,
                PrtTableFileName = PrtTableFileName;
            display: "p=340,281;i=block/table2";
    connections nocheck:
        signal_module.from_tcp_interface <-- tcp.to_appl++;
        signal_module.to_tcp_interface --> tcp.from_appl++;

        signal_module.to_udp_interface --> udp.from_app++;
        signal_module.from_udp_interface <-- udp.to_app++;

        udp.to_ip --> networkLayer.UDPIn;
        udp.from_ip <-- networkLayer.UDPOut;

        tcp.to_ip --> networkLayer.TCPIn;
        tcp.from_ip <-- networkLayer.TCPOut;

        for i=0..sizeof(out)-1 do
            in[i] --> ppp[i].physIn;
            out[i] <-- ppp[i].physOut;

            ppp[i].netwOut --> mplsModule.fromL2[i];
            ppp[i].netwIn <-- mplsModule.toL2[i];

            mplsModule.toL3[i] --> networkLayer.physIn[i];
            mplsModule.fromL3[i] <-- networkLayer.physOut[i];
        endfor;
endmodule




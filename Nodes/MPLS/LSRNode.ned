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
    "Blackboard",
    "NetworkLayer",
    "LDPproc",
    "tcp2ldp",
    "MPLSModule",
    "LDPInterface",
    "LIBTable",
    "UDPInterface";




//
// An LDP-capable router.
//
// LDP Capable Routers are main building blocks for an LDP MPLS network. The
// Router ID is assigned as the router local loopback interface. This is
// important for the consistency with \RSVP-TE module. In order to add the
// loopback interface, some modification has been made in the RoutingCore
// class in \IP suite. This modification can be anticipated for \IP suite
// development as the local loopback interface is necessary for some link
// state routing protocols like OSPF.
//
module LSRNode
    parameters:
        id : string,
        local_addr : string,
        ldpTimeout : numeric,
        ldpKeepAliveTime : numeric,
        udpInitTimeout : numeric,
        peerNo : numeric,
        isIR : bool,
        isER : bool,

        //# Network information
        routingFile : string,
        LibTableFileName : string,
        PrtTableFileName : string;
        //#discoveryAddr: string[];
        //#numOfAddr: int;

    gates:
        in: in[];
        out: out[];

    submodules:
        blackboard: Blackboard;
            display: "i=proc2;p=60,100";
        //# transport and application layers
        tcpApp: TCP2LDP;
            parameters:
                local_addr = local_addr,
                timeout = ldpTimeout,
                keepAliveTime = ldpKeepAliveTime;

            display: "p=125,117;b=40,24,rect";
        signal_module: LDPproc;
            parameters:
                peerNo = peerNo,
                local_addr = local_addr,
                udpInitTimeout = udpInitTimeout;
            display: "p=186,43;b=40,24";
        udpApp: UDPInterface;
            parameters:
                local_addr = local_addr;
            display: "p=246,117;b=40,24,rect";
        networkLayer: NetworkLayer;
            parameters:
                IPForward = false,
                routingFile = routingFile;
            gatesizes:
                physIn[sizeof(out)],
                physOut[sizeof(out)];
            display: "b=32,30;p=191,186;i=fork";
        ppp: PPPInterface[sizeof(out)];
            display: "p=87,320,row,90;q=queue;i=iface";
        mplsModule: MPLSModule;
            parameters:
                isIR = isIR,
                isER = isER;
            gatesizes:
                fromL3[sizeof(out)],
                toL3[sizeof(out)],
                fromL2[sizeof(out)],
                toL2[sizeof(out)];
            display: "o=#ff8000;p=191,261;b=66,14";
        libTable: LIBTable;
            parameters:
                LibTableFileName = LibTableFileName,
                PrtTableFileName = PrtTableFileName;
            display: "i=proc2;p=279,207;b=32,30";
    connections nocheck:
        //# transport connections
        networkLayer.TCPOut --> tcpApp.from_tcp;
        networkLayer.TCPIn <-- tcpApp.to_tcp;

        networkLayer.UDPOut --> udpApp.from_ip;
        networkLayer.UDPIn <-- udpApp.to_ip;
        signal_module.to_udp_interface --> udpApp.from_appl;
        signal_module.from_udp_interface <-- udpApp.to_appl;
        signal_module.from_tcp_interface <-- tcpApp.to_ldp;
        signal_module.to_tcp_interface --> tcpApp.from_ldp;
        //# ldp.to_mpls_switch --> mplsModule.fromLDP;

        for i=0..sizeof(out)-1 do
            in[i] --> ppp[i].physIn;
            out[i] <-- ppp[i].physOut;

            ppp[i].netwOut --> mplsModule.fromL2[i];
            ppp[i].netwIn <-- mplsModule.toL2[i];

            mplsModule.toL3[i] --> networkLayer.physIn[i];
            mplsModule.fromL3[i] <-- networkLayer.physOut[i];
        endfor;
endmodule




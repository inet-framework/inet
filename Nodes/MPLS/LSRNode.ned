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
    "NetworkLayersWithMPLS",
    //"ProcessorManager",
    "UDP",
    "tcp4ip",
    "ErrorHandling",
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
// important for the consistency with RSVP-TE module. In order to add the
// loopback interface, some modification has been made in the RoutingCore
// class in IP suite. This modification can be anticipated for IP suite
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
        numOfProcessors : numeric,
        numOfPorts : numeric,
        routingFile : string,
        LibTableFileName : string,
        PrtTableFileName : string;
        //#discoveryAddr: string[];
        //#numOfAddr: int;

    gates:
        in: in[];
        out: out[];

    submodules:
        //# transport and application layers
        tcpApp: TCP2LDP;
            parameters:
                local_addr = local_addr,
                timeout = ldpTimeout,
                keepAliveTime = ldpKeepAliveTime;

            display: "p=44,117;b=40,24,rect";
        signal_module: LDPproc;
            parameters:
                peerNo = peerNo,
                local_addr = local_addr,
                udpInitTimeout = udpInitTimeout;

            display: "p=102,30;b=40,24";
        udpApp: UDPInterface;
            parameters:
                local_addr = local_addr;
            display: "p=233,108;b=40,24,rect";
        errorHandling: ErrorHandling;
            parameters:
                nodename = routingFile;
            display: "b=32,30;p=275,206;i=sink";
        networkLayers: NetworkLayersWithMPLS;
            parameters:
                //# waiting times
                icmp_delay = 0,
                fragmentation_delay = 0.2 us,
                multicast_delay = 0.5 us,
                ipoutput_delay = 0,
                ipsend_delay = 0,
                iptunneling_delay = 0,
                localdeliver_delay = 0,
                prerouting_delay = 0.2 us,
                routing_delay = 1 us,
                enqueue_delay = 0,
                dequeue_delay = 0,
                inputqueue_delay = 0,
                nwi_delay = 0,

                //# hook names
                preRoutingHookName = "NF_IP_PRE_ROUTING",
                routingHookName = "NF_IP_FORWARD",
                localDeliverHookName = "NF_IP_LOCAL_IN",
                ipSendHookName = "NF_IP_LOCAL_OUT",
                ipOutputHookName = "NF_IP_POST_ROUTING",
                enqueueHookName = "L2_EnqueueHook",
                dequeueHookName = "L2_DequeueHook",

                //# other network parameters
                IPForward = false,
                NWIName = "MPLS2IP",
                numOfPorts = numOfPorts,
                routingFile = routingFile;
            gatesizes:
                physIn[numOfPorts],
                physOut[numOfPorts];
            display: "b=32,30;p=143,187;i=fork";
        mplsModule: MPLSModule;
            parameters:
                isIR = isIR,
                isER = isER,
                numOfPorts = numOfPorts,
                procdelay = 1;
            gatesizes:
                fromL3[numOfPorts],
                toL3[numOfPorts],
                fromL2[numOfPorts],
                toL2[numOfPorts];
            display: "p=133,323;b=72,42";
        libTable: LIBTable;
            parameters:
                LibTableFileName = LibTableFileName,
                PrtTableFileName = PrtTableFileName;

            display: "p=316,282;b=40,24";
        //# processorManager: ProcessorManager;
    connections nocheck:
        //# transport connections
        networkLayers.TCPOut --> tcpApp.from_tcp;
        networkLayers.TCPIn <-- tcpApp.to_tcp;

        networkLayers.UDPOut --> udpApp.from_ip;
        networkLayers.UDPIn <-- udpApp.to_ip;
        signal_module.to_udp_interface --> udpApp.from_appl;
        signal_module.from_udp_interface <-- udpApp.to_appl;
        signal_module.from_tcp_interface <-- tcpApp.to_ldp;
        signal_module.to_tcp_interface --> tcpApp.from_ldp;
        //# ldp.to_mpls_switch --> mplsModule.fromLDP;
        networkLayers.errorOut --> errorHandling.in;

        //# connections to network outside
        for i=0..numOfPorts-1 do
            mplsModule.toL3[i] --> networkLayers.physIn[i];
            mplsModule.fromL3[i] <-- networkLayers.physOut[i];
            in[i] --> mplsModule.fromL2[i];
            out[i] <-- mplsModule.toL2[i];
        endfor;
    display: "p=10,10;b=368,393";
endmodule




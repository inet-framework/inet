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
    "ProcessorManager",
    "UDP",
    "tcp4ip",
    "ErrorHandling",
    "MPLSModule",
    "LIBTable",
    "RSVPInterface",
    "RSVP",
    "OspfTe",
    "RSVPAppl";


//
// An RSVP-TE capable router.
//
// RSVP occupies the Transport layer; however, it is not a transport protocol
// itself. RSVP uses transport protocols to route packets. In the simulation,
// OSPF-TE has been built to replace the static routing available in IP-suite.
//
// An LSR consists of the following modules:
// - RSVP-TE: The signaling module.
// - RSVP Interface: This one plays the role of an adapter between layer 3
//   and layer 4 protocols. It is also to address the mismatch between the
//   two methods of representing an IP addresses.
// - Network Layers: The core of IP-suite.
// - OSPF-TE: The transport module of OSPF-TE protocol.
// - MPLS Module: Label switching module of MPLS.
// - LIB Table: The module that handles the mapping of labels, interfaces,
//   and FECs.
//
module RSVP_LSR_Node
    parameters:
        local_addr : string,
        dest_addr : string,
        isIR : bool,
        isER : bool,

        //# Network information
        numOfProcessors : numeric,
        numOfPorts : numeric,
        routingFile : string,
        LibTableFileName : string,
        PrtTableFileName : string;
        //# discoveryAddr: string[];
        //# numOfAddr: int;

    gates:
        in: in[];
        out: out[];
    submodules:
        signal_module: RSVPAppl;
            parameters:
                local_addr = local_addr,
                dest_addr = dest_addr,
                isIR = isIR,
                isER = isER;
            display: "p=142,114;b=72,48";
        rsvp: RSVP;
            parameters:
                local_addr = local_addr,
                isIR = isIR,
                isER = isER;
            display: "p=145,108;b=40,24,rect";
        rsvpInterface: RSVPInterface;
            display: "p=144,171;b=72,24";
        ospf_te: OspfTe;
            parameters:
                local_addr = local_addr;
            display: "p=45,220;b=40,24";
        errorHandling: ErrorHandling;
            parameters:
                nodename = routingFile;
            display: "b=32,30;p=267,222;i=sink";
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
                IPForward = true,
                NWIName = "MPLS2IP",
                numOfPorts = numOfPorts,
                routingFile = routingFile;
            gatesizes:
                physIn[numOfPorts],
                physOut[numOfPorts];
            display: "b=32,30;p=143,227;i=fork";
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
            display: "p=141,315;b=72,42";
        libTable: LIBTable;
            parameters:
                LibTableFileName = LibTableFileName,
                PrtTableFileName = PrtTableFileName;

            display: "p=140,314;b=40,24";
        //# processorManager: ProcessorManager;
    connections nocheck:
        networkLayers.RSVPOut --> rsvpInterface.from_ip;
        networkLayers.RSVPIn <-- rsvpInterface.to_ip;

        rsvp.to_ip --> rsvpInterface.from_rsvp_app;
        rsvp.from_ip <-- rsvpInterface.to_rsvp_app;

        networkLayers.OSPFOut --> ospf_te.from_ip;
        networkLayers.OSPFIn <-- ospf_te.to_ip;

        signal_module.to_rsvp --> rsvp.from_rsvp_app;
        signal_module.from_rsvp <-- rsvp.to_rsvp_app;




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




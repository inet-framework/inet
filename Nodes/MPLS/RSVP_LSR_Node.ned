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
    "MPLSModule",
    "LIBTable",
    "RSVPInterface",
    "RSVP",
    "OspfTe",
    "RSVPAppl";



//
// An \RSVP-TE capable router.
//
// \RSVP occupies the Transport layer; however, it is not a transport protocol
// itself. \RSVP uses transport protocols to route packets. In the simulation,
// OSPF-TE has been built to replace the static routing available in IP-suite.
//
// An LSR consists of the following modules:
// - \RSVP-TE: The signalling module.
// - \RSVP Interface: This one plays the role of an adapter between layer 3
//   and layer 4 protocols. It is also to address the mismatch between the
//   two methods of representing an \IP addresses.
// - Network Layers: The core of IP-suite.
// - OSPF-TE: The transport module of OSPF-TE protocol.
// - MPLS Module: Label switching module of MPLS.
// - LIB Table: The module that handles the mapping of labels, interfaces,
//   and FECs.
//
module RSVP_LSR_Node
    parameters:
        local_addr : string,
        dest_addr : string, //# FIXME what's this?
        isIR : bool,
        isER : bool,

        //# Network information
        routingFile : string,
        LibTableFileName : string,
        PrtTableFileName : string;
        //# discoveryAddr: string[];
        //# numOfAddr: int;

    gates:
        in: in[];
        out: out[];
    submodules:
        blackboard: Blackboard;
            display: "i=proc2;p=60,100";
        signal_module: RSVPAppl;
            parameters:
                local_addr = local_addr,
                dest_addr = dest_addr,
                isIR = isIR,
                isER = isER;
            display: "o=#8080ff;p=80,52;b=32,30";
        rsvp: RSVP;
            parameters:
                local_addr = local_addr,
                isIR = isIR,
                isER = isER;
            display: "o=#0080c0;p=190,51;b=40,24,rect";
        rsvpInterface: RSVPInterface;
            display: "o=#0080c0;p=191,106;b=58,12";
        ospf_te: OspfTe;
            parameters:
                local_addr = local_addr;
            display: "o=#008040;p=82,163;b=40,24";
        networkLayer: NetworkLayer;
            parameters:
                IPForward = true,
                routingFile = routingFile;
            gatesizes:
                physIn[sizeof(out)],
                physOut[sizeof(out)];
            display: "b=32,30;p=192,163;i=fork";
        ppp: PPPInterface[sizeof(out)];
            display: "p=88,297,row,90;q=queue;i=iface";
        mplsModule: MPLSModule;
            parameters:
                isIR = isIR,
                isER = isER;
            gatesizes:
                fromL3[sizeof(out)],
                toL3[sizeof(out)],
                fromL2[sizeof(out)],
                toL2[sizeof(out)];
            display: "o=#ff8000;p=192,238;b=66,14";
        libTable: LIBTable;
            parameters:
                LibTableFileName = LibTableFileName,
                PrtTableFileName = PrtTableFileName;
            display: "i=proc2;p=280,184;b=32,30";
    connections nocheck:
        networkLayer.RSVPOut --> rsvpInterface.from_ip;
        networkLayer.RSVPIn <-- rsvpInterface.to_ip;

        rsvp.to_ip --> rsvpInterface.from_rsvp_app;
        rsvp.from_ip <-- rsvpInterface.to_rsvp_app;

        signal_module.to_rsvp --> rsvp.from_rsvp_app;
        signal_module.from_rsvp <-- rsvp.to_rsvp_app;

        for i=0..sizeof(out)-1 do
            in[i] --> ppp[i].physIn;
            out[i] <-- ppp[i].physOut;

            ppp[i].netwOut --> mplsModule.fromL2[i];
            ppp[i].netwIn <-- mplsModule.toL2[i];

            mplsModule.toL3[i] --> networkLayer.physIn[i];
            mplsModule.fromL3[i] <-- networkLayer.physOut[i];
        endfor;
endmodule




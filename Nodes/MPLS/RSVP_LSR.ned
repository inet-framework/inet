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
    "RSVP",
    "OspfTe",
    "RSVPAppl";



//
// An \RSVP-TE capable router.
//
// \RSVP occupies the Transport layer; however, it is not a transport protocol
// itself. \RSVP uses transport protocols to route packets. OSPF-TE is used
// to calculate shortest paths.
//
module RSVP_LSR
    parameters:
        routerId : string,
        isIR : bool,
        isER : bool,
        routingFile : string,
        LibTableFileName : string,
        PrtTableFileName : string;
    gates:
        in: in[];
        out: out[];
    submodules:
        blackboard: Blackboard;
            display: "p=70,60;i=block/blackboard";
        routingTable: RoutingTable;
            parameters:
                IPForward = true,
                routerId = routerId,
                routingFile = routingFile;
            display: "p=70,142;i=block/table";
        ospf_te: OspfTe;
            display: "i=block/cogwheel;p=308,207;o=#008040";
        signal_module: RSVPAppl;
            parameters:
                isIR = isIR,
                isER = isER;
            display: "i=block/control;o=#8080ff;p=295,60";
        rsvp: RSVP;
            parameters:
                isIR = isIR,
                isER = isER;
            display: "i=block/square;o=#0080c0;p=192,60";
        networkLayer: NetworkLayer;
            gatesizes:
                physIn[sizeof(out)],
                physOut[sizeof(out)];
            display: "p=192,155;i=block/fork";
        ppp: PPPInterface[sizeof(out)];
            display: "p=88,330,row,90;q=queue;i=block/ifcard";
        mplsModule: MPLSModule;
            parameters:
                isIR = isIR,
                isER = isER;
            gatesizes:
                fromL3[sizeof(out)],
                toL3[sizeof(out)],
                fromL2[sizeof(out)],
                toL2[sizeof(out)];
            display: "i=block/switch;o=#ff8000;p=192,247";
        libTable: LIBTable;
            parameters:
                LibTableFileName = LibTableFileName,
                PrtTableFileName = PrtTableFileName;
            display: "i=block/table2;p=70,224";
    connections nocheck:
        signal_module.to_rsvp --> rsvp.from_rsvp_app;
        signal_module.from_rsvp <-- rsvp.to_rsvp_app;

        rsvp.to_ip --> networkLayer.RSVPIn;
        rsvp.from_ip <-- networkLayer.RSVPOut;

        for i=0..sizeof(out)-1 do
            in[i] --> ppp[i].physIn;
            out[i] <-- ppp[i].physOut;

            ppp[i].netwOut --> mplsModule.fromL2[i];
            ppp[i].netwIn <-- mplsModule.toL2[i];

            mplsModule.toL3[i] --> networkLayer.physIn[i];
            mplsModule.fromL3[i] <-- networkLayer.physOut[i];
        endfor;
endmodule




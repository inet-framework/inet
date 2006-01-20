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
    "NetworkLayer",
    "MPLS",
    "LIBTable",
    "RoutingTable",
    "InterfaceTable",
    "SimpleClassifier",
    "RSVP",
    "LinkStateRouting",
    "TED",
    "NAMTraceWriter";

//
// An \RSVP-TE capable router.
//
// \RSVP occupies the Transport layer; however, it is not a transport protocol
// itself. \RSVP uses transport protocols to route packets. TED is used
// to calculate shortest paths.
//
module RSVP_LSR
    parameters:
    
        namid : numeric,
    
        peers: string,
        routerId: string,
        routingFile: string;
    gates:
        in: in[];
        out: out[];

    submodules:
        notificationBoard: NotificationBoard;
            display: "p=74,57;i=block/control";
        routingTable: RoutingTable;
            parameters:
                IPForward = true,
                routerId = routerId,
                routingFile = routingFile;
            display: "p=68,138;i=block/table";
        interfaceTable: InterfaceTable;
            display: "p=68,208;i=block/table";
        ted: TED;
            display: "i=block/table;p=312,203";
        linkStateRouting: LinkStateRouting;
            parameters:
                peers = peers;
            display: "i=block/network2;p=254,115";
        rsvp: RSVP;
            parameters:
                peers = peers;
            display: "i=block/control;p=156,67";
        classifier: SimpleClassifier;
            display: "i=block/classifier;p=330,67";
        networkLayer: NetworkLayer;
            gatesizes:
                ifIn[sizeof(out)],
                ifOut[sizeof(out)];
            display: "p=192,195;i=block/fork";
        ppp: PPPInterface[sizeof(out)];
            display: "p=88,329,row,90;q=l2queue;i=block/ifcard";
        mpls: MPLS;
            parameters:
                //peers = peers;
                classifier = "classifier";
            gatesizes:
                netwIn[sizeof(out)],
                netwOut[sizeof(out)],
                ifIn[sizeof(out)],
                ifOut[sizeof(out)];
            display: "o=#ff8000;p=192,270;b=66,14";
        libTable: LIBTable;
            display: "i=block/table2;p=312,268";
        namTrace: NAMTraceWriter;
            parameters:
                 namid = namid;
            display: "p=70,270;i=old/floppy1";

    connections nocheck:
        linkStateRouting.to_ip --> networkLayer.OSPFIn;
        linkStateRouting.from_ip <-- networkLayer.OSPFOut;

        rsvp.to_ip --> networkLayer.RSVPIn;
        rsvp.from_ip <-- networkLayer.RSVPOut;

        for i=0..sizeof(out)-1 do
            in[i] --> ppp[i].physIn;
            out[i] <-- ppp[i].physOut;

            ppp[i].netwOut --> mpls.ifIn[i];
            ppp[i].netwIn <-- mpls.ifOut[i];

            mpls.netwOut[i] --> networkLayer.ifIn[i];
            mpls.netwIn[i] <-- networkLayer.ifOut[i];
        endfor;
endmodule


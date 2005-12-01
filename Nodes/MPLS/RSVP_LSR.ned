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
    "MPLSModule",
    "LIBTable",
    "RoutingTable",
    "InterfaceTable",
    "SimpleClassifier",
    "RSVP",
    "TED";

//
// An \RSVP-TE capable router.
//
// \RSVP occupies the Transport layer; however, it is not a transport protocol
// itself. \RSVP uses transport protocols to route packets. TED is used
// to calculate shortest paths.
//
module RSVP_LSR
    parameters:
        peers: string,
        routerId: string,
        routingFile: string;
    gates:
        in: in[];
        out: out[];

    submodules:
        notificationBoard: NotificationBoard;
            display: "p=330,145;i=block/control";
        routingTable: RoutingTable;
            parameters:
                IPForward = true,
                routerId = routerId,
                routingFile = routingFile;
            display: "p=60,74;i=block/table";
        interfaceTable: InterfaceTable;
            display: "p=60,152;i=block/table";
        ted_module: TED;
            parameters:
                peers = peers;
            display: "i=block/network2;p=158,51";
        rsvp: RSVP;
            parameters:
                peers = peers;
            display: "i=block/control;p=244,51";
        classifier: SimpleClassifier;
            display: "i=block/classifier;p=330,59";
        networkLayer: NetworkLayer;
            gatesizes:
                ifIn[sizeof(out)],
                ifOut[sizeof(out)];
            display: "p=192,163;i=block/fork";
        ppp: PPPInterface[sizeof(out)];
            display: "p=88,297,row,90;q=l2queue;i=block/ifcard";
        mplsModule: MPLSModule;
            parameters:
                //peers = peers;
                classifier = "classifier";
            gatesizes:
                netwIn[sizeof(out)],
                netwOut[sizeof(out)],
                ifIn[sizeof(out)],
                ifOut[sizeof(out)];
            display: "o=#ff8000;p=192,238;b=66,14";
        libTable: LIBTable;
            display: "i=block/table2;p=60,228";
    connections nocheck:
        ted_module.to_ip --> networkLayer.OSPFIn;
        ted_module.from_ip <-- networkLayer.OSPFOut;

        rsvp.to_ip --> networkLayer.RSVPIn;
        rsvp.from_ip <-- networkLayer.RSVPOut;

        for i=0..sizeof(out)-1 do
            in[i] --> ppp[i].physIn;
            out[i] <-- ppp[i].physOut;

            ppp[i].netwOut --> mplsModule.ifIn[i];
            ppp[i].netwIn <-- mplsModule.ifOut[i];

            mplsModule.netwOut[i] --> networkLayer.ifIn[i];
            mplsModule.netwIn[i] <-- networkLayer.ifOut[i];
        endfor;
endmodule


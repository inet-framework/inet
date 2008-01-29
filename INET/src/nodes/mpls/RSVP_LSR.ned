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



package org.omnetpp.inet.nodes.mpls;

import org.omnetpp.inet.base.NotificationBoard;
import org.omnetpp.inet.networkinterfaces.ppp.PPPInterface;
import org.omnetpp.inet.networklayer.contract.InterfaceTable;
import org.omnetpp.inet.networklayer.ipv4.RoutingTable;
import org.omnetpp.inet.networklayer.mpls.LIBTable;
import org.omnetpp.inet.networklayer.mpls.MPLS;
import org.omnetpp.inet.networklayer.rsvp_te.RSVP;
import org.omnetpp.inet.networklayer.rsvp_te.SimpleClassifier;
import org.omnetpp.inet.networklayer.ted.LinkStateRouting;
import org.omnetpp.inet.networklayer.ted.TED;
import org.omnetpp.inet.nodes.inet.NetworkLayer;
import org.omnetpp.inet.util.NAMTraceWriter;


//
// An \RSVP-TE capable router.
//
// \RSVP occupies the Transport layer; however, it is not a transport protocol
// itself. \RSVP uses transport protocols to route packets. TED is used
// to calculate shortest paths.
//
module RSVP_LSR
{
    parameters:
        int namid;
        string peers;
        string routerId = default("auto");
        string routingFile = default("");
        @display("i=abstract/router_s");
    gates:
        inout pppg[];
    submodules:
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=74,57");
        }
        routingTable: RoutingTable {
            parameters:
                IPForward = true;
                routerId = routerId;
                routingFile = routingFile;
                @display("p=68,138");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=68,208");
        }
        ted: TED {
            parameters:
                @display("p=312,203");
        }
        linkStateRouting: LinkStateRouting {
            parameters:
                peers = peers;
                @display("p=254,115");
        }
        rsvp: RSVP {
            parameters:
                peers = peers;
                @display("p=156,67");
        }
        classifier: SimpleClassifier {
            parameters:
                @display("p=330,67");
        }
        networkLayer: NetworkLayer {
            parameters:
                @display("p=192,195");
            gates:
                ifIn[sizeof(pppg)];
                ifOut[sizeof(pppg)];
        }
        ppp[sizeof(pppg)]: PPPInterface {
            parameters:
                @display("p=88,329,row,90;q=l2queue");
        }
        mpls: MPLS {
            parameters:
                //peers = peers;
                classifier = "classifier";
                @display("o=#ff8000;p=192,270;b=66,14");
            gates:
                netwIn[sizeof(pppg)];
                netwOut[sizeof(pppg)];
                ifIn[sizeof(pppg)];
                ifOut[sizeof(pppg)];
        }
        libTable: LIBTable {
            parameters:
                @display("p=312,268");
        }
        namTrace: NAMTraceWriter {
            parameters:
                namid = namid;
                @display("p=70,270");

        }
    connections allowunconnected:
        linkStateRouting.to_ip --> networkLayer.OSPFIn;
        linkStateRouting.from_ip <-- networkLayer.OSPFOut;

        rsvp.to_ip --> networkLayer.RSVPIn;
        rsvp.from_ip <-- networkLayer.RSVPOut;

        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;

            ppp[i].netwOut --> mpls.ifIn[i];
            ppp[i].netwIn <-- mpls.ifOut[i];

            mpls.netwOut[i] --> networkLayer.ifIn[i];
            mpls.netwIn[i] <-- networkLayer.ifOut[i];
        }
}


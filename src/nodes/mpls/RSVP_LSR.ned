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
package inet.nodes.mpls;

import inet.nodes.inet.NetworkLayer;
import inet.networklayer.ted.TED;
import inet.networklayer.ted.LinkStateRouting;
import inet.networklayer.rsvp_te.SimpleClassifier;
import inet.networklayer.rsvp_te.RSVP;
import inet.networklayer.mpls.MPLS;
import inet.networklayer.mpls.LIBTable;
import inet.networklayer.ipv4.RoutingTable;
import inet.networklayer.common.InterfaceTable;
import inet.linklayer.IWiredNic;
import inet.base.NotificationBoard;


//
// An RSVP-TE capable router.
//
// RSVP occupies the Transport layer; however, it is not a transport protocol
// itself. RSVP uses transport protocols to route packets. ~TED is used
// to calculate shortest paths.
//
module RSVP_LSR extends RSVP_ROUTER_BASE
{
    parameters:
        @node();
        @labels(node,mpls-node);
        @display("i=abstract/router");
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
        ppp[sizeof(pppg)]: <default("PPPInterface")> like IWiredNic {
            parameters:
                @display("p=88,329,row,90;q=l2queue");
        }
        mpls: MPLS {
            parameters:
                classifier = "classifier";
                @display("p=192,270");
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
    connections allowunconnected:
        linkStateRouting.ipOut --> networkLayer.ospfIn;
        linkStateRouting.ipIn <-- networkLayer.ospfOut;

        rsvp.ipOut --> networkLayer.rsvpIn;
        rsvp.ipIn <-- networkLayer.rsvpOut;

        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;

            ppp[i].upperLayerOut --> mpls.ifIn[i];
            ppp[i].upperLayerIn <-- mpls.ifOut[i];

            mpls.netwOut[i] --> networkLayer.ifIn[i];
            mpls.netwIn[i] <-- networkLayer.ifOut[i];
        }
}


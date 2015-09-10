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
package inet.node.mpls;

import inet.common.lifecycle.NodeStatus;
import inet.linklayer.contract.IWiredNic;
import inet.linklayer.loopback.LoopbackInterface;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.IPv4NetworkLayer;
import inet.networklayer.ipv4.IPv4RoutingTable;
import inet.networklayer.mpls.LIBTable;
import inet.networklayer.mpls.MPLS;
import inet.networklayer.rsvp_te.RSVP;
import inet.networklayer.rsvp_te.SimpleClassifier;
import inet.networklayer.ted.LinkStateRouting;
import inet.networklayer.ted.TED;


//
// An RSVP-TE capable router.
//
// RSVP occupies the Transport layer; however, it is not a transport protocol
// itself. RSVP uses transport protocols to route packets. ~TED is used
// to calculate shortest paths.
//
module RSVP_LSR
{
    parameters:
        @networkNode();
        @labels(node,mpls-node);
        @display("i=abstract/router");
        bool hasStatus = default(false);
        string peers;
        string routerId = default("auto");
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.routingTableModule = default(absPath(".routingTable"));
        *.tedModule = default(absPath(".ted"));
        *.rsvpModule = default(absPath(".rsvp"));
        *.libTableModule = default(absPath(".libTable"));
    gates:
        inout pppg[] @labels(PPPFrame-conn);
    submodules:
        status: NodeStatus if hasStatus {
            @display("p=58,50");
        }
        routingTable: IPv4RoutingTable {
            parameters:
                forwarding = true;
                routerId = routerId;
                @display("p=58,174;is=s");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=58,242;is=s");
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
                classifierModule = "^.classifier";
                @display("p=156,67");
        }
        classifier: SimpleClassifier {
            parameters:
                @display("p=330,67");
        }
        networkLayer: IPv4NetworkLayer {
            parameters:
                @display("p=192,195");
            gates:
                ifIn[sizeof(pppg)+1];
                ifOut[sizeof(pppg)+1];
        }
        ppp[sizeof(pppg)]: <default("PPPInterface")> like IWiredNic {
            parameters:
                @display("p=132,345,row,90;q=l2queue");
        }
        mpls: MPLS {
            parameters:
                classifierModule = "^.classifier";
                @display("p=192,270");
            gates:
                netwIn[sizeof(pppg)+1];
                netwOut[sizeof(pppg)+1];
                ifIn[sizeof(pppg)+1];
                ifOut[sizeof(pppg)+1];
        }
        libTable: LIBTable {
            parameters:
                @display("p=312,268");
        }
        lo0: LoopbackInterface {
            @display("p=50,345");
        }
    connections allowunconnected:
        linkStateRouting.ipOut --> networkLayer.transportIn++;
        linkStateRouting.ipIn <-- networkLayer.transportOut++;

        rsvp.ipOut --> networkLayer.transportIn++;
        rsvp.ipIn <-- networkLayer.transportOut++;

        lo0.upperLayerOut --> networkLayer.ifIn[0];
        networkLayer.ifOut[0] --> lo0.upperLayerIn;

        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;

            ppp[i].upperLayerOut --> mpls.ifIn[i+1];
            ppp[i].upperLayerIn <-- mpls.ifOut[i+1];

            mpls.netwOut[i+1] --> networkLayer.ifIn[i+1];
            mpls.netwIn[i+1] <-- networkLayer.ifOut[i+1];
        }
}


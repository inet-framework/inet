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

import inet.common.MessageDispatcher;
import inet.common.lifecycle.NodeStatus;
import inet.linklayer.contract.ILoopbackInterface;
import inet.linklayer.contract.IPppInterface;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.Ipv4NetworkLayer;
import inet.networklayer.mpls.LibTable;
import inet.networklayer.mpls.Mpls;
import inet.networklayer.rsvpte.RsvpTe;
import inet.networklayer.rsvpte.SimpleClassifier;
import inet.networklayer.ted.LinkStateRouting;
import inet.networklayer.ted.Ted;


//
// An RSVP-TE capable router.
//
// RsvpTe occupies the Transport layer; however, it is not a transport protocol
// itself. RsvpTe uses transport protocols to route packets. ~Ted is used
// to calculate shortest paths.
//
module RsvpMplsRouter
{
    parameters:
        @networkNode();
        @labels(node,mpls-node);
        @display("i=abstract/router");
        bool hasStatus = default(false);
        int numLoInterfaces = default(1);
        string peers;
        string routerId = default("auto");
        *.forwarding = true;
        *.routingTable.routerId = routerId;
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.routingTableModule = default(absPath(".ipv4.routingTable"));
        *.tedModule = default(absPath(".ted"));
        *.rsvpModule = default(absPath(".rsvp"));
        *.libTableModule = default(absPath(".libTable"));
    gates:
        inout pppg[] @labels(PppFrame-conn);
    submodules:
        status: NodeStatus if hasStatus {
            @display("p=58,50;is=s");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=58,272;is=s");
        }
        ted: Ted {
            parameters:
                @display("p=58,115;is=s");
        }
        linkStateRouting: LinkStateRouting {
            parameters:
                peers = peers;
                @display("p=545,106");
        }
        rsvp: RsvpTe {
            parameters:
                peers = peers;
                classifierModule = "^.classifier";
                @display("p=458,106");
        }
        classifier: SimpleClassifier {
            parameters:
                @display("p=58,362;is=s");
        }
        tn: MessageDispatcher {
            parameters:
                @display("p=409,185;b=580,5,,,,1");
        }
        ipv4: Ipv4NetworkLayer {
            parameters:
                @display("p=409,223");
        }
        nm: MessageDispatcher {
            parameters:
                @display("p=409,271;b=580,5,,,,1");
        }
        lo[numLoInterfaces]: <default("LoopbackInterface")> like ILoopbackInterface {
            @display("p=139,394");
        }
        ppp[sizeof(pppg)]: <default("PppInterface")> like IPppInterface {
            parameters:
                @display("p=271,394,row,90;q=l2queue");
        }
        mpls: Mpls {
            parameters:
                classifierModule = "^.classifier";
                @display("p=448,311");
        }
        libTable: LibTable {
            parameters:
                @display("p=58,186;is=s");
        }
        ml: MessageDispatcher {
            parameters:
                @display("p=409,350;b=580,5,,,,1");
        }
    connections allowunconnected:
        linkStateRouting.ipOut --> tn.in++;
        tn.out++ --> linkStateRouting.ipIn;
        ipv4.transportOut --> tn.in++;
        tn.out++ --> ipv4.transportIn;

        rsvp.ipOut --> tn.in++;
        rsvp.ipIn <-- tn.out++;

        ipv4.ifOut --> nm.in++;
        nm.out++ --> ipv4.ifIn;

        for i=0..numLoInterfaces-1 {
            lo[i].upperLayerOut --> nm.in++;
            nm.out++ --> lo[i].upperLayerIn;
        }

        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;

            ppp[i].upperLayerOut --> ml.in++;
            ml.out++ --> ppp[i].upperLayerIn;
        }
        mpls.ifOut --> ml.in++;
        ml.out++ --> mpls.ifIn;
        nm.out++ --> mpls.netwIn;
        mpls.netwOut --> nm.in++;
}


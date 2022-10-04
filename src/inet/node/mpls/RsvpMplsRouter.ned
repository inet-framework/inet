//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.node.mpls;

import inet.common.MessageDispatcher;
import inet.common.lifecycle.NodeStatus;
import inet.linklayer.contract.ILoopbackInterface;
import inet.linklayer.contract.IPppInterface;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.Ipv4NetworkLayer;
import inet.networklayer.mpls.IIngressClassifier;
import inet.networklayer.mpls.LibTable;
import inet.networklayer.mpls.Mpls;
import inet.networklayer.rsvpte.RsvpTe;
import inet.networklayer.ted.LinkStateRouting;
import inet.networklayer.ted.Ted;


//
// An RSVP-TE capable router.
//
// ~RsvpTe occupies the Transport layer; however, it is not a transport protocol
// itself. ~RsvpTe uses transport protocols to route packets. ~Ted is used
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
        *.routingTable.routerId = this.routerId;
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.routingTableModule = default(absPath(".ipv4.routingTable"));
        *.tedModule = default(absPath(".ted"));
        *.rsvpModule = default(absPath(".rsvp"));
        *.libTableModule = default(absPath(".libTable"));
    gates:
        inout pppg[] @labels(PppFrame-conn);
    submodules:
        status: NodeStatus if hasStatus {
            @display("p=100,400;is=s");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=100,200;is=s");
        }
        ted: Ted {
            parameters:
                @display("p=100,500;is=s");
        }
        linkStateRouting: LinkStateRouting {
            parameters:
                peers = parent.peers;
                @display("p=600,80");
        }
        rsvp: RsvpTe {
            parameters:
                peers = parent.peers;
                classifierModule = "^.classifier";
                @display("p=300,80");
        }
        classifier: <default("RsvpClassifier")> like IIngressClassifier {
            parameters:
                @display("p=100,100;is=s");
        }
        tn: MessageDispatcher {
            parameters:
                @display("p=450,160;b=500,5,,,,1");
        }
        ipv4: Ipv4NetworkLayer {
            parameters:
                @display("p=450,240");
        }
        nm: MessageDispatcher {
            parameters:
                @display("p=450,320;b=500,5,,,,1");
        }
        lo[numLoInterfaces]: <default("LoopbackInterface")> like ILoopbackInterface {
            @display("p=250,560");
        }
        ppp[sizeof(pppg)]: <default("PppInterface")> like IPppInterface {
            parameters:
                @display("p=400,560,row,150;q=l2queue");
        }
        mpls: Mpls {
            parameters:
                classifierModule = "^.classifier";
                @display("p=450,400");
        }
        libTable: LibTable {
            parameters:
                @display("p=100,300;is=s");
        }
        ml: MessageDispatcher {
            parameters:
                @display("p=450,480;b=500,5,,,,1");
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
        mpls.lowerLayerOut --> ml.in++;
        ml.out++ --> mpls.lowerLayerIn;
        nm.out++ --> mpls.upperLayerIn;
        mpls.upperLayerOut --> nm.in++;
}


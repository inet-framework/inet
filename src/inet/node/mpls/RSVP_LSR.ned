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
import inet.networklayer.mpls.LIBTable;
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
module RSVP_LSR extends MplsNetworkLayerNodeBase
{
    parameters:
        @networkNode();
        @labels(node,mpls-node);
        @display("i=abstract/router;bgb=916,851");
        string peers;
        string routerId = default("auto");
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.routingTableModule = default("^." + (hasIpv4 ? "ipv4.routingTable" : hasIpv6 ? "ipv6.routingTable" : hasGn ? "gn.routingTable" : ""));
        *.tedModule = default(absPath(".ted"));
        *.rsvpModule = default(absPath(".rsvp"));
        *.libTableModule = default(absPath(".libTable"));
        mpls.classifierModule = "^.classifier";
        @figure[submodules];
    submodules:
        ted: TED {
            parameters:
                @display("p=820,281");
        }
        rsvp: RSVP {
            parameters:
                peers = peers;
                classifierModule = "^.classifier";
                @display("p=600,334");
        }
        classifier: SimpleClassifier {
            parameters:
                @display("p=820,150");
        }
        linkStateRouting: LinkStateRouting {
            parameters:
                peers = peers;
                @display("p=730,334");
        }
        libTable: LIBTable {
            parameters:
                @display("p=820,346");
        }
        tn: MessageDispatcher {
            parameters:
                @display("b=580,5,,,,1;p=550,400");
        }
    connections allowunconnected:
        linkStateRouting.ipOut --> tn.upperLayerIn++;
        linkStateRouting.ipIn <-- tn.upperLayerOut++;

        rsvp.ipOut --> tn.upperLayerIn++;
        rsvp.ipIn <-- tn.upperLayerOut++;

        tn.lowerLayerOut++ --> ipv4.transportIn if hasIpv4;
        tn.lowerLayerIn++ <-- ipv4.transportOut if hasIpv4;

        tn.lowerLayerOut++ --> ipv4.icmpIn if hasIpv4;
        tn.lowerLayerIn++ <-- ipv4.icmpOut if hasIpv4;

        tn.lowerLayerOut++ --> ipv4.igmpIn if hasIpv4;
        tn.lowerLayerIn++ <-- ipv4.igmpOut if hasIpv4;

}


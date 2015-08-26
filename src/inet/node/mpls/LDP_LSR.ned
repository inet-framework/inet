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
import inet.networklayer.ldp.LDP;
import inet.networklayer.mpls.LIBTable;
import inet.networklayer.ted.LinkStateRouting;
import inet.networklayer.ted.TED;


//
// An LDP-capable router.
//
// LDP Capable Routers are the main building blocks for an LDP MPLS network.
//
module LDP_LSR extends MplsTransportLayerNodeBase
{
    parameters:
        @networkNode();
        @labels(node);
        @display("i=abstract/router");
        double holdTime @unit("s") = default(15s);
        double helloInterval @unit("s") = default(5s);
        string peers;
        string routerId = default("auto");
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.tedModule = default(absPath(".ted"));
        *.libTableModule = default(absPath(".libTable"));
        mpls.classifierModule = "^.ldp";
        forwarding = true;
    submodules:
        ldp: LDP {
            parameters:
                holdTime = holdTime;
                helloInterval = helloInterval;
                @display("p=680,100");
        }
        at: MessageDispatcher {
            parameters:
                @display("b=580,5,,,,1;p=550,200");
        }
        ted: TED {
            parameters:
                @display("p=786,269");
        }
        linkStateRouting: LinkStateRouting {
            parameters:
                peers = peers;
                @display("p=691,300");
        }
        libTable: LIBTable {
            parameters:
                @display("p=786,330");
        }
    connections allowunconnected:
        linkStateRouting.ipOut --> tn.upperLayerIn++;
        linkStateRouting.ipIn <-- tn.upperLayerOut++;

        ldp.tcpIn <-- at.upperLayerOut++;
        ldp.tcpOut --> at.upperLayerIn++;

        ldp.udpIn <-- at.upperLayerOut++;
        ldp.udpOut --> at.upperLayerIn++;

        at.lowerLayerOut++ --> udp.appIn if hasUdp;
        at.lowerLayerIn++ <-- udp.appOut if hasUdp;

        at.lowerLayerOut++ --> tcp.appIn if hasTcp;
        at.lowerLayerIn++ <-- tcp.appOut if hasTcp;

        at.lowerLayerOut++ --> sctp.appIn if hasSctp;
        at.lowerLayerIn++ <-- sctp.appOut if hasSctp;

        at.lowerLayerOut++ --> tn.upperLayerIn++;
        at.lowerLayerIn++ <-- tn.upperLayerOut++;
}


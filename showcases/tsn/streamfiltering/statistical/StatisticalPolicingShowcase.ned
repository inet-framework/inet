//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.showcases.tsn.streamfiltering.statistical;

import inet.networks.base.TsnNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.node.ethernet.EthernetLink;

network StatisticalPolicingShowcaseNetwork extends TsnNetworkBase
{
    parameters:
        *.eth[*].bitrate = default(100Mbps);
    submodules:
        client1: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=350,100");
        }
        client2: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=350,300");
        }
        switch: <default("TsnSwitch")> like IEthernetNetworkNode {
            @display("p=550,200");
        }
        server: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=750,200");
        }
    connections:
        client1.ethg++ <--> EthernetLink <--> switch.ethg++;
        client2.ethg++ <--> EthernetLink <--> switch.ethg++;
        switch.ethg++ <--> EthernetLink <--> server.ethg++;
}


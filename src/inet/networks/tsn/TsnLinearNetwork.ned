//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks.tsn;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.node.ethernet.EthernetLink;


network TsnLinearNetwork extends WiredNetworkBase
{
    parameters:
        *.eth[*].bitrate = default(100Mbps);
    submodules:
        client: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=350,200");
        }
        switch: <default("TsnSwitch")> like IEthernetNetworkNode {
            @display("p=550,200");
        }
        server: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=750,200");
        }
    connections:
        client.ethg++ <--> EthernetLink <--> switch.ethg++;
        switch.ethg++ <--> EthernetLink <--> server.ethg++;
}


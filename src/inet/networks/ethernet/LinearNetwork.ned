//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks.ethernet;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.node.ethernet.EthernetLink;

network LinearNetwork extends WiredNetworkBase
{
    parameters:
        *.eth[*].bitrate = default(100Mbps);
    submodules:
        client: <default("StandardHost")> like IEthernetNetworkNode {
            @display("p=350,200");
        }
        switch: <default("EthernetSwitch")> like IEthernetNetworkNode {
            @display("p=550,200");
        }
        server: <default("StandardHost")> like IEthernetNetworkNode {
            @display("p=750,200");
        }
    connections:
        client.ethg++ <--> EthernetLink <--> switch.ethg++;
        switch.ethg++ <--> EthernetLink <--> server.ethg++;
}


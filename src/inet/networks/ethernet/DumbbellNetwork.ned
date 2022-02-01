//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks.ethernet;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.node.ethernet.EthernetLink;

network DumbbellNetwork extends WiredNetworkBase
{
    parameters:
        *.eth[*].bitrate = default(100Mbps);
    submodules:
        client1: <default("StandardHost")> like IEthernetNetworkNode {
            @display("p=300,150");
        }
        client2: <default("StandardHost")> like IEthernetNetworkNode {
            @display("p=300,350");
        }
        switch1: <default("EthernetSwitch")> like IEthernetNetworkNode {
            @display("p=500,250");
        }
        switch2: <default("EthernetSwitch")> like IEthernetNetworkNode {
            @display("p=700,250");
        }
        server1: <default("StandardHost")> like IEthernetNetworkNode {
            @display("p=900,150");
        }
        server2: <default("StandardHost")> like IEthernetNetworkNode {
            @display("p=900,350");
        }
    connections:
        client1.ethg++ <--> EthernetLink <--> switch1.ethg++;
        client2.ethg++ <--> EthernetLink <--> switch1.ethg++;
        switch2.ethg++ <--> EthernetLink <--> server1.ethg++;
        switch2.ethg++ <--> EthernetLink <--> server2.ethg++;
        switch1.ethg++ <--> EthernetLink <--> switch2.ethg++;
}


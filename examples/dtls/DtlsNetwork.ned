// SPDX-License-Identifier: LGPL-3.0-or-later
package inet.examples.dtls;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.node.ethernet.EthernetLink;

// A DTLS client and listener using the ordinary host application slots and UDP.
network DtlsNetwork extends WiredNetworkBase
{
    parameters:
        double packetErrorRate = default(0);
        visualizer.typename = default("");
    submodules:
        client: <default("StandardHost")> like IEthernetNetworkNode {
            @display("p=300,200");
        }
        server: <default("StandardHost")> like IEthernetNetworkNode {
            @display("p=600,200");
        }
    connections:
        client.ethg++ <--> EthernetLink { datarate = 100Mbps; per = parent.packetErrorRate; } <--> server.ethg++;
}

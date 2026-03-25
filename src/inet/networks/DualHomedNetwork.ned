//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.common.ILink;

//
// A parameterizable dual-homed (multihomed) network topology.
//
// Two interconnected switches/routers, each host connects to both.
// Core pattern for redundancy, load balancing, and failover studies.
//
// <pre>
//       [sw0]-----[sw1]
//       / | \     / | \
//     [h0] [h1] ... [hN]
//     (each host connects to both sw0 and sw1)
// </pre>
//
network DualHomedNetwork extends WiredNetworkBase
{
    parameters:
        int numHosts; // number of dual-homed hosts
    submodules:
        sw[2]: <default("EthernetSwitch")> like IEthernetNetworkNode {
            @display("p=300,100,row,200");
        }
        host[numHosts]: <default("StandardHost")> like IEthernetNetworkNode {
            @display("p=300,300,row,100");
        }
    connections:
        // Inter-switch link
        sw[0].ethg++ <--> <default("EthernetLink")> like ILink <--> sw[1].ethg++;
        // Each host dual-homed to both switches
        for i=0..numHosts-1 {
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> sw[0].ethg++;
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> sw[1].ethg++;
        }
}

//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.common.ILink;

//
// A parameterizable hub-and-spoke network with backup ring.
//
// A central hub connects to all spokes (star). The spokes also form
// a ring among themselves for protection/redundancy.
// Common in metro/WAN with protection switching.
//
// <pre>
//       [sp1]---[sp2]
//       / |  \  / |  \
//      /  | [hub] |   \
//     /   |  / \  |    \
//   [sp0]-+      +-[sp3]
//      \              /
//       \---[sp4]---/       (ring among spokes)
// </pre>
//
network HubSpokeRingNetwork extends WiredNetworkBase
{
    parameters:
        int numSpokes; // number of spoke nodes
        volatile int numHostsPerSpoke = default(0); // number of hosts connected to each spoke
        object numHosts = default(repeatPar("numHostsPerSpoke", numSpokes));
    submodules:
        hub: <default("EthernetSwitch")> like IEthernetNetworkNode {
        }
        spoke[numSpokes]: <default("EthernetSwitch")> like IEthernetNetworkNode {
            @display("p=300,100,ring,150");
        }
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        // Star: hub to all spokes
        for i=0..numSpokes-1 {
            hub.ethg++ <--> <default("EthernetLink")> like ILink <--> spoke[i].ethg++;
        }
        // Backup ring among spokes
        for i=0..numSpokes-1 {
            spoke[i].ethg++ <--> <default("EthernetLink")> like ILink <--> spoke[(i+1) % numSpokes].ethg++;
        }
        // Hosts dangling from each spoke
        for i=0..sizeof(host)-1 {
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> spoke[groupIndex(numHosts,i)].ethg++;
        }
}

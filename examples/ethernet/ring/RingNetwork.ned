//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.ethernet.ring;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.node.ethernet.EthernetLink;

network RingNetwork extends WiredNetworkBase
{
    parameters:
        int numNodes = default(6);
        volatile int numHostsPerNode = default(intuniform(1, 3));
        object numHosts = default(repeatPar("numHostsPerNode", numNodes));
        *.eth[*].bitrate = default(100Mbps);
    submodules:
        node[numNodes]: <default("EthernetSwitch")> like IEthernetNetworkNode {
            @display("p=400,200,ring,150");
        }
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        for i=0..numNodes-1 {
            node[i].ethg++ <--> EthernetLink <--> node[(i+1) % numNodes].ethg++;
        }
        for i=0..sizeof(host)-1 {
            host[i].ethg++ <--> EthernetLink <--> node[groupIndex(numHosts,i)].ethg++;
        }
}

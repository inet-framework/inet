//
// Protocol Test Framework for INET -- a shared network: a host behind a tap, and its peers on
// a switch.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.protocoltest;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.node.ethernet.EthernetLink;

//
// host1 -- tap -- switch, and peer[0] .. peer[numPeers-1] on the switch. The tap sees every
// frame of host1, so a test can observe and change the traffic of one host with many peers:
// a client with several servers, or a server with several clients. The node types and the
// configurator are parameters, as in TapPairNetwork.
//
network TapSwitchNetwork extends WiredNetworkBase
{
    parameters:
        int numPeers = default(3);
        double linkDatarate @unit(bps) = default(100Mbps); // an Ethernet rate: 10Mbps, 100Mbps, 1Gbps, ...
        double linkDelay @unit(s) = default(0s);           // the propagation delay of each link
        visualizer.typename = default("");
        // The tap is a node without a MAC table, which the MAC forwarding configurator cannot
        // handle; a switch learns the addresses from the frames instead.
        macForwardingTableConfigurator.typename = default("");
    submodules:
        host1: <default("StandardHost")> like IEthernetNetworkNode {
            @display("p=300,200");
        }
        tap: PacketTap {
            @display("p=450,200");
        }
        switch: <default("EthernetSwitch")> like IEthernetNetworkNode {
            @display("p=600,200");
        }
        peer[numPeers]: <default("StandardHost")> like IEthernetNetworkNode {
            @display("p=750,100,c,100");
        }
    connections:
        host1.ethg++ <--> EthernetLink { datarate = parent.linkDatarate; delay = parent.linkDelay; } <--> tap.a;
        tap.b <--> EthernetLink { datarate = parent.linkDatarate; delay = parent.linkDelay; } <--> switch.ethg++;
        for i=0..numPeers-1 {
            peer[i].ethg++ <--> EthernetLink { datarate = parent.linkDatarate; delay = parent.linkDelay; } <--> switch.ethg++;
        }
}

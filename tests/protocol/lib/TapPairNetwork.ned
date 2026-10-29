//
// Protocol Test Framework for INET -- a shared network: two hosts with a tap between them.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.protocoltest;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.node.ethernet.EthernetLink;

//
// Two hosts on one Ethernet link, with a PacketTap between them: host1 -- tap -- host2. The
// tap passes each frame unchanged until a rule of the test program acts. A test selects the
// network with one ini line and keeps its own %file for nothing else:
//
//     network = inet.protocoltest.TapPairNetwork
//
// The node types and the configurator are parameters, so one network serves IPv4 and IPv6:
// set *.host*.typename = "StandardHost6" and *.configurator.typename =
// "Ipv6FlatNetworkConfigurator". The ARP is GlobalArp, so the wire carries no ARP frames.
//
network TapPairNetwork extends WiredNetworkBase
{
    parameters:
        double linkDatarate @unit(bps) = default(100Mbps); // an Ethernet rate: 10Mbps, 100Mbps, 1Gbps, ...
        double linkDelay @unit(s) = default(0s);           // the propagation delay of each half of the link
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
        host2: <default("StandardHost")> like IEthernetNetworkNode {
            @display("p=600,200");
        }
    connections:
        host1.ethg++ <--> EthernetLink { datarate = parent.linkDatarate; delay = parent.linkDelay; } <--> tap.a;
        tap.b <--> EthernetLink { datarate = parent.linkDatarate; delay = parent.linkDelay; } <--> host2.ethg++;
}

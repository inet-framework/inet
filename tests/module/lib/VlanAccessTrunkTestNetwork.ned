package inet.test.moduletest.lib;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.StandardHost;

//
// Two VLAN-aware bridges joined by a trunk, for testing how a bridge assigns
// VLANs to the frames of its access ports. bridge1 has the access ports of a1
// and a2 (VLAN 10) and of b1 (VLAN 20); bridge2 has those of a3 (VLAN 10) and
// b2 (VLAN 20). See vlan-access-trunk.ini.
//
network VlanAccessTrunkTestNetwork
{
    submodules:
        configurator: Ipv4NetworkConfigurator;
        a1: StandardHost;
        a2: StandardHost;
        b1: StandardHost;
        a3: StandardHost;
        b2: StandardHost;
        bridge1: EthernetSwitch;
        bridge2: EthernetSwitch;
    connections:
        a1.ethg++ <--> Eth100M <--> bridge1.ethg++;
        a2.ethg++ <--> Eth100M <--> bridge1.ethg++;
        b1.ethg++ <--> Eth100M <--> bridge1.ethg++;
        a3.ethg++ <--> Eth100M <--> bridge2.ethg++;
        b2.ethg++ <--> Eth100M <--> bridge2.ethg++;
        bridge1.ethg++ <--> Eth100M <--> bridge2.ethg++;
}

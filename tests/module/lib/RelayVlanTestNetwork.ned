package inet.test.moduletest.lib;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.StandardHost;

//
// Three host pairs on one switch, for testing how the switch relays
// VLAN-tagged frames. See relay-vlan.ini.
//
network RelayVlanTestNetwork
{
    submodules:
        configurator: Ipv4NetworkConfigurator;
        a1: StandardHost;
        a2: StandardHost;
        b1: StandardHost;
        b2: StandardHost;
        c1: StandardHost;
        c2: StandardHost;
        switch: EthernetSwitch;
    connections:
        a1.ethg++ <--> Eth100M <--> switch.ethg++;
        a2.ethg++ <--> Eth100M <--> switch.ethg++;
        b1.ethg++ <--> Eth100M <--> switch.ethg++;
        b2.ethg++ <--> Eth100M <--> switch.ethg++;
        c1.ethg++ <--> Eth100M <--> switch.ethg++;
        c2.ethg++ <--> Eth100M <--> switch.ethg++;
}

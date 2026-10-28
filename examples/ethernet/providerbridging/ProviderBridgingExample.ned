//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.ethernet.providerbridging;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.StandardHost;

//
// Two customers connected through a provider network (IEEE 802.1ad, Q-in-Q).
//
// Customer A has three sites and two VLANs, C-VLAN 10 and C-VLAN 20. Customer
// B has two sites and one VLAN, C-VLAN 10, the same VLAN id as customer A's.
// At each site, a customer bridge (cb*) puts its hosts into a VLAN, and sends
// their frames C-tagged to the provider.
//
// The provider edge bridges (peb*) put all frames of a customer into the
// S-VLAN of that customer, 100 for A and 200 for B, and the provider core
// bridge (pb) relays the frames by S-VLAN. The provider never looks at the
// C-tags. pb is a member of S-VLAN 200 only toward peb1 and peb2.
//
// Port numbers follow the order of the connections below: a customer bridge
// has its hosts on the first ports and the uplink to the provider on the last
// one; a provider edge bridge has its customers on the first ports and the
// provider network on the last one; pb has peb1, peb2 and peb3 on eth0, eth1
// and eth2.
//
network ProviderBridgingExample
{
    @display("bgb=1200,700");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100;is=s");
        }
        hostA1: StandardHost {
            @display("p=100,250");
        }
        hostA2: StandardHost {
            @display("p=100,350");
        }
        hostB1: StandardHost {
            @display("p=100,500");
        }
        cbA1: EthernetSwitch {
            @display("p=250,300");
        }
        cbB1: EthernetSwitch {
            @display("p=250,500");
        }
        peb1: EthernetSwitch {
            @display("p=400,400");
        }
        pb: EthernetSwitch {
            @display("p=600,400");
        }
        peb2: EthernetSwitch {
            @display("p=800,400");
        }
        cbA2: EthernetSwitch {
            @display("p=950,300");
        }
        cbB2: EthernetSwitch {
            @display("p=950,500");
        }
        hostA3: StandardHost {
            @display("p=1100,250");
        }
        hostA4: StandardHost {
            @display("p=1100,350");
        }
        hostB2: StandardHost {
            @display("p=1100,500");
        }
        peb3: EthernetSwitch {
            @display("p=600,550");
        }
        cbA3: EthernetSwitch {
            @display("p=600,650");
        }
        hostA5: StandardHost {
            @display("p=750,650");
        }
    connections:
        hostA1.ethg++ <--> Eth100M <--> cbA1.ethg++;
        hostA2.ethg++ <--> Eth100M <--> cbA1.ethg++;
        hostB1.ethg++ <--> Eth100M <--> cbB1.ethg++;
        hostA3.ethg++ <--> Eth100M <--> cbA2.ethg++;
        hostA4.ethg++ <--> Eth100M <--> cbA2.ethg++;
        hostB2.ethg++ <--> Eth100M <--> cbB2.ethg++;
        hostA5.ethg++ <--> Eth100M <--> cbA3.ethg++;

        cbA1.ethg++ <--> Eth100M <--> peb1.ethg++;
        cbB1.ethg++ <--> Eth100M <--> peb1.ethg++;
        cbA2.ethg++ <--> Eth100M <--> peb2.ethg++;
        cbB2.ethg++ <--> Eth100M <--> peb2.ethg++;
        cbA3.ethg++ <--> Eth100M <--> peb3.ethg++;

        peb1.ethg++ <--> Eth100M <--> pb.ethg++;
        peb2.ethg++ <--> Eth100M <--> pb.ethg++;
        peb3.ethg++ <--> Eth100M <--> pb.ethg++;
}

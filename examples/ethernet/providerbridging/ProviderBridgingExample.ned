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
// On the canvas, the sites of customer A are blue, the sites of customer B
// orange, and the provider network green. The labels on the links between
// bridges name the VLANs that the link carries tagged.
//
network ProviderBridgingExample
{
    parameters:
        @display("bgb=1500,800");
        @figure[customerA1](type=rectangle; pos=30,80; size=300,240; lineColor=#1f6fd1; cornerRadius=5; fillColor=#1f6fd1; fillOpacity=0.1);
        @figure[customerA1.title](type=text; pos=35,85; anchor=nw; text="customer A, site 1");
        @figure[customerB1](type=rectangle; pos=30,420; size=300,160; lineColor=#e07b00; cornerRadius=5; fillColor=#e07b00; fillOpacity=0.1);
        @figure[customerB1.title](type=text; pos=35,425; anchor=nw; text="customer B, site 1");
        @figure[customerA2](type=rectangle; pos=1170,80; size=300,240; lineColor=#1f6fd1; cornerRadius=5; fillColor=#1f6fd1; fillOpacity=0.1);
        @figure[customerA2.title](type=text; pos=1175,85; anchor=nw; text="customer A, site 2");
        @figure[customerB2](type=rectangle; pos=1170,420; size=300,160; lineColor=#e07b00; cornerRadius=5; fillColor=#e07b00; fillOpacity=0.1);
        @figure[customerB2.title](type=text; pos=1175,425; anchor=nw; text="customer B, site 2");
        @figure[customerA3](type=rectangle; pos=600,640; size=320,140; lineColor=#1f6fd1; cornerRadius=5; fillColor=#1f6fd1; fillOpacity=0.1);
        @figure[customerA3.title](type=text; pos=605,645; anchor=nw; text="customer A, site 3");
        @figure[provider](type=rectangle; pos=480,250; size=540,330; lineColor=#3c8c3c; cornerRadius=5; fillColor=#3c8c3c; fillOpacity=0.1);
        @figure[provider.title](type=text; pos=750,255; anchor=n; text="service provider network");
        @figure[cbA1Vlans](type=text; pos=405,247; anchor=s; color=#1f6fd1; text="C-VLAN 10, 20");
        @figure[cbB1Vlans](type=text; pos=405,461; anchor=n; color=#e07b00; text="C-VLAN 10");
        @figure[cbA2Vlans](type=text; pos=1095,247; anchor=s; color=#1f6fd1; text="C-VLAN 10, 20");
        @figure[cbB2Vlans](type=text; pos=1095,461; anchor=n; color=#e07b00; text="C-VLAN 10");
        @figure[cbA3Vlans](type=text; pos=760,610; anchor=w; color=#1f6fd1; text="C-VLAN 10");
        @figure[peb1Vlans](type=text; pos=655,358; anchor=n; color=#404040; text="S-VLAN 100, 200");
        @figure[peb2Vlans](type=text; pos=845,358; anchor=n; color=#404040; text="S-VLAN 100, 200");
        @figure[peb3Vlans](type=text; pos=760,425; anchor=w; color=#404040; text="S-VLAN 100");
        @figure[submodules];
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,720;is=s");
        }
        hostA1: StandardHost {
            @display("p=100,150");
        }
        hostA2: StandardHost {
            @display("p=100,260");
        }
        hostB1: StandardHost {
            @display("p=100,510");
        }
        cbA1: EthernetSwitch {
            @display("p=260,205;i=device/switch,#1f6fd1,30");
        }
        cbB1: EthernetSwitch {
            @display("p=260,510;i=device/switch,#e07b00,30");
        }
        peb1: EthernetSwitch {
            @display("p=560,350;i=device/switch,#3c8c3c,30");
        }
        pb: EthernetSwitch {
            @display("p=750,350;i=device/switch,#3c8c3c,30;t=provider core bridge");
        }
        peb2: EthernetSwitch {
            @display("p=940,350;i=device/switch,#3c8c3c,30");
        }
        cbA2: EthernetSwitch {
            @display("p=1240,205;i=device/switch,#1f6fd1,30");
        }
        cbB2: EthernetSwitch {
            @display("p=1240,510;i=device/switch,#e07b00,30");
        }
        hostA3: StandardHost {
            @display("p=1400,150");
        }
        hostA4: StandardHost {
            @display("p=1400,260");
        }
        hostB2: StandardHost {
            @display("p=1400,510");
        }
        peb3: EthernetSwitch {
            @display("p=750,500;i=device/switch,#3c8c3c,30");
        }
        cbA3: EthernetSwitch {
            @display("p=750,710;i=device/switch,#1f6fd1,30");
        }
        hostA5: StandardHost {
            @display("p=880,710");
        }
    connections:
        hostA1.ethg++ <--> Eth100M { @display("ls=#1f6fd1,1"); } <--> cbA1.ethg++;
        hostA2.ethg++ <--> Eth100M { @display("ls=#1f6fd1,1"); } <--> cbA1.ethg++;
        hostB1.ethg++ <--> Eth100M { @display("ls=#e07b00,1"); } <--> cbB1.ethg++;
        hostA3.ethg++ <--> Eth100M { @display("ls=#1f6fd1,1"); } <--> cbA2.ethg++;
        hostA4.ethg++ <--> Eth100M { @display("ls=#1f6fd1,1"); } <--> cbA2.ethg++;
        hostB2.ethg++ <--> Eth100M { @display("ls=#e07b00,1"); } <--> cbB2.ethg++;
        hostA5.ethg++ <--> Eth100M { @display("ls=#1f6fd1,1"); } <--> cbA3.ethg++;

        cbA1.ethg++ <--> Eth100M { @display("ls=#1f6fd1,2"); } <--> peb1.ethg++;
        cbB1.ethg++ <--> Eth100M { @display("ls=#e07b00,2"); } <--> peb1.ethg++;
        cbA2.ethg++ <--> Eth100M { @display("ls=#1f6fd1,2"); } <--> peb2.ethg++;
        cbB2.ethg++ <--> Eth100M { @display("ls=#e07b00,2"); } <--> peb2.ethg++;
        cbA3.ethg++ <--> Eth100M { @display("ls=#1f6fd1,2"); } <--> peb3.ethg++;

        peb1.ethg++ <--> Eth100M { @display("ls=#404040,3"); } <--> pb.ethg++;
        peb2.ethg++ <--> Eth100M { @display("ls=#404040,3"); } <--> pb.ethg++;
        peb3.ethg++ <--> Eth100M { @display("ls=#404040,3"); } <--> pb.ethg++;
}

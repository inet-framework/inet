//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.ethernet.sharedcore;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.StandardHost;

//
// Two service providers that share a core network (IEEE 802.1ad, Q-in-Q).
//
// Each service provider has two customers, and each customer has a site in
// the west and one in the east. The bridges of a service provider (sp*) are
// C-VLAN bridges: they put each customer into a C-VLAN, 10 for customers A and
// C and 20 for customers B and D, and send the frames C-tagged to the core.
// Both service providers use the same C-VLAN ids.
//
// The edge bridges of the core (pebW, pebE) put all frames of a service
// provider into the S-VLAN of that service provider, 100 for service provider
// 1 and 200 for service provider 2, and the core bridge (pb) relays the frames
// by S-VLAN. The core never looks at the C-tags. The hosts of service provider
// 2 use the MAC addresses of service provider 1's hosts, so only the S-VLAN
// keeps the two service providers apart.
//
// A customer site is either two hosts on access ports of the service
// provider's bridge, or a VLAN-unaware customer switch (switch*) with two
// hosts, connected to one access port. Every customer has one site of each
// kind.
//
// Port numbers follow the order of the connections below: a customer switch
// has its hosts on eth0 and eth1 and the uplink on eth2; a service provider
// bridge has its customer sites on the first ports and the uplink to the core
// on eth3; pebW and pebE have service provider 1 on eth0, service provider 2
// on eth1 and pb on eth2; pb has pebW on eth0 and pebE on eth1.
//
// On the canvas, the networks of service provider 1 are blue, those of service
// provider 2 orange, and the core network green; each customer site has a
// color of its own. The labels on the links between bridges name the VLANs
// that the link carries tagged.
//
network SharedCoreExample
{
    parameters:
        @display("bgb=1700,1260");
        @figure[provider1West](type=rectangle; pos=20,20; size=420,560; lineColor=#1f6fd1; cornerRadius=5; fillColor=#1f6fd1; fillOpacity=0.1);
        @figure[provider1West.title](type=text; pos=435,25; anchor=ne; text="service provider 1, S-VLAN 100");
        @figure[provider2West](type=rectangle; pos=20,640; size=420,560; lineColor=#e07b00; cornerRadius=5; fillColor=#e07b00; fillOpacity=0.1);
        @figure[provider2West.title](type=text; pos=435,645; anchor=ne; text="service provider 2, S-VLAN 200");
        @figure[provider1East](type=rectangle; pos=1260,20; size=420,560; lineColor=#1f6fd1; cornerRadius=5; fillColor=#1f6fd1; fillOpacity=0.1);
        @figure[provider1East.title](type=text; pos=1265,25; anchor=nw; text="service provider 1, S-VLAN 100");
        @figure[provider2East](type=rectangle; pos=1260,640; size=420,560; lineColor=#e07b00; cornerRadius=5; fillColor=#e07b00; fillOpacity=0.1);
        @figure[provider2East.title](type=text; pos=1265,645; anchor=nw; text="service provider 2, S-VLAN 200");
        @figure[core](type=rectangle; pos=490,515; size=720,190; lineColor=#3c8c3c; cornerRadius=5; fillColor=#3c8c3c; fillOpacity=0.1);
        @figure[core.title](type=text; pos=850,520; anchor=n; text="shared core network");
        @figure[customerA1](type=rectangle; pos=40,60; size=180,240; lineColor=#8e44ad; cornerRadius=5; fillColor=#8e44ad; fillOpacity=0.1);
        @figure[customerA1.title](type=text; pos=45,65; anchor=nw; text="customer A, site 1");
        @figure[customerB1](type=rectangle; pos=40,320; size=280,240; lineColor=#16a085; cornerRadius=5; fillColor=#16a085; fillOpacity=0.1);
        @figure[customerB1.title](type=text; pos=45,325; anchor=nw; text="customer B, site 1");
        @figure[customerC1](type=rectangle; pos=40,680; size=280,240; lineColor=#c0392b; cornerRadius=5; fillColor=#c0392b; fillOpacity=0.1);
        @figure[customerC1.title](type=text; pos=45,685; anchor=nw; text="customer C, site 1");
        @figure[customerD1](type=rectangle; pos=40,940; size=180,240; lineColor=#7d8a00; cornerRadius=5; fillColor=#7d8a00; fillOpacity=0.1);
        @figure[customerD1.title](type=text; pos=45,945; anchor=nw; text="customer D, site 1");
        @figure[customerA2](type=rectangle; pos=1380,60; size=280,240; lineColor=#8e44ad; cornerRadius=5; fillColor=#8e44ad; fillOpacity=0.1);
        @figure[customerA2.title](type=text; pos=1385,65; anchor=nw; text="customer A, site 2");
        @figure[customerB2](type=rectangle; pos=1480,320; size=180,240; lineColor=#16a085; cornerRadius=5; fillColor=#16a085; fillOpacity=0.1);
        @figure[customerB2.title](type=text; pos=1485,325; anchor=nw; text="customer B, site 2");
        @figure[customerC2](type=rectangle; pos=1480,680; size=180,240; lineColor=#c0392b; cornerRadius=5; fillColor=#c0392b; fillOpacity=0.1);
        @figure[customerC2.title](type=text; pos=1485,685; anchor=nw; text="customer C, site 2");
        @figure[customerD2](type=rectangle; pos=1380,940; size=280,240; lineColor=#7d8a00; cornerRadius=5; fillColor=#7d8a00; fillOpacity=0.1);
        @figure[customerD2.title](type=text; pos=1385,945; anchor=nw; text="customer D, site 2");
        @figure[sp1wVlans](type=text; pos=470,420; anchor=w; color=#1f6fd1; text="C-VLAN 10, 20");
        @figure[sp2wVlans](type=text; pos=470,800; anchor=w; color=#e07b00; text="C-VLAN 10, 20");
        @figure[sp1eVlans](type=text; pos=1230,420; anchor=e; color=#1f6fd1; text="C-VLAN 10, 20");
        @figure[sp2eVlans](type=text; pos=1230,800; anchor=e; color=#e07b00; text="C-VLAN 10, 20");
        @figure[pebWVlans](type=text; pos=705,618; anchor=n; color=#404040; text="S-VLAN 100, 200");
        @figure[pebEVlans](type=text; pos=995,618; anchor=n; color=#404040; text="S-VLAN 100, 200");
        @figure[submodules];
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=850,1185;is=s");
        }
        hostA1a: StandardHost {
            @display("p=110,125");
        }
        hostA1b: StandardHost {
            @display("p=110,245");
        }
        hostB1a: StandardHost {
            @display("p=110,385");
        }
        hostB1b: StandardHost {
            @display("p=110,505");
        }
        switchB1: EthernetSwitch {
            @display("p=250,445");
        }
        sp1w: EthernetSwitch {
            @display("p=370,310;i=device/switch,#1f6fd1,30");
        }
        hostC1a: StandardHost {
            @display("p=110,745");
        }
        hostC1b: StandardHost {
            @display("p=110,865");
        }
        switchC1: EthernetSwitch {
            @display("p=250,805");
        }
        hostD1a: StandardHost {
            @display("p=110,1005");
        }
        hostD1b: StandardHost {
            @display("p=110,1125");
        }
        sp2w: EthernetSwitch {
            @display("p=370,930;i=device/switch,#e07b00,30");
        }
        pebW: EthernetSwitch {
            @display("p=560,610;i=device/switch,#3c8c3c,30");
        }
        pb: EthernetSwitch {
            @display("p=850,610;i=device/switch,#3c8c3c,30;t=core bridge");
        }
        pebE: EthernetSwitch {
            @display("p=1140,610;i=device/switch,#3c8c3c,30");
        }
        sp1e: EthernetSwitch {
            @display("p=1330,310;i=device/switch,#1f6fd1,30");
        }
        switchA2: EthernetSwitch {
            @display("p=1450,185");
        }
        hostA2a: StandardHost {
            @display("p=1590,125");
        }
        hostA2b: StandardHost {
            @display("p=1590,245");
        }
        hostB2a: StandardHost {
            @display("p=1590,385");
        }
        hostB2b: StandardHost {
            @display("p=1590,505");
        }
        sp2e: EthernetSwitch {
            @display("p=1330,930;i=device/switch,#e07b00,30");
        }
        hostC2a: StandardHost {
            @display("p=1590,745");
        }
        hostC2b: StandardHost {
            @display("p=1590,865");
        }
        switchD2: EthernetSwitch {
            @display("p=1450,1065");
        }
        hostD2a: StandardHost {
            @display("p=1590,1005");
        }
        hostD2b: StandardHost {
            @display("p=1590,1125");
        }
    connections:
        hostB1a.ethg++ <--> Eth100M { @display("ls=#16a085,1"); } <--> switchB1.ethg++;
        hostB1b.ethg++ <--> Eth100M { @display("ls=#16a085,1"); } <--> switchB1.ethg++;
        hostC1a.ethg++ <--> Eth100M { @display("ls=#c0392b,1"); } <--> switchC1.ethg++;
        hostC1b.ethg++ <--> Eth100M { @display("ls=#c0392b,1"); } <--> switchC1.ethg++;
        hostA2a.ethg++ <--> Eth100M { @display("ls=#8e44ad,1"); } <--> switchA2.ethg++;
        hostA2b.ethg++ <--> Eth100M { @display("ls=#8e44ad,1"); } <--> switchA2.ethg++;
        hostD2a.ethg++ <--> Eth100M { @display("ls=#7d8a00,1"); } <--> switchD2.ethg++;
        hostD2b.ethg++ <--> Eth100M { @display("ls=#7d8a00,1"); } <--> switchD2.ethg++;

        hostA1a.ethg++ <--> Eth100M { @display("ls=#8e44ad,1"); } <--> sp1w.ethg++;
        hostA1b.ethg++ <--> Eth100M { @display("ls=#8e44ad,1"); } <--> sp1w.ethg++;
        switchB1.ethg++ <--> Eth100M { @display("ls=#16a085,1"); } <--> sp1w.ethg++;
        switchC1.ethg++ <--> Eth100M { @display("ls=#c0392b,1"); } <--> sp2w.ethg++;
        hostD1a.ethg++ <--> Eth100M { @display("ls=#7d8a00,1"); } <--> sp2w.ethg++;
        hostD1b.ethg++ <--> Eth100M { @display("ls=#7d8a00,1"); } <--> sp2w.ethg++;
        switchA2.ethg++ <--> Eth100M { @display("ls=#8e44ad,1"); } <--> sp1e.ethg++;
        hostB2a.ethg++ <--> Eth100M { @display("ls=#16a085,1"); } <--> sp1e.ethg++;
        hostB2b.ethg++ <--> Eth100M { @display("ls=#16a085,1"); } <--> sp1e.ethg++;
        hostC2a.ethg++ <--> Eth100M { @display("ls=#c0392b,1"); } <--> sp2e.ethg++;
        hostC2b.ethg++ <--> Eth100M { @display("ls=#c0392b,1"); } <--> sp2e.ethg++;
        switchD2.ethg++ <--> Eth100M { @display("ls=#7d8a00,1"); } <--> sp2e.ethg++;

        sp1w.ethg++ <--> Eth100M { @display("ls=#1f6fd1,2"); } <--> pebW.ethg++;
        sp2w.ethg++ <--> Eth100M { @display("ls=#e07b00,2"); } <--> pebW.ethg++;
        sp1e.ethg++ <--> Eth100M { @display("ls=#1f6fd1,2"); } <--> pebE.ethg++;
        sp2e.ethg++ <--> Eth100M { @display("ls=#e07b00,2"); } <--> pebE.ethg++;

        pebW.ethg++ <--> Eth100M { @display("ls=#404040,3"); } <--> pb.ethg++;
        pebE.ethg++ <--> Eth100M { @display("ls=#404040,3"); } <--> pb.ethg++;
}

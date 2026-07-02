//
// IPv6 conformance suite -- Proxy Mobile IPv6 (RFC 5213) network. One Local Mobility Anchor
// (LMA) and two Mobile Access Gateways (MAGs) form a PMIPv6 domain; a plain wireless IPv6
// host (the mobile node, running NO mobility software) starts under mag1 and roams to mag2.
// Each MAG, on detecting the attachment, registers the node with the LMA on its behalf
// (Proxy Binding Update / Acknowledgement over the backhaul) and advertises the node's home
// network prefix on the access link, so the node keeps its address across the handover. A
// correspondent node behind the LMA exchanges traffic with the node, tunnelled LMA<->MAG.
//
// Modelled on examples/ipv6/pmipv6/Pmipv6Network, with a ProtocolTester added so the suite
// can observe the backhaul PBU/PBA signalling.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
package inet.protocoltest.ipv6;

import inet.networklayer.configurator.ipv6.Ipv6NetworkConfigurator;
import inet.node.ipv6.StandardHost6;
import inet.node.mipv6.WirelessHost6;
import inet.node.pmipv6.LocalMobilityAnchor;
import inet.node.pmipv6.MobileAccessGateway;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.protocoltest.ProtocolTester;

network Pmipv6Demo
{
    parameters:
        **.mgmt.numChannels = 5;
        **.ipv6.configurator.networkConfiguratorModule = "configurator";
        @display("bgb=800,500");
    types:
        channel ethernetline extends ned.DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
        }
    submodules:
        configurator: Ipv6NetworkConfigurator {
            @display("p=80,60;is=s");
        }
        tester: ProtocolTester {
            @display("p=80,240");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=80,160;is=s");
        }
        lma: LocalMobilityAnchor {
            @display("p=400,150;i=abstract/router");
        }
        cn: StandardHost6 {
            @display("p=400,60");
        }
        mag1: MobileAccessGateway {
            @display("p=250,260");
        }
        mag2: MobileAccessGateway {
            @display("p=550,260");
        }
        mn: WirelessHost6 {
            @display("p=250,400");
        }
    connections allowunconnected:
        cn.ethg++ <--> ethernetline <--> lma.ethg++;
        mag1.ethg++ <--> ethernetline <--> lma.ethg++;
        mag2.ethg++ <--> ethernetline <--> lma.ethg++;
}

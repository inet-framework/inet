//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.ipv6.pmipv6;

import inet.networklayer.configurator.ipv6.Ipv6NetworkConfigurator;
import inet.node.ipv6.StandardHost6;
import inet.node.pmipv6.LocalMobilityAnchor;
import inet.node.pmipv6.MobileAccessGateway;
import inet.node.mipv6.WirelessHost6;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;


channel ethernetline extends ned.DatarateChannel
{
    parameters:
        delay = 0.1us;
        datarate = 100Mbps;
}

//
// A Proxy Mobile IPv6 (RFC 5213) domain with one Local Mobility Anchor and two
// Mobile Access Gateways. A single mobile node -- a plain wireless IPv6 host with
// no mobility software -- starts under mag1 and roams to mag2. Each gateway, on
// detecting the attachment, registers the mobile node with the anchor on its
// behalf and advertises the node's home network prefix on the access link, so the
// node keeps its address throughout. A correspondent node behind the anchor
// exchanges traffic with the mobile node across the handover.
//
network Pmipv6Network
{
    parameters:
        **.mgmt.numChannels = 5;
        **.ipv6.configurator.networkConfiguratorModule = "configurator";
        @display("bgb=800,500");

    submodules:
        configurator: Ipv6NetworkConfigurator {
            parameters:
                config = xmldoc("config.xml");
                // the mobile node autoconfigures its (home) address via SLAAC from the MAG's RA
                assignAddressesToHosts = false;
                @display("p=80,60;is=s");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=80,160;is=s");
        }
        lma: LocalMobilityAnchor {
            parameters:
                @display("p=400,150;i=abstract/router");
        }
        cn: StandardHost6 {
            parameters:
                @display("p=400,60");
        }
        mag1: MobileAccessGateway {
            parameters:
                @display("p=250,260");
        }
        mag2: MobileAccessGateway {
            parameters:
                @display("p=550,260");
        }
        mn: WirelessHost6 {
            parameters:
                @display("p=250,400");
        }

    connections allowunconnected:
        cn.ethg++ <--> ethernetline <--> lma.ethg++;
        mag1.ethg++ <--> ethernetline <--> lma.ethg++;
        mag2.ethg++ <--> ethernetline <--> lma.ethg++;
}

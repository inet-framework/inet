//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.showcases.mobileip.mipv6;

import inet.networklayer.configurator.ipv6.Ipv6NetworkConfigurator;
import inet.node.contract.INetworkNode;
import inet.node.ethernet.EthernetSwitch;
import inet.node.ipv6.Router6;
import inet.node.ipv6.StandardHost6;
import inet.node.mipv6.CorrespondentNode6;
import inet.node.mipv6.HomeAgent6;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;


channel ethernetLine extends ned.DatarateChannel
{
    parameters:
        delay = default(0.1us);
        datarate = default(100Mbps);
}

//
// Network for the Mobile IPv6 showcase. The mobile node starts on its home link,
// behind the home agent, and moves once to the foreign link.
//
// The home agent is a stub hanging off the backbone router, not a node on the path
// between the correspondent nodes and the foreign link. Traffic that goes through the
// home agent therefore crosses the backbone-to-home-agent link twice in each direction,
// while route-optimized traffic never touches it at all.
//
network MobileIpv6Showcase
{
    parameters:
        double homeAgentLinkDelay @unit(s) = default(5ms); // one-way delay of the stub link that only tunnelled traffic crosses
        **.mgmt.numChannels = 5;
        **.ipv6.configurator.networkConfiguratorModule = "ipv6Configurator";
        @display("bgb=900,750");

    submodules:
        ipv6Configurator: Ipv6NetworkConfigurator {
            parameters:
                assignAddressesToHosts = false;
                @display("p=80,120;is=s");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=80,220;is=s");
        }
        visualizer: IntegratedCanvasVisualizer {
            parameters:
                @display("p=80,320;is=s");
        }
        cnRouteOptimized: CorrespondentNode6 {
            @display("p=350,660");
        }
        cnPlain: StandardHost6 {
            @display("p=550,660");
        }
        switch: EthernetSwitch {
            @display("p=450,560;i=device/hub_s");
        }
        backbone: Router6 {
            @display("p=450,440");
        }
        foreignRouter: Router6 {
            @display("p=700,440");
        }
        homeAgent: HomeAgent6 {
            @display("p=300,320;i=abstract/router");
        }
        apHome: AccessPoint {
            @display("p=300,200;i=device/accesspoint_s");
        }
        apForeign: AccessPoint {
            @display("p=600,200;i=device/accesspoint_s");
        }
        mn: <default("WirelessHost6")> like INetworkNode {
            @display("p=300,120");
        }

    connections allowunconnected:
        cnRouteOptimized.ethg++ <--> ethernetLine <--> switch.ethg++;
        cnPlain.ethg++ <--> ethernetLine <--> switch.ethg++;
        switch.ethg++ <--> ethernetLine <--> backbone.ethg++;

        backbone.ethg++ <--> ethernetLine <--> foreignRouter.ethg++;
        foreignRouter.ethg++ <--> ethernetLine <--> apForeign.ethg++;

        backbone.ethg++ <--> ethernetLine { delay = parent.homeAgentLinkDelay; } <--> homeAgent.ethg++;
        homeAgent.ethg++ <--> ethernetLine <--> apHome.ethg++;
}

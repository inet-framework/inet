//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.showcases.mobileip.pmipv6;

import inet.networklayer.configurator.ipv6.Ipv6NetworkConfigurator;
import inet.node.contract.IEthernetNetworkNode;
import inet.node.ipv6.StandardHost6;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;

channel backhaul extends ned.DatarateChannel
{
    parameters:
        delay = default(0.1us);
        datarate = default(100Mbps);
}

channel anchorline extends ned.DatarateChannel
{
    parameters:
        delay = default(5ms);
        datarate = default(100Mbps);
}

//
// Network for the Proxy Mobile IPv6 showcase. A correspondent node and two access
// routers hang off an ordinary router; the mobility anchor hangs off the same
// router. A single mobile node -- a plain wireless IPv6 host -- starts under the
// first access router and drives to the second one.
//
// The node types of `anchor`, `ar1` and `ar2` are set from the ini file: the
// showcase's two configurations differ in those three types and in nothing else.
//
network Pmipv6Showcase
{
    parameters:
        **.ipv6.configurator.networkConfiguratorModule = "configurator";
        @display("bgb=800,500");

    submodules:
        configurator: Ipv6NetworkConfigurator {
            parameters:
                assignAddressesToHosts = default(false);
                @display("p=80,60;is=s");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=80,160;is=s");
        }
        visualizer: <default(firstAvailableOrEmpty("IntegratedCanvasVisualizer"))> like IIntegratedVisualizer if typename != "" {
            parameters:
                @display("p=80,260;is=s");
        }
        cn: StandardHost6 {
            parameters:
                @display("p=400,60");
        }
        core: <default("Router6")> like IEthernetNetworkNode {
            parameters:
                @display("p=400,150;i=abstract/router");
        }
        anchor: <default("LocalMobilityAnchor")> like IEthernetNetworkNode {
            parameters:
                @display("p=620,150;i=abstract/router");
        }
        ar1: <default("MobileAccessGateway")> like IEthernetNetworkNode {
            parameters:
                @display("p=250,260");
        }
        ar2: <default("MobileAccessGateway")> like IEthernetNetworkNode {
            parameters:
                @display("p=550,260");
        }
        mn: StandardHost6 {
            parameters:
                numWlanInterfaces = default(1);
                @display("p=250,400;i=device/wifilaptop");
        }

    // The order of the connections below fixes the interface indices on `core`:
    // eth0 faces cn, eth1 faces anchor, eth2 faces ar1, eth3 faces ar2. The route
    // in config.xml names eth1, so this order is part of the configuration.
    connections allowunconnected:
        cn.ethg++ <--> backhaul <--> core.ethg++;
        core.ethg++ <--> anchorline <--> anchor.ethg++;
        ar1.ethg++ <--> backhaul <--> core.ethg++;
        ar2.ethg++ <--> backhaul <--> core.ethg++;
}

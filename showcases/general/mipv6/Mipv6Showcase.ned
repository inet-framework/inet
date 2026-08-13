//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.showcases.general.mipv6;

import inet.networklayer.configurator.ipv6.Ipv6NetworkConfigurator;
import inet.node.contract.INetworkNode;
import inet.node.ipv6.Router6;
import inet.node.mipv6.CorrespondentNode6;
import inet.node.mipv6.HomeAgent6;
import inet.node.mipv6.WirelessHost6;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.common.IntegratedVisualizer;

channel AccessLan extends ned.DatarateChannel
{
    delay = 0.1us;
    datarate = 100Mbps;
}

channel WanLink extends ned.DatarateChannel
{
    datarate = 100Mbps;
}

network Mipv6Showcase
{
    parameters:
        **.mgmt.numChannels = 3;
        **.ipv6.configurator.networkConfiguratorModule = "configurator";
        @display("bgb=750,640");

    submodules:
        configurator: Ipv6NetworkConfigurator {
            parameters:
                // the mobile node autoconfigures its home/care-of addresses (SLAAC),
                // so the configurator addresses only the routers
                assignAddressesToHosts = false;
                @display("p=80,100;is=s");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=80,180;is=s");
        }
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=80,260;is=s");
        }
        homeAgent: HomeAgent6 {
            parameters:
                @display("p=300,300;i=abstract/router");
        }
        foreignRouter: Router6 {
            parameters:
                @display("p=600,300");
        }
        backbone: Router6 {
            parameters:
                @display("p=450,440");
        }
        correspondentNode: CorrespondentNode6 {
            parameters:
                @display("p=450,570");
        }
        apHome: AccessPoint {
            parameters:
                @display("p=300,200;i=device/accesspoint_s");
        }
        apForeign: AccessPoint {
            parameters:
                @display("p=600,200;i=device/accesspoint_s");
        }
        mobileNode: <default("WirelessHost6")> like INetworkNode {
            parameters:
                @display("p=300,150;i=device/wifilaptop");
        }

    connections:
        apHome.ethg++ <--> AccessLan <--> homeAgent.ethg++;
        apForeign.ethg++ <--> AccessLan <--> foreignRouter.ethg++;
        homeAgent.ethg++ <--> WanLink { delay = 5ms; } <--> backbone.ethg++;
        foreignRouter.ethg++ <--> WanLink { delay = 8ms; } <--> backbone.ethg++;
        correspondentNode.ethg++ <--> WanLink { delay = 1ms; } <--> backbone.ethg++;
}

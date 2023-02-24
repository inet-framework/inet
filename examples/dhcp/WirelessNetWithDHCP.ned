//
// Copyright (C) 2010 Juan-Carlos Maureira
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.dhcp;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;


network WirelessNetWithDHCP
{
    int numHosts;
    **.mgmt.numChannels = 5;
    submodules:
        visualizer: <default(firstAvailableOrEmpty("IntegratedCanvasVisualizer"))> like IIntegratedVisualizer if typename != "" {
            @display("p=100,300;is=s");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=100,200;is=s");
        }
        gateway: Router {
            @display("p=365,35");
        }
        client[numHosts]: WirelessHost {
            @display("r=,,#707070;p=245,285");
        }
        ap: AccessPoint {
            @display("p=126,98");
        }
        dhcpServer: StandardHost {
            @display("p=64,307");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        remote: StandardHost {
            @display("p=540,204");
        }
    connections:
        gateway.ethg++ <--> Eth100M <--> ap.ethg++;
        gateway.ethg++ <--> Eth100M <--> remote.ethg++;
        dhcpServer.ethg++ <--> Eth100M <--> ap.ethg++;
}

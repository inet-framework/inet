//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.dhcp;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.StandardHost;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;


network WirelessNetWith2DHCP
{
    int numHosts;
    submodules:
        visualizer: <default(firstAvailableOrEmpty("IntegratedCanvasVisualizer"))> like IIntegratedVisualizer if typename != "" {
            @display("p=100,300;is=s");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=100,200;is=s");
        }
        client: WirelessHost {
            @display("p=64,124");
        }
        ap1: AccessPoint {
            @display("p=64,238");
        }
        dhcpServer1: StandardHost {
            @display("p=64,307");
        }
        ap2: AccessPoint {
            @display("p=511,231");
        }
        dhcpServer2: StandardHost {
            @display("p=511,307");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100;is=s");
        }
        server: StandardHost {
            @display("p=283,363");
        }
    connections:
        dhcpServer1.ethg++ <--> Eth100M <--> ap1.ethg++;
        dhcpServer2.ethg++ <--> Eth100M <--> ap2.ethg++;
        dhcpServer1.ethg++ <--> Eth100M <--> server.ethg++;
        dhcpServer2.ethg++ <--> Eth100M <--> server.ethg++;
}


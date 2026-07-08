//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.showcases.wireless.ratevisualizer;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.StandardHost;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;

//
// Demonstrates the per-station IEEE 802.11 data rate bar chart visualizer. A
// wired server sends UDP downlink traffic through the access point to three
// wireless stations. The access point transmits to each station at a different
// configured PHY data rate (6/24/54 Mbps), which the rate visualizer shows as a
// bar chart above the access point.
//
network RateVisualizerShowcase
{
    submodules:
        visualizer: <default(firstAvailableOrEmpty("IntegratedCanvasVisualizer"))> like IIntegratedVisualizer if typename != "" {
            @display("p=100,300;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100;is=s");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=100,200;is=s");
        }
        server: StandardHost {
            @display("p=100,450");
        }
        accessPoint: AccessPoint {
            @display("p=300,450");
        }
        sta1: WirelessHost {
            @display("p=500,300");
        }
        sta2: WirelessHost {
            @display("p=550,450");
        }
        sta3: WirelessHost {
            @display("p=500,600");
        }
    connections:
        server.ethg++ <--> Eth100M <--> accessPoint.ethg++;
}

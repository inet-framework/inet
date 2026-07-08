//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.wireless.perstationrate;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.StandardHost;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;

//
// Demonstrates per-station IEEE 802.11 data rate selection: the access point
// transmits to each station at a different, per-receiver configured PHY data
// rate (the `dataFrameBitratePerReceiver` rate-selection parameter). A wired
// server sends UDP downlink traffic through the access point to three wireless
// stations, which the access point serves at 6, 24 and 54 Mbps respectively.
//
// The per-station rates can be watched live above the access point with the
// IEEE 802.11 rate visualizer (secondary; enabled in the ini file).
//
network PerStationRateExample
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

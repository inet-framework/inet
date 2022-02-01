//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.wireless.synchronized;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;

network Synchronized
{
    parameters:
        int nodeCount;
        **.bitrate = 11Mbps;
        **.arp.typename = "GlobalArp";
        @display("bgb=650,450;bgl=3");
    submodules:
        visualizer: <default(firstAvailableOrEmpty("IntegratedCanvasVisualizer"))> like IIntegratedVisualizer if typename != "" {
            parameters:
                @display("p=100,300;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,200;is=s");
        }
        node[nodeCount]: AdhocHost {
            parameters:
                numApps = 1;
                app[0].typename = "UdpBasicApp";
                app[0].destAddresses = "224.0.0.1";
                app[0].localPort = 1;
                app[0].destPort = 1;
                app[0].messageLength = 100B;
                app[0].joinLocalMulticastGroups = true;
        }
}


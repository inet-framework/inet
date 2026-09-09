//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.visualizer.vsgsignal;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.WirelessHost;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.vsg.integrated.IntegratedVsgVisualizer;

//
// A small wireless network for tuning the VSG signal-propagation visualization.
// Three hosts on a 300x300 ground plane, positioned within each other's ~100m
// communication range (the transmitter power in omnetpp.ini is set so the range
// is ~100m, keeping the translucent transmission discs a sensible size relative
// to the scene). Because the nodes are in range, transmissions are received, so
// the data-link visualizer also draws connection arrows. host3 moves slowly to
// show growing transmissions as nodes move around. Run under Qtenv, switch to the
// 3D view, and press Run.
//
network VsgSignalShowcase
{
    @display("bgb=300,300");
    submodules:
        visualizer: IntegratedVsgVisualizer {
            @display("p=40,40");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=40,110");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=40,180");
        }
        host1: WirelessHost {
            @display("p=135,165");
        }
        host2: WirelessHost {
            @display("p=165,165");
        }
        host3: WirelessHost {
            @display("p=150,135");
        }
}
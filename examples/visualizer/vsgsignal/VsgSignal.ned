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
// Three hosts on a 300x300 ground plane, within each other's communication range
// (~150m), so the translucent transmission discs are a sensible size relative to
// the scene. host3 moves to show growing transmissions as nodes move around.
// Run under Qtenv and switch to the 3D view.
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
            @display("p=80,200");
        }
        host2: WirelessHost {
            @display("p=220,200");
        }
        host3: WirelessHost {
            @display("p=150,100");
        }
}
//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.visualizer.vsgicons;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.vsg.integrated.IntegratedVsgVisualizer;

//
// A VSG 3D-visualizer scene built to exercise the visuals that the ad-hoc vsgwireless example does
// NOT show: an infrastructure WiFi network (an AccessPoint + associating WirelessHosts) so the
// Ieee80211 SSID badges appear; a TCP flow (connection icons); a UDP flood over a weak link (packet
// drops); plus the usual node icons, labels, data links, routes, communication ranges and the
// wave-modulated medium signals. Run under Qtenv and switch to the 3D view.
//
network VsgIconShowcase
{
    @display("bgb=400,400");
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
        accessPoint: AccessPoint {
            @display("p=200,200");
        }
        hostA: WirelessHost {
            @display("p=120,260");
        }
        hostB: WirelessHost {
            @display("p=280,260");
        }
        hostC: WirelessHost {
            @display("p=200,90");
        }
}

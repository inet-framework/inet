//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.visualizer.vsgwireless;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.WirelessHost;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.vsg.integrated.IntegratedVsgVisualizer;

//
// A small wireless ad-hoc network used as a comprehensive VulkanSceneGraph (VSG)
// 3D-visualizer checkpoint: with UDP traffic it exercises the medium (signals,
// transmissions, communication ranges), data links, network routes, interface
// tables, statistics and (for host3) mobility trails — all rendered by the VSG
// integrated visualizer. Run under Qtenv and switch to the 3D view.
//
network VsgWirelessShowcase
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
        host1: WirelessHost {
            @display("p=120,300");
        }
        host2: WirelessHost {
            @display("p=320,300");
        }
        host3: WirelessHost {
            @display("p=220,150");
        }
}
